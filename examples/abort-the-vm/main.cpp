// custom config to enable `vm->_ceval_on_step` and handle KeyboardInterrupt
#define PK_USER_CONFIG_H
#include "pocketpy.h"

#include <thread>
#include <atomic>
#include <iostream>
#include <signal.h>

using namespace pkpy;

class MyVM: public VM{
public:
    // use atomic to protect the flag
    std::atomic<bool> _flag = false;

    MyVM(): VM(){
        this->_ceval_on_step = [](VM* _vm, Frame* frame, Bytecode bc){
            MyVM* vm = (MyVM*)_vm;
            if(vm->_flag){
                vm->_flag = false;
                vm->KeyboardInterrupt();
            }
        };
    }
    void KeyboardInterrupt(){
        _error("KeyboardInterrupt", "");
    }
};

MyVM* vm;

void set_the_flag_handler(int _){
    vm->_flag = true;
}

int main(){
    signal(SIGINT, set_the_flag_handler);

    vm = new MyVM();
    REPL* repl = new REPL(vm);
    bool need_more_lines = false;
    while(true){
        std::cout << (need_more_lines ? "... " : ">>> ");
        std::string line;
        std::getline(std::cin, line);

        vm->_flag = false;  // reset the flag before each input

        // here I use linux signal to interrupt the vm
        // you can run this line in a thread for more flexibility
        need_more_lines = repl->input(line);
    }

    delete repl;
    delete vm;
    return 0;
}
