# 无法测试 -----------------------------------------------
#     #####:   41:static dylib_entry_t load_dylib(const char* path){
#     #####:   42:    std::error_code ec;
#     #####:   43:    auto p = std::filesystem::absolute(path, ec);
#     #####:   44:    if(ec) return nullptr;
#     #####:   45:    void* handle = dlopen(p.c_str(), RTLD_LAZY);
#     #####:   46:    if(!handle) return nullptr;
#     #####:   47:    return (dylib_entry_t)dlsym(handle, "pkpy_module__init__");
#     #####:   48:}

# -----------------------------------------------
#       128:  107:    _vm->bind_builtin_func<2>("super", [](VM* vm, ArgsView args) {
#         8:  108:        vm->check_non_tagged_type(args[0], vm->tp_type);
#         8:  109:        Type type = PK_OBJ_GET(Type, args[0]);
#         8:  110:        if(!vm->isinstance(args[1], type)){
#     #####:  111:            Str _0 = obj_type_name(vm, PK_OBJ_GET(Type, vm->_t(args[1])));
#     #####:  112:            Str _1 = obj_type_name(vm, type);
#     #####:  113:            vm->TypeError("super(): " + _0.escape() + " is not an instance of " + _1.escape());
#     #####:  114:        }
#         8:  115:        Type base = vm->_all_types[type].base;
#        16:  116:        return vm->heap.gcnew(vm->tp_super, Super(args[1], base));
#         8:  117:    });
# test super:
class TestSuperBase():
    def __init__(self):
        self.base_attr = 1
        
    def base_method(self):
        return self.base_attr
    
    def error(self):
        raise Expection('未能拦截错误')
    

class TestSuperChild1(TestSuperBase):
    def __init__(self):
        super(TestSuperChild1, self).__init__()
    
    def child_method(self):
        return super(TestSuperChild1, self).base_method()
    
    def error_handling(self):
        try:
            super(TestSuperChild1, self).error()
        except:
            pass

class TestSuperChild2(TestSuperBase):
    pass


test_base = TestSuperBase()
# 测试属性
assert test_base.base_attr == 1
# 测试方法
assert test_base.base_method() == 1

test_child1 = TestSuperChild1()
# 测试继承的属性
assert test_child1.base_attr == 1
# 测试继承的方法
assert test_child1.base_method() == 1
# 测试子类添加的方法
assert test_child1.child_method() == 1
# 测试子类的错误拦截
test_child1.error_handling()

test_child2 = TestSuperChild2()
# 测试继承的属性
assert test_child2.base_attr == 1
# 测试继承的方法
assert test_child2.base_method() == 1


class TestSuperNoBaseMethod(TestSuperBase):
    def __init__(self):
        super(TestSuperNoBaseMethod, self).append(1)

try:
    t = TestSuperNoParent()
    print('未能拦截错误')
    exit(1)
except:
    pass

try:
    t = TestSuperNoBaseMethod()
    print('未能拦截错误')
    exit(1)
except:
    pass

class B():
    pass

class C():
    def method(self):
        super(C, self).method()

class D():
    def method(self):
        super(B, self).method()

try:
    c = C()
    c.method()
    print('未能拦截错误')
    exit(1)
except:
    pass

try:
    d = D()
    d.method()
    print('未能拦截错误')
    exit(1)
except:
    pass


class A():
    def __init__(self):
        self.a = 1
        
    @staticmethod
    def static_method(txt):
        return txt
    
    # @classmethod
    # def class_method(cls, txt):
    #     return cls.__name__ + txt

assert A.static_method(123) == 123
# assert A.class_method(123) == 'A123'

# 无法测试 -----------------------------------------------
#       248:  192:    _vm->bind_builtin_func<1>("__import__", [](VM* vm, ArgsView args) {
#        67:  193:        const Str& name = CAST(Str&, args[0]);
#        67:  194:        auto dot = name.sv().find_last_of(".");
#        67:  195:        if(dot != std::string_view::npos){
#     #####:  196:            auto ext = name.sv().substr(dot);
#     #####:  197:            if(ext == ".so" || ext == ".dll" || ext == ".dylib"){
#     #####:  198:                dylib_entry_t entry = load_dylib(name.c_str());
#     #####:  199:                if(!entry){
#     #####:  200:                    vm->_error("ImportError", "cannot load dynamic library: " + name.escape());
#     #####:  201:                }
#     #####:  202:                vm->_c.s_view.push(ArgsView(vm->s_data.end(), vm->s_data.end()));
#     #####:  203:                const char* name = entry(vm, PK_VERSION);
#     #####:  204:                vm->_c.s_view.pop();
#     #####:  205:                if(name == nullptr){
#     #####:  206:                    vm->_error("ImportError", "module initialization failed: " + Str(name).escape());
#     #####:  207:                }
#     #####:  208:                return vm->_modules[name];
#     #####:  209:            }
#     #####:  210:        }
#        67:  211:        return vm->py_import(name);
#        67:  212:    });

