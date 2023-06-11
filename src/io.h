#pragma once

#include "ceval.h"
#include "cffi.h"
#include "common.h"

#if PK_ENABLE_OS

#include <filesystem>
#include <cstdio>

namespace pkpy{
inline std::filesystem::path get_rel(const VM* vm, const std::filesystem::path & rel) {
    return rel.lexically_relative(vm->_lowest_isolated_cwd_path);
}

inline bool check_if_path_is_isolated(const std::string & path_to_check_str,
                                      const std::string & toplevel_path_str) {
    //.lexically_normal() expands somepath/somdir/.. to somepath/
    auto toplevel_path = std::filesystem::path(toplevel_path_str).lexically_normal();
    auto path_to_check = (toplevel_path / std::filesystem::path(path_to_check_str)).lexically_normal();

    //toplevel_path is part of toplevel_path, so if toplevel_path is less than path_to_check, then toplevel_path is certainly not a part of it
    if (path_to_check < toplevel_path) {return false;}
    if (path_to_check == toplevel_path) {return true;}
    while (true) {
        auto temp = path_to_check.parent_path();
        if (path_to_check == temp) { break;}
        path_to_check = temp;
        if (path_to_check < toplevel_path) {return false;}
        if (path_to_check == toplevel_path) {return true;}
    }

    return false;
}

static inline void helper_process_isolated(VM *vm, std::filesystem::path & path) {
    if (!vm->isolated_os) { return;}
    path = std::filesystem::path(vm->_lowest_isolated_cwd_path) / path;
    if (!check_if_path_is_isolated(path, vm->_lowest_isolated_cwd_path)) {vm->IOError(Str("Invalid path."));}
}


struct FileIO {
    PY_CLASS(FileIO, io, FileIO)

    Str file;
    Str mode;
    FILE* fp= nullptr;

    bool is_text() const { return mode != "rb" && mode != "wb" && mode != "ab"; }

    FileIO(VM* vm, std::string file, std::string mode): file(file), mode(mode) {
        if (vm->isolated_os && !check_if_path_is_isolated(file, vm->_lowest_isolated_cwd_path)) {vm->IOError(strerror(errno));}
        if (vm->isolated_os) {file = vm->_lowest_isolated_cwd_path / std::filesystem::path(file);}
        fp = fopen(file.c_str(), mode.c_str());
        if(!fp) vm->IOError(strerror(errno));
    }

    void close(){
        if(fp == nullptr) return;
        fclose(fp);
        fp = nullptr;
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            return VAR_T(FileIO, 
                vm, CAST(Str&, args[1]).str(), CAST(Str&, args[2]).str()
            );
        });

        vm->bind_method<0>(type, "read", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            fseek(io.fp, 0, SEEK_END);
            std::vector<char> buffer(ftell(io.fp));
            fseek(io.fp, 0, SEEK_SET);
            fread(buffer.data(), 1, buffer.size(), io.fp);
            Bytes b(std::move(buffer));
            if(io.is_text()) return VAR(Str(b.str()));
            return VAR(std::move(b));
        });

        vm->bind_method<1>(type, "write", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            if(io.is_text()){
                Str& s = CAST(Str&, args[1]);
                fwrite(s.data, 1, s.length(), io.fp);
            }else{
                Bytes& buffer = CAST(Bytes&, args[1]);
                fwrite(buffer.data(), 1, buffer.size(), io.fp);
            }
            return vm->None;
        });

        vm->bind_method<0>(type, "close", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            io.close();
            return vm->None;
        });

        vm->bind_method<0>(type, "__exit__", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            io.close();
            return vm->None;
        });

        vm->bind_method<0>(type, "__enter__", CPP_LAMBDA(vm->None));
    }
};

inline void add_module_io(VM* vm){
    PyObject* mod = vm->new_module("io");
    FileIO::register_class(vm, mod);
    vm->bind_builtin_func<2>("open", [](VM* vm, ArgsView args){
        static StrName m_io("io");
        static StrName m_FileIO("FileIO");
        return vm->call(vm->_modules[m_io]->attr(m_FileIO), args[0], args[1]);
    });
}

inline void add_module_os(VM* vm){
    PyObject* mod = vm->new_module("os");
    PyObject* path_obj = vm->heap.gcnew<DummyInstance>(vm->tp_object, {});
    mod->attr().set("path", path_obj);
    
    // Working directory is shared by all VMs!!
    vm->bind_func<0>(mod, "getcwd", [](VM* vm, ArgsView args){
        if (!vm->isolated_os) {
            return VAR(std::filesystem::current_path().string());
        } else {
            return VAR(get_rel(vm, std::filesystem::current_path()).string());//TODO BUG! while technically correct, cwd needs to change to a file path upon executing
        }
    });

    vm->bind_func<1>(mod, "chdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        helper_process_isolated(vm, path);
        std::filesystem::current_path(path);
        return vm->None;
    });

    vm->bind_func<1>(mod, "listdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        helper_process_isolated(vm, path);

        std::filesystem::directory_iterator di;
        try{
            di = std::filesystem::directory_iterator(path);
        }catch(std::filesystem::filesystem_error& e){
            std::string msg = e.what();
            auto pos = msg.find_last_of(':');
            if(pos != std::string::npos) msg = msg.substr(pos + 1);
            vm->IOError(Str(msg).lstrip());
        }
        List ret;
        if (!vm->isolated_os) {
        for (auto &p: di) ret.push_back(VAR(p.path().filename().string()));
        } else {
        for(auto& p: di) ret.push_back(VAR(get_rel(vm, p.path()).filename().string()));
        }
        return VAR(ret);
    });

    vm->bind_func<1>(mod, "remove", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        helper_process_isolated(vm, path);

        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("Operation failed.");
        return vm->None;
    });

    vm->bind_func<1>(mod, "mkdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        helper_process_isolated(vm, path);

        std::cout << path.string().c_str() << "\n";

        bool ok = std::filesystem::create_directory(path);
        if(!ok) vm->IOError("Operation failed.");
        return vm->None;
    });

    vm->bind_func<1>(mod, "rmdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        helper_process_isolated(vm, path);

        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("Operation failed.");
        return vm->None;
    });

    vm->bind_func<-1>(path_obj, "join", [](VM* vm, ArgsView args){
        std::filesystem::path path;
        for(int i=0; i<args.size(); i++){
            path /= CAST(Str&, args[i]).sv();
        }
        return VAR(path.string());
    });

    vm->bind_func<1>(path_obj, "exists", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        if (vm->isolated_os) {
            path = vm->_lowest_isolated_cwd_path / path;
            if (!check_if_path_is_isolated(path, vm->_lowest_isolated_cwd_path)) {return VAR(false);}
        }

        bool exists = std::filesystem::exists(path);
        return VAR(exists);
    });

    vm->bind_func<1>(path_obj, "basename", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        return VAR(path.filename().string());
    });
}

} // namespace pkpy


#else

namespace pkpy{
inline void add_module_io(void* vm){}
inline void add_module_os(void* vm){}
inline Bytes _default_import_handler(const Str& name) { return Bytes(); }
} // namespace pkpy

#endif