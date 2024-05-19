#pragma once

#include "common.h"
#include "namedict.h"
#include "tuplelist.h"

namespace pkpy {
    
#if PK_ENABLE_STD_FUNCTION
using NativeFuncC = std::function<PyVar(VM*, ArgsView)>;
#else
typedef PyVar (*NativeFuncC)(VM*, ArgsView);
#endif

enum class BindType{
    DEFAULT,
    STATICMETHOD,
    CLASSMETHOD,
};

struct BoundMethod {
    PyVar self;
    PyVar func;
    BoundMethod(PyVar self, PyVar func) : self(self), func(func) {}
    void _gc_mark(VM*) const;
};

struct StaticMethod{
    PyVar func;
    StaticMethod(PyVar func) : func(func) {}
    void _gc_mark(VM*) const;
};

struct ClassMethod{
    PyVar func;
    ClassMethod(PyVar func) : func(func) {}
    void _gc_mark(VM*) const;
};

struct Property{
    PyVar getter;
    PyVar setter;
    Property(PyVar getter, PyVar setter) : getter(getter), setter(setter) {}
    void _gc_mark(VM*) const;
};

struct Range {
    i64 start = 0;
    i64 stop = -1;
    i64 step = 1;
};

struct StarWrapper{
    int level;      // either 1 or 2
    PyVar obj;
    StarWrapper(int level, PyVar obj) : level(level), obj(obj) {}
    void _gc_mark(VM*) const;
};

struct Bytes{
    unsigned char* _data;
    int _size;

    int size() const noexcept { return _size; }
    int operator[](int i) const noexcept { return (int)_data[i]; }
    const unsigned char* data() const noexcept { return _data; }

    bool operator==(const Bytes& rhs) const;
    bool operator!=(const Bytes& rhs) const;

    Str str() const noexcept { return Str((char*)_data, _size); }
    std::string_view sv() const noexcept { return std::string_view((char*)_data, _size); }

    Bytes() : _data(nullptr), _size(0) {}
    Bytes(unsigned char* p, int size): _data(p), _size(size) {}
    Bytes(const Str& str): Bytes(str.sv()) {}
    operator bool() const noexcept { return _data != nullptr; }

    Bytes(std::string_view sv);
    Bytes(const Bytes& rhs);
    Bytes(Bytes&& rhs) noexcept;
    Bytes& operator=(Bytes&& rhs) noexcept;
    Bytes& operator=(const Bytes& rhs) = delete;
    std::pair<unsigned char*, int> detach() noexcept;

    ~Bytes(){ delete[] _data;}
};

struct Super{
    PyVar first;
    Type second;
    Super(PyVar first, Type second) : first(first), second(second) {}
    void _gc_mark(VM*) const;
};

struct Slice {
    PyVar start;
    PyVar stop;
    PyVar step;
    Slice(PyVar start, PyVar stop, PyVar step) : start(start), stop(stop), step(step) {}
    void _gc_mark(VM*) const;
};

struct PyObject final{
    bool gc_marked;     // whether this object is marked
    NameDict* _attr;

    bool is_attr_valid() const noexcept { return _attr != nullptr; }
    void* _value_ptr() noexcept { return 1 + &_attr; }

    NameDict& attr() {
        PK_DEBUG_ASSERT(is_attr_valid())
        return *_attr;
    }

    PyVar attr(StrName name) const {
        PK_DEBUG_ASSERT(is_attr_valid())
        return (*_attr)[name];
    }

    PyObject() : gc_marked(false), _attr(nullptr) {}

    template<typename T, typename ...Args>
    void placement_new(Args&&... args){
        static_assert(std::is_same_v<T, std::decay_t<T>>);
        new(_value_ptr()) T(std::forward<Args>(args)...);

        // backdoor for important builtin types
        if constexpr(std::is_same_v<T, DummyInstance>){
            _enable_instance_dict();
        }else if constexpr(std::is_same_v<T, Type>){
            _enable_instance_dict(PK_TYPE_ATTR_LOAD_FACTOR);
        }else if constexpr(std::is_same_v<T, DummyModule>){
            _enable_instance_dict(PK_TYPE_ATTR_LOAD_FACTOR);
        }
    }

    void _enable_instance_dict() {
        _attr = new(pool128_alloc<NameDict>()) NameDict();
    }
    void _enable_instance_dict(float lf){
        _attr = new(pool128_alloc<NameDict>()) NameDict(lf);
    }
};

template<typename T>
inline constexpr int py_sizeof = sizeof(PyObject) + sizeof(T);

static_assert(sizeof(PyObject) == 16);

const int kTpIntIndex = 3;
const int kTpFloatIndex = 4;

inline bool is_tagged(PyVar p) noexcept { return p.is_sso; }
inline bool is_float(PyVar p) noexcept { return p.type.index == kTpFloatIndex; }
inline bool is_int(PyVar p) noexcept { return p.type.index == kTpIntIndex; }

inline bool is_type(PyVar obj, Type type) {
    PK_DEBUG_ASSERT(obj != nullptr)
    return obj.type == type;
}

template <typename, typename=void> struct has_gc_marker : std::false_type {};
template <typename T> struct has_gc_marker<T, std::void_t<decltype(&T::_gc_mark)>> : std::true_type {};

struct MappingProxy{
    PyVar obj;
    MappingProxy(PyVar obj) : obj(obj) {}
    NameDict& attr() { return obj->attr(); }
    void _gc_mark(VM*) const;
};

StrName _type_name(VM* vm, Type type);
template<typename T> T to_void_p(VM*, PyVar);
PyVar from_void_p(VM*, void*);


template<typename T>
obj_get_t<T> PyVar::obj_get(){
    if constexpr(is_sso_v<T>){
        return as<T>();
    }else{
        PK_DEBUG_ASSERT(!is_sso)
        void* v = ((PyObject*)_1)->_value_ptr();
        return *reinterpret_cast<T*>(v);
    }
}

#define PK_OBJ_GET(T, obj) (obj).obj_get<T>()

#define PK_OBJ_MARK(obj) \
    if(!is_tagged(obj) && !(obj)->gc_marked) {                          \
        vm->__obj_gc_mark(obj);                                         \
    }

#define VAR(x) py_var(vm, x)
#define CAST(T, x) py_cast<T>(vm, x)
#define _CAST(T, x) _py_cast<T>(vm, x)

#define CAST_F(x) py_cast<f64>(vm, x)
#define CAST_DEFAULT(T, x, default_value) (x != vm->None) ? py_cast<T>(vm, x) : (default_value)

/*****************************************************************/
inline bool try_cast_int(PyVar obj, i64* val) noexcept {
    if(is_int(obj)){
        *val = obj.as<i64>();
        return true;
    }
    return false;
}

extern PyVar const PY_NULL;
extern PyVar const PY_OP_CALL;
extern PyVar const PY_OP_YIELD;

}   // namespace pkpy