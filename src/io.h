#pragma once

#include "ceval.h"
#include "cffi.h"
#include "common.h"

#if PK_ENABLE_OS

#include <fstream>
#include <filesystem>

namespace pkpy{

inline Bytes _read_file_cwd(const Str& name){
    std::filesystem::path path(name.sv());
    bool exists = std::filesystem::exists(path);
    if(!exists) return Bytes();
    std::ifstream ifs(path);
    std::vector<char> buffer(std::istreambuf_iterator<char>(ifs), {});
    ifs.close();
    return Bytes(std::move(buffer));
}

struct FileIO {
    PY_CLASS(FileIO, io, FileIO)

    Str file;
    Str mode;
    std::fstream _fs;

    bool is_text() const { return mode != "rb" && mode != "wb" && mode != "ab"; }

    FileIO(VM* vm, Str file, Str mode): file(file), mode(mode) {
        if(mode == "rt" || mode == "r" || mode == "rb"){
            _fs.open(file.sv(), std::ios::in);
        }else if(mode == "wt" || mode == "w" || mode == "wb"){
            _fs.open(file.sv(), std::ios::out);
        }else if(mode == "at" || mode == "a" || mode == "ab"){
            _fs.open(file.sv(), std::ios::app);
        }else{
            vm->ValueError("invalid mode");
        }
        if(!_fs.is_open()) vm->IOError(strerror(errno));
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_static_method<2>(type, "__new__", [](VM* vm, ArgsView args){
            return VAR_T(FileIO, 
                vm, CAST(Str, args[0]), CAST(Str, args[1])
            );
        });

        vm->bind_method<0>(type, "read", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            std::vector<char> buffer;
            while(true){
                char c = io._fs.get();
                if(io._fs.eof()) break;
                buffer.push_back(c);
            }
            Bytes b(std::move(buffer));
            if(io.is_text()) return VAR(Str(b.str()));
            return VAR(std::move(b));
        });

        vm->bind_method<1>(type, "write", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            if(io.is_text()) io._fs << CAST(Str&, args[1]);
            else{
                Bytes& buffer = CAST(Bytes&, args[1]);
                io._fs.write(buffer.data(), buffer.size());
            }
            return vm->None;
        });

        vm->bind_method<0>(type, "close", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            io._fs.close();
            return vm->None;
        });

        vm->bind_method<0>(type, "__exit__", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            io._fs.close();
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
        return VAR(std::filesystem::current_path().string());
    });

    vm->bind_func<1>(mod, "chdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        std::filesystem::current_path(path);
        return vm->None;
    });

    vm->bind_func<1>(mod, "listdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        std::filesystem::directory_iterator di;
        try{
            di = std::filesystem::directory_iterator(path);
        }catch(std::filesystem::filesystem_error& e){
            std::string msg = e.what();
            auto pos = msg.find_last_of(":");
            if(pos != std::string::npos) msg = msg.substr(pos + 1);
            vm->IOError(Str(msg).lstrip());
        }
        List ret;
        for(auto& p: di) ret.push_back(VAR(p.path().filename().string()));
        return VAR(ret);
    });

    vm->bind_func<1>(mod, "remove", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<1>(mod, "mkdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool ok = std::filesystem::create_directory(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<1>(mod, "rmdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("operation failed");
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
        bool exists = std::filesystem::exists(path);
        return VAR(exists);
    });
}

} // namespace pkpy


#else

namespace pkpy{
inline void add_module_io(VM* vm){}
inline void add_module_os(VM* vm){}

inline Bytes _read_file_cwd(const Str& name){
    return Bytes();
}

} // namespace pkpy

#endif