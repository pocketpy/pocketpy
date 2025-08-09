#include "pocketpy/pocketpy.h"

#if PY_SYS_PLATFORM == 0

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>

#elif PY_SYS_PLATFORM == 3 || PY_SYS_PLATFORM == 5

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>

// 保存原始终端设置
static struct termios orig_termios;
static bool orig_termios_set;

// 还原终端设置
static void reset_terminal_mode() { tcsetattr(0, TCSANOW, &orig_termios); }

// 设置终端为非阻塞模式
static void set_conio_terminal_mode_if_needed() {
    if(orig_termios_set) return;
    struct termios new_termios;

    // 获取当前终端设置
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    // 禁用缓冲和回显
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new_termios);

    atexit(reset_terminal_mode);
    orig_termios_set = true;
}

// 检查是否有按键按下
int _kbhit() {
    set_conio_terminal_mode_if_needed();

    struct termios term;
    int oldf;
    int ch;
    int old_flags;

    // 获取终端设置
    tcgetattr(0, &term);
    oldf = term.c_lflag;
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &term);

    // 设置文件描述符为非阻塞
    old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);

    // 检查是否有输入
    ch = getchar();

    // 还原文件描述符设置
    fcntl(STDIN_FILENO, F_SETFL, old_flags);

    // 还原终端设置
    term.c_lflag = oldf;
    tcsetattr(0, TCSANOW, &term);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

// 获取一个字符
int _getch() {
    set_conio_terminal_mode_if_needed();

    int ch;
    struct termios oldt, newt;

    // 获取当前终端设置
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // 禁用缓冲和回显
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // 读取字符
    ch = getchar();

    // 还原终端设置
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}
#endif

#if PK_IS_DESKTOP_PLATFORM && PK_ENABLE_OS
static bool conio_kbhit(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    int ret = _kbhit();
    py_newint(py_retval(), ret);
    return true;
}

static bool conio_getch(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    int ret = _getch();
    py_newint(py_retval(), ret);
    return true;
}

void pk__add_module_conio() {
    py_Ref mod = py_newmodule("conio");
    py_bindfunc(mod, "_kbhit", conio_kbhit);
    py_bindfunc(mod, "_getch", conio_getch);
}

#else

void pk__add_module_conio() {}

#endif
