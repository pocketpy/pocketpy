#include "HttpMessage.h"
#include "libhv_bindings.hpp"
#include "pocketpy.h"
#include "http/client/WebSocketClient.h"

struct libhv_WebSocketClient {
    hv::WebSocketClient ws;

    libhv_MQ<std::pair<WsMessageType, std::string>> mq;

    libhv_WebSocketClient() {
        ws.onopen = [this]() {
            mq.push({WsMessageType::onopen, ""});
        };
        ws.onclose = [this]() {
            mq.push({WsMessageType::onclose, ""});
        };
        ws.onmessage = [this](const std::string& msg) {
            mq.push({WsMessageType::onmessage, msg});
        };

        // reconnect: 1,2,4,8,10,10,10...
        reconn_setting_t reconn;
        reconn_setting_init(&reconn);
        reconn.min_delay = 1000;
        reconn.max_delay = 10000;
        reconn.delay_policy = 2;
        ws.setReconnect(&reconn);
    }
};

py_Type libhv_register_WebSocketClient(py_GlobalRef mod) {
    py_Type type = py_newtype("WebSocketClient", tp_object, mod, [](void* ud) {
        libhv_WebSocketClient* self = (libhv_WebSocketClient*)ud;
        self->~libhv_WebSocketClient();
    });

    py_bindmagic(type, __new__, [](int argc, py_Ref argv) {
        PY_CHECK_ARGC(1);
        libhv_WebSocketClient* self = (libhv_WebSocketClient*)
            py_newobject(py_retval(), py_totype(argv), 0, sizeof(libhv_WebSocketClient));
        new (self) libhv_WebSocketClient();
        return true;
    });

    py_bindmethod(type, "open", [](int argc, py_Ref argv) {
        libhv_WebSocketClient* self = (libhv_WebSocketClient*)py_touserdata(argv);
        PY_CHECK_ARG_TYPE(1, tp_str);
        const char* url = py_tostr(py_arg(1));
        http_headers headers = DefaultHeaders;
        if(argc == 2) {
            // open(self, url)
        } else if(argc == 3) {
            // open(self, url, headers)
            if(!py_checktype(py_arg(2), tp_dict)) return false;
            bool ok = py_dict_apply(
                py_arg(2),
                [](py_Ref key, py_Ref value, void* ctx) {
                    http_headers* p_headers = (http_headers*)ctx;
                    if(!py_checkstr(key)) return false;
                    if(!py_checkstr(value)) return false;
                    p_headers->operator[] (py_tostr(key)) = py_tostr(value);
                    return true;
                },
                &headers);
            if(!ok) return false;
        } else {
            return TypeError("open() takes 2 or 3 arguments");
        }
        int code = self->ws.open(url, headers);
        py_newint(py_retval(), code);
        return true;
    });

    py_bindmethod(type, "close", [](int argc, py_Ref argv) {
        PY_CHECK_ARGC(1);
        libhv_WebSocketClient* self = (libhv_WebSocketClient*)py_touserdata(argv);
        int code = self->ws.close();
        py_newint(py_retval(), code);
        return true;
    });

    py_bindmethod(type, "send", [](int argc, py_Ref argv) {
        PY_CHECK_ARGC(2);
        libhv_WebSocketClient* self = (libhv_WebSocketClient*)py_touserdata(argv);
        PY_CHECK_ARG_TYPE(1, tp_str);
        const char* msg = py_tostr(py_arg(1));
        int code = self->ws.send(msg);
        py_newint(py_retval(), code);
        return true;
    });

    py_bindmethod(type, "recv", [](int argc, py_Ref argv) {
        PY_CHECK_ARGC(1);
        libhv_WebSocketClient* self = (libhv_WebSocketClient*)py_touserdata(py_arg(0));

        std::pair<WsMessageType, std::string> mq_msg;
        if(!self->mq.pop(&mq_msg)) {
            py_newnone(py_retval());
            return true;
        } else {
            py_Ref p = py_newtuple(py_retval(), 2);
            switch(mq_msg.first) {
                case WsMessageType::onopen: {
                    py_newstr(py_offset(p, 0), "onopen");
                    py_newnone(py_offset(p, 1));
                    break;
                }
                case WsMessageType::onclose: {
                    py_newstr(py_offset(p, 0), "onclose");
                    py_newnone(py_offset(p, 1));
                    break;
                }
                case WsMessageType::onmessage: {
                    py_newstr(py_offset(p, 0), "onmessage");
                    py_newstrv(py_offset(p, 1), {mq_msg.second.data(), (int)mq_msg.second.size()});
                    break;
                }
            }
            return true;
        }
    });
    return type;
}
