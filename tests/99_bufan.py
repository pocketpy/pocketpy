# ------------------------------------------------
'''source code in cffi.cpp :
       57:  245:        vm->bind_method<0>(type, "__repr__", [](VM* vm, ArgsView args){
    #####:  246:            C99ReflType& self = _CAST(C99ReflType&, args[0]);
    #####:  247:            return VAR("<ctype '" + Str(self.name) + "'>");
    #####:  248:        });
    #####:  249:
       57:  250:        vm->bind_method<0>(type, "name", [](VM* vm, ArgsView args){
    #####:  251:            C99ReflType& self = _CAST(C99ReflType&, args[0]);
    #####:  252:            return VAR(self.name);
    #####:  253:        });
'''
# test :
import c

c_int = c.refl("int")
assert c_int.name() == "int"
assert c_int.__repr__() == '<ctype \'int\'>'
# ------------------------------------------------
'''source code in cffi.cpp :
       57:  180:        vm->bind__eq__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
    #####:  181:            C99Struct& self = _CAST(C99Struct&, lhs);
    #####:  182:            if(!is_non_tagged_type(rhs, C99Struct::_type(vm))) return vm->NotImplemented;
    #####:  183:            C99Struct& other = _CAST(C99Struct&, rhs);
    #####:  184:            bool ok = self.size == other.size && memcmp(self.p, other.p, self.size) == 0;
    #####:  185:            return VAR(ok);
    #####:  186:        });
        -:  187:
'''
# test :
import c

c_int_1 = c.refl("int")
c_struct_1 = c_int_1()
assert (c_int_1() == c_int_1()) == False
assert (c_struct_1 == c_struct_1) == True

# ------------------------------------------------
'''source code in cffi.cpp :
      114:    8:        vm->bind_func<1>(type, "from_hex", [](VM* vm, ArgsView args){
    #####:    9:            std::string s = CAST(Str&, args[0]).str();
    #####:   10:            size_t size;
    #####:   11:            intptr_t ptr = std::stoll(s, &size, 16);
    #####:   12:            if(size != s.size()) vm->ValueError("invalid literal for void_p(): " + s);
    #####:   13:            return VAR_T(VoidP, (void*)ptr);
    #####:   14:        });
      114:   15:        vm->bind_method<0>(type, "hex", [](VM* vm, ArgsView args){
    #####:   16:            VoidP& self = _CAST(VoidP&, args[0]);
    #####:   17:            return VAR(self.hex());
    #####:   18:        });
        -:   19:
'''
# test :
import c

assert c.void_p.from_hex('0x2568b60').hex() == '0x2568b60'

# ------------------------------------------------
'''source code in cffi.cpp :
      114:   70:        vm->bind__add__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
    #####:   71:            VoidP& self = _CAST(VoidP&, lhs);
    #####:   72:            i64 offset = CAST(i64, rhs);
    #####:   73:            return VAR_T(VoidP, (char*)self.ptr + offset);
        -:   74:        });
        -:   75:
      114:   76:        vm->bind__sub__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
    #####:   77:            VoidP& self = _CAST(VoidP&, lhs);
    #####:   78:            i64 offset = CAST(i64, rhs);
    #####:   79:            return VAR_T(VoidP, (char*)self.ptr - offset);
        -:   80:        });
        -:   81:
        -:   19:
'''

# test :
class HexAddress:
    def __init__(self, address):
        if not address.startswith("0x"):  # 确保地址以0x开头
            raise ValueError("Address should start with '0x'.")
        self.address = address[2:]  # 去除0x前缀，并保存十六进制字符串

    def __str__(self):
        return "0x" + self.address  

    def __add__(self, other):
        if isinstance(other, int):
            return HexAddress(hex(int(self.address, 16) + other))  # 将字符串地址转为整数进行运算
        elif isinstance(other, HexAddress):
            return HexAddress(hex(int(self.address, 16) + int(other.address, 16)))  # 将字符串地址转为整数进行运算
        else:
            raise TypeError("Unsupported operand type for +: HexAddress and {}".format(type(other)))

    def __sub__(self, other):
        if isinstance(other, int):
            return HexAddress(hex(int(self.address, 16) - other))  # 将字符串地址转为整数进行运算
        elif isinstance(other, HexAddress):
            return HexAddress(hex(int(self.address, 16) - int(other.address, 16)))  # 将字符串地址转为整数进行运算
        else:
            raise TypeError("Unsupported operand type for -: HexAddress and {}".format(type(other)))


import c

c_void_1 = c.malloc(8)

assert (c_void_1 + 8).hex() == c.void_p.from_hex(str(HexAddress(c_void_1.hex()) + 8)).hex()
assert (c_void_1 - 8).hex() == c.void_p.from_hex(str(HexAddress(c_void_1.hex()) - 8)).hex()

# ------------------------------------------------
'''source code in cffi.cpp :
        -:  107:
      114:  108:        vm->bind_method<1>(type, "read_bytes", [](VM* vm, ArgsView args){
    #####:  109:            VoidP& self = _CAST(VoidP&, args[0]);
    #####:  110:            i64 size = CAST(i64, args[1]);
    #####:  111:            std::vector<char> buffer(size);
    #####:  112:            memcpy(buffer.data(), self.ptr, size);
    #####:  113:            return VAR(Bytes(std::move(buffer)));
    #####:  114:        });
        -:  115:
      114:  116:        vm->bind_method<1>(type, "write_bytes", [](VM* vm, ArgsView args){
    #####:  117:            VoidP& self = _CAST(VoidP&, args[0]);
    #####:  118:            Bytes& bytes = CAST(Bytes&, args[1]);
    #####:  119:            memcpy(self.ptr, bytes.data(), bytes.size());
    #####:  120:            return vm->None;
        -:  121:        });
        -:  122:
'''