# test hash:
# 测试整数类型的输入
assert hash(0) == 0
assert hash(123) == 123
assert hash(-456) == -456

# 测试字符串类型的输入
assert type(hash("hello")) is int

# 测试浮点数类型的输入
assert type(hash(3.14)) is int 
assert type(hash(-2.71828)) is int

# 测试边界情况
assert type(hash(None)) is int 
assert hash(True) == 1
assert hash(False) == 0

# 测试元组
assert type(hash((4, 5, 6, (1234,1122), 2.3983, 'abcd'))) is int

# 测试自定义类和对象的输入
class A():
    pass

a = A()

assert type(hash(A)) is int
assert type(hash(a)) is int

# 测试函数的输入
def f():
    pass
assert type(hash(a)) is int

# 测试不可哈希对象
try:
    hash({1:1})
    print('未能拦截错误')
    exit(1)
except:
    pass

try:
    hash([1])
    print('未能拦截错误')
    exit(1)
except:
    pass


# -----------------------------------------------
#       114:  259:    _vm->bind_builtin_func<1>("chr", [](VM* vm, ArgsView args) {
#     #####:  260:        i64 i = CAST(i64, args[0]);
#     #####:  261:        if (i < 0 || i > 128) vm->ValueError("chr() arg not in range(128)");
#     #####:  262:        return VAR(std::string(1, (char)i));
#     #####:  263:    });
# test chr
l = []
for i in range(128):
    l.append(f'{i} {chr(i)}')
assert l == ['0 \x00', '1 \x01', '2 \x02', '3 \x03', '4 \x04', '5 \x05', '6 \x06', '7 \x07', '8 \x08', '9 \t', '10 \n', '11 \x0b', '12 \x0c', '13 \r', '14 \x0e', '15 \x0f', '16 \x10', '17 \x11', '18 \x12', '19 \x13', '20 \x14', '21 \x15', '22 \x16', '23 \x17', '24 \x18', '25 \x19', '26 \x1a', '27 \x1b', '28 \x1c', '29 \x1d', '30 \x1e', '31 \x1f', '32  ', '33 !', '34 "', '35 #', '36 $', '37 %', '38 &', "39 '", '40 (', '41 )', '42 *', '43 +', '44 ,', '45 -', '46 .', '47 /', '48 0', '49 1', '50 2', '51 3', '52 4', '53 5', '54 6', '55 7', '56 8', '57 9', '58 :', '59 ;', '60 <', '61 =', '62 >', '63 ?', '64 @', '65 A', '66 B', '67 C', '68 D', '69 E', '70 F', '71 G', '72 H', '73 I', '74 J', '75 K', '76 L', '77 M', '78 N', '79 O', '80 P', '81 Q', '82 R', '83 S', '84 T', '85 U', '86 V', '87 W', '88 X', '89 Y', '90 Z', '91 [', '92 \\', '93 ]', '94 ^', '95 _', '96 `', '97 a', '98 b', '99 c', '100 d', '101 e', '102 f', '103 g', '104 h', '105 i', '106 j', '107 k', '108 l', '109 m', '110 n', '111 o', '112 p', '113 q', '114 r', '115 s', '116 t', '117 u', '118 v', '119 w', '120 x', '121 y', '122 z', '123 {', '124 |', '125 }', '126 ~', '127 \x7f']


assert type(bin(1234)) is str

# 无法测试, 不能覆盖-----------------------------------------------
#       136:  285:    _vm->bind_builtin_func<1>("dir", [](VM* vm, ArgsView args) {
#        10:  286:        std::set<StrName> names;
#        10:  287:        if(!is_tagged(args[0]) && args[0]->is_attr_valid()){
#     #####:  288:            std::vector<StrName> keys = args[0]->attr().keys();
#     #####:  289:            names.insert(keys.begin(), keys.end());
#     #####:  290:        }
#        10:  291:        const NameDict& t_attr = vm->_t(args[0])->attr();
#        10:  292:        std::vector<StrName> keys = t_attr.keys();
#        10:  293:        names.insert(keys.begin(), keys.end());
#        10:  294:        List ret;
#       305:  295:        for (StrName name : names) ret.push_back(VAR(name.sv()));
#        10:  296:        return VAR(std::move(ret));
#        10:  297:    });
# test dir:

# test __repr__:
class A():
    def __init__(self):
        self.attr = 0

