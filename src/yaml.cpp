#include "pocketpy/yaml.h"

namespace pkpy{

class YAMLCompiler: public CompilerBase{
public:
    VM* vm;
    YAMLCompiler(VM* vm, const Str& s): vm(vm) {
        auto src = std::make_shared<SourceData>(s, "<yaml>", JSON_MODE);
        this->lexer = std::make_unique<Lexer>(src);
    }

    PyObject* EXPR(bool* valid_key){
        // BASIC: True False None @str @num
        // CONTAINER: [] {}
        *valid_key = false;
        switch(curr().type){
            case TK("True"): advance(); return vm->True; break;
            case TK("False"): advance(); return vm->False; break;
            case TK("None"): advance(); return vm->None; break;
            case TK("@num"):{
                advance();
                TokenValue value = prev().value;
                if(std::holds_alternative<i64>(value)){
                    return VAR(std::get<i64>(value));
                }else if(std::holds_alternative<f64>(value)){
                    return VAR(std::get<f64>(value));
                }
                FATAL_ERROR();
            }
            case TK("@str"):
                advance();
                *valid_key = true;
                return VAR(std::get<Str>(prev().value));
            case TK("@id"):
                *valid_key = true;
                advance();
                return VAR(prev().sv());
            case TK("["): case TK("{"): {
                // parse the whole line as json
                return NULL;
            }
            default: SyntaxError();
            return NULL;
        }
    }

    PyObject* compile_block(){
        consume(TK(":"));
        if(curr().type!=TK("@eol") && curr().type!=TK("@eof")){
            bool _;
            return EXPR(&_);  // inline block
        }

        PyObject* block = VAR(Dict(vm));
        Dict& d = PK_OBJ_GET(Dict, block);

        consume(TK("@indent"));
        while (curr().type != TK("@dedent")) {
            bool valid_key;
            PyObject* key = EXPR(&valid_key);
            if(!valid_key) SyntaxError();
        }
        consume(TK("@dedent"));
    }

    Dict compile(){
        tokens = lexer->run();
        Dict d(vm);
        advance();          // skip @sof, so prev() is always valid

        while (!match(TK("@eof"))) {

        }
        return d;
    }
};

void add_module_yaml(VM* vm){
    PyObject* mod = vm->new_module("yaml");

    vm->bind(mod, "loads(s: str) -> dict", [](VM* vm, ArgsView args){
        const Str& s = CAST(Str&, args[0]);
        YAMLCompiler compiler(vm, s);
        return VAR(compiler.compile());
    });
}

}   // namespace pkpy