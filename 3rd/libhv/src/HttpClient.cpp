#include "HttpMessage.h"
#include "libhv_bindings.hpp"
#include "base/herr.h"
#include "http/client/HttpClient.h"

struct libhv_HttpResponse {
    HttpRequestPtr request;
    HttpResponsePtr response;
    bool ok;

    bool is_valid() { return ok && response != NULL; }

    libhv_HttpResponse(HttpRequestPtr request) : request(request), response(NULL), ok(false) {}
};

static bool libhv_HttpResponse_status_code(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpResponse* resp = (libhv_HttpResponse*)py_touserdata(argv);
    if(!resp->is_valid()) return RuntimeError("HttpResponse: no response");
    py_newint(py_retval(), resp->response->status_code);
    return true;
};

static bool libhv_HttpResponse_headers(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpResponse* resp = (libhv_HttpResponse*)py_touserdata(argv);
    if(!resp->is_valid()) return RuntimeError("HttpResponse: no response");
    py_Ref headers = py_getslot(argv, 0);
    if(py_isnil(headers)) {
        py_newdict(headers);
        py_Ref _0 = py_pushtmp();
        py_Ref _1 = py_pushtmp();
        for(auto& kv: resp->response->headers) {
            py_newstr(_0, kv.first.c_str());
            py_newstr(_1, kv.second.c_str());
            py_dict_setitem(headers, _0, _1);
        }
        py_shrink(2);
    }
    py_assign(py_retval(), headers);
    return true;
};

static bool libhv_HttpResponse_text(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpResponse* resp = (libhv_HttpResponse*)py_touserdata(argv);
    if(!resp->is_valid()) return RuntimeError("HttpResponse: no response");
    py_Ref text = py_getslot(argv, 1);
    if(py_isnil(text)) {
        c11_sv sv;
        sv.data = resp->response->body.c_str();
        sv.size = resp->response->body.size();
        py_newstrv(text, sv);
    }
    py_assign(py_retval(), text);
    return true;
};

static bool libhv_HttpResponse_content(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpResponse* resp = (libhv_HttpResponse*)py_touserdata(argv);
    if(!resp->is_valid()) return RuntimeError("HttpResponse: no response");
    py_Ref content = py_getslot(argv, 2);
    if(py_isnil(content)) {
        int size = resp->response->body.size();
        unsigned char* buf = py_newbytes(content, size);
        memcpy(buf, resp->response->body.data(), size);
    }
    py_assign(py_retval(), content);
    return true;
};

static bool libhv_HttpResponse_json(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpResponse* resp = (libhv_HttpResponse*)py_touserdata(argv);
    if(!resp->is_valid()) return RuntimeError("HttpResponse: no response");
    const char* source = resp->response->body.c_str();  // json string is null-terminated
    return py_json_loads(source);
};

static bool libhv_HttpResponse_completed(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpResponse* resp = (libhv_HttpResponse*)py_touserdata(argv);
    py_newbool(py_retval(), resp->is_valid());
    return true;
}

static bool libhv_HttpResponse__new__(int argc, py_Ref argv) {
    return py_exception(tp_NotImplementedError, "");
}

static bool libhv_HttpResponse__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_assign(py_retval(), argv);
    return true;
}

static bool libhv_HttpResponse__next__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpResponse* resp = (libhv_HttpResponse*)py_touserdata(argv);
    if(!resp->is_valid()) {
        py_newnone(py_retval());
        return true;
    } else {
        if(!py_tpcall(tp_StopIteration, 1, argv)) return false;
        return py_raise(py_retval());
    }
}

static bool libhv_HttpResponse__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpResponse* resp = (libhv_HttpResponse*)py_touserdata(argv);
    if(!resp->is_valid()) {
        py_newstr(py_retval(), "<HttpResponse: no response>");
    } else {
        py_newfstr(py_retval(), "<HttpResponse: %d>", (int)resp->response->status_code);
    }
    return true;
}

static bool libhv_HttpResponse_cancel(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpResponse* resp = (libhv_HttpResponse*)py_touserdata(argv);
    resp->request->Cancel();
    py_newnone(py_retval());
    return true;
}

static py_Type libhv_register_HttpResponse(py_GlobalRef mod) {
    py_Type type = py_newtype("HttpResponse", tp_object, mod, [](void* ud) {
        ((libhv_HttpResponse*)ud)->~libhv_HttpResponse();
    });

    py_bindproperty(type, "status_code", libhv_HttpResponse_status_code, NULL);
    py_bindproperty(type, "headers", libhv_HttpResponse_headers, NULL);
    py_bindproperty(type, "text", libhv_HttpResponse_text, NULL);
    py_bindproperty(type, "content", libhv_HttpResponse_content, NULL);
    py_bindmethod(type, "json", libhv_HttpResponse_json);

    py_bindmagic(type, __new__, libhv_HttpResponse__new__);
    py_bindmagic(type, __iter__, libhv_HttpResponse__iter__);
    py_bindmagic(type, __next__, libhv_HttpResponse__next__);
    py_bindmagic(type, __repr__, libhv_HttpResponse__repr__);
    // completed
    py_bindproperty(type, "completed", libhv_HttpResponse_completed, NULL);
    // cancel
    py_bindmethod(type, "cancel", libhv_HttpResponse_cancel);
    return type;
}

