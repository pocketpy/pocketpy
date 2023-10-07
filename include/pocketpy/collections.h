#pragma once

#include "obj.h"
#include "common.h"
#include "memory.h"
#include "str.h"
#include "iter.h"
#include "cffi.h"

namespace pkpy
{
    struct DequeNode{
        int obj; //TODO: change to PyObject*
        DequeNode* prev;
        DequeNode* next;
        DequeNode(){
            this->obj = -1; // ignored value
            this->prev = nullptr;
            this->next = nullptr;
        }
        DequeNode(int obj){
            this->obj = obj;
            this->prev = nullptr;
            this->next = nullptr;
        }
        
        ~DequeNode(){
        }

        void printNode(){
            printf("%d ", this->obj);
        }
    };

    struct MyDoublyLinkedList{
        int len;
        DequeNode* head;
        DequeNode* tail;

        MyDoublyLinkedList(){ //creates an empty list
            this->len = 0;
            this->head = new DequeNode(); // dummy
            this->tail = new DequeNode(); // dummy
            this->head->next = this->tail;
            this->tail->prev = this->head;
        }

        ~MyDoublyLinkedList(){
            if(this->head == nullptr) return;
            this->makeListEmpty();
            delete this->head;
            delete this->tail;
        }

        void push_back(DequeNode* node){
            node->prev = this->tail->prev;
            node->next = this->tail;
            this->tail->prev->next = node;
            this->tail->prev = node;
            this->len++;
        }
        void push_front(DequeNode* node){
            node->prev = this->head;
            node->next = this->head->next;
            this->head->next->prev = node;
            this->head->next = node;
            this->len++;
        }

        DequeNode* pop_back(){
            if(this->empty()) throw std::runtime_error("DoubleLinkedList::pop_back() called on empty list");
            DequeNode* node = this->tail->prev;
            this->tail->prev->prev->next = this->tail;
            this->tail->prev = this->tail->prev->prev;
            this->len--;
            return node;
        }

        DequeNode* pop_front(){
            if(this->empty()) throw std::runtime_error("DoubleLinkedList::pop_front() called on empty list");
            DequeNode* node = this->head->next;
            this->head->next->next->prev = this->head;
            this->head->next = this->head->next->next;
            this->len--;
            return node;
        }

        bool empty() const {
            if(this->len == 0){
                if(this->head->next != this->tail || this->tail->prev != this->head){
                    throw std::runtime_error("DoubleLinkedList::size() returned 0 but the list is not empty");
                }
                return true;
            }
            return false;
        }

        int count(int obj){ // TODO: change to PyObject*
            int cnt = 0;
            DequeNode* p = this->head->next;
            while(p != this->tail){
                if(p->obj == obj) cnt++;
                p = p->next;
            }
            return cnt;
        }

        void makeListEmpty(){
            while(!this->empty()){
                this->pop_back();
            }
        }
        
        void printList(){
            if(this->empty()){
                printf("Empty List\n");
                return;
            }
            DequeNode* p = this->head->next;
            while(p != this->tail){
                p->printNode();
                p = p->next;
            }
            printf("\n");
        }

        
    };

    // STARTING HERE
    struct PyDeque
    {
        PY_CLASS(PyDeque, collections, deque);

        // some fields can be defined here
        int len;
        MyDoublyLinkedList *dequeItems;
        

        void appendLeft(int item); // TODO: change to PyObject*
        void append(int item); // TODO: change to PyObject*
        int popLeft(); // TODO: change to PyObject*
        int pop(); // TODO: change to PyObject*
        int count(int obj); // TODO: change to PyObject*
        
        void clear();
        void print(VM *vm);

        PyDeque(): len(0), dequeItems(new MyDoublyLinkedList()){}
        PyDeque(VM *vm) : len(0), dequeItems(new MyDoublyLinkedList()){}
        void printHelloWorld();
        static void _register(VM *vm, PyObject *mod, PyObject *type);

        void _gc_mark() const; // needed for container types
    };

    void add_module_mycollections(VM *vm);
} // namespace pkpy