#pragma once

#include "common.h"

namespace pkpy{

template<typename T, int __Bucket, int __BucketSize=32, bool __ZeroCheck=true>
struct FreeListA {
    std::vector<T*> buckets[__Bucket+1];

    T* alloc(int n){
        if constexpr(__ZeroCheck) if(n == 0) return nullptr;
        if(n > __Bucket || buckets[n].empty()){
            return new T[n];
        }else{
            T* p = buckets[n].back();
            buckets[n].pop_back();
            return p;
        }
    }

    void dealloc(T* p, int n){
        if constexpr(__ZeroCheck) if(n == 0) return;
        if(n > __Bucket || buckets[n].size() >= __BucketSize){
            delete[] p;
        }else{
            buckets[n].push_back(p);
        }
    }

    ~FreeListA(){
        for(int i=0; i<=__Bucket; i++){
            for(T* p : buckets[i]) delete[] p;
        }
    }
};

};  // namespace pkpy
