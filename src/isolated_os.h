#pragma once

#include "ceval.h"
#include "cffi.h"
#include "common.h"

#if PK_ENABLE_OS

#include <filesystem>
#include <cstdio>

namespace pkpy {

inline std::filesystem::path get_rel(const VM* vm, const std::filesystem::path & rel) {
    return rel.lexically_relative(vm->_lowest_isolated_cwd_path);
}

bool check_if_path_is_isolated(const std::string & path_to_check_str,
                               const std::string & toplevel_path_str) {
    //.lexically_normal() expands somepath/somdir/.. to somepath/
    auto toplevel_path = std::filesystem::path(toplevel_path_str).lexically_normal();
    auto path_to_check = std::filesystem::path(toplevel_path_str+path_to_check_str).lexically_normal();

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

inline void add_module_isolated_os(VM *vm) {
    if (vm->_lowest_isolated_cwd_path.empty()) {
        throw std::invalid_argument("vm->_lowest_isolated_cwd_path is empty. Change it to a path of VM's allowed operation.");
    }
    if (!std::filesystem::exists(vm->_lowest_isolated_cwd_path)) {
        throw std::invalid_argument("vm->_lowest_isolated_cwd_path doesn't exist.");
    }

    PyObject* mod = vm->new_module("os");
    PyObject* path_obj = vm->heap.gcnew<DummyInstance>(vm->tp_object, {});
    mod->attr().set("path", path_obj);

    vm->bind_func<0>(mod, "getcwd", [](VM* vm, ArgsView args){
        return VAR(get_rel(vm, std::filesystem::current_path()).string());
    });

    vm->bind_func<1>(mod, "chdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        if (!check_if_path_is_isolated(path, vm->_lowest_isolated_cwd_path)) {
            std::filesystem::current_path(vm->_lowest_isolated_cwd_path);
            return vm->None;
        }
        std::filesystem::current_path(vm->_lowest_isolated_cwd_path / path);
        return vm->None;
    });

    vm->bind_func<1>(mod, "listdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        if (!check_if_path_is_isolated(path, vm->_lowest_isolated_cwd_path)) {vm->IOError(Str("Invalid path."));}
        path = vm->_lowest_isolated_cwd_path / path;

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
        for(auto& p: di) ret.push_back(VAR(get_rel(vm, p.path()).filename().string()));
        return VAR(ret);
    });

    vm->bind_func<1>(mod, "remove", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        if (!check_if_path_is_isolated(path, vm->_lowest_isolated_cwd_path)) {vm->IOError(Str("Operation failed."));}
        path = vm->_lowest_isolated_cwd_path / path;

        bool ok = std::filesystem::remove(path);
        if(!ok) vm->IOError("Operation failed.");
        return vm->None;
    });

    vm->bind_func<1>(mod, "mkdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        if (!check_if_path_is_isolated(path, vm->_lowest_isolated_cwd_path)) {vm->IOError(Str("Operation failed."));}
        path = vm->_lowest_isolated_cwd_path / path;

        bool ok = std::filesystem::create_directory(path);
        if(!ok) vm->IOError("Operation failed.");
        return vm->None;
    });

    vm->bind_func<1>(mod, "rmdir", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        if (!check_if_path_is_isolated(path, vm->_lowest_isolated_cwd_path)) {vm->IOError(Str("Operation failed."));}
        path = vm->_lowest_isolated_cwd_path / path;

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
        path = vm->_lowest_isolated_cwd_path / path;
        bool exists = std::filesystem::exists(path);
        return VAR(exists);
    });

    vm->bind_func<1>(path_obj, "basename", [](VM* vm, ArgsView args){
        std::filesystem::path path(CAST(Str&, args[0]).sv());
        return VAR(path.filename().string());
    });
}

}// namespace pkpy

#else

namespace pkpy{
inline void add_module_isolated_os(void* vm){}
} // namespace pkpy

#endif
