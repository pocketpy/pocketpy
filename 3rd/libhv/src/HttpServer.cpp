#include "libhv_bindings.hpp"
#include "http/server/HttpServer.h"
#include "base/herr.h"

#include <deque>
#include <atomic>

template <typename T_in, typename T_out>
struct libhv_MQ {
    std::atomic<bool> lock_in;
    std::atomic<bool> lock_out;
    std::deque<T_in> queue_in;
    std::deque<T_out> queue_out;

    void begin_in() {
        while(lock_in.exchange(true)) {
            hv_delay(1);
        }
    }

    void end_in() { lock_in.store(false); }

    void begin_out() {
        while(lock_out.exchange(true)) {
            hv_delay(1);
        }
    }

    void end_out() { lock_out.store(false); }
};

struct libhv_HttpServer {
    hv::HttpService service;
    hv::HttpServer server;
    libhv_MQ<HttpContextPtr, std::pair<HttpContextPtr, int>> mq;
};

static bool libhv_HttpServer__new__(int argc, py_Ref argv) {
    libhv_HttpServer* self =
        (libhv_HttpServer*)py_newobject(py_retval(), py_totype(argv), 0, sizeof(libhv_HttpServer));
    new (self) libhv_HttpServer();
    return true;
}

static bool libhv_HttpServer__init__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    libhv_HttpServer* self = (libhv_HttpServer*)py_touserdata(py_arg(0));
    PY_CHECK_ARG_TYPE(1, tp_str);
    PY_CHECK_ARG_TYPE(2, tp_int);
    const char* host = py_tostr(py_arg(1));
    int port = py_toint(py_arg(2));

    self->service.AllowCORS();
    http_ctx_handler internal_handler = [self](const HttpContextPtr& ctx) {
        self->mq.begin_in();
        self->mq.queue_in.push_back(ctx);
        self->mq.end_in();

        while(true) {
            self->mq.begin_out();
            if(!self->mq.queue_out.empty()) {
                auto& msg = self->mq.queue_out.front();
                if(msg.first == ctx) {
                    self->mq.queue_out.pop_front();
                    self->mq.end_out();
                    return msg.second;
                }
            }
            self->mq.end_out();
            hv_delay(1);
        }
    };
    self->service.Any("*", internal_handler);
    self->server.registerHttpService(&self->service);
    self->server.setHost(host);
    self->server.setPort(port);
    py_newnone(py_retval());
    return true;
}

static bool libhv_HttpServer_dispatch(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    libhv_HttpServer* self = (libhv_HttpServer*)py_touserdata(py_arg(0));
    py_Ref callable = py_arg(1);
    if(!py_callable(callable)) return TypeError("dispatcher must be callable");

    self->mq.begin_in();
    if(self->mq.queue_in.empty()) {
        self->mq.end_in();
        py_newbool(py_retval(), false);
        return true;
    } else {
        HttpContextPtr ctx = self->mq.queue_in.front();
        self->mq.queue_in.pop_front();
        self->mq.end_in();

        const char* method = ctx->request->Method();
        std::string path = ctx->request->Path();
        const http_headers& headers = ctx->request->headers;
        const std::string& data = ctx->request->body;

        py_OutRef msg = py_pushtmp();
        py_newdict(msg);
        py_Ref _0 = py_pushtmp();
        py_Ref _1 = py_pushtmp();
        py_Ref _2 = py_pushtmp();
        py_Ref _3 = py_pushtmp();

        // method
        py_newstr(_0, "method");
        py_newstr(_1, method);
        py_dict_setitem(msg, _0, _1);
        // path
        py_newstr(_0, "path");
        py_newstr(_1, path.c_str());
        py_dict_setitem(msg, _0, _1);
        // headers
        py_newstr(_0, "headers");
        py_newdict(_1);
        py_dict_setitem(msg, _0, _1);
        for(auto& header: headers) {
            py_newstr(_2, header.first.c_str());
            py_newstr(_3, header.second.c_str());
            py_dict_setitem(_1, _2, _3);
        }
        // data
        py_newstr(_0, "data");
        auto content_type = ctx->request->ContentType();
        bool is_text_data = content_type == TEXT_PLAIN || content_type == APPLICATION_JSON ||
                            content_type == APPLICATION_XML || content_type == TEXT_HTML ||
                            content_type == CONTENT_TYPE_NONE;
        if(is_text_data) {
            py_newstrv(_1, {data.c_str(), (int)data.size()});
        } else {
            unsigned char* buf = py_newbytes(_1, data.size());
            memcpy(buf, data.data(), data.size());
        }
        py_dict_setitem(msg, _0, _1);
        py_assign(py_retval(), msg);
        py_shrink(5);

        // call dispatcher
        if(!py_call(callable, 1, py_retval())) { return false; }

        py_Ref object;
        int status_code = 200;
        if(py_istuple(py_retval())) {
            // "Hello, world!", 200
            if(py_tuple_len(py_retval()) != 2) {
                return ValueError("dispatcher should return `object | tuple[object, int]`");
            }
            object = py_tuple_getitem(py_retval(), 0);
            py_ItemRef status_code_object = py_tuple_getitem(py_retval(), 1);
            if(!py_checkint(status_code_object)) return false;
            status_code = py_toint(status_code_object);
        } else {
            // "Hello, world!"
            object = py_retval();
        }

        switch(py_typeof(object)) {
            case tp_bytes: {
                int size;
                unsigned char* buf = py_tobytes(object, &size);
                ctx->response->Data(buf, size, false);
                break;
            }
            case tp_str: {
                c11_sv sv = py_tosv(object);
                ctx->response->String(std::string(sv.data, sv.size));
                break;
            }
            case tp_NoneType: {
                break;
            }
            default: {
                if(!py_json_dumps(object)) return false;
                c11_sv sv = py_tosv(py_retval());
                ctx->response->String(std::string(sv.data, sv.size));
                ctx->response->SetContentType(APPLICATION_JSON);
                break;
            }
        }

        self->mq.begin_out();
        self->mq.queue_out.push_back({ctx, status_code});
        self->mq.end_out();
    }
    py_newbool(py_retval(), true);
    return true;
}

static bool libhv_HttpServer_start(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpServer* self = (libhv_HttpServer*)py_touserdata(py_arg(0));
    int code = self->server.start();
    if(code != 0) {
        return RuntimeError("HttpServer start failed: %s (%d)", hv_strerror(code), code);
    }
    py_newnone(py_retval());
    return true;
}

static bool libhv_HttpServer_stop(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpServer* self = (libhv_HttpServer*)py_touserdata(py_arg(0));
    self->server.stop();
    py_newnone(py_retval());
    return true;
}

py_Type libhv_register_HttpServer(py_GlobalRef mod) {
    py_Type type = py_newtype("HttpServer", tp_object, mod, [](void* ud) {
        libhv_HttpServer* self = (libhv_HttpServer*)ud;
        self->~libhv_HttpServer();
    });

    py_bindmagic(type, __new__, libhv_HttpServer__new__);
    py_bindmagic(type, __init__, libhv_HttpServer__init__);
    py_bindmethod(type, "dispatch", libhv_HttpServer_dispatch);
    py_bindmethod(type, "start", libhv_HttpServer_start);
    py_bindmethod(type, "stop", libhv_HttpServer_stop);
    return type;
}