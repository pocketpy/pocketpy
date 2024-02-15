#pragma once

#include "common.h"
#include "namedict.h"
#include "tuplelist.h"

namespace pkpy {
    
struct Frame;
class VM;

#if PK_ENABLE_STD_FUNCTION
using NativeFuncC = std::function<PyObject*(VM*, ArgsView)>;
#else
typedef PyObject* (*NativeFuncC)(VM*, ArgsView);
#endif

enum class BindType{
    DEFAULT,
    STATICMETHOD,
    CLASSMETHOD,
};

struct BoundMethod {
    PyObject* self;
    PyObject* func;
    BoundMethod(PyObject* self, PyObject* func) : self(self), func(func) {}
};

struct StaticMethod{
    PyObject* func;
    StaticMethod(PyObject* func) : func(func) {}
};

struct ClassMethod{
    PyObject* func;
    ClassMethod(PyObject* func) : func(func) {}
};

struct Property{
    PyObject* getter;
    PyObject* setter;
    Str signature;
    Property(PyObject* getter, PyObject* setter, Str signature) : getter(getter), setter(setter), signature(signature) {}
};

struct Range {
    i64 start = 0;
    i64 stop = -1;
    i64 step = 1;
};

struct StarWrapper{
    int level;      // either 1 or 2
    PyObject* obj;
    StarWrapper(int level, PyObject* obj) : level(level), obj(obj) {}
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

    Bytes(const std::vector<unsigned char>& v);
    Bytes(std::string_view sv);
    Bytes(const Bytes& rhs);
    Bytes(Bytes&& rhs) noexcept;
    Bytes& operator=(Bytes&& rhs) noexcept;
    Bytes& operator=(const Bytes& rhs) = delete;
    std::pair<unsigned char*, int> detach() noexcept;

    ~Bytes(){ delete[] _data;}
};

using Super = std::pair<PyObject*, Type>;

struct Slice {
    PyObject* start;
    PyObject* stop;
    PyObject* step;
    Slice(PyObject* start, PyObject* stop, PyObject* step) : start(start), stop(stop), step(step) {}
};

struct GCHeader {
    bool enabled;   // whether this object is managed by GC
    bool marked;    // whether this object is marked
    GCHeader() : enabled(true), marked(false) {}
};

struct PyObject{
    GCHeader gc;
    Type type;
    NameDict* _attr;

    bool is_attr_valid() const noexcept { return _attr != nullptr; }
    NameDict& attr() noexcept { return *_attr; }
    PyObject* attr(StrName name) const noexcept { return (*_attr)[name]; }

    // PyObject* operator[](StrName name) const noexcept { return (*_attr)[name]; }

    virtual void _obj_gc_mark() = 0;
    virtual void* _value_ptr() = 0;

    PyObject(Type type) : type(type), _attr(nullptr) {}

    virtual ~PyObject();

    void _enable_instance_dict() {
        _attr = new(pool128_alloc<NameDict>()) NameDict();
    }

    void _enable_instance_dict(float lf){
        _attr = new(pool128_alloc<NameDict>()) NameDict(lf);
    }
};

struct PySignalObject: PyObject {
    PySignalObject() : PyObject(0) {
        gc.enabled = false;
    }
    void _obj_gc_mark() override {}
    void* _value_ptr() override { return nullptr; }
};

inline PyObject* const PY_NULL = new PySignalObject();
inline PyObject* const PY_OP_CALL = new PySignalObject();
inline PyObject* const PY_OP_YIELD = new PySignalObject();

const int kTpIntIndex = 2;
const int kTpFloatIndex = 3;

inline bool is_tagged(PyObject* p) noexcept { return (PK_BITS(p) & 0b11) != 0b00; }
inline bool is_small_int(PyObject* p) noexcept { return (PK_BITS(p) & 0b11) == 0b10; }
inline bool is_heap_int(PyObject* p) noexcept { return !is_tagged(p) && p->type.index == kTpIntIndex; }
inline bool is_float(PyObject* p) noexcept { return (PK_BITS(p) & 1) == 1; }    // 01 or 11
inline bool is_int(PyObject* p) noexcept { return is_small_int(p) || is_heap_int(p); }

inline bool is_type(PyObject* obj, Type type) {
#if PK_DEBUG_EXTRA_CHECK
    if(obj == nullptr) throw std::runtime_error("is_type() called with nullptr");
#endif
    switch(type.index){
        case kTpIntIndex: return is_int(obj);
        case kTpFloatIndex: return is_float(obj);
        default: return !is_tagged(obj) && obj->type == type;
    }
}

inline bool is_non_tagged_type(PyObject* obj, Type type) {
#if PK_DEBUG_EXTRA_CHECK
    if(obj == nullptr) throw std::runtime_error("is_non_tagged_type() called with nullptr");
#endif
    return !is_tagged(obj) && obj->type == type;
}

template <typename, typename=void> struct has_gc_marker : std::false_type {};
template <typename T> struct has_gc_marker<T, std::void_t<decltype(&T::_gc_mark)>> : std::true_type {};

template <typename T>
struct Py_ final: PyObject {
    T _value;
    void _obj_gc_mark() override {
        if constexpr (has_gc_marker<T>::value) {
            _value._gc_mark();
        }
    }

    void* _value_ptr() override { return &_value; }
    