static bool libhv_HttpClient__send_request(py_Ref arg_self,
                                           enum http_method method,
                                           py_Ref arg_url,
                                           py_Ref arg_params,
                                           py_Ref arg_headers,
                                           py_Ref arg_data,
                                           py_Ref arg_json,
                                           py_Ref arg_timeout) {
    hv::HttpClient* cli = (hv::HttpClient*)py_touserdata(arg_self);
    if(!py_checkstr(arg_url)) return false;
    const char* url = py_tostr(arg_url);
    if(!py_checkint(arg_timeout)) return false;
    int timeout = py_toint(arg_timeout);

    auto req = std::make_shared<HttpRequest>();
    req->method = method;
    req->url = url;
    if(!py_isnone(arg_params)) {
        if(!py_checktype(arg_params, tp_dict)) return false;

        bool ok = py_dict_apply(
            arg_params,
            [](py_Ref key, py_Ref value, void* ctx) {
                HttpRequest* p_req = (HttpRequest*)ctx;
                if(!py_checkstr(key)) return false;
                if(!py_str(value)) return false;  // key: str(value)
                p_req->SetParam(py_tostr(key), py_tostr(py_retval()));
                return true;
            },
            req.get());
        if(!ok) return false;
    }

    req->headers["Connection"] = "keep-alive";

    if(!py_isnone(arg_headers)) {
        if(!py_checktype(arg_headers, tp_dict)) return false;

        bool ok = py_dict_apply(
            arg_headers,
            [](py_Ref key, py_Ref value, void* ctx) {
                HttpRequest* p_req = (HttpRequest*)ctx;
                if(!py_checkstr(key)) return false;
                if(!py_str(value)) return false;  // key: str(value)
                p_req->headers[py_tostr(key)] = py_tostr(py_retval());
                return true;
            },
            req.get());
        if(!ok) return false;
    }

    if(!py_isnone(arg_data)) {
        // data must be str or bytes
        if(py_istype(arg_data, tp_str)) {
            req->body = py_tostr(arg_data);
        } else if(py_istype(arg_data, tp_bytes)) {
            int size;
            unsigned char* buf = py_tobytes(arg_data, &size);
            req->body.assign((const char*)buf, size);
        } else {
            return TypeError("HttpClient: data must be str or bytes");
        }
    }

    if(!py_isnone(arg_json)) {
        if(!py_isnone(arg_data)) {
            return ValueError("HttpClient: data and json cannot be set at the same time");
        }

        if(!py_json_dumps(arg_json, 0)) return false;
        req->body = py_tostr(py_retval());
        req->headers["Content-Type"] = "application/json";
    }

    req->timeout = timeout;

    libhv_HttpResponse* retval =
        (libhv_HttpResponse*)py_newobject(py_retval(),
                                          py_gettype("libhv", py_name("HttpResponse")),
                                          3,  // headers, text, content
                                          sizeof(libhv_HttpResponse));
    // placement new
    new (retval) libhv_HttpResponse(req);

    int code = cli->sendAsync(req, [retval](const HttpResponsePtr& resp) {
        if(resp == NULL) {
            retval->ok = false;
            retval->response = NULL;
        } else {
            retval->ok = true;
            retval->response = resp;
        }
    });
    if(code != 0) {
        const char* msg = hv_strerror(code);
        return RuntimeError("HttpClient: %s (%d)", msg, code);
    }
    return true;
}

py_Type libhv_register_HttpClient(py_GlobalRef mod) {
    py_Type type = py_newtype("HttpClient", tp_object, mod, [](void* ud) {
        ((hv::HttpClient*)ud)->~HttpClient();
    });
    py_GlobalRef type_object = py_tpobject(type);
    libhv_register_HttpResponse(mod);

    py_bindmagic(type, __new__, [](int argc, py_Ref argv) {
        PY_CHECK_ARGC(1);
        hv::HttpClient* ud =
            (hv::HttpClient*)py_newobject(py_retval(), py_totype(argv), 0, sizeof(hv::HttpClient));
        new (ud) hv::HttpClient();
        return true;
    });

    py_bind(type_object,
            "get(self, url: str, params=None, headers=None, timeout=10)",
            [](int argc, py_Ref argv) {
                return libhv_HttpClient__send_request(py_arg(0),   // self
                                                      HTTP_GET,    // method
                                                      py_arg(1),   // url
                                                      py_arg(2),   // params
                                                      py_arg(3),   // headers
                                                      py_None(),   // data
                                                      py_None(),   // json
                                                      py_arg(4));  // timeout
            });

    py_bind(type_object,
            "post(self, url: str, params=None, headers=None, data=None, json=None, timeout=10)",
            [](int argc, py_Ref argv) {
                return libhv_HttpClient__send_request(py_arg(0),   // self
                                                      HTTP_POST,   // method
                                                      py_arg(1),   // url
                                                      py_arg(2),   // params
                                                      py_arg(3),   // headers
                                                      py_arg(4),   // data
                                                      py_arg(5),   // json
                                                      py_arg(6));  // timeout
            });

    py_bind(type_object,
            "put(self, url: str, params=None, headers=None, data=None, json=None, timeout=10)",
            [](int argc, py_Ref argv) {
                return libhv_HttpClient__send_request(py_arg(0),   // self
                                                      HTTP_PUT,    // method
                                                      py_arg(1),   // url
                                                      py_arg(2),   // params
                                                      py_arg(3),   // headers
                                                      py_arg(4),   // data
                                                      py_arg(5),   // json
                                                      py_arg(6));  // timeout
            });

    py_bind(type_object,
            "delete(self, url: str, params=None, headers=None, timeout=10)",
            [](int argc, py_Ref argv) {
                return libhv_HttpClient__send_request(py_arg(0),    // self
                                                      HTTP_DELETE,  // method
                                                      py_arg(1),    // url
                                                      py_arg(2),    // params
                                                      py_arg(3),    // headers
                                                      py_None(),    // data
                                                      py_None(),    // json
                                                      py_arg(4));   // timeout
            });
    return type;
}
