#include "pocketpy/obj.h"

namespace pkpy{
    bool Bytes::operator==(const Bytes& rhs) const{
        if(_size != rhs._size) return false;
        for(int i=0; i<_size; i++) if(_data[i] != rhs._data[i]) return false;
        return true;
    }
    bool Bytes::operator!=(const Bytes& rhs) const{ return !(*this == rhs); }

    Bytes::Bytes(std::string_view sv){
        _data = new unsigned char[sv.size()];
        _size = sv.size();
        for(int i=0; i<_size; i++) _data[i] = sv[i];
    }

    // copy constructor
    Bytes::Bytes(const Bytes& rhs){
        _data = new unsigned char[rhs._size];
        _size = rhs._size;
        for(int i=0; i<_size; i++) _data[i] = rhs._data[i];
    }

    // move constructor
    Bytes::Bytes(Bytes&& rhs) noexcept {
        _data = rhs._data;
        _size = rhs._size;
        rhs._data = nullptr;
        rhs._size = 0;
    }

    // move assignment
    Bytes& Bytes::operator=(Bytes&& rhs) noexcept {
        delete[] _data;
        _data = rhs._data;
        _size = rhs._size;
        rhs._data = nullptr;
        rhs._size = 0;
        return *this;
    }

    std::pair<unsigned char*, int> Bytes::detach() noexcept {
        unsigned char* p = _data;
        int size = _size;
        _data = nullptr;
        _size = 0;
        return {p, size};
    }

}   // namespace pkpy