repr(A())


# 未完全测试准确性-----------------------------------------------
#     33600:  318:    _vm->bind_constructor<-1>("range", [](VM* vm, ArgsView args) {
#     16742:  319:        args._begin += 1;   // skip cls
#     16742:  320:        Range r;
#     16742:  321:        switch (args.size()) {
#      8735:  322:            case 1: r.stop = CAST(i64, args[0]); break;
#      3867:  323:            case 2: r.start = CAST(i64, args[0]); r.stop = CAST(i64, args[1]); break;
#      4140:  324:            case 3: r.start = CAST(i64, args[0]); r.stop = CAST(i64, args[1]); r.step = CAST(i64, args[2]); break;
#     #####:  325:            default: vm->TypeError("expected 1-3 arguments, got " + std::to_string(args.size()));
#     #####:  326:        }
#     33484:  327:        return VAR(r);
#     16742:  328:    });
#         -:  329:
# test range:

try:
    range(1,2,3,4)
    print('未能拦截错误, 在测试 range')
    exit(1)
except:
    pass

# /************ int ************/
try:
    int('asad')
    print('未能拦截错误, 在测试 int')
    exit(1)
except:
    pass

try:
    int(123, 16)
    print('未能拦截错误, 在测试 int')
    exit(1)
except:
    pass

# 未完全测试准确性-----------------------------------------------
#       116:  392:    _vm->bind_method<0>("int", "bit_length", [](VM* vm, ArgsView args) {
#     #####:  393:        i64 x = _CAST(i64, args[0]);
#     #####:  394:        if(x < 0) x = -x;
#         -:  395:        int bits = 0;
#     #####:  396:        while(x){ x >>= 1; bits++; }
#     #####:  397:        return VAR(bits);
#         -:  398:    });
# test int.bit_length:
assert type(int.bit_length(100)) is int

