#pragma once

#include "common.h"
#include "obj.h"
#include "vm.h"
#include "_generated.h"

#if __has_include("httplib.h")
#include "httplib.h"

namespace pkpy {

inline void add_module_requests(VM* vm){
    static StrName m_requests("requests");
    static StrName m_Response("Response");
    PyObject* mod = vm->new_module(m_requests);
    CodeObject_ code = vm->compile(kPythonLibs["requests"], "requests.py", EXEC_MODE);
    vm->_exec(code, mod);

    vm->bind_func<4>(mod, "_request", [](VM* vm, ArgsView args){
        Str method = CAST(Str&, args[0]);
        Str url = CAST(Str&, args[1]);
        PyObject* headers = args[2];            // a dict object
        PyObject* body = args[3];               // a bytes object

        if(url.index("http://") != 0){
            vm->ValueError("url must start with http://");
        }

        for(char c: url){
            switch(c){
                case '.':
                case '-':
                case '_':
                case '~':
                case ':':
                case '/':
                break;
                default:
                    if(!isalnum(c)){
                        vm->ValueError(fmt("invalid character in url: '", c, "'"));
                    }
            }
        }

        int slash = url.index("/", 7);
        Str path = "/";
        if(slash != -1){
            path = url.substr(slash);
            url = url.substr(0, slash);
            if(path.empty()) path = "/";
        }

        httplib::Client client(url.str());

        httplib::Headers h;
        if(headers != vm->None){
            List list = CAST(List&, headers);
            for(auto& item : list){
                Tuple t = CAST(Tuple&, item);
                Str key = CAST(Str&, t[0]);
                Str value = CAST(Str&, t[1]);
                h.emplace(key.str(), value.str());
            }
        }

        auto _to_resp = [=](const httplib::Result& res){
            std::vector<char> buf(res->body.size());
            for(int i=0; i<res->body.size(); i++) buf[i] = res->body[i];
            return vm->call(
                vm->_modules[m_requests]->attr(m_Response),
                VAR(res->status),
                VAR(res->reason),
                VAR(Bytes(std::move(buf)))
            );
        };

        if(method == "GET"){
            httplib::Result res = client.Get(path.str(), h);
            return _to_resp(res);
        }else if(method == "POST"){
            Bytes b = CAST(Bytes&, body);
            httplib::Result res = client.Post(path.str(), h, b.data(), b.size(), "application/octet-stream");
            return _to_resp(res);
        }else if(method == "PUT"){
            Bytes b = CAST(Bytes&, body);
            httplib::Result res = client.Put(path.str(), h, b.data(), b.size(), "application/octet-stream");
            return _to_resp(res);
        }else if(method == "DELETE"){
            httplib::Result res = client.Delete(path.str(), h);
            return _to_resp(res);
        }else{
            vm->ValueError("invalid method");
        }
        UNREACHABLE();
    });
}

}   // namespace pkpy

#else

inline void add_module_requests(VM* vm){ }

#endif