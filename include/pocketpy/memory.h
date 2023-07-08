#pragma once

#include "common.h"

namespace pkpy{

struct LinkedListNode{
    LinkedListNode* prev;
    LinkedListNode* next;
};

template<typename T>
struct DoubleLinkedList{
    static_assert(std::is_base_of_v<LinkedListNode, T>);
    int _size;
    LinkedListNode head;
    LinkedListNode tail;
    
    DoubleLinkedList(): _size(0){
        head.prev = nullptr;
        head.next = &tail;
        tail.prev = &head;
        tail.next = nullptr;
    }

    void push_back(T* node){
        node->prev = tail.prev;
        node->next = &tail;
        tail.prev->next = node;
        tail.prev = node;
        _size++;
    }

    void push_front(T* node){
        node->prev = &head;
        node->next = head.next;
        head.next->prev = node;
        head.next = node;
        _size++;
    }

    void pop_back(){
#if PK_DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::pop_back() called on empty list");
#endif
        tail.prev->prev->next = &tail;
        tail.prev = tail.prev->prev;
        _size--;
    }

    void pop_front(){
#if PK_DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::pop_front() called on empty list");
#endif
        head.next->next->prev = &head;
        head.next = head.next->next;
        _size--;
    }

    T* back() const {
#if PK_DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::back() called on empty list");
#endif
        return static_cast<T*>(tail.prev);
    }

    T* front() const {
#if PK_DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::front() called on empty list");
#endif
        return static_cast<T*>(head.next);
    }

    void erase(T* node){
#if PK_DEBUG_MEMORY_POOL
        if(empty()) throw std::runtime_error("DoubleLinkedList::erase() called on empty list");
        LinkedListNode* n = head.next;
        while(n != &tail){
            if(n == node) break;
            n = n->next;
        }
        if(n != node) throw std::runtime_error("DoubleLinkedList::erase() called on node not in the list");
#endif
        node->prev->next = node->next;
        node->next->prev = node->prev;
        _size--;
    }

    // void move_all_back(DoubleLinkedList<T>& other){
    //     if(other.empty()) return;
    //     other.tail.prev->next = &tail;
    //     tail.prev->next = other.head.next;
    //     other.head.next->prev = tail.prev;
    //     tail.prev = other.tail.prev;
    //     _size += other._size;
    //     other.head.next = &other.tail;
    //     other.tail.prev = &other.head;
    //     other._size = 0;
    // }

    bool empty() const {
#if PK_DEBUG_MEMORY_POOL
        if(size() == 0){
            if(head.next != &tail || tail.prev != &head){
                throw std::runtime_error("DoubleLinkedList::size() returned 0 but the list is not empty");
            }
            return true;
        }
#endif
        return _size == 0;
    }

    int size() const { return _size; }

    template<typename Func>
    void apply(Func func){
        LinkedListNode* p = head.next;
        while(p != &tail){
            LinkedListNode* next = p->next;
            func(static_cast<T*>(p));
            p = next;
        }
    }
};

template<int __BlockSize=128>
struct MemoryPool{
    static const size_t __MaxBlocks = 256*1024 / __BlockSize;
    struct Block{
        void* arena;
        char data[__BlockSize];
    };

    struct Arena: LinkedListNode{
        Block _blocks[__MaxBlocks];
        Block* _free_list[__MaxBlocks];
        int _free_list_size;
        bool dirty;
        
        Arena(): _free_list_size(__MaxBlocks), dirty(false){
            for(int i=0; i<__MaxBlocks; i++){
                _blocks[i].arena = this;
                _free_list[i] = &_blocks[i];
            }
        }

        bool empty() const { return _free_list_size == 0; }
        bool full() const { return _free_list_size == __MaxBlocks; }

        size_t allocated_size() const{
            return __BlockSize * (__MaxBlocks - _free_list_size);
        }

        Block* alloc(){
#if PK_DEBUG_MEMORY_POOL
            if(empty()) throw std::runtime_error("Arena::alloc() called on empty arena");
#endif
            _free_list_size--;
            return _free_list[_free_list_size];
        }

        void dealloc(Block* block){
#if PK_DEBUG_MEMORY_POOL
            if(full()) throw std::runtime_error("Arena::dealloc() called on full arena");
#endif
            _free_list[_free_list_size] = block;
            _free_list_size++;
        }
    };

    MemoryPool() = default;
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    MemoryPool(MemoryPool&&) = delete;
    MemoryPool& operator=(MemoryPool&&) = delete;

    DoubleLinkedList<Arena> _arenas;
    DoubleLinkedList<Arena> _empty_arenas;

