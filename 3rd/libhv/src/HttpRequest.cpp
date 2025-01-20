#include "libhv_bindings.hpp"
#include "HttpMessage.h"

struct libhv_HttpRequest {
    HttpRequestPtr ptr;

    libhv_HttpRequest(HttpRequestPtr ptr) : ptr(ptr) {}
};

void libhv_HttpRequest_create(py_OutRef out, HttpRequestPtr ptr) {
    py_Type type = py_gettype("libhv", py_name("HttpRequest"));
    libhv_HttpRequest* self =
        (libhv_HttpRequest*)py_newobject(out, type, 2, sizeof(libhv_HttpRequest));
    new (self) libhv_HttpRequest(ptr);
}

py_Type libhv_register_HttpRequest(py_GlobalRef mod) {
    py_Type type = py_newtype("HttpRequest", tp_object, mod, [](void* ud) {
        ((libhv_HttpRequest*)ud)->~libhv_HttpRequest();
    });

    py_bindmagic(type, __new__, [](int argc, py_Ref argv) {
        return py_exception(tp_NotImplementedError, "");
    });

    py_bindproperty(
        type,
        "method",
        [](int argc, py_Ref argv) {
            PY_CHECK_ARGC(1);
            libhv_HttpRequest* req = (libhv_HttpRequest*)py_touserdata(argv);
            py_newstr(py_retval(), req->ptr->Method());
            return true;
        },
        NULL);

    py_bindproperty(
        type,
        "url",
        [](int argc, py_Ref argv) {
            PY_CHECK_ARGC(1);
            libhv_HttpRequest* req = (libhv_HttpRequest*)py_touserdata(argv);
            py_newstr(py_retval(), req->ptr->Url().c_str());
            return true;
        },
        NULL);

    py_bindproperty(
        type,
        "path",
        [](int argc, py_Ref argv) {
            PY_CHECK_ARGC(1);
            libhv_HttpRequest* req = (libhv_HttpRequest*)py_touserdata(argv);
            py_newstr(py_retval(), req->ptr->Path().c_str());
            return true;
        },
        NULL);

    // headers (cache in slots[0])
    py_bindproperty(
        type,
        "headers",
        [](int argc, py_Ref argv) {
            PY_CHECK_ARGC(1);
            libhv_HttpRequest* req = (libhv_HttpRequest*)py_touserdata(argv);
            py_Ref headers = py_getslot(argv, 0);
            if(py_isnil(headers)) {
                py_newdict(headers);
                py_Ref _0 = py_pushtmp();
                py_Ref _1 = py_pushtmp();
                for(auto& kv: req->ptr->headers) {
                    py_newstr(_0, kv.first.c_str());  // TODO: tolower
                    py_newstr(_1, kv.second.c_str());
                    py_dict_setitem(headers, _0, _1);
                }
                py_shrink(2);
            }
            py_assign(py_retval(), headers);
            return true;
        },
        NULL);

    // data (cache in slots[1])
    py_bindproperty(
        type,
        "data",
        [](int argc, py_Ref argv) {
            PY_CHECK_ARGC(1);
            libhv_HttpRequest* req = (libhv_HttpRequest*)py_touserdata(argv);
            py_Ref data = py_getslot(argv, 1);

            if(py_isnil(data)) {
                auto content_type = req->ptr->ContentType();
                bool is_text_data = content_type == TEXT_PLAIN ||
                                    content_type == APPLICATION_JSON ||
                                    content_type == APPLICATION_XML || content_type == TEXT_HTML ||
                                    content_type == CONTENT_TYPE_NONE;
                if(is_text_data) {
                    c11_sv sv;
                    sv.data = req->ptr->body.data();
                    sv.size = req->ptr->body.size();
                    py_newstrv(data, sv);
                } else {
                    unsigned char* buf = py_newbytes(data, req->ptr->body.size());
                    memcpy(buf, req->ptr->body.data(), req->ptr->body.size());
                }
            }
            py_assign(py_retval(), data);
            return true;
        },
        NULL);

    return type;
}