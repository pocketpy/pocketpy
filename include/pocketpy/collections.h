#pragma once

#include "obj.h"
#include "common.h"
#include "memory.h"
#include "str.h"
#include "iter.h"
#include "cffi.h"

namespace pkpy
{
    struct DequeNode
    {
        PyObject *obj;
        DequeNode *prev;
        DequeNode *next;
        DequeNode()
        {
            this->obj = nullptr; // ignored value, sentinnel
            this->prev = nullptr;
            this->next = nullptr;
        }
        DequeNode(PyObject *obj)
        {
            this->obj = obj;
            this->prev = nullptr;
            this->next = nullptr;
        }

        ~DequeNode()
        {
        }
    };

    struct MyDoublyLinkedList
    {
        int len;
        DequeNode *head;
        DequeNode *tail;

        MyDoublyLinkedList()
        { // creates an empty list
            this->len = 0;
            this->head = new DequeNode(); // dummy
            this->tail = new DequeNode(); // dummy
            this->head->next = this->tail;
            this->tail->prev = this->head;
        }

        ~MyDoublyLinkedList()
        {
            if (this->head == nullptr)
                return;
            this->makeListEmpty();
            delete this->head;
            delete this->tail;
        }

        void push_back(DequeNode *node)
        {
            node->prev = this->tail->prev;
            node->next = this->tail;
            this->tail->prev->next = node;
            this->tail->prev = node;
            this->len++;
        }
        void push_front(DequeNode *node)
        {
            node->prev = this->head;
            node->next = this->head->next;
            this->head->next->prev = node;
            this->head->next = node;
            this->len++;
        }

        DequeNode *pop_back()
        {
            if (this->empty())
                throw std::runtime_error("DoubleLinkedList::pop_back() called on empty list");
            DequeNode *node = this->tail->prev;
            this->tail->prev->prev->next = this->tail;
            this->tail->prev = this->tail->prev->prev;
            this->len--;
            return node;
        }

        DequeNode *pop_front()
        {
            if (this->empty())
                throw std::runtime_error("DoubleLinkedList::pop_front() called on empty list");
            DequeNode *node = this->head->next;
            this->head->next->next->prev = this->head;
            this->head->next = this->head->next->next;
            this->len--;
            return node;
        }

        bool empty() const
        {
            if (this->len == 0)
            {
                if (this->head->next != this->tail || this->tail->prev != this->head)
                {
                    throw std::runtime_error("DoubleLinkedList::size() returned 0 but the list is not empty");
                }
                return true;
            }
            return false;
        }

        int count(VM *vm, PyObject *obj)
        {
            int cnt = 0;
            DequeNode *p = this->head->next;
            while (p != this->tail)
            {
                if (vm->py_equals(p->obj, obj))
                    cnt++;
                p = p->next;
            }
            return cnt;
        }

        int size() const
        {
            return this->len;
        }

        void makeListEmpty()
        {
            while (!this->empty())
            {
                this->pop_back();
            }
        }

        void reverse()
        {
            DequeNode *p = this->head->next;
            while (p != this->tail)
            {
                DequeNode *tmp = p->next;
                p->next = p->prev;
                p->prev = tmp;
                p = tmp;
            }
            DequeNode *tmp = this->head->next;
            this->head->next = this->tail->prev;
            this->tail->prev = tmp;
        }

    };

    // STARTING HERE
    struct PyDeque
    {
        PY_CLASS(PyDeque, mycollections, deque);

        // some fields can be defined here
        MyDoublyLinkedList *dequeItems;

        void appendLeft(PyObject *item);
        void append(PyObject *item);
        PyObject *popLeft();
        PyObject *pop();
        std::stringstream getRepr(VM *vm);
        void reverse();

        int count(VM *vm, PyObject *obj); // vm is needed for the py_equals
        void clear();

        PyDeque() : dequeItems(new MyDoublyLinkedList()) {}
        static void _register(VM *vm, PyObject *mod, PyObject *type);

        void _gc_mark() const; // needed for container types
    };

    void add_module_mycollections(VM *vm);
} // namespace pkpy