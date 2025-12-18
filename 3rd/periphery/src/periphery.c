#include "pocketpy.h"
#include <string.h>
#include <stdint.h>
#include "periphery.h"

#define ADD_ENUM(name) py_newint(py_emplacedict(mod, py_name(#name)), name)


static bool gpio_config__new__(int argc, py_Ref argv) {
    py_Type cls = py_totype(argv);
    py_newobject(py_retval(), cls, 0, sizeof(gpio_config_t));
    return true;
}
static bool gpio_config__init__(int argc, py_Ref argv) {
    gpio_config_t* self = py_touserdata(argv);
    if(argc == 1) {
        memset(self, 0, sizeof(gpio_config_t));
    } else if(argc == 1 + 8) {
        if(!py_checkint(py_arg(1))) return false;
        self->direction = (gpio_direction_t)py_toint(py_arg(1));
        if(!py_checkint(py_arg(2))) return false;
        self->edge = (gpio_edge_t)py_toint(py_arg(2));
        if(!py_checkint(py_arg(3))) return false;
        self->event_clock = (gpio_event_clock_t)py_toint(py_arg(3));
        if(!py_checkint(py_arg(4))) return false;
        self->debounce_us = py_toint(py_arg(4));
        if(!py_checkint(py_arg(5))) return false;
        self->bias = (gpio_bias_t)py_toint(py_arg(5));
        if(!py_checkint(py_arg(6))) return false;
        self->drive = (gpio_drive_t)py_toint(py_arg(6));
        if(!py_checkbool(py_arg(7))) return false;
        self->inverted = py_tobool(py_arg(7));
        // _7 label is read-only
    } else {
        return TypeError("expected 1 or 9 arguments");
    }
    py_newnone(py_retval());
    return true;
}
static bool gpio_config__get_direction(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_config_t* self = py_touserdata(argv);
    py_newint(py_retval(), (py_i64)self->direction);
    return true;
}
static bool gpio_config__set_direction(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_config_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->direction = (gpio_direction_t)py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool gpio_config__get_edge(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_config_t* self = py_touserdata(argv);
    py_newint(py_retval(), (py_i64)self->edge);
    return true;
}
static bool gpio_config__set_edge(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_config_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->edge = (gpio_edge_t)py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool gpio_config__get_event_clock(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_config_t* self = py_touserdata(argv);
    py_newint(py_retval(), (py_i64)self->event_clock);
    return true;
}
static bool gpio_config__set_event_clock(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_config_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->event_clock = (gpio_event_clock_t)py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool gpio_config__get_debounce_us(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_config_t* self = py_touserdata(argv);
    py_newint(py_retval(), self->debounce_us);
    return true;
}
static bool gpio_config__set_debounce_us(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_config_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->debounce_us = py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool gpio_config__get_bias(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_config_t* self = py_touserdata(argv);
    py_newint(py_retval(), (py_i64)self->bias);
    return true;
}
static bool gpio_config__set_bias(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_config_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->bias = (gpio_bias_t)py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool gpio_config__get_drive(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_config_t* self = py_touserdata(argv);
    py_newint(py_retval(), (py_i64)self->drive);
    return true;
}
static bool gpio_config__set_drive(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_config_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->drive = (gpio_drive_t)py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool gpio_config__get_inverted(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_config_t* self = py_touserdata(argv);
    py_newbool(py_retval(), self->inverted);
    return true;
}
static bool gpio_config__set_inverted(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_config_t* self = py_touserdata(argv);
    if(!py_checkbool(py_arg(1))) return false;
    self->inverted = py_tobool(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool gpio_config__get_label(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_config_t* self = py_touserdata(argv);
    py_newstr(py_retval(), self->label);
    return true;
}
static py_Type register__gpio_config(py_GlobalRef mod) {
    py_Type type = py_newtype("gpio_config", tp_stdc_Memory, mod, NULL);
    py_newint(py_emplacedict(py_tpobject(type), py_name("size")), sizeof(gpio_config_t));
    py_bindmethod(type, "__new__", gpio_config__new__);
    py_bindmethod(type, "__init__", gpio_config__init__);
    py_bindproperty(type, "direction", gpio_config__get_direction, gpio_config__set_direction);
    py_bindproperty(type, "edge", gpio_config__get_edge, gpio_config__set_edge);
    py_bindproperty(type, "event_clock", gpio_config__get_event_clock, gpio_config__set_event_clock);
    py_bindproperty(type, "debounce_us", gpio_config__get_debounce_us, gpio_config__set_debounce_us);
    py_bindproperty(type, "bias", gpio_config__get_bias, gpio_config__set_bias);
    py_bindproperty(type, "drive", gpio_config__get_drive, gpio_config__set_drive);
    py_bindproperty(type, "inverted", gpio_config__get_inverted, gpio_config__set_inverted);
    py_bindproperty(type, "label", gpio_config__get_label, NULL);
    return type;
}
static py_Type tp_user_gpio_config;
static bool spi_msg__new__(int argc, py_Ref argv) {
    py_Type cls = py_totype(argv);
    py_newobject(py_retval(), cls, 0, sizeof(spi_msg_t));
    return true;
}
static bool spi_msg__init__(int argc, py_Ref argv) {
    spi_msg_t* self = py_touserdata(argv);
    if(argc == 1) {
        memset(self, 0, sizeof(spi_msg_t));
    } else if(argc == 1 + 6) {
        // _0 txbuf is read-only
        if(!py_checkint(py_arg(2))) return false;
        self->rxbuf = (uint8_t*)py_toint(py_arg(2));
        if(!py_checkint(py_arg(3))) return false;
        self->len = py_toint(py_arg(3));
        if(!py_checkbool(py_arg(4))) return false;
        self->deselect = py_tobool(py_arg(4));
        if(!py_checkint(py_arg(5))) return false;
        self->deselect_delay_us = py_toint(py_arg(5));
        if(!py_checkint(py_arg(6))) return false;
        self->word_delay_us = py_toint(py_arg(6));
    } else {
        return TypeError("expected 1 or 7 arguments");
    }
    py_newnone(py_retval());
    return true;
}
static bool spi_msg__get_txbuf(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    spi_msg_t* self = py_touserdata(argv);
    py_newint(py_retval(), (py_i64)self->txbuf);
    return true;
}
static bool spi_msg__get_rxbuf(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    spi_msg_t* self = py_touserdata(argv);
    py_newint(py_retval(), (py_i64)self->rxbuf);
    return true;
}
static bool spi_msg__set_rxbuf(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_msg_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->rxbuf = (uint8_t*)py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool spi_msg__get_len(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    spi_msg_t* self = py_touserdata(argv);
    py_newint(py_retval(), self->len);
    return true;
}
static bool spi_msg__set_len(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_msg_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->len = py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool spi_msg__get_deselect(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    spi_msg_t* self = py_touserdata(argv);
    py_newbool(py_retval(), self->deselect);
    return true;
}
static bool spi_msg__set_deselect(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_msg_t* self = py_touserdata(argv);
    if(!py_checkbool(py_arg(1))) return false;
    self->deselect = py_tobool(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool spi_msg__get_deselect_delay_us(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    spi_msg_t* self = py_touserdata(argv);
    py_newint(py_retval(), self->deselect_delay_us);
    return true;
}
static bool spi_msg__set_deselect_delay_us(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_msg_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->deselect_delay_us = py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool spi_msg__get_word_delay_us(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    spi_msg_t* self = py_touserdata(argv);
    py_newint(py_retval(), self->word_delay_us);
    return true;
}
static bool spi_msg__set_word_delay_us(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_msg_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->word_delay_us = py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static py_Type register__spi_msg(py_GlobalRef mod) {
    py_Type type = py_newtype("spi_msg", tp_stdc_Memory, mod, NULL);
    py_newint(py_emplacedict(py_tpobject(type), py_name("size")), sizeof(spi_msg_t));
    py_bindmethod(type, "__new__", spi_msg__new__);
    py_bindmethod(type, "__init__", spi_msg__init__);
    py_bindproperty(type, "txbuf", spi_msg__get_txbuf, NULL);
    py_bindproperty(type, "rxbuf", spi_msg__get_rxbuf, spi_msg__set_rxbuf);
    py_bindproperty(type, "len", spi_msg__get_len, spi_msg__set_len);
    py_bindproperty(type, "deselect", spi_msg__get_deselect, spi_msg__set_deselect);
    py_bindproperty(type, "deselect_delay_us", spi_msg__get_deselect_delay_us, spi_msg__set_deselect_delay_us);
    py_bindproperty(type, "word_delay_us", spi_msg__get_word_delay_us, spi_msg__set_word_delay_us);
    return type;
}
static py_Type tp_user_spi_msg;
static bool periphery_version__new__(int argc, py_Ref argv) {
    py_Type cls = py_totype(argv);
    py_newobject(py_retval(), cls, 0, sizeof(periphery_version_t));
    return true;
}
static bool periphery_version__init__(int argc, py_Ref argv) {
    periphery_version_t* self = py_touserdata(argv);
    if(argc == 1) {
        memset(self, 0, sizeof(periphery_version_t));
    } else if(argc == 1 + 4) {
        if(!py_checkint(py_arg(1))) return false;
        self->major = py_toint(py_arg(1));
        if(!py_checkint(py_arg(2))) return false;
        self->minor = py_toint(py_arg(2));
        if(!py_checkint(py_arg(3))) return false;
        self->patch = py_toint(py_arg(3));
        // _3 commit_id is read-only
    } else {
        return TypeError("expected 1 or 5 arguments");
    }
    py_newnone(py_retval());
    return true;
}
static bool periphery_version__get_major(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    periphery_version_t* self = py_touserdata(argv);
    py_newint(py_retval(), self->major);
    return true;
}
static bool periphery_version__set_major(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    periphery_version_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->major = py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool periphery_version__get_minor(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    periphery_version_t* self = py_touserdata(argv);
    py_newint(py_retval(), self->minor);
    return true;
}
static bool periphery_version__set_minor(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    periphery_version_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->minor = py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool periphery_version__get_patch(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    periphery_version_t* self = py_touserdata(argv);
    py_newint(py_retval(), self->patch);
    return true;
}
static bool periphery_version__set_patch(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    periphery_version_t* self = py_touserdata(argv);
    if(!py_checkint(py_arg(1))) return false;
    self->patch = py_toint(py_arg(1));
    py_newnone(py_retval());
    return true;
}
static bool periphery_version__get_commit_id(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    periphery_version_t* self = py_touserdata(argv);
    py_newstr(py_retval(), self->commit_id);
    return true;
}
static py_Type register__periphery_version(py_GlobalRef mod) {
    py_Type type = py_newtype("periphery_version", tp_stdc_Memory, mod, NULL);
    py_newint(py_emplacedict(py_tpobject(type), py_name("size")), sizeof(periphery_version_t));
    py_bindmethod(type, "__new__", periphery_version__new__);
    py_bindmethod(type, "__init__", periphery_version__init__);
    py_bindproperty(type, "major", periphery_version__get_major, periphery_version__set_major);
    py_bindproperty(type, "minor", periphery_version__get_minor, periphery_version__set_minor);
    py_bindproperty(type, "patch", periphery_version__get_patch, periphery_version__set_patch);
    py_bindproperty(type, "commit_id", periphery_version__get_commit_id, NULL);
    return type;
}
static py_Type tp_user_periphery_version;
/* functions */
static bool cfunc__gpio_new(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    gpio_t* res = gpio_new();
    py_newint(py_retval(), (py_i64)res);
    return true;
}
static bool cfunc__gpio_open(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    const char* _1;
    if(!py_checkstr(py_arg(1))) return false;
    _1 = py_tostr(py_arg(1));
    unsigned _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    gpio_direction_t _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = (gpio_direction_t)py_toint(py_arg(3));
    int res = gpio_open(_0, _1, _2, _3);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_open_name(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    const char* _1;
    if(!py_checkstr(py_arg(1))) return false;
    _1 = py_tostr(py_arg(1));
    const char* _2;
    if(!py_checkstr(py_arg(2))) return false;
    _2 = py_tostr(py_arg(2));
    gpio_direction_t _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = (gpio_direction_t)py_toint(py_arg(3));
    int res = gpio_open_name(_0, _1, _2, _3);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_open_advanced(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    const char* _1;
    if(!py_checkstr(py_arg(1))) return false;
    _1 = py_tostr(py_arg(1));
    unsigned _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    const gpio_config_t* _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = (const gpio_config_t*)py_toint(py_arg(3));
    int res = gpio_open_advanced(_0, _1, _2, _3);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_open_name_advanced(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    const char* _1;
    if(!py_checkstr(py_arg(1))) return false;
    _1 = py_tostr(py_arg(1));
    const char* _2;
    if(!py_checkstr(py_arg(2))) return false;
    _2 = py_tostr(py_arg(2));
    const gpio_config_t* _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = (const gpio_config_t*)py_toint(py_arg(3));
    int res = gpio_open_name_advanced(_0, _1, _2, _3);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_open_sysfs(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    unsigned _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    gpio_direction_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = (gpio_direction_t)py_toint(py_arg(2));
    int res = gpio_open_sysfs(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_read(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    bool* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (bool*)py_toint(py_arg(1));
    int res = gpio_read(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_write(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    bool _1;
    if(!py_checkbool(py_arg(1))) return false;
    _1 = py_tobool(py_arg(1));
    int res = gpio_write(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_poll(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    int _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = gpio_poll(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_close(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    int res = gpio_close(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_free(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    gpio_free(_0);
    py_newnone(py_retval());
    return true;
}
static bool cfunc__gpio_read_event(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    gpio_edge_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (gpio_edge_t*)py_toint(py_arg(1));
    uint64_t* _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = (uint64_t*)py_toint(py_arg(2));
    int res = gpio_read_event(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_poll_multiple(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    gpio_t** _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t**)py_toint(py_arg(0));
    size_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    bool* _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = (bool*)py_toint(py_arg(3));
    int res = gpio_poll_multiple(_0, _1, _2, _3);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_get_direction(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    gpio_direction_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (gpio_direction_t*)py_toint(py_arg(1));
    int res = gpio_get_direction(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_get_edge(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    gpio_edge_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (gpio_edge_t*)py_toint(py_arg(1));
    int res = gpio_get_edge(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_get_event_clock(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    gpio_event_clock_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (gpio_event_clock_t*)py_toint(py_arg(1));
    int res = gpio_get_event_clock(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_get_debounce_us(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    uint32_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (uint32_t*)py_toint(py_arg(1));
    int res = gpio_get_debounce_us(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_get_bias(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    gpio_bias_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (gpio_bias_t*)py_toint(py_arg(1));
    int res = gpio_get_bias(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_get_drive(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    gpio_drive_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (gpio_drive_t*)py_toint(py_arg(1));
    int res = gpio_get_drive(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_get_inverted(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    bool* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (bool*)py_toint(py_arg(1));
    int res = gpio_get_inverted(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_set_direction(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    gpio_direction_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (gpio_direction_t)py_toint(py_arg(1));
    int res = gpio_set_direction(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_set_edge(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    gpio_edge_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (gpio_edge_t)py_toint(py_arg(1));
    int res = gpio_set_edge(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_set_event_clock(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    gpio_event_clock_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (gpio_event_clock_t)py_toint(py_arg(1));
    int res = gpio_set_event_clock(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_set_debounce_us(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    uint32_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = gpio_set_debounce_us(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_set_bias(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    gpio_bias_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (gpio_bias_t)py_toint(py_arg(1));
    int res = gpio_set_bias(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_set_drive(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    gpio_drive_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (gpio_drive_t)py_toint(py_arg(1));
    int res = gpio_set_drive(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_set_inverted(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    bool _1;
    if(!py_checkbool(py_arg(1))) return false;
    _1 = py_tobool(py_arg(1));
    int res = gpio_set_inverted(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_line(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    unsigned res = gpio_line(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_fd(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    int res = gpio_fd(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_name(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    char* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (char*)py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = gpio_name(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_label(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    char* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (char*)py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = gpio_label(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_chip_fd(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    int res = gpio_chip_fd(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_chip_name(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    char* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (char*)py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = gpio_chip_name(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_chip_label(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    char* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (char*)py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = gpio_chip_label(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_tostring(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    char* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (char*)py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = gpio_tostring(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_errno(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    int res = gpio_errno(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__gpio_errmsg(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    gpio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (gpio_t*)py_toint(py_arg(0));
    const char* res = gpio_errmsg(_0);
    py_newstr(py_retval(), res);
    return true;
}
static bool cfunc__mmio_new(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    mmio_t* res = mmio_new();
    py_newint(py_retval(), (py_i64)res);
    return true;
}
static bool cfunc__mmio_open(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = mmio_open(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_open_advanced(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    const char* _3;
    if(!py_checkstr(py_arg(3))) return false;
    _3 = py_tostr(py_arg(3));
    int res = mmio_open_advanced(_0, _1, _2, _3);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_ptr(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    void* res = mmio_ptr(_0);
    py_newint(py_retval(), (py_i64)res);
    return true;
}
static bool cfunc__mmio_read64(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    uint64_t* _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = (uint64_t*)py_toint(py_arg(2));
    int res = mmio_read64(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_read32(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    uint32_t* _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = (uint32_t*)py_toint(py_arg(2));
    int res = mmio_read32(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_read16(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    uint16_t* _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = (uint16_t*)py_toint(py_arg(2));
    int res = mmio_read16(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_read8(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    uint8_t* _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = (uint8_t*)py_toint(py_arg(2));
    int res = mmio_read8(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_read(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    uint8_t* _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = (uint8_t*)py_toint(py_arg(2));
    size_t _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = py_toint(py_arg(3));
    int res = mmio_read(_0, _1, _2, _3);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_write64(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    uint64_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = mmio_write64(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_write32(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    uint32_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = mmio_write32(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_write16(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    uint16_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = mmio_write16(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_write8(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    uint8_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = mmio_write8(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_write(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    const uint8_t* _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = (const uint8_t*)py_toint(py_arg(2));
    size_t _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = py_toint(py_arg(3));
    int res = mmio_write(_0, _1, _2, _3);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_close(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    int res = mmio_close(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_free(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    mmio_free(_0);
    py_newnone(py_retval());
    return true;
}
static bool cfunc__mmio_base(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    uintptr_t res = mmio_base(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_size(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    size_t res = mmio_size(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_tostring(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    char* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (char*)py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = mmio_tostring(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_errno(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    int res = mmio_errno(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__mmio_errmsg(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    mmio_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (mmio_t*)py_toint(py_arg(0));
    const char* res = mmio_errmsg(_0);
    py_newstr(py_retval(), res);
    return true;
}
static bool cfunc__pwm_new(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    pwm_t* res = pwm_new();
    py_newint(py_retval(), (py_i64)res);
    return true;
}
static bool cfunc__pwm_open(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    unsigned _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    unsigned _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = pwm_open(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_enable(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    int res = pwm_enable(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_disable(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    int res = pwm_disable(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_close(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    int res = pwm_close(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_free(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    pwm_free(_0);
    py_newnone(py_retval());
    return true;
}
static bool cfunc__pwm_get_enabled(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    bool* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (bool*)py_toint(py_arg(1));
    int res = pwm_get_enabled(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_get_period_ns(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    uint64_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (uint64_t*)py_toint(py_arg(1));
    int res = pwm_get_period_ns(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_get_duty_cycle_ns(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    uint64_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (uint64_t*)py_toint(py_arg(1));
    int res = pwm_get_duty_cycle_ns(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_get_period(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    double* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (double*)py_toint(py_arg(1));
    int res = pwm_get_period(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_get_duty_cycle(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    double* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (double*)py_toint(py_arg(1));
    int res = pwm_get_duty_cycle(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_get_frequency(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    double* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (double*)py_toint(py_arg(1));
    int res = pwm_get_frequency(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_get_polarity(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    pwm_polarity_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (pwm_polarity_t*)py_toint(py_arg(1));
    int res = pwm_get_polarity(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_set_enabled(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    bool _1;
    if(!py_checkbool(py_arg(1))) return false;
    _1 = py_tobool(py_arg(1));
    int res = pwm_set_enabled(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_set_period_ns(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    uint64_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = pwm_set_period_ns(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_set_duty_cycle_ns(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    uint64_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = pwm_set_duty_cycle_ns(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_set_period(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    double _1;
    if(!py_castfloat(py_arg(1), &_1)) return false;
    int res = pwm_set_period(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_set_duty_cycle(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    double _1;
    if(!py_castfloat(py_arg(1), &_1)) return false;
    int res = pwm_set_duty_cycle(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_set_frequency(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    double _1;
    if(!py_castfloat(py_arg(1), &_1)) return false;
    int res = pwm_set_frequency(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_set_polarity(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    pwm_polarity_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (pwm_polarity_t)py_toint(py_arg(1));
    int res = pwm_set_polarity(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_chip(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    unsigned res = pwm_chip(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_channel(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    unsigned res = pwm_channel(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_tostring(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    char* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (char*)py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = pwm_tostring(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_errno(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    int res = pwm_errno(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__pwm_errmsg(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    pwm_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (pwm_t*)py_toint(py_arg(0));
    const char* res = pwm_errmsg(_0);
    py_newstr(py_retval(), res);
    return true;
}
static bool cfunc__serial_new(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    serial_t* res = serial_new();
    py_newint(py_retval(), (py_i64)res);
    return true;
}
static bool cfunc__serial_open(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    const char* _1;
    if(!py_checkstr(py_arg(1))) return false;
    _1 = py_tostr(py_arg(1));
    uint32_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = serial_open(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_open_advanced(int argc, py_Ref argv) {
    PY_CHECK_ARGC(8);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    const char* _1;
    if(!py_checkstr(py_arg(1))) return false;
    _1 = py_tostr(py_arg(1));
    uint32_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    unsigned _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = py_toint(py_arg(3));
    serial_parity_t _4;
    if(!py_checkint(py_arg(4))) return false;
    _4 = (serial_parity_t)py_toint(py_arg(4));
    unsigned _5;
    if(!py_checkint(py_arg(5))) return false;
    _5 = py_toint(py_arg(5));
    bool _6;
    if(!py_checkbool(py_arg(6))) return false;
    _6 = py_tobool(py_arg(6));
    bool _7;
    if(!py_checkbool(py_arg(7))) return false;
    _7 = py_tobool(py_arg(7));
    int res = serial_open_advanced(_0, _1, _2, _3, _4, _5, _6, _7);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_read(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    uint8_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (uint8_t*)py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = py_toint(py_arg(3));
    int res = serial_read(_0, _1, _2, _3);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_write(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    const uint8_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (const uint8_t*)py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = serial_write(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_flush(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    int res = serial_flush(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_input_waiting(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    unsigned* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (unsigned*)py_toint(py_arg(1));
    int res = serial_input_waiting(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_output_waiting(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    unsigned* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (unsigned*)py_toint(py_arg(1));
    int res = serial_output_waiting(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_poll(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    int _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = serial_poll(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_close(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    int res = serial_close(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_free(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    serial_free(_0);
    py_newnone(py_retval());
    return true;
}
static bool cfunc__serial_get_baudrate(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    uint32_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (uint32_t*)py_toint(py_arg(1));
    int res = serial_get_baudrate(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_get_databits(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    unsigned* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (unsigned*)py_toint(py_arg(1));
    int res = serial_get_databits(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_get_parity(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    serial_parity_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (serial_parity_t*)py_toint(py_arg(1));
    int res = serial_get_parity(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_get_stopbits(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    unsigned* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (unsigned*)py_toint(py_arg(1));
    int res = serial_get_stopbits(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_get_xonxoff(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    bool* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (bool*)py_toint(py_arg(1));
    int res = serial_get_xonxoff(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_get_rtscts(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    bool* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (bool*)py_toint(py_arg(1));
    int res = serial_get_rtscts(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_get_vmin(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    unsigned* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (unsigned*)py_toint(py_arg(1));
    int res = serial_get_vmin(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_get_vtime(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    float* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (float*)py_toint(py_arg(1));
    int res = serial_get_vtime(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_set_baudrate(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    uint32_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = serial_set_baudrate(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_set_databits(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    unsigned _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = serial_set_databits(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_set_parity(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    serial_parity_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (serial_parity_t)py_toint(py_arg(1));
    int res = serial_set_parity(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_set_stopbits(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    unsigned _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = serial_set_stopbits(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_set_xonxoff(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    bool _1;
    if(!py_checkbool(py_arg(1))) return false;
    _1 = py_tobool(py_arg(1));
    int res = serial_set_xonxoff(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_set_rtscts(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    bool _1;
    if(!py_checkbool(py_arg(1))) return false;
    _1 = py_tobool(py_arg(1));
    int res = serial_set_rtscts(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_set_vmin(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    unsigned _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = serial_set_vmin(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_set_vtime(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    float _1;
    if(!py_castfloat32(py_arg(1), &_1)) return false;
    int res = serial_set_vtime(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_fd(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    int res = serial_fd(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_tostring(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    char* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (char*)py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = serial_tostring(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_errno(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    int res = serial_errno(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__serial_errmsg(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    serial_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (serial_t*)py_toint(py_arg(0));
    const char* res = serial_errmsg(_0);
    py_newstr(py_retval(), res);
    return true;
}
static bool cfunc__spi_new(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    spi_t* res = spi_new();
    py_newint(py_retval(), (py_i64)res);
    return true;
}
static bool cfunc__spi_open(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    const char* _1;
    if(!py_checkstr(py_arg(1))) return false;
    _1 = py_tostr(py_arg(1));
    unsigned _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    uint32_t _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = py_toint(py_arg(3));
    int res = spi_open(_0, _1, _2, _3);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_open_advanced(int argc, py_Ref argv) {
    PY_CHECK_ARGC(7);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    const char* _1;
    if(!py_checkstr(py_arg(1))) return false;
    _1 = py_tostr(py_arg(1));
    unsigned _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    uint32_t _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = py_toint(py_arg(3));
    spi_bit_order_t _4;
    if(!py_checkint(py_arg(4))) return false;
    _4 = (spi_bit_order_t)py_toint(py_arg(4));
    uint8_t _5;
    if(!py_checkint(py_arg(5))) return false;
    _5 = py_toint(py_arg(5));
    uint8_t _6;
    if(!py_checkint(py_arg(6))) return false;
    _6 = py_toint(py_arg(6));
    int res = spi_open_advanced(_0, _1, _2, _3, _4, _5, _6);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_open_advanced2(int argc, py_Ref argv) {
    PY_CHECK_ARGC(7);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    const char* _1;
    if(!py_checkstr(py_arg(1))) return false;
    _1 = py_tostr(py_arg(1));
    unsigned _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    uint32_t _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = py_toint(py_arg(3));
    spi_bit_order_t _4;
    if(!py_checkint(py_arg(4))) return false;
    _4 = (spi_bit_order_t)py_toint(py_arg(4));
    uint8_t _5;
    if(!py_checkint(py_arg(5))) return false;
    _5 = py_toint(py_arg(5));
    uint32_t _6;
    if(!py_checkint(py_arg(6))) return false;
    _6 = py_toint(py_arg(6));
    int res = spi_open_advanced2(_0, _1, _2, _3, _4, _5, _6);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_transfer(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    const uint8_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (const uint8_t*)py_toint(py_arg(1));
    uint8_t* _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = (uint8_t*)py_toint(py_arg(2));
    size_t _3;
    if(!py_checkint(py_arg(3))) return false;
    _3 = py_toint(py_arg(3));
    int res = spi_transfer(_0, _1, _2, _3);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_transfer_advanced(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    const spi_msg_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (const spi_msg_t*)py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = spi_transfer_advanced(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_close(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    int res = spi_close(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_free(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    spi_free(_0);
    py_newnone(py_retval());
    return true;
}
static bool cfunc__spi_get_mode(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    unsigned* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (unsigned*)py_toint(py_arg(1));
    int res = spi_get_mode(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_get_max_speed(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    uint32_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (uint32_t*)py_toint(py_arg(1));
    int res = spi_get_max_speed(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_get_bit_order(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    spi_bit_order_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (spi_bit_order_t*)py_toint(py_arg(1));
    int res = spi_get_bit_order(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_get_bits_per_word(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    uint8_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (uint8_t*)py_toint(py_arg(1));
    int res = spi_get_bits_per_word(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_get_extra_flags(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    uint8_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (uint8_t*)py_toint(py_arg(1));
    int res = spi_get_extra_flags(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_get_extra_flags32(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    uint32_t* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (uint32_t*)py_toint(py_arg(1));
    int res = spi_get_extra_flags32(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_set_mode(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    unsigned _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = spi_set_mode(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_set_max_speed(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    uint32_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = spi_set_max_speed(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_set_bit_order(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    spi_bit_order_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (spi_bit_order_t)py_toint(py_arg(1));
    int res = spi_set_bit_order(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_set_bits_per_word(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    uint8_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = spi_set_bits_per_word(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_set_extra_flags(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    uint8_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = spi_set_extra_flags(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_set_extra_flags32(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    uint32_t _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = py_toint(py_arg(1));
    int res = spi_set_extra_flags32(_0, _1);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_fd(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    int res = spi_fd(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_tostring(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    char* _1;
    if(!py_checkint(py_arg(1))) return false;
    _1 = (char*)py_toint(py_arg(1));
    size_t _2;
    if(!py_checkint(py_arg(2))) return false;
    _2 = py_toint(py_arg(2));
    int res = spi_tostring(_0, _1, _2);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_errno(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    int res = spi_errno(_0);
    py_newint(py_retval(), res);
    return true;
}
static bool cfunc__spi_errmsg(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    spi_t* _0;
    if(!py_checkint(py_arg(0))) return false;
    _0 = (spi_t*)py_toint(py_arg(0));
    const char* res = spi_errmsg(_0);
    py_newstr(py_retval(), res);
    return true;
}
static bool cfunc__periphery_version(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    const char* res = periphery_version();
    py_newstr(py_retval(), res);
    return true;
}
static bool cfunc__periphery_version_info(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    const periphery_version_t* res = periphery_version_info();
    py_newint(py_retval(), (py_i64)res);
    return true;
}
void py__add_module_periphery() {
    py_GlobalRef mod = py_newmodule("periphery");
    /* structs */
    tp_user_gpio_config = register__gpio_config(mod);
    tp_user_spi_msg = register__spi_msg(mod);
    tp_user_periphery_version = register__periphery_version(mod);
    /* aliases */
    py_setdict(mod, py_name("gpio_direction_t"), py_tpobject(tp_int));
    py_setdict(mod, py_name("gpio_edge_t"), py_tpobject(tp_int));
    py_setdict(mod, py_name("gpio_event_clock_t"), py_tpobject(tp_int));
    py_setdict(mod, py_name("gpio_bias_t"), py_tpobject(tp_int));
    py_setdict(mod, py_name("gpio_drive_t"), py_tpobject(tp_int));
    py_setdict(mod, py_name("pwm_polarity_t"), py_tpobject(tp_int));
    py_setdict(mod, py_name("serial_parity_t"), py_tpobject(tp_int));
    py_setdict(mod, py_name("spi_bit_order_t"), py_tpobject(tp_int));
    /* functions */
    py_bindfunc(mod, "gpio_new", &cfunc__gpio_new);
    py_bindfunc(mod, "gpio_open", &cfunc__gpio_open);
    py_bindfunc(mod, "gpio_open_name", &cfunc__gpio_open_name);
    py_bindfunc(mod, "gpio_open_advanced", &cfunc__gpio_open_advanced);
    py_bindfunc(mod, "gpio_open_name_advanced", &cfunc__gpio_open_name_advanced);
    py_bindfunc(mod, "gpio_open_sysfs", &cfunc__gpio_open_sysfs);
    py_bindfunc(mod, "gpio_read", &cfunc__gpio_read);
    py_bindfunc(mod, "gpio_write", &cfunc__gpio_write);
    py_bindfunc(mod, "gpio_poll", &cfunc__gpio_poll);
    py_bindfunc(mod, "gpio_close", &cfunc__gpio_close);
    py_bindfunc(mod, "gpio_free", &cfunc__gpio_free);
    py_bindfunc(mod, "gpio_read_event", &cfunc__gpio_read_event);
    py_bindfunc(mod, "gpio_poll_multiple", &cfunc__gpio_poll_multiple);
    py_bindfunc(mod, "gpio_get_direction", &cfunc__gpio_get_direction);
    py_bindfunc(mod, "gpio_get_edge", &cfunc__gpio_get_edge);
    py_bindfunc(mod, "gpio_get_event_clock", &cfunc__gpio_get_event_clock);
    py_bindfunc(mod, "gpio_get_debounce_us", &cfunc__gpio_get_debounce_us);
    py_bindfunc(mod, "gpio_get_bias", &cfunc__gpio_get_bias);
    py_bindfunc(mod, "gpio_get_drive", &cfunc__gpio_get_drive);
    py_bindfunc(mod, "gpio_get_inverted", &cfunc__gpio_get_inverted);
    py_bindfunc(mod, "gpio_set_direction", &cfunc__gpio_set_direction);
    py_bindfunc(mod, "gpio_set_edge", &cfunc__gpio_set_edge);
    py_bindfunc(mod, "gpio_set_event_clock", &cfunc__gpio_set_event_clock);
    py_bindfunc(mod, "gpio_set_debounce_us", &cfunc__gpio_set_debounce_us);
    py_bindfunc(mod, "gpio_set_bias", &cfunc__gpio_set_bias);
    py_bindfunc(mod, "gpio_set_drive", &cfunc__gpio_set_drive);
    py_bindfunc(mod, "gpio_set_inverted", &cfunc__gpio_set_inverted);
    py_bindfunc(mod, "gpio_line", &cfunc__gpio_line);
    py_bindfunc(mod, "gpio_fd", &cfunc__gpio_fd);
    py_bindfunc(mod, "gpio_name", &cfunc__gpio_name);
    py_bindfunc(mod, "gpio_label", &cfunc__gpio_label);
    py_bindfunc(mod, "gpio_chip_fd", &cfunc__gpio_chip_fd);
    py_bindfunc(mod, "gpio_chip_name", &cfunc__gpio_chip_name);
    py_bindfunc(mod, "gpio_chip_label", &cfunc__gpio_chip_label);
    py_bindfunc(mod, "gpio_tostring", &cfunc__gpio_tostring);
    py_bindfunc(mod, "gpio_errno", &cfunc__gpio_errno);
    py_bindfunc(mod, "gpio_errmsg", &cfunc__gpio_errmsg);
    py_bindfunc(mod, "mmio_new", &cfunc__mmio_new);
    py_bindfunc(mod, "mmio_open", &cfunc__mmio_open);
    py_bindfunc(mod, "mmio_open_advanced", &cfunc__mmio_open_advanced);
    py_bindfunc(mod, "mmio_ptr", &cfunc__mmio_ptr);
    py_bindfunc(mod, "mmio_read64", &cfunc__mmio_read64);
    py_bindfunc(mod, "mmio_read32", &cfunc__mmio_read32);
    py_bindfunc(mod, "mmio_read16", &cfunc__mmio_read16);
    py_bindfunc(mod, "mmio_read8", &cfunc__mmio_read8);
    py_bindfunc(mod, "mmio_read", &cfunc__mmio_read);
    py_bindfunc(mod, "mmio_write64", &cfunc__mmio_write64);
    py_bindfunc(mod, "mmio_write32", &cfunc__mmio_write32);
    py_bindfunc(mod, "mmio_write16", &cfunc__mmio_write16);
    py_bindfunc(mod, "mmio_write8", &cfunc__mmio_write8);
    py_bindfunc(mod, "mmio_write", &cfunc__mmio_write);
    py_bindfunc(mod, "mmio_close", &cfunc__mmio_close);
    py_bindfunc(mod, "mmio_free", &cfunc__mmio_free);
    py_bindfunc(mod, "mmio_base", &cfunc__mmio_base);
    py_bindfunc(mod, "mmio_size", &cfunc__mmio_size);
    py_bindfunc(mod, "mmio_tostring", &cfunc__mmio_tostring);
    py_bindfunc(mod, "mmio_errno", &cfunc__mmio_errno);
    py_bindfunc(mod, "mmio_errmsg", &cfunc__mmio_errmsg);
    py_bindfunc(mod, "pwm_new", &cfunc__pwm_new);
    py_bindfunc(mod, "pwm_open", &cfunc__pwm_open);
    py_bindfunc(mod, "pwm_enable", &cfunc__pwm_enable);
    py_bindfunc(mod, "pwm_disable", &cfunc__pwm_disable);
    py_bindfunc(mod, "pwm_close", &cfunc__pwm_close);
    py_bindfunc(mod, "pwm_free", &cfunc__pwm_free);
    py_bindfunc(mod, "pwm_get_enabled", &cfunc__pwm_get_enabled);
    py_bindfunc(mod, "pwm_get_period_ns", &cfunc__pwm_get_period_ns);
    py_bindfunc(mod, "pwm_get_duty_cycle_ns", &cfunc__pwm_get_duty_cycle_ns);
    py_bindfunc(mod, "pwm_get_period", &cfunc__pwm_get_period);
    py_bindfunc(mod, "pwm_get_duty_cycle", &cfunc__pwm_get_duty_cycle);
    py_bindfunc(mod, "pwm_get_frequency", &cfunc__pwm_get_frequency);
    py_bindfunc(mod, "pwm_get_polarity", &cfunc__pwm_get_polarity);
    py_bindfunc(mod, "pwm_set_enabled", &cfunc__pwm_set_enabled);
    py_bindfunc(mod, "pwm_set_period_ns", &cfunc__pwm_set_period_ns);
    py_bindfunc(mod, "pwm_set_duty_cycle_ns", &cfunc__pwm_set_duty_cycle_ns);
    py_bindfunc(mod, "pwm_set_period", &cfunc__pwm_set_period);
    py_bindfunc(mod, "pwm_set_duty_cycle", &cfunc__pwm_set_duty_cycle);
    py_bindfunc(mod, "pwm_set_frequency", &cfunc__pwm_set_frequency);
    py_bindfunc(mod, "pwm_set_polarity", &cfunc__pwm_set_polarity);
    py_bindfunc(mod, "pwm_chip", &cfunc__pwm_chip);
    py_bindfunc(mod, "pwm_channel", &cfunc__pwm_channel);
    py_bindfunc(mod, "pwm_tostring", &cfunc__pwm_tostring);
    py_bindfunc(mod, "pwm_errno", &cfunc__pwm_errno);
    py_bindfunc(mod, "pwm_errmsg", &cfunc__pwm_errmsg);
    py_bindfunc(mod, "serial_new", &cfunc__serial_new);
    py_bindfunc(mod, "serial_open", &cfunc__serial_open);
    py_bindfunc(mod, "serial_open_advanced", &cfunc__serial_open_advanced);
    py_bindfunc(mod, "serial_read", &cfunc__serial_read);
    py_bindfunc(mod, "serial_write", &cfunc__serial_write);
    py_bindfunc(mod, "serial_flush", &cfunc__serial_flush);
    py_bindfunc(mod, "serial_input_waiting", &cfunc__serial_input_waiting);
    py_bindfunc(mod, "serial_output_waiting", &cfunc__serial_output_waiting);
    py_bindfunc(mod, "serial_poll", &cfunc__serial_poll);
    py_bindfunc(mod, "serial_close", &cfunc__serial_close);
    py_bindfunc(mod, "serial_free", &cfunc__serial_free);
    py_bindfunc(mod, "serial_get_baudrate", &cfunc__serial_get_baudrate);
    py_bindfunc(mod, "serial_get_databits", &cfunc__serial_get_databits);
    py_bindfunc(mod, "serial_get_parity", &cfunc__serial_get_parity);
    py_bindfunc(mod, "serial_get_stopbits", &cfunc__serial_get_stopbits);
    py_bindfunc(mod, "serial_get_xonxoff", &cfunc__serial_get_xonxoff);
    py_bindfunc(mod, "serial_get_rtscts", &cfunc__serial_get_rtscts);
    py_bindfunc(mod, "serial_get_vmin", &cfunc__serial_get_vmin);
    py_bindfunc(mod, "serial_get_vtime", &cfunc__serial_get_vtime);
    py_bindfunc(mod, "serial_set_baudrate", &cfunc__serial_set_baudrate);
    py_bindfunc(mod, "serial_set_databits", &cfunc__serial_set_databits);
    py_bindfunc(mod, "serial_set_parity", &cfunc__serial_set_parity);
    py_bindfunc(mod, "serial_set_stopbits", &cfunc__serial_set_stopbits);
    py_bindfunc(mod, "serial_set_xonxoff", &cfunc__serial_set_xonxoff);
    py_bindfunc(mod, "serial_set_rtscts", &cfunc__serial_set_rtscts);
    py_bindfunc(mod, "serial_set_vmin", &cfunc__serial_set_vmin);
    py_bindfunc(mod, "serial_set_vtime", &cfunc__serial_set_vtime);
    py_bindfunc(mod, "serial_fd", &cfunc__serial_fd);
    py_bindfunc(mod, "serial_tostring", &cfunc__serial_tostring);
    py_bindfunc(mod, "serial_errno", &cfunc__serial_errno);
    py_bindfunc(mod, "serial_errmsg", &cfunc__serial_errmsg);
    py_bindfunc(mod, "spi_new", &cfunc__spi_new);
    py_bindfunc(mod, "spi_open", &cfunc__spi_open);
    py_bindfunc(mod, "spi_open_advanced", &cfunc__spi_open_advanced);
    py_bindfunc(mod, "spi_open_advanced2", &cfunc__spi_open_advanced2);
    py_bindfunc(mod, "spi_transfer", &cfunc__spi_transfer);
    py_bindfunc(mod, "spi_transfer_advanced", &cfunc__spi_transfer_advanced);
    py_bindfunc(mod, "spi_close", &cfunc__spi_close);
    py_bindfunc(mod, "spi_free", &cfunc__spi_free);
    py_bindfunc(mod, "spi_get_mode", &cfunc__spi_get_mode);
    py_bindfunc(mod, "spi_get_max_speed", &cfunc__spi_get_max_speed);
    py_bindfunc(mod, "spi_get_bit_order", &cfunc__spi_get_bit_order);
    py_bindfunc(mod, "spi_get_bits_per_word", &cfunc__spi_get_bits_per_word);
    py_bindfunc(mod, "spi_get_extra_flags", &cfunc__spi_get_extra_flags);
    py_bindfunc(mod, "spi_get_extra_flags32", &cfunc__spi_get_extra_flags32);
    py_bindfunc(mod, "spi_set_mode", &cfunc__spi_set_mode);
    py_bindfunc(mod, "spi_set_max_speed", &cfunc__spi_set_max_speed);
    py_bindfunc(mod, "spi_set_bit_order", &cfunc__spi_set_bit_order);
    py_bindfunc(mod, "spi_set_bits_per_word", &cfunc__spi_set_bits_per_word);
    py_bindfunc(mod, "spi_set_extra_flags", &cfunc__spi_set_extra_flags);
    py_bindfunc(mod, "spi_set_extra_flags32", &cfunc__spi_set_extra_flags32);
    py_bindfunc(mod, "spi_fd", &cfunc__spi_fd);
    py_bindfunc(mod, "spi_tostring", &cfunc__spi_tostring);
    py_bindfunc(mod, "spi_errno", &cfunc__spi_errno);
    py_bindfunc(mod, "spi_errmsg", &cfunc__spi_errmsg);
    py_bindfunc(mod, "periphery_version", &cfunc__periphery_version);
    py_bindfunc(mod, "periphery_version_info", &cfunc__periphery_version_info);
    /* enums */
    ADD_ENUM(GPIO_ERROR_ARG);
    ADD_ENUM(GPIO_ERROR_OPEN);
    ADD_ENUM(GPIO_ERROR_NOT_FOUND);
    ADD_ENUM(GPIO_ERROR_QUERY);
    ADD_ENUM(GPIO_ERROR_CONFIGURE);
    ADD_ENUM(GPIO_ERROR_UNSUPPORTED);
    ADD_ENUM(GPIO_ERROR_INVALID_OPERATION);
    ADD_ENUM(GPIO_ERROR_IO);
    ADD_ENUM(GPIO_ERROR_CLOSE);
    ADD_ENUM(GPIO_DIR_IN);
    ADD_ENUM(GPIO_DIR_OUT);
    ADD_ENUM(GPIO_DIR_OUT_LOW);
    ADD_ENUM(GPIO_DIR_OUT_HIGH);
    ADD_ENUM(GPIO_EDGE_NONE);
    ADD_ENUM(GPIO_EDGE_RISING);
    ADD_ENUM(GPIO_EDGE_FALLING);
    ADD_ENUM(GPIO_EDGE_BOTH);
    ADD_ENUM(GPIO_EVENT_CLOCK_REALTIME);
    ADD_ENUM(GPIO_EVENT_CLOCK_MONOTONIC);
    ADD_ENUM(GPIO_EVENT_CLOCK_HTE);
    ADD_ENUM(GPIO_BIAS_DEFAULT);
    ADD_ENUM(GPIO_BIAS_PULL_UP);
    ADD_ENUM(GPIO_BIAS_PULL_DOWN);
    ADD_ENUM(GPIO_BIAS_DISABLE);
    ADD_ENUM(GPIO_DRIVE_DEFAULT);
    ADD_ENUM(GPIO_DRIVE_OPEN_DRAIN);
    ADD_ENUM(GPIO_DRIVE_OPEN_SOURCE);
    ADD_ENUM(MMIO_ERROR_ARG);
    ADD_ENUM(MMIO_ERROR_OPEN);
    ADD_ENUM(MMIO_ERROR_CLOSE);
    ADD_ENUM(PWM_ERROR_ARG);
    ADD_ENUM(PWM_ERROR_OPEN);
    ADD_ENUM(PWM_ERROR_QUERY);
    ADD_ENUM(PWM_ERROR_CONFIGURE);
    ADD_ENUM(PWM_ERROR_CLOSE);
    ADD_ENUM(PWM_POLARITY_NORMAL);
    ADD_ENUM(PWM_POLARITY_INVERSED);
    ADD_ENUM(SERIAL_ERROR_ARG);
    ADD_ENUM(SERIAL_ERROR_OPEN);
    ADD_ENUM(SERIAL_ERROR_QUERY);
    ADD_ENUM(SERIAL_ERROR_CONFIGURE);
    ADD_ENUM(SERIAL_ERROR_IO);
    ADD_ENUM(SERIAL_ERROR_CLOSE);
    ADD_ENUM(PARITY_NONE);
    ADD_ENUM(PARITY_ODD);
    ADD_ENUM(PARITY_EVEN);
    ADD_ENUM(SPI_ERROR_ARG);
    ADD_ENUM(SPI_ERROR_OPEN);
    ADD_ENUM(SPI_ERROR_QUERY);
    ADD_ENUM(SPI_ERROR_CONFIGURE);
    ADD_ENUM(SPI_ERROR_TRANSFER);
    ADD_ENUM(SPI_ERROR_CLOSE);
    ADD_ENUM(SPI_ERROR_UNSUPPORTED);
    ADD_ENUM(MSB_FIRST);
    ADD_ENUM(LSB_FIRST);
}