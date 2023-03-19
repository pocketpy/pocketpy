#pragma once

#include "ceval.h"
#include "cffi.h"

#if PK_ENABLE_FILEIO

#include <fstream>
#include <filesystem>

namespace pkpy{

Str _read_file_cwd(const Str& name, bool* ok){
    std::filesystem::path path(name.c_str());
    bool exists = std::filesystem::exists(path);
    if(!exists){
        *ok = false;
        return Str();
    }
    std::ifstream ifs(path);
    std::string buffer((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    ifs.close();
    *ok = true;
    return Str(std::move(buffer));
}

struct FileIO {
    PY_CLASS(FileIO, io, FileIO)

    Str file;
    Str mode;
    std::fstream _fs;

    FileIO(VM* vm, Str file, Str mode): file(file), mode(mode) {
        if(mode == "rt" || mode == "r"){
            _fs.open(file, std::ios::in);
        }else if(mode == "wt" || mode == "w"){
            _fs.open(file, std::ios::out);
        }else if(mode == "at" || mode == "a"){
            _fs.open(file, std::ios::app);
        }
        if(!_fs.is_open()) vm->IOError(strerror(errno));
    }

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<2>(type, "__new__", [](VM* vm, Args& args){
            return VAR_T(FileIO, 
                vm, CAST(Str, args[0]), CAST(Str, args[1])
            );
        });

        vm->bind_method<0>(type, "read", [](VM* vm, Args& args){
            FileIO& io = CAST(FileIO&, args[0]);
            std::string buffer;
            io._fs >> buffer;
            return VAR(buffer);
        });

        vm->bind_method<1>(type, "write", [](VM* vm, Args& args){
            FileIO& io = CAST(FileIO&, args[0]);
            io._fs << CAST(Str&, args[1]);
            return vm->None;
        });

        vm->bind_method<0>(type, "close", [](VM* vm, Args& args){
            FileIO& io = CAST(FileIO&, args[0]);
            io._fs.close();
            return vm->None;
        });

        vm->bind_method<0>(type, "__exit__", [](VM* vm, Args& args){
            FileIO& io = CAST(FileIO&, args[0]);
            io._fs.close();
            return vm->None;
        });

        vm->bind_method<0>(type, "__enter__", CPP_LAMBDA(vm->None));
    }
};

void add_module_io(VM* vm){
    PyVar mod = vm->new_module("io");
    PyVar type = FileIO::register_class(vm, mod);
    vm->bind_builtin_func<2>("open", [type](VM* vm, const Args& args){
        return vm->call(type, args);
    });
}

void add_module_os(VM* vm){
    PyVar mod = vm->new_module("os");
    vm->bind_func<0>(mod, "getcwd", [](VM* vm, const Args& args){
        return VAR(std::filesystem::current_path().string());
    });

    vm->bind_func<1>(mod, "listdir", [](VM* vm, const Args& args){
        std::filesystem::path path(CAST(Str&, args[0]).c_str());
        std::filesystem::directory_iterator di;
        try{
            di = std::filesystem::directory_iterator(path);
        }catch(std::filesystem::filesystem_error& e){
            Str msg = e.what();
            auto pos = msg.find_last_of(":");
            if(pos != Str::npos) msg = msg.substr(pos + 1);
            vm->IOError(msg.lstrip());
        }
        List ret;
        for(auto& p: di) ret.push_back(VAR(p.path().filename().string()));
        return VAR(ret);
    });

    vm->bind_func<1>(mod, "remove", [](VM* vm, const Args& args){
        std::filesystem::path path(CAST(Str&, args[0]).c_str());
        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<1>(mod, "mkdir", [](VM* vm, const Args& args){
        std::filesystem::path path(CAST(Str&, args[0]).c_str());
        bool ok = std::filesystem::create_directory(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<1>(mod, "rmdir", [](VM* vm, const Args& args){
        std::filesystem::path path(CAST(Str&, args[0]).c_str());
        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<-1>(mod, "path_join", [](VM* vm, const Args& args){
        std::filesystem::path path;
        for(int i=0; i<args.size(); i++){
            path /= CAST(Str&, args[i]).c_str();
        }
        return VAR(path.string());
    });

    vm->bind_func<1>(mod, "path_exists", [](VM* vm, const Args& args){
        std::filesystem::path path(CAST(Str&, args[0]).c_str());
        bool exists = std::filesystem::exists(path);
        return VAR(exists);
    });
}

} // namespace pkpy


#else

namespace pkpy{
void add_module_io(VM* vm){}
void add_module_os(VM* vm){}

Str _read_file_cwd(const Str& name, bool* ok){
    *ok = false;
    return Str();
}

} // namespace pkpy

#endif