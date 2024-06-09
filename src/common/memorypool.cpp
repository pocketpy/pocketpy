#include "pocketpy/common/memorypool.hpp"
#include "pocketpy/common/config.h"

#include <cstdlib>
#include <cstring>
#include <cassert>

namespace pkpy {

struct LinkedListNode {
    LinkedListNode* prev;
    LinkedListNode* next;
};

template <typename T>
struct DoubleLinkedList {
    static_assert(std::is_base_of_v<LinkedListNode, T>);
    int _size;
    LinkedListNode head;
    LinkedListNode tail;

    DoubleLinkedList() : _size(0) {
        head.prev = nullptr;
        head.next = &tail;
        tail.prev = &head;
        tail.next = nullptr;
    }

    void push_back(T* node) {
        node->prev = tail.prev;
        node->next = &tail;
        tail.prev->next = node;
        tail.prev = node;
        _size++;
    }

    void push_front(T* node) {
        node->prev = &head;
        node->next = head.next;
        head.next->prev = node;
        head.next = node;
        _size++;
    }

    void pop_back() {
        assert(!empty());
        tail.prev->prev->next = &tail;
        tail.prev = tail.prev->prev;
        _size--;
    }

    void pop_front() {
        assert(!empty());
        head.next->next->prev = &head;
        head.next = head.next->next;
        _size--;
    }

    T* back() const {
        assert(!empty());
        return static_cast<T*>(tail.prev);
    }

    T* front() const {
        assert(!empty());
        return static_cast<T*>(head.next);
    }

    void erase(T* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        _size--;
    }

    bool empty() const { return _size == 0; }

    int size() const { return _size; }

    template <typename Func>
    void apply(Func func) {
        LinkedListNode* p = head.next;
        while(p != &tail) {
            LinkedListNode* next = p->next;
            func(static_cast<T*>(p));
            p = next;
        }
    }
};

struct MemoryPool {
    const static int __BlockSize = kPoolObjectBlockSize;
    const static int __MaxBlocks = 256 * 1024 / __BlockSize;
    const static int __MinArenaCount = PK_GC_MIN_THRESHOLD * 100 / (256 * 1024);

    struct Block {
        void* arena;
        char data[__BlockSize];
    };

    struct Arena : LinkedListNode {
        Block _blocks[__MaxBlocks];
        Block* _free_list[__MaxBlocks];
        int _free_list_size;

        Arena() : _free_list_size(__MaxBlocks) {
            for(int i = 0; i < __MaxBlocks; i++) {
                _blocks[i].arena = this;
                _free_list[i] = &_blocks[i];
            }
        }

        bool empty() const { return _free_list_size == 0; }

        bool full() const { return _free_list_size == __MaxBlocks; }

        int total_bytes() const { return __BlockSize * __MaxBlocks; }
        int used_bytes() const { return __BlockSize * (__MaxBlocks - _free_list_size); }

        Block* alloc() {
            assert(!empty());
            _free_list_size--;
            return _free_list[_free_list_size];
        }

        void dealloc(Block* block) {
            assert(!full());
            _free_list[_free_list_size] = block;
            _free_list_size++;
        }
    };

    MemoryPool() = default;
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator= (const MemoryPool&) = delete;
    MemoryPool(MemoryPool&&) = delete;
    MemoryPool& operator= (MemoryPool&&) = delete;

    DoubleLinkedList<Arena> _arenas;
    DoubleLinkedList<Arena> _empty_arenas;

    void* alloc() {
        PK_GLOBAL_SCOPE_LOCK();
        if(_arenas.empty()) { _arenas.push_back(new Arena()); }
        Arena* arena = _arenas.back();
        void* p = arena->alloc()->data;
        if(arena->empty()) {
            _arenas.pop_back();
            _empty_arenas.push_back(arena);
        }
        return p;
    }

    void dealloc(void* p) {
        PK_GLOBAL_SCOPE_LOCK();
        assert(p != nullptr);
        Block* block = (Block*)((char*)p - sizeof(void*));
        assert(block->arena != nullptr);
        Arena* arena = (Arena*)block->arena;
        if(arena->empty()) {
            _empty_arenas.erase(arena);
            _arenas.push_front(arena);
            arena->dealloc(block);
        } else {
            arena->dealloc(block);
        }
    }

    void shrink_to_fit() {
        PK_GLOBAL_SCOPE_LOCK();
        if(_arenas.size() < __MinArenaCount) return;
        _arenas.apply([this](Arena* arena) {
            if(arena->full()) {
                _arenas.erase(arena);
                delete arena;
            }
        });
    }