    template<typename __T>
    void* alloc() { return alloc(sizeof(__T)); }

    void* alloc(size_t size){
        PK_GLOBAL_SCOPE_LOCK();
#if PK_DEBUG_NO_MEMORY_POOL
        return malloc(size);
#endif
        if(size > __BlockSize){
            void* p = malloc(sizeof(void*) + size);
            memset(p, 0, sizeof(void*));
            return (char*)p + sizeof(void*);
        }

        if(_arenas.empty()){
            // std::cout << _arenas.size() << ',' << _empty_arenas.size() << ',' << _full_arenas.size() << std::endl;
            _arenas.push_back(new Arena());
        }
        Arena* arena = _arenas.back();
        void* p = arena->alloc()->data;
        if(arena->empty()){
            _arenas.pop_back();
            arena->dirty = true;
            _empty_arenas.push_back(arena);
        }
        return p;
    }

    void dealloc(void* p){
        PK_GLOBAL_SCOPE_LOCK();
#if PK_DEBUG_NO_MEMORY_POOL
        free(p);
        return;
#endif
#if PK_DEBUG_MEMORY_POOL
        if(p == nullptr) throw std::runtime_error("MemoryPool::dealloc() called on nullptr");
#endif
        Block* block = (Block*)((char*)p - sizeof(void*));
        if(block->arena == nullptr){
            free(block);
        }else{
            Arena* arena = (Arena*)block->arena;
            if(arena->empty()){
                _empty_arenas.erase(arena);
                _arenas.push_front(arena);
                arena->dealloc(block);
            }else{
                arena->dealloc(block);
                if(arena->full() && arena->dirty){
                    _arenas.erase(arena);
                    delete arena;
                }
            }
        }
    }

    size_t allocated_size() {
        size_t n = 0;
        _arenas.apply([&n](Arena* arena){ n += arena->allocated_size(); });
        _empty_arenas.apply([&n](Arena* arena){ n += arena->allocated_size(); });
        return n;
    }

    ~MemoryPool(){
        _arenas.apply([](Arena* arena){ delete arena; });
        _empty_arenas.apply([](Arena* arena){ delete arena; });
    }
};

PK_EXPORT inline MemoryPool<64> pool64;
PK_EXPORT inline MemoryPool<128> pool128;

template <typename T>
struct shared_ptr {
    int* counter;

    T* _t() const noexcept { return (T*)(counter + 1); }
    void _inc_counter() { if(counter) ++(*counter); }
    void _dec_counter() { if(counter && --(*counter) == 0) {((T*)(counter + 1))->~T(); pool128.dealloc(counter);} }

public:
    shared_ptr() : counter(nullptr) {}
    shared_ptr(int* counter) : counter(counter) {}
    shared_ptr(const shared_ptr& other) : counter(other.counter) {
        _inc_counter();
    }
    shared_ptr(shared_ptr&& other) noexcept : counter(other.counter) {
        other.counter = nullptr;
    }
    ~shared_ptr() { _dec_counter(); }

    bool operator==(const shared_ptr& other) const { return counter == other.counter; }
    bool operator!=(const shared_ptr& other) const { return counter != other.counter; }
    bool operator<(const shared_ptr& other) const { return counter < other.counter; }
    bool operator>(const shared_ptr& other) const { return counter > other.counter; }
    bool operator<=(const shared_ptr& other) const { return counter <= other.counter; }
    bool operator>=(const shared_ptr& other) const { return counter >= other.counter; }
    bool operator==(std::nullptr_t) const { return counter == nullptr; }
    bool operator!=(std::nullptr_t) const { return counter != nullptr; }

    shared_ptr& operator=(const shared_ptr& other) {
        _dec_counter();
        counter = other.counter;
        _inc_counter();
        return *this;
    }

    shared_ptr& operator=(shared_ptr&& other) noexcept {
        _dec_counter();
        counter = other.counter;
        other.counter = nullptr;
        return *this;
    }

    T& operator*() const { return *_t(); }
    T* operator->() const { return _t(); }
    T* get() const { return _t(); }

    int use_count() const { 
        return counter ? *counter : 0;
    }

    void reset(){
        _dec_counter();
        counter = nullptr;
    }
};

template <typename T, typename... Args>
shared_ptr<T> make_sp(Args&&... args) {
    int* p = (int*)pool128.alloc(sizeof(int) + sizeof(T));
    *p = 1;
    new(p+1) T(std::forward<Args>(args)...);
    return shared_ptr<T>(p);
}

};  // namespace pkpy
