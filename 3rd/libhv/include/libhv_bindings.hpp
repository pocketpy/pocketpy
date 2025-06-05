#pragma once

#include "pocketpy.h"
#include "http/HttpMessage.h"
#include "base/hplatform.h"

#include "pocketpy/common/name.h"

extern "C" void pk__add_module_libhv();

void libhv_HttpRequest_create(py_OutRef out, HttpRequestPtr ptr);

py_Type libhv_register_HttpRequest(py_GlobalRef mod);
py_Type libhv_register_HttpClient(py_GlobalRef mod);
py_Type libhv_register_HttpServer(py_GlobalRef mod);
py_Type libhv_register_WebSocketClient(py_GlobalRef mod);

#include <deque>
#include <atomic>
#include <thread>

template <typename T>
class libhv_MQ {
private:
    std::atomic<bool> lock;
    std::deque<T> queue;

public:
    void push(T msg) {
        while(lock.exchange(true)) {
            std::this_thread::yield();
        }
        queue.push_back(msg);
        lock.store(false);
    }

    bool pop(T* msg) {
        while(lock.exchange(true)) {
            std::this_thread::yield();
        }
        if(queue.empty()) {
            lock.store(false);
            return false;
        }
        *msg = queue.front();
        queue.pop_front();
        lock.store(false);
        return true;
    }
};

enum class WsMessageType {
    onopen,
    onclose,
    onmessage,
};
