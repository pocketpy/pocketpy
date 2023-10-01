#include "pocketpy/io.h"
#include "pocketpy/common.h"

namespace pkpy{

static FILE* io_fopen(const char* name, const char* mode){
#if _MSC_VER
    FILE* fp;
    errno_t err = fopen_s(&fp, name, mode);
    if(err != 0) return nullptr;
    return fp;
#else
    return fopen(name, mode);
#endif
}

static size_t io_fread(void* buffer, size_t size, size_t count, FILE* fp){
#if _MSC_VER
    return fread_s(buffer, std::numeric_limits<size_t>::max(), size, count, fp);
#else
    return fread(buffer, size, count, fp);
#endif
}


Bytes _default_import_handler(const Str& name){
#if PK_ENABLE_OS
    std::filesystem::path path(name.sv());
    bool exists = std::filesystem::exists(path);
    if(!exists) return Bytes();
    std::string cname = name.str();
    FILE* fp = io_fopen(cname.c_str(), "rb");
    if(!fp) return Bytes();
    fseek(fp, 0, SEEK_END);
    int buffer_size = ftell(fp);
    unsigned char* buffer = new unsigned char[buffer_size];
    fseek(fp, 0, SEEK_SET);
    size_t sz = io_fread(buffer, 1, buffer_size, fp);
    PK_UNUSED(sz);
    fclose(fp);
    return Bytes(buffer, buffer_size);
#else
    return Bytes();
#endif
};


#if PK_ENABLE_OS
    void FileIO::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            return VAR_T(FileIO, 
                vm, CAST(Str&, args[1]).str(), CAST(Str&, args[2]).str()
            );
        });

        vm->bind_method<0>(type, "read", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            fseek(io.fp, 0, SEEK_END);
            int buffer_size = ftell(io.fp);
            unsigned char* buffer = new unsigned char[buffer_size];
            fseek(io.fp, 0, SEEK_SET);
            size_t actual_size = io_fread(buffer, 1, buffer_size, io.fp);
            PK_ASSERT(actual_size <= buffer_size);
            // in text mode, CR may be dropped, which may cause `actual_size < buffer_size`
            Bytes b(buffer, actual_size);
            if(io.is_text()) return VAR(b.str());
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

        vm->bind_method<0>(type, "__enter__", PK_LAMBDA(vm->None));
    }

    FileIO::FileIO(VM* vm, std::string file, std::string mode): file(file), mode(mode) {
        fp = io_fopen(file.c_str(), mode.c_str());
        if(!fp) vm->IOError(strerror(errno));
    }

    void FileIO::close(){
        if(fp == nullptr) return;
        fclose(fp);
        fp = nullptr;
    }

#endif

void add_module_io(VM* vm){
#if PK_ENABLE_OS
    PyObject* mod = vm->new_module("io");
    FileIO::register_class(vm, mod);
    vm->bind(vm->builtins, "open(path, mode='r')", [](VM* vm, ArgsView args){
        PK_LOCAL_STATIC StrName m_io("io");
        PK_LOCAL_STATIC StrName m_FileIO("FileIO");
        return vm->call(vm->_modules[m_io]->attr(m_FileIO), args[0], args[1]);
    });
#endif
}

void add_module_os(VM* vm){
#if PK_ENABLE_OS
    PyObject* mod = vm->new_module("os");
    PyObject* path_obj = vm->heap.gcnew<DummyInstance>(vm->tp_object);
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

    // system
    vm->bind_func<1>(mod, "system", [](VM* vm, ArgsView args){
        std::string cmd = CAST(Str&, args[0]).str();
        int ret = system(cmd.c_str());
        return VAR(ret);
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

    vm->bind_func<1>(path_obj, "basename", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        return VAR(path.filename().string());
    });
#endif
}

}   // namespace pkpy