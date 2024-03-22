#include "pocketpy/memory.h"

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
    static const int __MaxBlocks = 256*1024 / __BlockSize;
    static const int __MinArenaCount = PK_GC_MIN_THRESHOLD*100 / (256*1024);

    struct Block{
        void* arena;
        char data[__BlockSize];
    };

    struct Arena: LinkedListNode{
        Block _blocks[__MaxBlocks];
        Block* _free_list[__MaxBlocks];
        int _free_list_size;
        
        Arena(): _free_list_size(__MaxBlocks) {
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
            }
        }
    }

    void shrink_to_fit(){
        PK_GLOBAL_SCOPE_LOCK();
        if(_arenas.size() < __MinArenaCount) return;
        _arenas.apply([this](Arena* arena){
            if(arena->full()){
                _arenas.erase(arena);
                delete arena;
            }
        });
    }

    std::string info(){
        int n_used_arenas = _arenas.size();
        int n_total_arenas = n_used_arenas + _empty_arenas.size();
        size_t allocated_size = 0;
        size_t total_size = 0;
        _arenas.apply([&](Arena* arena){
            allocated_size += arena->allocated_size();
            total_size += __BlockSize * __MaxBlocks;
        });
        _empty_arenas.apply([&](Arena* arena){
            total_size += __BlockSize * __MaxBlocks;
        });
        char buffer[512];
        snprintf(
            buffer,
            sizeof(buffer),
            "pool%d: %.2f/%.2f MB (%d/%d arenas)",
            __BlockSize,
            (float)allocated_size / (1024*1024),
            (float)total_size / (1024*1024),
            n_used_arenas,
            n_total_arenas
        );
        return buffer;
    }

    ~MemoryPool(){
        _arenas.apply([](Arena* arena){ delete arena; });
        _empty_arenas.apply([](Arena* arena){ delete arena; });
    }
};

static MemoryPool<64> pool64;
static MemoryPool<128> pool128;

void* pool64_alloc(size_t size) noexcept { return pool64.alloc(size); }
void pool64_dealloc(void* p) noexcept { pool64.dealloc(p); }

void* pool128_alloc(size_t size) noexcept { return pool128.alloc(size); }
void pool128_dealloc(void* p) noexcept { pool128.dealloc(p); }

void pools_shrink_to_fit() noexcept {
    pool64.shrink_to_fit();
    pool128.shrink_to_fit();
}

std::string pool64_info() noexcept { return pool64.info(); }
std::string pool128_info() noexcept { return pool128.info(); }

}