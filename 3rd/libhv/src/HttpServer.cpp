#include "HttpMessage.h"
#include "WebSocketChannel.h"
#include "libhv_bindings.hpp"
#include "http/server/WebSocketServer.h"
#include "pocketpy.h"

struct libhv_HttpServer {
    hv::HttpService http_service;
    hv::WebSocketService ws_service;
    hv::WebSocketServer server;

    libhv_MQ<std::pair<HttpContextPtr, std::atomic<int>>*> mq;

    struct WsMessage {
        WsMessageType type;
        hv::WebSocketChannel* channel;
        HttpRequestPtr request;
        std::string body;
    };

    libhv_MQ<WsMessage> ws_mq;
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
    self->server.setHost(host);
    self->server.setPort(port);

    // http
    self->http_service.AllowCORS();
    http_ctx_handler internal_handler = [self](const HttpContextPtr& ctx) {
        std::pair<HttpContextPtr, std::atomic<int>> msg(ctx, 0);
        self->mq.push(&msg);
        int code;
        do {
            code = msg.second.load();
        } while(code == 0);
        return code;
    };
    self->http_service.Any("*", internal_handler);
    self->server.registerHttpService(&self->http_service);

    // websocket
    self->ws_service.onopen = [self](const WebSocketChannelPtr& channel,
                                     const HttpRequestPtr& req) {
        self->ws_mq.push({WsMessageType::onopen, channel.get(), req, ""});
    };
    self->ws_service.onmessage = [self](const WebSocketChannelPtr& channel,
                                        const std::string& msg) {
        self->ws_mq.push({WsMessageType::onmessage, channel.get(), nullptr, msg});
    };
    self->ws_service.onclose = [self](const WebSocketChannelPtr& channel) {
        self->ws_mq.push({WsMessageType::onclose, channel.get(), nullptr, ""});
    };
    self->server.registerWebSocketService(&self->ws_service);

    py_newnone(py_retval());
    return true;
}

static bool libhv_HttpServer_dispatch(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    libhv_HttpServer* self = (libhv_HttpServer*)py_touserdata(py_arg(0));
    py_Ref callable = py_arg(1);
    if(!py_callable(callable)) return TypeError("dispatcher must be callable");

    std::pair<HttpContextPtr, std::atomic<int>>* mq_msg;
    if(!self->mq.pop(&mq_msg)) {
        py_newbool(py_retval(), false);
        return true;
    } else {
        HttpContextPtr ctx = mq_msg->first;
        libhv_HttpRequest_create(py_retval(), ctx->request);
        // call dispatcher
        if(!py_call(callable, 1, py_retval())) return false;

        py_Ref object;
        int status_code = 200;
        if(py_istuple(py_retval())) {
            int length = py_tuple_len(py_retval());
            if(length == 2 || length == 3) {
                // "Hello, world!", 200
                object = py_tuple_getitem(py_retval(), 0);
                py_ItemRef status_code_object = py_tuple_getitem(py_retval(), 1);
                if(!py_checkint(status_code_object)) return false;
                status_code = py_toint(status_code_object);

                if(length == 3) {
                    // "Hello, world!", 200, {"Content-Type": "text/plain"}
                    py_ItemRef headers_object = py_tuple_getitem(py_retval(), 2);
                    if(!py_checktype(headers_object, tp_dict)) return false;
                    bool ok = py_dict_apply(
                        headers_object,
                        [](py_Ref key, py_Ref value, void* ctx_) {
                            if(!py_checkstr(key) || !py_checkstr(value)) return false;
                            ((hv::HttpContext*)ctx_)
                                ->response->SetHeader(py_tostr(key), py_tostr(value));
                            return true;
                        },
                        ctx.get());
                    if(!ok) return false;
                }
            } else {
                return TypeError("dispatcher return tuple must have 2 or 3 elements");
            }
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
                if(!py_json_dumps(object, 0)) return false;
                c11_sv sv = py_tosv(py_retval());
                ctx->response->String(std::string(sv.data, sv.size));
                ctx->response->SetContentType(APPLICATION_JSON);
                break;
            }
        }

        mq_msg->second.store(status_code);
    }
    py_newbool(py_retval(), true);
    return true;
}

