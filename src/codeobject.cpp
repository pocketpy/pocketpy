#include "pocketpy/codeobject.h"

namespace pkpy{

    CodeObject::CodeObject(std::shared_ptr<SourceData> src, const Str& name):
        src(src), name(name) {}

    void CodeObject::_gc_mark() const {
        for(PyObject* v : consts) PK_OBJ_MARK(v);
        for(auto& decl: func_decls) decl->_gc_mark();
    }

    NativeFunc::NativeFunc(NativeFuncC f, int argc, bool method){
        this->f = f;
        this->argc = argc;
        if(argc != -1) this->argc += (int)method;
    }

    NativeFunc::NativeFunc(NativeFuncC f, FuncDecl_ decl){
        this->f = f;
        this->argc = -1;
        this->decl = decl;
    }


//     void CodeObject::write(VM* vm, CodeObjectSerializer& ss) const{
//         ss.write_begin_mark();          // [
//         ss.write_str(src->filename);    // src->filename
//         ss.write_int(src->mode);        // src->mode
//         ss.write_end_mark();            // ]
//         ss.write_str(name);             // name
//         ss.write_bool(is_generator);    // is_generator
//         ss.write_begin_mark();          // [
//             for(Bytecode bc: codes){
//                 if(StrName::is_valid(bc.arg)) ss.names.insert(StrName(bc.arg));
//                 ss.write_bytes(bc);
//             }
//         ss.write_end_mark();            // ]
//         ss.write_begin_mark();          // [
//             for(int line: lines){
//                 ss.write_int(line);         // line
//             }
//         ss.write_end_mark();            // ]
//         ss.write_begin_mark();          // [
//             for(PyObject* o: consts){
//                 ss.write_object(vm, o);
//             }
//         ss.write_end_mark();            // ]
//         ss.write_begin_mark();          // [
//             for(StrName vn: varnames){
//                 ss.write_name(vn);        // name
//             }
//         ss.write_end_mark();            // ]
//         ss.write_begin_mark();          // [
//             for(CodeBlock block: blocks){
//                 ss.write_bytes(block);      // block
//             }
//         ss.write_end_mark();            // ]
//         ss.write_begin_mark();          // [
//             for(auto& label: labels.items()){
//                 ss.write_name(label.first);     // label.first
//                 ss.write_int(label.second);     // label.second
//             }
//         ss.write_end_mark();            // ]
//         ss.write_begin_mark();          // [
//             for(auto& decl: func_decls){
//                 ss.write_code(vm, decl->code.get()); // decl->code
//                 ss.write_begin_mark();      // [
//                     for(int arg: decl->args) ss.write_int(arg);
//                 ss.write_end_mark();        // ]
                
//                 ss.write_begin_mark();      // [
//                     for(auto kw: decl->kwargs){
//                         ss.write_int(kw.key);           // kw.key
//                         ss.write_object(vm, kw.value);  // kw.value
//                     }
//                 ss.write_end_mark();        // ]

//                 ss.write_int(decl->starred_arg);
//                 ss.write_int(decl->starred_kwarg);
//                 ss.write_bool(decl->nested);
//             }
//         ss.write_end_mark();            // ]
//     }

//     Str CodeObject::serialize(VM* vm) const{
//         CodeObjectSerializer ss;
//         ss.write_code(vm, this);
//         return ss.str();
//     }


//     void CodeObjectSerializer::write_int(i64 v){
//         buffer += 'i';
//         buffer += std::to_string(v);
//         buffer += END;
//     }

//     void CodeObjectSerializer::write_float(f64 v){
//         buffer += 'f';
//         buffer += std::to_string(v);
//         buffer += END;
//     }

//     void CodeObjectSerializer::write_str(const Str& v){
//         buffer += 's';
//         buffer += v.escape(false).str();
//         buffer += END;
//     }

//     void CodeObjectSerializer::write_none(){
//         buffer += 'N';
//         buffer += END;
//     }

//     void CodeObjectSerializer::write_ellipsis(){
//         buffer += 'E';
//         buffer += END;
//     }

//     void CodeObjectSerializer::write_bool(bool v){
//         buffer += 'b';
//         buffer += v ? '1' : '0';
//         buffer += END;
//     }

//     void CodeObjectSerializer::write_begin_mark(){
//         buffer += '[';
//         buffer += END;
//         depth++;
//     }

//     void CodeObjectSerializer::write_name(StrName name){
//         PK_ASSERT(StrName::is_valid(name.index));
//         buffer += 'n';
//         buffer += std::to_string(name.index);
//         buffer += END;
//         names.insert(name);
//     }

//     void CodeObjectSerializer::write_end_mark(){
//         buffer += ']';
//         buffer += END;
//         depth--;
//         PK_ASSERT(depth >= 0);
//     }

//     std::string CodeObjectSerializer::str(){
//         PK_ASSERT(depth == 0);
//         for(auto name: names){
//             PK_ASSERT(StrName::is_valid(name.index));
//             write_name(name);
//             write_str(name.sv());
//         }
//         return std::move(buffer);
//     }

//     CodeObjectSerializer::CodeObjectSerializer(){
//         write_str(PK_VERSION);
//     }

// void CodeObjectSerializer::write_code(VM* vm, const CodeObject* co){
//     buffer += '(';
//     buffer += END;
//     co->write(vm, *this);
//     buffer += ')';
//     buffer += END;
// }

}   // namespace pkpy