# 未完全测试准确性-----------------------------------------------
#       116:  400:    _vm->bind__floordiv__(_vm->tp_int, [](VM* vm, PyObject* lhs_, PyObject* rhs_) {
#     #####:  401:        i64 rhs = CAST(i64, rhs_);
#     #####:  402:        return VAR(_CAST(i64, lhs_) / rhs);
#         -:  403:    });
# test int.__floordiv__:
assert type(10//11) is int


# 未完全测试准确性-----------------------------------------------
#       116:  405:    _vm->bind__mod__(_vm->tp_int, [](VM* vm, PyObject* lhs_, PyObject* rhs_) {
#     #####:  406:        i64 rhs = CAST(i64, rhs_);
#     #####:  407:        return VAR(_CAST(i64, lhs_) % rhs);
# test int.__mod__:
assert type(11%2) is int

try:
    float('asad')
    print('未能拦截错误, 在测试 float')
    exit(1)
except:
    pass

try:
    float([])
    print('未能拦截错误, 在测试 float')
    exit(1)
except:
    pass

# /************ str ************/
# test str.__rmul__:
assert type(12 * '12') is str

# 未完全测试准确性-----------------------------------------------
#       116:  554:    _vm->bind_method<1>("str", "index", [](VM* vm, ArgsView args) {
#     #####:  555:        const Str& self = _CAST(Str&, args[0]);
#     #####:  556:        const Str& sub = CAST(Str&, args[1]);
#     #####:  557:        int index = self.index(sub);
#     #####:  558:        if(index == -1) vm->ValueError("substring not found");
#     #####:  559:        return VAR(index);
#     #####:  560:    });
# test str.index:
assert type('25363546'.index('63')) is int
try:
    '25363546'.index('err')
    print('未能拦截错误, 在测试 str.index')
    exit(1)
except:
    pass


# 未完全测试准确性-----------------------------------------------
#       116:  562:    _vm->bind_method<1>("str", "find", [](VM* vm, ArgsView args) {
#     #####:  563:        const Str& self = _CAST(Str&, args[0]);
#     #####:  564:        const Str& sub = CAST(Str&, args[1]);
#     #####:  565:        return VAR(self.index(sub));
#         -:  566:    });
# test str.find:
assert type('25363546'.find('63')) is int
assert type('25363546'.find('err')) is int


# /************ list ************/
# 未完全测试准确性-----------------------------------------------
#       174:  615:    _vm->bind_constructor<-1>("list", [](VM* vm, ArgsView args) {
#        29:  616:        if(args.size() == 1+0) return VAR(List());
#        29:  617:        if(args.size() == 1+1){
#        29:  618:            return vm->py_list(args[1]);
#         -:  619:        }
#     #####:  620:        vm->TypeError("list() takes 0 or 1 arguments");
#     #####:  621:        return vm->None;
#        29:  622:    });
# test list:
try:
    list(1,2)
    print('未能拦截错误, 在测试 list')
    exit(1)
except:
    pass

# 未完全测试准确性----------------------------------------------
#       116:  648:    _vm->bind_method<1>("list", "index", [](VM* vm, ArgsView args) {
#     #####:  649:        List& self = _CAST(List&, args[0]);
#     #####:  650:        PyObject* obj = args[1];
#     #####:  651:        for(int i=0; i<self.size(); i++){
#     #####:  652:            if(vm->py_eq(self[i], obj)) return VAR(i);
#         -:  653:        }
#     #####:  654:        vm->ValueError(_CAST(Str&, vm->py_repr(obj)) + " is not in list");
#     #####:  655:        return vm->None;
#     #####:  656:    });
# test list.index:
assert type([1,2,3,4,5].index(4)) is int
try:
    [1,2,3,4,5].index(6)
    print('未能拦截错误, 在测试 list.index')
    exit(1)
except:
    pass



# 未完全测试准确性----------------------------------------------
#       118:  658:    _vm->bind_method<1>("list", "remove", [](VM* vm, ArgsView args) {
#         1:  659:        List& self = _CAST(List&, args[0]);
#         1:  660:        PyObject* obj = args[1];
#         2:  661:        for(int i=0; i<self.size(); i++){
#         2:  662:            if(vm->py_eq(self[i], obj)){
#         1:  663:                self.erase(i);
#         1:  664:                return vm->None;
#         -:  665:            }
#         -:  666:        }
#     #####:  667:        vm->ValueError(_CAST(Str&, vm->py_repr(obj)) + " is not in list");
#     #####:  668:        return vm->None;
#         1:  669:    });
# test list.remove:
try:
    [1,2,3,4,5].remove(6)
    print('未能拦截错误, 在测试 list.remove')
    exit(1)
except:
    pass


# 未完全测试准确性----------------------------------------------
#      2536:  671:    _vm->bind_method<-1>("list", "pop", [](VM* vm, ArgsView args) {
#      1210:  672:        List& self = _CAST(List&, args[0]);
#      1210:  673:        if(args.size() == 1+0){
#      1208:  674:            if(self.empty()) vm->IndexError("pop from empty list");
#      1208:  675:            return self.popx_back();
#         -:  676:        }
#         2:  677:        if(args.size() == 1+1){
#         2:  678:            int index = CAST(int, args[1]);
#         2:  679:            index = vm->normalized_index(index, self.size());
#         2:  680:            PyObject* ret = self[index];
#         2:  681:            self.erase(index);
#         -:  682:            return ret;
#         -:  683:        }
#     #####:  684:        vm->TypeError("pop() takes at most 1 argument");
#     #####:  685:        return vm->None;
#      1210:  686:    });
# test list.pop:
try:
    [1,2,3,4,5].pop(1,2,3,4)
    print('未能拦截错误, 在测试 list.pop')
    exit(1)
except:
    pass



# 未完全测试准确性-----------------------------------------------
# test list.__rmul__:
assert type(12 * [12]) is list


# /************ tuple ************/
# 未完全测试准确性-----------------------------------------------
#       180:  783:    _vm->bind_constructor<-1>("tuple", [](VM* vm, ArgsView args) {
#        32:  784:        if(args.size() == 1+0) return VAR(Tuple(0));
#        32:  785:        if(args.size() == 1+1){
#        32:  786:            List list = CAST(List, vm->py_list(args[1]));
#        32:  787:            return VAR(Tuple(std::move(list)));
#        32:  788:        }
#     #####:  789:        vm->TypeError("tuple() takes at most 1 argument");
#     #####:  790:        return vm->None;
#        32:  791:    });
#         -:  792:
# test tuple:
try:
    tuple(1,2)
    print('未能拦截错误, 在测试 tuple')
    exit(1)
except:
    pass


# 未完全测试准确性-----------------------------------------------
#       118:  793:    _vm->bind__contains__(_vm->tp_tuple, [](VM* vm, PyObject* obj, PyObject* item) {
#         1:  794:        Tuple& self = _CAST(Tuple&, obj);
#         3:  795:        for(PyObject* i: self) if(vm->py_eq(i, item)) return vm->True;
#     #####:  796:        return vm->False;
#         1:  797:    });
# test tuple.__contains__:
assert (1,2,3).__contains__(5) == False


# 未完全测试准确性-----------------------------------------------
#       116:  799:    _vm->bind_method<1>("tuple", "count", [](VM* vm, ArgsView args) {
#     #####:  800:        Tuple& self = _CAST(Tuple&, args[0]);
#         -:  801:        int count = 0;
#     #####:  802:        for(PyObject* i: self) if(vm->py_eq(i, args[1])) count++;
#     #####:  803:        return VAR(count);
#         -:  804:    });
# test tuple.count:
assert (1,2,2,3,3,3).count(3) == 3
assert (1,2,2,3,3,3).count(0) == 0


# /************ bool ************/
# -----------------------------------------------
#       116:  842:    _vm->bind__repr__(_vm->tp_bool, [](VM* vm, PyObject* self) {
#     #####:  843:        bool val = _CAST(bool, self);
#     #####:  844:        return VAR(val ? "True" : "False");
#         -:  845:    });
# test bool.__repr__:
assert repr(True) == 'True'
assert repr(False) == 'False'


# 未完全测试准确性-----------------------------------------------
#       116:  882:    _vm->bind__and__(_vm->tp_bool, [](VM* vm, PyObject* lhs, PyObject* rhs) {
#     #####:  883:        return VAR(_CAST(bool, lhs) && CAST(bool, rhs));
#         -:  884:    });
# test bool.__and__:
assert True & True == 1

# 未完全测试准确性-----------------------------------------------
#       116:  885:    _vm->bind__or__(_vm->tp_bool, [](VM* vm, PyObject* lhs, PyObject* rhs) {
#     #####:  886:        return VAR(_CAST(bool, lhs) || CAST(bool, rhs));
#         -:  887:    });
# test bool.__or__:
assert True | True == 1

# 未完全测试准确性-----------------------------------------------
#       116:  888:    _vm->bind__xor__(_vm->tp_bool, [](VM* vm, PyObject* lhs, PyObject* rhs) {
#     #####:  889:        return VAR(_CAST(bool, lhs) != CAST(bool, rhs));
#         -:  890:    });
# test bool.__xor__:
assert (True ^ True) == 0

# 未完全测试准确性-----------------------------------------------
#       120:  891:    _vm->bind__eq__(_vm->tp_bool, [](VM* vm, PyObject* lhs, PyObject* rhs) {
#         2:  892:        if(is_non_tagged_type(rhs, vm->tp_bool)) return VAR(lhs == rhs);
#     #####:  893:        if(is_int(rhs)) return VAR(_CAST(bool, lhs) == (bool)CAST(i64, rhs));
#     #####:  894:        return vm->NotImplemented;
#         2:  895:    });
# test bool.__eq__:
assert (True == True) == 1


# /************ bytes ************/
# 未完全测试准确性-----------------------------------------------
#       116:  922:    _vm->bind__hash__(_vm->tp_bytes, [](VM* vm, PyObject* obj) {
#     #####:  923:        const Bytes& self = _CAST(Bytes&, obj);
#     #####:  924:        std::string_view view(self.data(), self.size());
#     #####:  925:        return (i64)std::hash<std::string_view>()(view);
#     #####:  926:    });
# test bytes.__hash__:
assert type(hash(bytes([0x41, 0x42, 0x43]))) is int


# 未完全测试准确性-----------------------------------------------
# test bytes.__repr__:
assert type(repr(bytes([0x41, 0x42, 0x43]))) is str


# /************ slice ************/
# 未完全测试准确性-----------------------------------------------
#       116:  953:    _vm->bind_constructor<4>("slice", [](VM* vm, ArgsView args) {
#     #####:  954:        return VAR(Slice(args[1], args[2], args[3]));
#         -:  955:    });
# test slice:
assert type(slice(0.1, 0.2, 0.3)) is slice


# 未完全测试准确性-----------------------------------------------
#       116: 1529:    bind_property(_t(tp_slice), "start", [](VM* vm, ArgsView args){
#     #####: 1530:        return CAST(Slice&, args[0]).start;
#         -: 1531:    });
#       116: 1532:    bind_property(_t(tp_slice), "stop", [](VM* vm, ArgsView args){
#     #####: 1533:        return CAST(Slice&, args[0]).stop;
#         -: 1534:    });
#       116: 1535:    bind_property(_t(tp_slice), "step", [](VM* vm, ArgsView args){
#     #####: 1536:        return CAST(Slice&, args[0]).step;
#         -: 1537:    });
s = slice(1, 2, 3)
assert type(s) is slice
assert s.start == 1
assert s.stop == 2
assert s.step == 3
assert slice.__dict__['start'].__signature__ == 'start'

# 未完全测试准确性-----------------------------------------------
# test slice.__repr__
assert type(repr(slice(1,1,1))) is str

# /************ mappingproxy ************/
# 未完全测试准确性-----------------------------------------------
#       116:  968:    _vm->bind_method<0>("mappingproxy", "keys", [](VM* vm, ArgsView args) {
#     #####:  969:        MappingProxy& self = _CAST(MappingProxy&, args[0]);
#     #####:  970:        List keys;
#     #####:  971:        for(StrName name : self.attr().keys()) keys.push_back(VAR(name.sv()));
#     #####:  972:        return VAR(std::move(keys));
#     #####:  973:    });
# test mappingproxy.keys:
class A():
    def __init__(self):
        self.a = 10
    def method(self):
        pass


my_mappingproxy = A().__dict__
assert type(my_mappingproxy.keys()) is list


# 未完全测试准确性-----------------------------------------------
#       116:  975:    _vm->bind_method<0>("mappingproxy", "values", [](VM* vm, ArgsView args) {
#     #####:  976:        MappingProxy& self = _CAST(MappingProxy&, args[0]);
#     #####:  977:        List values;
#     #####:  978:        for(auto& item : self.attr().items()) values.push_back(item.second);
#     #####:  979:        return VAR(std::move(values));
#     #####:  980:    });
# test mappingproxy.values:
class A():
    def __init__(self):
        self.a = 10
    def method(self):
        pass


my_mappingproxy = A().__dict__
assert type(my_mappingproxy.values()) is list



# 未完全测试准确性-----------------------------------------------
#       116:  992:    _vm->bind__len__(_vm->tp_mappingproxy, [](VM* vm, PyObject* obj) {
#     #####:  993:        return (i64)_CAST(MappingProxy&, obj).attr().size();
#         -:  994:    });
# test mappingproxy.__len__:
class A():
    def __init__(self):
        self.a = 10
    def method(self):
        pass


my_mappingproxy = A().__dict__
assert type(len(my_mappingproxy)) is int


# 未完全测试准确性-----------------------------------------------
#       116:  996:    _vm->bind__hash__(_vm->tp_mappingproxy, [](VM* vm, PyObject* obj) {
#     #####:  997:        vm->TypeError("unhashable type: 'mappingproxy'");
#     #####:  998:        return (i64)0;
#     #####:  999:    });
# test mappingproxy.__hash__:
class A():
    def __init__(self):
        self.a = 10
    def method(self):
        pass


my_mappingproxy = A().__dict__

try:
    hash(my_mappingproxy)
    print('未能拦截错误, 在测试 mappingproxy.__hash__')
    exit(1)
except TypeError:
    pass

a = hash(object())  # object is hashable
a = hash(A())       # A is hashable
class B:
    def __eq__(self, o): return True

try:
    hash(B())
    print('未能拦截错误, 在测试 B.__hash__')
    exit(1)
except TypeError:
    pass

# 未完全测试准确性-----------------------------------------------
# test mappingproxy.__repr__:
class A():
    def __init__(self):
        self.a = 10
    def method(self):
        pass


my_mappingproxy = A().__dict__
assert type(repr(my_mappingproxy)) is str


# /************ dict ************/
# 未完全测试准确性-----------------------------------------------
#       202: 1033:    _vm->bind_method<-1>("dict", "__init__", [](VM* vm, ArgsView args){
#        43: 1034:        if(args.size() == 1+0) return vm->None;
#        42: 1035:        if(args.size() == 1+1){
#        42: 1036:            auto _lock = vm->heap.gc_scope_lock();
#        42: 1037:            Dict& self = _CAST(Dict&, args[0]);
#        42: 1038:            List& list = CAST(List&, args[1]);
#       165: 1039:            for(PyObject* item : list){
#       123: 1040:                Tuple& t = CAST(Tuple&, item);
#       123: 1041:                if(t.size() != 2){
#     #####: 1042:                    vm->ValueError("dict() takes an iterable of tuples (key, value)");
#     #####: 1043:                    return vm->None;
#         -: 1044:                }
#       123: 1045:                self.set(t[0], t[1]);
#       246: 1046:            }
#        42: 1047:            return vm->None;
#        42: 1048:        }
#     #####: 1049:        vm->TypeError("dict() takes at most 1 argument");
#     #####: 1050:        return vm->None;
#        43: 1051:    });
# test dict:
assert type(dict([(1,2)])) is dict

try:
    dict([(1, 2, 3)])
    print('未能拦截错误, 在测试 dict')
    exit(1)
except:
    pass

try:
    dict([(1, 2)], 1)
    print('未能拦截错误, 在测试 dict')
    exit(1)
except:
    pass

# 未完全测试准确性-----------------------------------------------
#       116: 1057:    _vm->bind__hash__(_vm->tp_dict, [](VM* vm, PyObject* obj) {
#     #####: 1058:        vm->TypeError("unhashable type: 'dict'");
#     #####: 1059:        return (i64)0;
#     #####: 1060:    });
# test dict.__hash__
try:
    hash(dict([(1,2)]))
    print('未能拦截错误, 在测试 dict.__hash__')
    exit(1)
except:
    pass

# 未完全测试准确性-----------------------------------------------
#       116: 1093:    _vm->bind__iter__(_vm->tp_dict, [](VM* vm, PyObject* obj) {
#     #####: 1094:        const Dict& self = _CAST(Dict&, obj);
#     #####: 1095:        return vm->py_iter(VAR(self.keys()));
#     #####: 1096:    });
# test dict.__iter__
for k in {1:2, 2:3, 3:4}:
    assert k in [1,2,3]


# 未完全测试准确性-----------------------------------------------
#       166: 1098:    _vm->bind_method<-1>("dict", "get", [](VM* vm, ArgsView args) {
#        25: 1099:        Dict& self = _CAST(Dict&, args[0]);
#        25: 1100:        if(args.size() == 1+1){
#     #####: 1101:            PyObject* ret = self.try_get(args[1]);
#     #####: 1102:            if(ret != nullptr) return ret;
#     #####: 1103:            return vm->None;
#        25: 1104:        }else if(args.size() == 1+2){
#        25: 1105:            PyObject* ret = self.try_get(args[1]);
#        25: 1106:            if(ret != nullptr) return ret;
#        19: 1107:            return args[2];
#         -: 1108:        }
#     #####: 1109:        vm->TypeError("get() takes at most 2 arguments");
#     #####: 1110:        return vm->None;
#        25: 1111:    });
# test dict.get

assert {1:2, 3:4}.get(1) == 2
assert {1:2, 3:4}.get(2) is None
assert {1:2, 3:4}.get(20, 100) == 100

try:
    {1:2, 3:4}.get(1,1, 1)
    print('未能拦截错误, 在测试 dict.get')
    exit(1)
except:
    pass

# 未完全测试准确性-----------------------------------------------
# test dict.__repr__
assert type(repr({1:2, 3:4})) is str

# /************ property ************/
class A():
    def __init__(self):
        self._name = '123'

    @property
    def value(self):
        return 2

    def get_name(self):
        '''
        doc string 1
        '''
        return self._name

    def set_name(self, val):
        '''
        doc string 2
        '''
        self._name = val

assert A().value == 2
assert A.__dict__['value'].__signature__ == ''

A.name = property(A.get_name, A.set_name, "name: str")
assert A.__dict__['name'].__signature__ == 'name: str'
try:
    property(A.get_name, A.set_name, 1)
    print('未能拦截错误, 在测试 property')
    exit(1)
except:
    pass

class Vector2:
    def __init__(self) -> None:
        self._x = 0

    @property
    def x(self):
        return self._x
    
    @x.setter
    def x(self, val):
        self._x = val

v = Vector2()
assert v.x == 0
v.x = 10
assert v.x == 10

# /************ module timeit ************/
import timeit

def aaa():
    for i in range(10):
        for j in range(10):
            pass
    
assert type(timeit.timeit(aaa, 2)) is float

# 未完全测试准确性-----------------------------------------------
#       116: 1218:    _vm->bind_property(_vm->_t(_vm->tp_function), "__doc__", [](VM* vm, ArgsView args) {
#     #####: 1219:        Function& func = _CAST(Function&, args[0]);
#     #####: 1220:        return VAR(func.decl->docstring);
#         -: 1221:    });
# function.__doc__
def aaa():
    '12345'
    pass
assert type(aaa.__doc__) is str

# 未完全测试准确性-----------------------------------------------
#       116: 1229:    _vm->bind_property(_vm->_t(_vm->tp_function), "__signature__", [](VM* vm, ArgsView args) {
#     #####: 1230:        Function& func = _CAST(Function&, args[0]);
#     #####: 1231:        return VAR(func.decl->signature);
#         -: 1232:    });
# function.__signature__
def aaa():
    pass
assert type(aaa.__signature__) is str


# /************ module time ************/
import time
# 未完全测试准确性-----------------------------------------------
#       116: 1267:    vm->bind_func<1>(mod, "sleep", [](VM* vm, ArgsView args) {
#     #####: 1268:        f64 seconds = CAST_F(args[0]);
#     #####: 1269:        auto begin = std::chrono::system_clock::now();
#     #####: 1270:        while(true){
#     #####: 1271:            auto now = std::chrono::system_clock::now();
#     #####: 1272:            f64 elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - begin).count() / 1000.0;
#     #####: 1273:            if(elapsed >= seconds) break;
#     #####: 1274:        }
#     #####: 1275:        return vm->None;
#     #####: 1276:    });
# test time.time
assert type(time.time()) is float

local_t = time.localtime()
assert type(local_t.tm_year) is int
assert type(local_t.tm_mon) is int
assert type(local_t.tm_mday) is int
assert type(local_t.tm_hour) is int
assert type(local_t.tm_min) is int
assert type(local_t.tm_sec) is int
assert type(local_t.tm_wday) is int
assert type(local_t.tm_yday) is int
assert type(local_t.tm_isdst) is int

# 未完全测试准确性-----------------------------------------------
#       116: 1267:    vm->bind_func<1>(mod, "sleep", [](VM* vm, ArgsView args) {
#     #####: 1268:        f64 seconds = CAST_F(args[0]);
#     #####: 1269:        auto begin = std::chrono::system_clock::now();
#     #####: 1270:        while(true){
#     #####: 1271:            auto now = std::chrono::system_clock::now();
#     #####: 1272:            f64 elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - begin).count() / 1000.0;
#     #####: 1273:            if(elapsed >= seconds) break;
#     #####: 1274:        }
#     #####: 1275:        return vm->None;
#     #####: 1276:    });
# test time.sleep
time.sleep(0.1)

# 未完全测试准确性-----------------------------------------------
#       116: 1278:    vm->bind_func<0>(mod, "localtime", [](VM* vm, ArgsView args) {
#     #####: 1279:        auto now = std::chrono::system_clock::now();
#     #####: 1280:        std::time_t t = std::chrono::system_clock::to_time_t(now);
#     #####: 1281:        std::tm* tm = std::localtime(&t);
#     #####: 1282:        Dict d(vm);
#     #####: 1283:        d.set(VAR("tm_year"), VAR(tm->tm_year + 1900));
#     #####: 1284:        d.set(VAR("tm_mon"), VAR(tm->tm_mon + 1));
#     #####: 1285:        d.set(VAR("tm_mday"), VAR(tm->tm_mday));
#     #####: 1286:        d.set(VAR("tm_hour"), VAR(tm->tm_hour));
#     #####: 1287:        d.set(VAR("tm_min"), VAR(tm->tm_min));
#     #####: 1288:        d.set(VAR("tm_sec"), VAR(tm->tm_sec + 1));
#     #####: 1289:        d.set(VAR("tm_wday"), VAR((tm->tm_wday + 6) % 7));
#     #####: 1290:        d.set(VAR("tm_yday"), VAR(tm->tm_yday + 1));
#     #####: 1291:        d.set(VAR("tm_isdst"), VAR(tm->tm_isdst));
#     #####: 1292:        return VAR(std::move(d));
#     #####: 1293:    });
#        58: 1294:}
# test time.localtime
assert type(time.localtime()) is time.struct_time

# /************ module dis ************/
import dis
#       116: 1487:    vm->bind_func<1>(mod, "dis", [](VM* vm, ArgsView args) {
#     #####: 1488:        CodeObject_ code = get_code(vm, args[0]);
#     #####: 1489:        vm->_stdout(vm, vm->disassemble(code));
#     #####: 1490:        return vm->None;
#     #####: 1491:    });
# test dis.dis
def aaa():
    pass
assert dis.dis(aaa) is None

# test min/max
assert min(1, 2) == 1
assert min(1, 2, 3) == 1
assert min([1, 2]) == 1
assert min([1, 2], key=lambda x: -x) == 2

assert max(1, 2) == 2
assert max(1, 2, 3) == 3
assert max([1, 2]) == 2
assert max([1, 2, 3], key=lambda x: -x) == 1

assert min([
    (1, 2),
    (1, 3),
    (1, 4),
]) == (1, 2)

assert min(1, 2) == 1
assert max(1, 2) == 2


# test callable
assert callable(lambda: 1) is True          # function
assert callable(1) is False                 # int
assert callable(object) is True             # type
assert callable(object()) is False
assert callable([].append) is True      # bound method
assert callable([].__getitem__) is True # bound method

class A:
    def __init__(self):
        pass

    def __call__(self):
        pass

assert callable(A) is True      # type
assert callable(A()) is True    # instance with __call__
assert callable(A.__call__) is True  # bound method
assert callable(A.__init__) is True  # bound method
assert callable(print) is True  # builtin function
assert callable(isinstance) is True  # builtin function


assert id(0) is None
assert id(2**62) is not None

# test issubclass
assert issubclass(int, int) is True
assert issubclass(int, object) is True
assert issubclass(object, int) is False
assert issubclass(object, object) is True
assert issubclass(int, type) is False
assert issubclass(type, type) is True
assert issubclass(float, int) is False