static bool libhv_HttpServer_start(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpServer* self = (libhv_HttpServer*)py_touserdata(py_arg(0));
    int code = self->server.start();
    py_newint(py_retval(), code);
    return true;
}

static bool libhv_HttpServer_stop(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpServer* self = (libhv_HttpServer*)py_touserdata(py_arg(0));
    int code = self->server.stop();
    py_newint(py_retval(), code);
    return true;
}

static bool libhv_HttpServer_ws_set_ping_interval(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    libhv_HttpServer* self = (libhv_HttpServer*)py_touserdata(py_arg(0));
    PY_CHECK_ARG_TYPE(1, tp_int);
    int interval = py_toint(py_arg(1));
    self->ws_service.setPingInterval(interval);
    py_newnone(py_retval());
    return true;
}

static bool libhv_HttpServer_ws_close(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    libhv_HttpServer* self = (libhv_HttpServer*)py_touserdata(py_arg(0));
    PY_CHECK_ARG_TYPE(1, tp_int);
    py_i64 channel = py_toint(py_arg(1));
    int code = reinterpret_cast<hv::WebSocketChannel*>(channel)->close();
    py_newint(py_retval(), code);
    return true;
}

static bool libhv_HttpServer_ws_send(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    libhv_HttpServer* self = (libhv_HttpServer*)py_touserdata(py_arg(0));
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_str);
    py_i64 channel = py_toint(py_arg(1));
    const char* msg = py_tostr(py_arg(2));

    hv::WebSocketChannel* p_channel = reinterpret_cast<hv::WebSocketChannel*>(channel);
    int code = p_channel->send(msg);
    py_newint(py_retval(), code);
    return true;
}

static bool libhv_HttpServer_ws_recv(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    libhv_HttpServer* self = (libhv_HttpServer*)py_touserdata(py_arg(0));
    libhv_HttpServer::WsMessage msg;
    if(!self->ws_mq.pop(&msg)) {
        py_newnone(py_retval());
        return true;
    }
    py_Ref data = py_newtuple(py_retval(), 2);
    switch(msg.type) {
        case WsMessageType::onopen: {
            // "onopen", (channel, request)
            assert(msg.request != nullptr);
            py_newstr(py_offset(data, 0), "onopen");
            py_Ref p = py_newtuple(py_offset(data, 1), 2);
            py_newint(py_offset(p, 0), (py_i64)msg.channel);
            libhv_HttpRequest_create(py_offset(p, 1), msg.request);
            break;
        }
        case WsMessageType::onclose: {
            // "onclose", channel
            py_newstr(py_offset(data, 0), "onclose");
            py_newint(py_offset(data, 1), (py_i64)msg.channel);
            break;
        }
        case WsMessageType::onmessage: {
            // "onmessage", (channel, body)
            py_newstr(py_offset(data, 0), "onmessage");
            py_Ref p = py_newtuple(py_offset(data, 1), 2);
            py_newint(py_offset(p, 0), (py_i64)msg.channel);
            c11_sv sv;
            sv.data = msg.body.data();
            sv.size = msg.body.size();
            py_newstrv(py_offset(p, 1), sv);
            break;
        }
    }
    return true;
}

py_Type libhv_register_HttpServer(py_GlobalRef mod) {
    py_Type type = py_newtype("HttpServer", tp_object, mod, [](void* ud) {
        libhv_HttpServer* self = (libhv_HttpServer*)ud;
        self->~libhv_HttpServer();
    });

    py_bindmagic(type, __new__, libhv_HttpServer__new__);
    py_bindmagic(type, __init__, libhv_HttpServer__init__);
    py_bindmethod(type, "start", libhv_HttpServer_start);
    py_bindmethod(type, "stop", libhv_HttpServer_stop);
    py_bindmethod(type, "dispatch", libhv_HttpServer_dispatch);

    py_bindmethod(type, "ws_set_ping_interval", libhv_HttpServer_ws_set_ping_interval);
    py_bindmethod(type, "ws_close", libhv_HttpServer_ws_close);
    py_bindmethod(type, "ws_send", libhv_HttpServer_ws_send);
    py_bindmethod(type, "ws_recv", libhv_HttpServer_ws_recv);
    return type;
}
