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


# -----------------------------------------------
#       116:  152:    _vm->bind_builtin_func<3>("pow", [](VM* vm, ArgsView args) {
#         1:  153:        i64 lhs = CAST(i64, args[0]);   // assume lhs>=0
#         1:  154:        i64 rhs = CAST(i64, args[1]);   // assume rhs>=0
#         1:  155:        i64 mod = CAST(i64, args[2]);   // assume mod>0, mod*mod should not overflow
#         -:  156:
#         1:  157:        if(rhs <= 0){
#     #####:  158:            vm->ValueError("pow(): rhs should be positive");
#     #####:  159:        }
#         -:  160:
#        18:  161:        PK_LOCAL_STATIC const auto _mul = [](i64 a, i64 b, i64 c){
#        18:  162:            if(c < 16384) return (a%c) * (b%c) % c;
#         -:  163:            i64 res = 0;
#       365:  164:            while(b > 0){
#       347:  165:                if(b & 1) res = (res + a) % c;
#       347:  166:                a = (a << 1) % c;
#       347:  167:                b >>= 1;
#         -:  168:            }
#         -:  169:            return res;
#        18:  170:        };
#         -:  171:
#         -:  172:        i64 res = 1;
#         1:  173:        lhs %= mod;
#        14:  174:        while(rhs){
#        13:  175:            if(rhs & 1) res = _mul(res, lhs, mod);
#        13:  176:            lhs = _mul(lhs, lhs, mod);
#        13:  177:            rhs >>= 1;
#         -:  178:        }
#         1:  179:        return VAR(res);
#     #####:  180:    });
# test pow:
try:
    pow(1, -1, 2)
    print('未能拦截错误')
    exit(1)
except:
    pass

# -----------------------------------------------
#       114:  188:    _vm->bind_builtin_func<1>("staticmethod", [](VM* vm, ArgsView args) {
#     #####:  189:        return args[0];
#         -:  190:    });
# test staticmethod:
class A():
    def __init__(self):
        self.a = 1
        
    @ staticmethod
    def static_method(txt):
        return txt

assert A.static_method(123) == 123

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


# 无法测试 -----------------------------------------------
#       114:  238:    _vm->bind_builtin_func<-1>("exit", [](VM* vm, ArgsView args) {
#     #####:  239:        if(args.size() == 0) std::exit(0);
#     #####:  240:        else if(args.size() == 1) std::exit(CAST(int, args[0]));
#     #####:  241:        else vm->TypeError("exit() takes at most 1 argument");
#     #####:  242:        return vm->None;
#     #####:  243:    });

# -----------------------------------------------
#       114:  253:    _vm->bind_builtin_func<1>("hash", [](VM* vm, ArgsView args){
#     #####:  254:        i64 value = vm->py_hash(args[0]);
#     #####:  255:        if(((value << 2) >> 2) != value) value >>= 2;
#     #####:  256:        return VAR(value);
#         -:  257:    });
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


# try:
#     hash({1:1})
#     print('未能拦截错误')
#     exit(1)
# except:
#     pass

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




