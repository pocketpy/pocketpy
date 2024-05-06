#include "pocketpy/io.h"

#if PK_ENABLE_OS
#include <filesystem>
#include <cstdio>
#include <functional>
#include <optional>
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <fnmatch.h>
#endif
#endif

namespace pkpy{

#if PK_ENABLE_OS

struct FileIO {
    FILE* fp;
    bool is_text;

    FileIO(VM* vm, const Str& file, const Str& mode);
    void close();
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

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

unsigned char* _default_import_handler(const char* name, int* out_size){
    bool exists = std::filesystem::exists(std::filesystem::path(name));
    if(!exists) return nullptr;
    FILE* fp = io_fopen(name, "rb");
    if(!fp) return nullptr;
    fseek(fp, 0, SEEK_END);
    int buffer_size = ftell(fp);
    unsigned char* buffer = new unsigned char[buffer_size];
    fseek(fp, 0, SEEK_SET);
    size_t sz = io_fread(buffer, 1, buffer_size, fp);
    (void)sz;   // suppress warning
    fclose(fp);
    *out_size = buffer_size;
    return buffer;
};

void FileIO::_register(VM* vm, PyObject* mod, PyObject* type){
    vm->bind_func(type, __new__, 3, [](VM* vm, ArgsView args){
        Type cls = PK_OBJ_GET(Type, args[0]);
        return vm->heap.gcnew<FileIO>(cls, vm,
                    py_cast<Str&>(vm, args[1]),
                    py_cast<Str&>(vm, args[2]));
    });

    vm->bind(type, "read(self, size=-1)", [](VM* vm, ArgsView args){
        FileIO& io = PK_OBJ_GET(FileIO, args[0]);
        i64 size = CAST(i64, args[1]);
        i64 buffer_size;
        if(size < 0){
            long current = ftell(io.fp);
            fseek(io.fp, 0, SEEK_END);
            buffer_size = ftell(io.fp);
            fseek(io.fp, current, SEEK_SET);
        }else{
            buffer_size = size;
        }
        unsigned char* buffer = new unsigned char[buffer_size];
        i64 actual_size = io_fread(buffer, 1, buffer_size, io.fp);
        PK_ASSERT(actual_size <= buffer_size);
        // in text mode, CR may be dropped, which may cause `actual_size < buffer_size`
        Bytes b(buffer, actual_size);
        if(io.is_text) return VAR(b.str());
        return VAR(std::move(b));
    });

    vm->bind_func(type, "write", 2, [](VM* vm, ArgsView args){
        FileIO& io = PK_OBJ_GET(FileIO, args[0]);
        if(io.is_text){
            Str& s = CAST(Str&, args[1]);
            fwrite(s.data, 1, s.length(), io.fp);
        }else{
            Bytes& buffer = CAST(Bytes&, args[1]);
            fwrite(buffer.data(), 1, buffer.size(), io.fp);
        }
        return vm->None;
    });

    vm->bind_func(type, "tell", 1, [](VM* vm, ArgsView args){
        FileIO& io = PK_OBJ_GET(FileIO, args[0]);
        long pos = ftell(io.fp);
        if(pos == -1) vm->IOError(strerror(errno));
        return VAR(pos);
    });

    vm->bind_func(type, "seek", 3, [](VM* vm, ArgsView args){
        FileIO& io = PK_OBJ_GET(FileIO, args[0]);
        long offset = CAST(long, args[1]);
        int whence = CAST(int, args[2]);
        int ret = fseek(io.fp, offset, whence);
        if(ret != 0) vm->IOError(strerror(errno));
        return vm->None;
    });

    vm->bind_func(type, "close", 1, [](VM* vm, ArgsView args){
        FileIO& io = PK_OBJ_GET(FileIO, args[0]);
        io.close();
        return vm->None;
    });

    vm->bind_func(type, __exit__, 1, [](VM* vm, ArgsView args){
        FileIO& io = PK_OBJ_GET(FileIO, args[0]);
        io.close();
        return vm->None;
    });

    vm->bind_func(type, __enter__, 1, PK_LAMBDA(args[0]));
}

FileIO::FileIO(VM* vm, const Str& file, const Str& mode){
    this->is_text = mode.sv().find("b") == std::string::npos;
    fp = io_fopen(file.c_str(), mode.c_str());
    if(!fp) vm->IOError(strerror(errno));
}

void FileIO::close(){
    if(fp == nullptr) return;
    fclose(fp);
    fp = nullptr;
}

void add_module_io(VM* vm){
    PyObject* mod = vm->new_module("io");
    vm->register_user_class<FileIO>(mod, "FileIO");

    mod->attr().set("SEEK_SET", VAR(SEEK_SET));
    mod->attr().set("SEEK_CUR", VAR(SEEK_CUR));
    mod->attr().set("SEEK_END", VAR(SEEK_END));

    vm->bind(vm->builtins, "open(path, mode='r')", [](VM* vm, ArgsView args){
        PK_LOCAL_STATIC StrName m_io("io");
        PK_LOCAL_STATIC StrName m_FileIO("FileIO");
        return vm->call(vm->_modules[m_io]->attr(m_FileIO), args[0], args[1]);
    });
}

void add_module_os(VM* vm){
    PyObject* mod = vm->new_module("os");
    PyObject* path_obj = vm->heap.gcnew<DummyInstance>(vm->tp_object);
    mod->attr().set("path", path_obj);
    
    // Working directory is shared by all VMs!!
    vm->bind_func(mod, "getcwd", 0, [](VM* vm, ArgsView args){
        return VAR(std::filesystem::current_path().string());
    });

    vm->bind_func(mod, "chdir", 1, [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        std::filesystem::current_path(path);
        return vm->None;
    });

    vm->bind_func(mod, "listdir", 1, [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        std::filesystem::directory_iterator di;
        try{
            di = std::filesystem::directory_iterator(path);
        }catch(std::filesystem::filesystem_error&){
            vm->IOError(path.string());
        }
        List ret;
        for(auto& p: di) ret.push_back(VAR(p.path().filename().string()));
        return VAR(ret);
    });

    vm->bind_func(mod, "remove", 1, [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func(mod, "mkdir", 1, [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool ok = std::filesystem::create_directory(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func(mod, "rmdir", 1, [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func(path_obj, "join", -1, [](VM* vm, ArgsView args){
        std::filesystem::path path;
        for(int i=0; i<args.size(); i++){
            path /= CAST(Str&, args[i]).sv();
        }
        return VAR(path.string());
    });

    vm->bind_func(path_obj, "exists", 1, [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool exists = std::filesystem::exists(path);
        return VAR(exists);
    });

    vm->bind_func(path_obj, "basename", 1, [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        return VAR(path.filename().string());
    });

    vm->bind_func(path_obj, "isdir", 1, [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool isdir = std::filesystem::is_directory(path);
        return VAR(isdir);
    });

    vm->bind_func(path_obj, "isfile", 1, [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        bool isfile = std::filesystem::is_regular_file(path);
        return VAR(isfile);
    });

    vm->bind_func(path_obj, "abspath", 1, [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        return VAR(std::filesystem::absolute(path).string());
    });
}

void add_module_stat(VM* vm){
    PyObject* mod = vm->new_module("stat");

    mod->attr().set("S_IFDIR", VAR(0040000));
    mod->attr().set("S_IFCHR", VAR(0020000));
    mod->attr().set("S_IFBLK", VAR(0060000));
    mod->attr().set("S_IFREG", VAR(0100000));
    mod->attr().set("S_IFIFO", VAR(0010000));
    mod->attr().set("S_IFLNK", VAR(0120000));
    mod->attr().set("S_IFSOCK", VAR(0140000));

    mod->attr().set("S_ISUID", VAR(04000));
    mod->attr().set("S_ISGID", VAR(02000));
    mod->attr().set("S_ISVTX", VAR(01000));

    mod->attr().set("S_IRWXU", VAR(00700));
    mod->attr().set("S_IRUSR", VAR(00400));
    mod->attr().set("S_IWUSR", VAR(00200));
    mod->attr().set("S_IXUSR", VAR(00100));

    mod->attr().set("S_IRWXG", VAR(00070));
    mod->attr().set("S_IRGRP", VAR(00040));
    mod->attr().set("S_IWGRP", VAR(00020));
    mod->attr().set("S_IXGRP", VAR(00010));

    mod->attr().set("S_IRWXO", VAR(00007));
    mod->attr().set("S_IROTH", VAR(00004));
    mod->attr().set("S_IWOTH", VAR(00002));
    mod->attr().set("S_IXOTH", VAR(00001));

    struct stat_result {
        int st_mode;
        uint64_t st_ino;
        uint64_t st_dev;
        uint64_t st_nlink;
        uint64_t st_uid;
        uint64_t st_gid;
        int64_t st_size;
        double st_atime;
        double st_mtime;
        double st_ctime;

        stat_result(const std::filesystem::file_status& status, const std::filesystem::directory_entry& entry)
            : st_mode(static_cast<int>(status.permissions())), 
              st_ino(entry.file_size()),
              st_dev(0),
              st_nlink(entry.hard_link_count()), 
              st_uid(0), 
              st_gid(0),
              st_size(entry.file_size()),
              st_atime(std::chrono::duration_cast<std::chrono::seconds>(entry.last_write_time().time_since_epoch()).count()),
              st_mtime(std::chrono::duration_cast<std::chrono::seconds>(entry.last_write_time().time_since_epoch()).count()),
              st_ctime(std::chrono::duration_cast<std::chrono::seconds>(entry.last_write_time().time_since_epoch()).count()) {}
    };

    vm->bind_func(mod, "stat", 1, [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        std::filesystem::directory_entry entry;
        try{
            entry = std::filesystem::directory_entry(path);
        }catch(std::filesystem::filesystem_error&){
            vm->OSError(path.string());
        }
        auto status = entry.status();
        stat_result result(status, entry);

        PyObject* stat_obj = vm->heap.gcnew<DummyInstance>(vm->tp_object);
        stat_obj->attr().set("st_mode", VAR(result.st_mode));
        stat_obj->attr().set("st_ino", VAR(result.st_ino));
        stat_obj->attr().set("st_dev", VAR(result.st_dev));
        stat_obj->attr().set("st_nlink", VAR(result.st_nlink));
        stat_obj->attr().set("st_uid", VAR(result.st_uid));
        stat_obj->attr().set("st_gid", VAR(result.st_gid));
        stat_obj->attr().set("st_size", VAR(result.st_size));
        stat_obj->attr().set("st_atime", VAR(result.st_atime));
        stat_obj->attr().set("st_mtime", VAR(result.st_mtime));
        stat_obj->attr().set("st_ctime", VAR(result.st_ctime));

        return stat_obj;
    });

    vm->bind_func(mod, "S_ISDIR", 1, [](VM* vm, ArgsView args){
        int mode = CAST(int, args[0]);
        return VAR((mode & 0170000) == 0040000);
    });

    vm->bind_func(mod, "S_ISCHR", 1, [](VM* vm, ArgsView args){
        int mode = CAST(int, args[0]);
        return VAR((mode & 0170000) == 0020000);
    });

    vm->bind_func(mod, "S_ISBLK", 1, [](VM* vm, ArgsView args){
        int mode = CAST(int, args[0]);
        return VAR((mode & 0170000) == 0060000);
    });

    vm->bind_func(mod, "S_ISREG", 1, [](VM* vm, ArgsView args){
        int mode = CAST(int, args[0]);
        return VAR((mode & 0170000) == 0100000);
    });

    vm->bind_func(mod, "S_ISFIFO", 1, [](VM* vm, ArgsView args){
        int mode = CAST(int, args[0]);
        return VAR((mode & 0170000) == 0010000);
    });

    vm->bind_func(mod, "S_ISLNK", 1, [](VM* vm, ArgsView args){
        int mode = CAST(int, args[0]);
        return VAR((mode & 0170000) == 0120000);
    });

    vm->bind_func(mod, "S_ISSOCK", 1, [](VM* vm, ArgsView args){
        int mode = CAST(int, args[0]);
        return VAR((mode & 0170000) == 0140000);
    });

    vm->bind_func(mod, "S_IMODE", 1, [](VM* vm, ArgsView args){
        int mode = CAST(int, args[0]);
        return VAR(mode & 07777);
    });

    vm->bind_func(mod, "S_IFMT", 1, [](VM* vm, ArgsView args){
            int mode = CAST(int, args[0]);
            return VAR(mode & 0170000);
        });
}

void add_module_glob(VM* vm){
    PyObject* mod = vm->new_module("glob");

    vm->bind_func(mod, "glob", 1, [](VM* vm, ArgsView args){
        std::string_view pattern = CAST(Str&, args[0]).sv();
        std::optional<std::string_view> root_dir_opt = std::nullopt;
        bool recursive = false;
        bool include_hidden = false;

        if (args.size() > 1)
            root_dir_opt = CAST(Str&, args[1]).sv();
        if (args.size() > 2)
            recursive = CAST(bool, args[2]);
        if (args.size() > 3)
            include_hidden = CAST(bool, args[3]);
        std::vector<std::string> result;

        std::string root_dir = root_dir_opt ? std::string(*root_dir_opt) : ".";
        std::filesystem::path base_dir(root_dir);
        std::filesystem::path search_pattern(pattern.data());

        std::function<void(const std::filesystem::path&)> search_dir = [&](const std::filesystem::path& dir) {
            #ifdef _WIN32
            WIN32_FIND_DATAA data;
            std::string pattern_str = (dir / search_pattern).string();
            HANDLE hFind = FindFirstFileA(pattern_str.c_str(), &data);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if (include_hidden || data.cFileName[0] != '.') {
                        result.push_back((dir / data.cFileName).string());
                    }
                } while (FindNextFileA(hFind, &data));
                FindClose(hFind);
            }
            #else
            DIR* dp = opendir(dir.c_str());
            if (dp != nullptr) {
                dirent* entry;
                while ((entry = readdir(dp)) != nullptr) {
                    std::filesystem::path entry_path = dir / entry->d_name;
                    if (fnmatch(search_pattern.c_str(), entry->d_name, FNM_PATHNAME) == 0 &&
                        (include_hidden || entry->d_name[0] != '.')) {
                        result.push_back(entry_path.string());
                    }
                }
                closedir(dp);
            }
            #endif
        };

        search_dir(base_dir);

        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(base_dir)) {
                if (entry.is_directory()) {
                    search_dir(entry.path());
                }
            }
        }

        List ret;
        for (const auto& path : result) {
            ret.push_back(VAR(path));
        }
        return VAR(ret);
    });

    vm->bind_func(mod, "escape", 1, [](VM* vm, ArgsView args){
        std::string_view pattern = CAST(Str&, args[0]).sv();
        std::string escaped;
        for (char c : pattern) {
            if (c == '*' || c == '?' || c == '[' || c == ']') {
                escaped += '[';
                escaped += c;
                escaped += ']';
            } else {
                escaped += c;
            }
        }
        return VAR(escaped);
    });

    vm->bind_func(mod, "has_magic", 1, [](VM* vm, ArgsView args){
        std::string_view pattern = CAST(Str&, args[0]).sv();
        return VAR(pattern.find_first_of("*?[") != std::string::npos);
    });
}
#else

void add_module_io(VM* vm){}
void add_module_os(VM* vm){}
unsigned char* _default_import_handler(const char* name, int* out_size){
    return nullptr;
}

#endif

}   // namespace pkpy