    ~MemoryPool() {
        _arenas.apply([](Arena* arena) {
            delete arena;
        });
        _empty_arenas.apply([](Arena* arena) {
            delete arena;
        });
    }
};

template <int BlockSize, int BlockCount>
struct FixedMemoryPool {
    struct Block {
        char data[BlockSize];
    };

    static_assert(BlockSize % 4 == 0);
    static_assert(sizeof(Block) == BlockSize);

    Block _blocks[BlockCount];
    Block* _free_list[BlockCount];
    Block** _free_list_end;
    int _exceeded_bytes;

    int total_bytes() const { return BlockSize * BlockCount; }
    int used_bytes() const { return (BlockCount - (_free_list_end - _free_list)) * BlockSize; }
    int exceeded_bytes() const { return _exceeded_bytes; }

    FixedMemoryPool() {
        _exceeded_bytes = 0;
        _free_list_end = _free_list + BlockCount;
        for(int i = 0; i < BlockCount; ++i) {
            _free_list[i] = _blocks + i;
        }
    }

    bool is_valid(void* p) { return p >= _blocks && p < _blocks + BlockCount; }

    void* alloc() {
        PK_GLOBAL_SCOPE_LOCK()
        if(_free_list_end != _free_list) {
            --_free_list_end;
            return *_free_list_end;
        } else {
            _exceeded_bytes += BlockSize;
            return std::malloc(BlockSize);
        }
    }

    void dealloc(void* p) {
        PK_GLOBAL_SCOPE_LOCK()
        if(is_valid(p)) {
            *_free_list_end = static_cast<Block*>(p);
            ++_free_list_end;
        } else {
            _exceeded_bytes -= BlockSize;
            std::free(p);
        }
    }
};

static FixedMemoryPool<kPoolExprBlockSize, 64> PoolExpr;
static FixedMemoryPool<kPoolFrameBlockSize, 128> PoolFrame;
static MemoryPool PoolObject;

void* PoolExpr_alloc() noexcept { return PoolExpr.alloc(); }

void PoolExpr_dealloc(void* p) noexcept { PoolExpr.dealloc(p); }

void* PoolFrame_alloc() noexcept { return PoolFrame.alloc(); }

void PoolFrame_dealloc(void* p) noexcept { PoolFrame.dealloc(p); }

void* PoolObject_alloc() noexcept { return PoolObject.alloc(); }

void PoolObject_dealloc(void* p) noexcept { PoolObject.dealloc(p); }

void PoolObject_shrink_to_fit() noexcept { PoolObject.shrink_to_fit(); }

void Pools_debug_info(char* buffer, int size) noexcept {
    double BYTES_PER_MB = 1024.0f * 1024.0f;
    double BYTES_PER_KB = 1024.0f;
    int n = 0;
    n = snprintf(
        buffer, size,  "PoolExpr: %.2f KB (used) / %.2f KB (total) - %.2f KB (exceeded)\n",
        PoolExpr.used_bytes() / BYTES_PER_KB,
        PoolExpr.total_bytes() / BYTES_PER_KB,
        PoolExpr.exceeded_bytes() / BYTES_PER_KB
    );
    buffer += n; size -= n;
    n = snprintf(
        buffer, size, "PoolFrame: %.2f KB (used) / %.2f KB (total) - %.2f KB (exceeded)\n",
        PoolFrame.used_bytes() / BYTES_PER_KB,
        PoolFrame.total_bytes() / BYTES_PER_KB,
        PoolFrame.exceeded_bytes() / BYTES_PER_KB
    );
    buffer += n; size -= n;
    // PoolObject
    int empty_arenas = PoolObject._empty_arenas.size();
    int arenas = PoolObject._arenas.size();
    // print empty arenas count
    n = snprintf(
        buffer, size, "PoolObject: %d empty arenas, %d arenas\n",
        empty_arenas, arenas
    );
    buffer += n; size -= n;
    // log each non-empty arena
    PoolObject._arenas.apply([&](MemoryPool::Arena* arena) {
        n = snprintf(
            buffer, size, "  - %p: %.2f MB (used) / %.2f MB (total)\n",
            (void*)arena,
            arena->used_bytes() / BYTES_PER_MB,
            arena->total_bytes() / BYTES_PER_MB
        );
        buffer += n; size -= n;
    });
}

}  // namespace pkpy