# test :
import c
# 此处测试并不完全
c_void_1 = c.malloc(8)
c_void_1.read_bytes(5)
c_void_1.write_bytes(c_void_1.read_bytes(5))



# ------------------------------------------------
'''source code in cffi.cpp :
       57:  126:    void C99Struct::_register(VM* vm, PyObject* mod, PyObject* type){
      114:  127:        vm->bind_constructor<-1>(type, [](VM* vm, ArgsView args){
    #####:  128:            if(args.size() == 1+1){
    #####:  129:                if(is_int(args[1])){
    #####:  130:                    int size = _CAST(int, args[1]);
    #####:  131:                    return VAR_T(C99Struct, size);
    #####:  132:                }
    #####:  133:                if(is_non_tagged_type(args[1], vm->tp_str)){
    #####:  134:                    const Str& s = _CAST(Str&, args[1]);
    #####:  135:                    return VAR_T(C99Struct, (void*)s.data, s.size);
    #####:  136:                }
    #####:  137:                if(is_non_tagged_type(args[1], vm->tp_bytes)){
    #####:  138:                    const Bytes& b = _CAST(Bytes&, args[1]);
    #####:  139:                    return VAR_T(C99Struct, (void*)b.data(), b.size());
    #####:  140:                }
    #####:  141:                vm->TypeError("expected int, str or bytes");
    #####:  142:                return vm->None;
        -:  143:            }
    #####:  144:            if(args.size() == 1+2){
    #####:  145:                void* p = CAST(void*, args[1]);
    #####:  146:                int size = CAST(int, args[2]);
    #####:  147:                return VAR_T(C99Struct, p, size);
    #####:  148:            }
    #####:  149:            vm->TypeError("expected 1 or 2 arguments");
    #####:  150:            return vm->None;
    #####:  151:        });
'''

# test :
import c

my_struct1 = c.struct(16)

c_void_1 = c.malloc(8)
my_struct2 = c.struct(c_void_1, 32)

data_str = "Hello, World!"
my_struct3 = c.struct(data_str)

data_bytes = bytes([1,2,3])
my_struct4 = c.struct(data_bytes)

try:
   c.struct(True)
   raise Exception('c.struct 的构造方法未能触发 TypeError("expected int, str or bytes")')
except TypeError:
   pass

try:
   c.struct(1,2,3)
   raise Exception('c.struct 的构造方法未能触发 TypeError("expected 1 or 2 arguments")')
except TypeError:
   pass



# ------------------------------------------------
'''source code in cffi.cpp :
      114:  158:        vm->bind_method<0>(type, "size", [](VM* vm, ArgsView args){
    #####:  159:            C99Struct& self = _CAST(C99Struct&, args[0]);
    #####:  160:            return VAR(self.size);
        -:  161:        });
        -:  162:
      114:  163:        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
    #####:  164:            const C99Struct& self = _CAST(C99Struct&, args[0]);
    #####:  165:            return VAR_T(C99Struct, self);
    #####:  166:        });
        -:  167:
      114:  168:        vm->bind_method<0>(type, "to_string", [](VM* vm, ArgsView args){
    #####:  169:            C99Struct& self = _CAST(C99Struct&, args[0]);
    #####:  170:            return VAR(Str(self.p, self.size));
    #####:  171:        });
        -:  172:
      114:  173:        vm->bind_method<0>(type, "to_bytes", [](VM* vm, ArgsView args){
    #####:  174:            C99Struct& self = _CAST(C99Struct&, args[0]);
    #####:  175:            std::vector<char> buffer(self.size);
    #####:  176:            memcpy(buffer.data(), self.p, self.size);
    #####:  177:            return VAR(Bytes(std::move(buffer)));
    #####:  178:        });
'''

# test :
import c

my_struct1 = c.struct(16)

assert my_struct1.size() == 16

# 对 c.struct 的 copy 方法的测试不完全
assert my_struct1.copy().size() == 16

data_str = "Hello, World!"
my_struct3 = c.struct(data_str)
assert my_struct3.to_string() == data_str

data_bytes = bytes([1,2,3])
my_struct4 = c.struct(data_bytes)
assert my_struct4.to_bytes() == data_bytes


# ------------------------------------------------
'''source code in cffi.cpp :
      114:  222:        vm->bind_method<1>(type, "read_struct", [](VM* vm, ArgsView args){
    #####:  223:            VoidP& self = _CAST(VoidP&, args[0]);
    #####:  224:            const Str& type = CAST(Str&, args[1]);
    #####:  225:            int size = c99_sizeof(vm, type);
    #####:  226:            return VAR_T(C99Struct, self.ptr, size);
    #####:  227:        });
        -:  228:
      114:  229:        vm->bind_method<1>(type, "write_struct", [](VM* vm, ArgsView args){
    #####:  230:            VoidP& self = _CAST(VoidP&, args[0]);
    #####:  231:            C99Struct& other = CAST(C99Struct&, args[1]);
    #####:  232:            memcpy(self.ptr, other.p, other.size);
    #####:  233:            return vm->None;
        -:  234:        });
       57:  235:    }
'''

# test :
import c

# 此处测试并不完全
c_void_1 = c.malloc(8)
my_struct1 = c.struct(16)
c_void_1.read_struct('long')
c_void_1.write_struct(my_struct1)