    template <typename... Args>
    Py_(Type type, Args&&... args) : PyObject(type), _value(std::forward<Args>(args)...) { }
};

struct MappingProxy{
    PyObject* obj;
    MappingProxy(PyObject* obj) : obj(obj) {}
    NameDict& attr() noexcept { return obj->attr(); }
};

#define PK_OBJ_GET(T, obj) (((Py_<T>*)(obj))->_value)

#define PK_OBJ_MARK(obj) \
    if(!is_tagged(obj) && !(obj)->gc.marked) {                      \
        (obj)->gc.marked = true;                                    \
        (obj)->_obj_gc_mark();                                      \
        if((obj)->is_attr_valid()) gc_mark_namedict((obj)->attr()); \
    }

inline void gc_mark_namedict(NameDict& t){
    if(t.size() == 0) return;
    t.apply([]([[maybe_unused]]StrName name, PyObject* obj){
        PK_OBJ_MARK(obj);
    });
}

StrName _type_name(VM* vm, Type type);

template <typename, typename=void> struct is_py_class : std::false_type {};
template <typename T> struct is_py_class<T, std::void_t<decltype(T::_type)>> : std::true_type {};

template<typename T> T to_void_p(VM*, PyObject*);

template<typename __T>
__T py_cast(VM* vm, PyObject* obj) {
    using T = std::decay_t<__T>;
    if constexpr(std::is_enum_v<T>){
        return (__T)py_cast<i64>(vm, obj);
    }else if constexpr(std::is_pointer_v<T>){
        return to_void_p<T>(vm, obj);
    }else if constexpr(is_py_class<T>::value){
        T::_check_type(vm, obj);
        return PK_OBJ_GET(T, obj);
    }else {
        return Discarded();
    }
}

template<typename __T>
__T _py_cast([[maybe_unused]]VM* vm, PyObject* obj) {
    using T = std::decay_t<__T>;
    if constexpr(std::is_enum_v<T>){
        return (__T)_py_cast<i64>(vm, obj);
    }else if constexpr(std::is_pointer_v<__T>){
        return to_void_p<__T>(vm, obj);
    }else if constexpr(is_py_class<T>::value){
        return PK_OBJ_GET(T, obj);
    }else {
        return Discarded();
    }
}

#define VAR(x) py_var(vm, x)
#define CAST(T, x) py_cast<T>(vm, x)
#define _CAST(T, x) _py_cast<T>(vm, x)

#define CAST_F(x) py_cast<f64>(vm, x)
#define CAST_DEFAULT(T, x, default_value) (x != vm->None) ? py_cast<T>(vm, x) : (default_value)

/*****************************************************************/
template<>
struct Py_<i64> final: PyObject {
    i64 _value;
    Py_(Type type, i64 val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {}
    void* _value_ptr() override { return &_value; }
};

inline bool try_cast_int(PyObject* obj, i64* val) noexcept {
    if(is_small_int(obj)){
        *val = PK_BITS(obj) >> 2;
        return true;
    }else if(is_heap_int(obj)){
        *val = PK_OBJ_GET(i64, obj);
        return true;
    }else{
        return false;
    }
}

template<>
struct Py_<List> final: PyObject {
    List _value;
    Py_(Type type, List&& val): PyObject(type), _value(std::move(val)) {}
    Py_(Type type, const List& val): PyObject(type), _value(val) {}

    void _obj_gc_mark() override {
        for(PyObject* obj: _value) PK_OBJ_MARK(obj);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<Tuple> final: PyObject {
    Tuple _value;
    Py_(Type type, Tuple&& val): PyObject(type), _value(std::move(val)) {}
    Py_(Type type, const Tuple& val): PyObject(type), _value(val) {}

    void _obj_gc_mark() override {
        for(PyObject* obj: _value) PK_OBJ_MARK(obj);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<MappingProxy> final: PyObject {
    MappingProxy _value;
    Py_(Type type, MappingProxy val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.obj);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<BoundMethod> final: PyObject {
    BoundMethod _value;
    Py_(Type type, BoundMethod val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.self);
        PK_OBJ_MARK(_value.func);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<StarWrapper> final: PyObject {
    StarWrapper _value;
    Py_(Type type, StarWrapper val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.obj);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<StaticMethod> final: PyObject {
    StaticMethod _value;
    Py_(Type type, StaticMethod val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.func);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<ClassMethod> final: PyObject {
    ClassMethod _value;
    Py_(Type type, ClassMethod val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.func);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<Property> final: PyObject {
    Property _value;
    Py_(Type type, Property val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.getter);
        PK_OBJ_MARK(_value.setter);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<Slice> final: PyObject {
    Slice _value;
    Py_(Type type, Slice val): PyObject(type), _value(val) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.start);
        PK_OBJ_MARK(_value.stop);
        PK_OBJ_MARK(_value.step);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<Super> final: PyObject {
    Super _value;
    template<typename... Args>
    Py_(Type type, Args&&... args): PyObject(type), _value(std::forward<Args>(args)...) {}
    void _obj_gc_mark() override {
        PK_OBJ_MARK(_value.first);
    }
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<DummyInstance> final: PyObject {
    Py_(Type type): PyObject(type) {
        _enable_instance_dict();
    }
    void _obj_gc_mark() override {}
    void* _value_ptr() override { return nullptr; }
};

template<>
struct Py_<Type> final: PyObject {
    Type _value;
    Py_(Type type, Type val): PyObject(type), _value(val) {
        _enable_instance_dict(PK_TYPE_ATTR_LOAD_FACTOR);
    }
    void _obj_gc_mark() override {}
    void* _value_ptr() override { return &_value; }
};

template<>
struct Py_<DummyModule> final: PyObject {
    Py_(Type type): PyObject(type) {
        _enable_instance_dict(PK_TYPE_ATTR_LOAD_FACTOR);
    }
    void _obj_gc_mark() override {}
    void* _value_ptr() override { return nullptr; }
};

}   // namespace pkpy