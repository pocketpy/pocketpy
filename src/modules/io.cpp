#include "pocketpy/modules/io.hpp"
#include "pocketpy/interpreter/bindings.hpp"

#if PK_ENABLE_OS
#include <filesystem>
#include <cstdio>
#endif

namespace pkpy {

#if PK_ENABLE_OS

struct FileIO {
    FILE* fp;
    bool is_text;

    FileIO(VM* vm, const Str& file, const Str& mode);
    void close();
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

static FILE* io_fopen(const char* name, const char* mode) {
#if _MSC_VER
    FILE* fp;
    errno_t err = fopen_s(&fp, name, mode);
    if(err != 0) return nullptr;
    return fp;
#else
    return fopen(name, mode);
#endif
}

static size_t io_fread(void* buffer, size_t size, size_t count, FILE* fp) {
#if _MSC_VER
    return fread_s(buffer, std::numeric_limits<size_t>::max(), size, count, fp);
#else
    return fread(buffer, size, count, fp);
#endif
}

unsigned char* _default_import_handler(const char* name, int* out_size) {
    bool exists = std::filesystem::exists(std::filesystem::path(name));
    if(!exists) return nullptr;
    FILE* fp = io_fopen(name, "rb");
    if(!fp) return nullptr;
    fseek(fp, 0, SEEK_END);
    int buffer_size = ftell(fp);
    unsigned char* buffer = new unsigned char[buffer_size];
    fseek(fp, 0, SEEK_SET);
    size_t sz = io_fread(buffer, 1, buffer_size, fp);
    (void)sz;  // suppress warning
    fclose(fp);
    *out_size = buffer_size;
    return buffer;
};

void FileIO::_register(VM* vm, PyObject* mod, PyObject* type) {
    vm->bind_func(type, __new__, 3, [](VM* vm, ArgsView args) {
        Type cls = PK_OBJ_GET(Type, args[0]);
        return vm->new_object<FileIO>(cls, vm, py_cast<Str&>(vm, args[1]), py_cast<Str&>(vm, args[2]));
    });

    vm->bind(type, "read(self, size=-1)", [](VM* vm, ArgsView args) {
        FileIO& io = PK_OBJ_GET(FileIO, args[0]);
        i64 size = CAST(i64, args[1]);
        i64 buffer_size;
        if(size < 0) {
            long current = ftell(io.fp);
            fseek(io.fp, 0, SEEK_END);
            buffer_size = ftell(io.fp);
            fseek(io.fp, current, SEEK_SET);
        } else {
            buffer_size = size;
        }
        unsigned char* buffer = (unsigned char*)std::malloc(buffer_size);
        i64 actual_size = io_fread(buffer, 1, buffer_size, io.fp);
        assert(actual_size <= buffer_size);
        // in text mode, CR may be dropped, which may cause `actual_size < buffer_size`
        Bytes b(buffer, actual_size);
        if(io.is_text) { return VAR(std::string_view((char*)b.data(), b.size())); }
        return VAR(std::move(b));
    });

    vm->bind_func(type, "write", 2, [](VM* vm, ArgsView args) {
        FileIO& io = PK_OBJ_GET(FileIO, args[0]);
        if(io.is_text) {
            Str& s = CAST(Str&, args[1]);
            fwrite(s.data, 1, s.length(), io.fp);
        } else {
            Bytes& buffer = CAST(Bytes&, args[1]);
            fwrite(buffer.data(), 1, buffer.size(), io.fp);
        }
        return vm->None;
    });

    vm->bind_func(type, "tell", 1, [](VM* vm, ArgsView args) {
        FileIO& io = PK_OBJ_GET(FileIO, args[0]);
        long pos = ftell(io.fp);
        if(pos == -1) vm->IOError(strerror(errno));
        return VAR(pos);
    });

    vm->bind_func(type, "seek", 3, [](VM* vm, ArgsView args) {
        FileIO& io = PK_OBJ_GET(FileIO, args[0]);
        long offset = CAST(long, args[1]);
        int whence = CAST(int, args[2]);
        int ret = fseek(io.fp, offset, whence);
        if(ret != 0) vm->IOError(strerror(errno));
        return vm->None;
    });

    vm->bind_func(type, "close", 1, [](VM* vm, ArgsView args) {
        FileIO& io = PK_OBJ_GET(FileIO, args[0]);
        io.close();
        return vm->None;
    });

    vm->bind_func(type, __exit__, 1, [](VM* vm, ArgsView args) {
        FileIO& io = PK_OBJ_GET(FileIO, args[0]);
        io.close();
        return vm->None;
    });

    vm->bind_func(type, __enter__, 1, PK_LAMBDA(args[0]));
}

FileIO::FileIO(VM* vm, const Str& file, const Str& mode) {
    this->is_text = mode.sv().find("b") == std::string::npos;
    fp = io_fopen(file.c_str(), mode.c_str());
    if(!fp) vm->IOError(strerror(errno));
}

void FileIO::close() {
    if(fp == nullptr) return;
    fclose(fp);
    fp = nullptr;
}

void add_module_io(VM* vm) {
    PyObject* mod = vm->new_module("io");
    vm->register_user_class<FileIO>(mod, "FileIO");

    mod->attr().set("SEEK_SET", VAR(SEEK_SET));
    mod->attr().set("SEEK_CUR", VAR(SEEK_CUR));
    mod->attr().set("SEEK_END", VAR(SEEK_END));

    vm->bind(vm->builtins, "open(path, mode='r')", [](VM* vm, ArgsView args) {
        return vm->call(vm->_modules["io"]->attr("FileIO"), args[0], args[1]);
    });
}

void add_module_os(VM* vm) {
    PyObject* mod = vm->new_module("os");
    PyObject* path_obj = vm->heap.gcnew<DummyInstance>(VM::tp_object);
    mod->attr().set("path", path_obj);

    // Working directory is shared by all VMs!!
    vm->bind_func(mod, "getcwd", 0, [](VM* vm, ArgsView args) {
        return VAR(std::filesystem::current_path().string());
    });

    vm->bind_func(mod, "chdir", 1, [](VM* vm, ArgsView args) {
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        std::filesystem::current_path(path);
        return vm->None;
    });

    vm->bind_func(mod, "listdir", 1, [](VM* vm, ArgsView args) {
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        std::filesystem::directory_iterator di;
        try {
            di = std::filesystem::directory_iterator(path);
        } catch(std::filesystem::filesystem_error&) { vm->IOError(path.string()); }
        List ret;
        for(auto& p: di)
            ret.push_back(VAR(p.path().filename().string()));
        return VAR(std::move(ret));
    });

    vm->bind_func(mod, "remove", 1, [](VM* vm, ArgsView args) {
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func(mod, "mkdir", 1, [](VM* vm, ArgsView args) {
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool ok = std::filesystem::create_directory(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func(mod, "rmdir", 1, [](VM* vm, ArgsView args) {
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func(path_obj, "join", -1, [](VM* vm, ArgsView args) {
        std::filesystem::path path;
        for(int i = 0; i < args.size(); i++) {
            path /= CAST(Str&, args[i]).sv();
        }
        return VAR(path.string());
    });

    vm->bind_func(path_obj, "exists", 1, [](VM* vm, ArgsView args) {
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool exists = std::filesystem::exists(path);
        return VAR(exists);
    });

    vm->bind_func(path_obj, "basename", 1, [](VM* vm, ArgsView args) {
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        return VAR(path.filename().string());
    });

    vm->bind_func(path_obj, "isdir", 1, [](VM* vm, ArgsView args) {
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool isdir = std::filesystem::is_directory(path);
        return VAR(isdir);
    });

    vm->bind_func(path_obj, "isfile", 1, [](VM* vm, ArgsView args) {
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool isfile = std::filesystem::is_regular_file(path);
        return VAR(isfile);
    });

    vm->bind_func(path_obj, "abspath", 1, [](VM* vm, ArgsView args) {
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        return VAR(std::filesystem::absolute(path).string());
    });
}
#else

void add_module_io(VM* vm) {}

void add_module_os(VM* vm) {}

unsigned char* _default_import_handler(const char* name, int* out_size) { return nullptr; }

#endif

}  // namespace pkpy
