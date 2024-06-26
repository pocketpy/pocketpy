#include "pocketpy/compiler/expr.hpp"
#include "pocketpy/interpreter/vm.hpp"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/public.h"

namespace pkpy {

inline bool is_identifier(std::string_view s) {
    if(s.empty()) return false;
    if(!isalpha(s[0]) && s[0] != '_') return false;
    for(char c: s)
        if(!isalnum(c) && c != '_') return false;
    return true;
}

inline bool is_small_int(i64 value) { return value >= INT16_MIN && value <= INT16_MAX; }

int CodeEmitContext::get_loop() const noexcept{
    int index = curr_iblock;
    while(index >= 0) {
        CodeBlock* block = c11__at(CodeBlock, &co->blocks, index);
        if(block->type == CodeBlockType_FOR_LOOP) break;
        if(block->type == CodeBlockType_WHILE_LOOP) break;
        index = block->parent;
    }
    return index;
}

CodeBlock* CodeEmitContext::enter_block(CodeBlockType type) noexcept{
    CodeBlock block = {type, curr_iblock, co->codes.count, -1, -1};
    c11_vector__push(CodeBlock, &co->blocks, block);
    curr_iblock = co->blocks.count - 1;
    return c11__at(CodeBlock, &co->blocks, curr_iblock);
}

void CodeEmitContext::exit_block() noexcept{
    CodeBlock* block = c11__at(CodeBlock, &co->blocks, curr_iblock);
    CodeBlockType curr_type = block->type;
    block->end = co->codes.count;
    curr_iblock = block->parent;
    assert(curr_iblock >= 0);
    if(curr_type == CodeBlockType_FOR_LOOP) {
        // add a no op here to make block check work
        emit_(OP_NO_OP, BC_NOARG, BC_KEEPLINE, true);
    }
}

void CodeEmitContext::s_emit_decorators(int count) noexcept{
    assert(s_size() >= count);
    // [obj]
    for(int i=0; i<count; i++) {
        Expr* deco = s_popx();
        deco->emit_(this);                           // [obj, f]
        emit_(OP_ROT_TWO, BC_NOARG, deco->line);     // [f, obj]
        emit_(OP_LOAD_NULL, BC_NOARG, BC_KEEPLINE);  // [f, obj, NULL]
        emit_(OP_ROT_TWO, BC_NOARG, BC_KEEPLINE);    // [obj, NULL, f]
        emit_(OP_CALL, 1, deco->line);               // [obj]
        delete_expr(deco);
    }
}

int CodeEmitContext::emit_(Opcode opcode, uint16_t arg, int line, bool is_virtual) noexcept{
    c11_vector__push(Bytecode, &co->codes, (Bytecode{(uint8_t)opcode, arg}));
    c11_vector__push(BytecodeEx, &co->codes_ex, (BytecodeEx{line, is_virtual, curr_iblock}));
    int i = co->codes.count - 1;
    BytecodeEx* codes_ex = (BytecodeEx*)co->codes_ex.data;
    if(line == BC_KEEPLINE) {
        codes_ex[i].lineno = i>=1 ? codes_ex[i-1].lineno : 1;
    }
    return i;
}

void CodeEmitContext::revert_last_emit_() noexcept{
    c11_vector__pop(Bytecode, &co->codes);
    c11_vector__pop(BytecodeEx, &co->codes_ex);
}

void CodeEmitContext::try_merge_for_iter_store(int i) noexcept{
    // [FOR_ITER, STORE_?, ]
    Bytecode* co_codes = (Bytecode*)co->codes.data;
    if(co_codes[i].op != OP_FOR_ITER) return;
    if(co->codes.count - i != 2) return;
    uint16_t arg = co_codes[i + 1].arg;
    if(co_codes[i + 1].op == OP_STORE_FAST) {
        revert_last_emit_();
        co_codes[i].op = OP_FOR_ITER_STORE_FAST;
        co_codes[i].arg = arg;
        return;
    }
    if(co_codes[i + 1].op == OP_STORE_GLOBAL) {
        revert_last_emit_();
        co_codes[i].op = OP_FOR_ITER_STORE_GLOBAL;
        co_codes[i].arg = arg;
        return;
    }
}

int CodeEmitContext::emit_int(i64 value, int line) noexcept{
    if(is_small_int(value)) {
        return emit_(OP_LOAD_SMALL_INT, (uint16_t)value, line);
    } else {
        return emit_(OP_LOAD_CONST, add_const(VAR(value)), line);
    }
}

void CodeEmitContext::patch_jump(int index) noexcept{
    Bytecode* co_codes = (Bytecode*)co->codes.data;
    int target = co->codes.count;
    Bytecode__set_signed_arg(&co_codes[index], target - index);
}

bool CodeEmitContext::add_label(StrName name) noexcept{
    bool ok = c11_smallmap_n2i__contains(&co->labels, name.index);
    if(ok) return false;
    c11_smallmap_n2i__set(&co->labels, name.index, co->codes.count);
    return true;
}

int CodeEmitContext::add_varname(StrName name) noexcept{
    // PK_MAX_CO_VARNAMES will be checked when pop_context(), not here
    int index = c11_smallmap_n2i__get(&co->varnames_inv, name.index, -1);
    if(index >= 0) return index;
    c11_vector__push(uint16_t, &co->varnames, name.index);
    co->nlocals++;
    index = co->varnames.count - 1;
    c11_smallmap_n2i__set(&co->varnames_inv, name.index, index);
    return index;
}

int CodeEmitContext::add_const_string(std::string_view key) noexcept{
    uint16_t* val = c11_smallmap_s2n__try_get(&_co_consts_string_dedup_map, {key.data(), (int)key.size()});
    if(val) {
        return *val;
    } else {
        // co->consts.push_back(VAR(key));
        c11_vector__push(PyVar, &co->consts, VAR(key));
        int index = co->consts.count - 1;
        key = c11__getitem(PyVar, &co->consts, index).obj_get<Str>().sv();
        c11_smallmap_s2n__set(&_co_consts_string_dedup_map, {key.data(), (int)key.size()}, index);
        return index;
    }
}

int CodeEmitContext::add_const(PyVar v) noexcept{
    assert(!is_type(v, VM::tp_str));
    c11_vector__push(PyVar, &co->consts, v);
    return co->consts.count - 1;
}

void CodeEmitContext::emit_store_name(NameScope scope, StrName name, int line) noexcept{
    switch(scope) {
        case NAME_LOCAL: emit_(OP_STORE_FAST, add_varname(name), line); break;
        case NAME_GLOBAL: emit_(OP_STORE_GLOBAL, StrName(name).index, line); break;
        case NAME_GLOBAL_UNKNOWN: emit_(OP_STORE_NAME, StrName(name).index, line); break;
        default: assert(false); break;
    }
}

void NameExpr::emit_(CodeEmitContext* ctx) {
    int index = c11_smallmap_n2i__get(&ctx->co->varnames_inv, name.index, -1);
    if(scope == NAME_LOCAL && index >= 0) {
        ctx->emit_(OP_LOAD_FAST, index, line);
    } else {
        Opcode op = ctx->level <= 1 ? OP_LOAD_GLOBAL : OP_LOAD_NONLOCAL;
        if(ctx->is_compiling_class && scope == NAME_GLOBAL) {
            // if we are compiling a class, we should use OP_LOAD_ATTR_GLOBAL instead of OP_LOAD_GLOBAL
            // this supports @property.setter
            op = OP_LOAD_CLASS_GLOBAL;
            // exec()/eval() won't work with OP_LOAD_ATTR_GLOBAL in class body
        } else {
            // we cannot determine the scope when calling exec()/eval()
            if(scope == NAME_GLOBAL_UNKNOWN) op = OP_LOAD_NAME;
        }
        ctx->emit_(op, StrName(name).index, line);
    }
}

bool NameExpr::emit_del(CodeEmitContext* ctx) {
    switch(scope) {
        case NAME_LOCAL: ctx->emit_(OP_DELETE_FAST, ctx->add_varname(name), line); break;
        case NAME_GLOBAL: ctx->emit_(OP_DELETE_GLOBAL, StrName(name).index, line); break;
        case NAME_GLOBAL_UNKNOWN: ctx->emit_(OP_DELETE_NAME, StrName(name).index, line); break;
        default: assert(false); break;
    }
    return true;
}

bool NameExpr::emit_store(CodeEmitContext* ctx) {
    if(ctx->is_compiling_class) {
        ctx->emit_(OP_STORE_CLASS_ATTR, name.index, line);
        return true;
    }
    ctx->emit_store_name(scope, name, line);
    return true;
}

void InvertExpr::emit_(CodeEmitContext* ctx) {
    child->emit_(ctx);
    ctx->emit_(OP_UNARY_INVERT, BC_NOARG, line);
}

void StarredExpr::emit_(CodeEmitContext* ctx) {
    child->emit_(ctx);
    ctx->emit_(OP_UNARY_STAR, level, line);
}

bool StarredExpr::emit_store(CodeEmitContext* ctx) {
    if(level != 1) return false;
    // simply proxy to child
    return child->emit_store(ctx);
}

void NotExpr::emit_(CodeEmitContext* ctx) {
    child->emit_(ctx);
    ctx->emit_(OP_UNARY_NOT, BC_NOARG, line);
}

void AndExpr::emit_(CodeEmitContext* ctx) {
    lhs->emit_(ctx);
    int patch = ctx->emit_(OP_JUMP_IF_FALSE_OR_POP, BC_NOARG, line);
    rhs->emit_(ctx);
    ctx->patch_jump(patch);
}

void OrExpr::emit_(CodeEmitContext* ctx) {
    lhs->emit_(ctx);
    int patch = ctx->emit_(OP_JUMP_IF_TRUE_OR_POP, BC_NOARG, line);
    rhs->emit_(ctx);
    ctx->patch_jump(patch);
}

void Literal0Expr::emit_(CodeEmitContext* ctx) {
    switch(token) {
        case TK_NONE: ctx->emit_(OP_LOAD_NONE, BC_NOARG, line); break;
        case TK_TRUE: ctx->emit_(OP_LOAD_TRUE, BC_NOARG, line); break;
        case TK_FALSE: ctx->emit_(OP_LOAD_FALSE, BC_NOARG, line); break;
        case TK_DOTDOTDOT: ctx->emit_(OP_LOAD_ELLIPSIS, BC_NOARG, line); break;
        default: assert(false);
    }
}

void LongExpr::emit_(CodeEmitContext* ctx) {
    ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(s.sv()), line);
    ctx->emit_(OP_BUILD_LONG, BC_NOARG, line);
}

void ImagExpr::emit_(CodeEmitContext* ctx) {
    VM* vm = ctx->vm;
    ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(value)), line);
    ctx->emit_(OP_BUILD_IMAG, BC_NOARG, line);
}

void BytesExpr::emit_(CodeEmitContext* ctx) {
    ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(s.sv()), line);
    ctx->emit_(OP_BUILD_BYTES, BC_NOARG, line);
}

void LiteralExpr::emit_(CodeEmitContext* ctx) {
    VM* vm = ctx->vm;
    if(std::holds_alternative<i64>(value)) {
        i64 _val = std::get<i64>(value);
        ctx->emit_int(_val, line);
        return;
    }
    if(std::holds_alternative<f64>(value)) {
        f64 _val = std::get<f64>(value);
        ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(_val)), line);
        return;
    }
    if(std::holds_alternative<Str>(value)) {
        std::string_view key = std::get<Str>(value).sv();
        ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(key), line);
        return;
    }
}

void NegatedExpr::emit_(CodeEmitContext* ctx) {
    VM* vm = ctx->vm;
    // if child is a int of float, do constant folding
    if(child->is_literal()) {
        LiteralExpr* lit = static_cast<LiteralExpr*>(child);
        if(std::holds_alternative<i64>(lit->value)) {
            i64 _val = -std::get<i64>(lit->value);
            ctx->emit_int(_val, line);
            return;
        }
        if(std::holds_alternative<f64>(lit->value)) {
            f64 _val = -std::get<f64>(lit->value);
            ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(_val)), line);
            return;
        }
    }
    child->emit_(ctx);
    ctx->emit_(OP_UNARY_NEGATIVE, BC_NOARG, line);
}

void SliceExpr::emit_(CodeEmitContext* ctx) {
    if(start) {
        start->emit_(ctx);
    } else {
        ctx->emit_(OP_LOAD_NONE, BC_NOARG, line);
    }

    if(stop) {
        stop->emit_(ctx);
    } else {
        ctx->emit_(OP_LOAD_NONE, BC_NOARG, line);
    }

    if(step) {
        step->emit_(ctx);
    } else {
        ctx->emit_(OP_LOAD_NONE, BC_NOARG, line);
    }

    ctx->emit_(OP_BUILD_SLICE, BC_NOARG, line);
}

void DictItemExpr::emit_(CodeEmitContext* ctx) {
    if(is_starred()) {
        assert(key == nullptr);
        value->emit_(ctx);
    } else {
        key->emit_(ctx);
        value->emit_(ctx);
        ctx->emit_(OP_BUILD_TUPLE, 2, line);
    }
}

bool TupleExpr::emit_store(CodeEmitContext* ctx) {
    // TOS is an iterable
    // items may contain StarredExpr, we should check it
    int starred_i = -1;
    for(int i = 0; i < items.size(); i++) {
        if(!items[i]->is_starred()) continue;
        if(starred_i == -1)
            starred_i = i;
        else
            return false;  // multiple StarredExpr not allowed
    }

    if(starred_i == -1) {
        Bytecode* prev = c11__at(Bytecode, &ctx->co->codes, ctx->co->codes.count - 1);
        if(prev->op == OP_BUILD_TUPLE && prev->arg == items.size()) {
            // build tuple and unpack it is meaningless
            ctx->revert_last_emit_();
        } else {
            if(prev->op == OP_FOR_ITER) {
                prev->op = OP_FOR_ITER_UNPACK;
                prev->arg = items.size();
            } else {
                ctx->emit_(OP_UNPACK_SEQUENCE, items.size(), line);
            }
        }
    } else {
        // starred assignment target must be in a tuple
        if(items.size() == 1) return false;
        // starred assignment target must be the last one (differ from cpython)
        if(starred_i != items.size() - 1) return false;
        // a,*b = [1,2,3]
        // stack is [1,2,3] -> [1,[2,3]]
        ctx->emit_(OP_UNPACK_EX, items.size() - 1, line);
    }
    // do reverse emit
    for(int i = items.size() - 1; i >= 0; i--) {
        bool ok = items[i]->emit_store(ctx);
        if(!ok) return false;
    }
    return true;
}

bool TupleExpr::emit_del(CodeEmitContext* ctx) {
    for(auto& e: items) {
        bool ok = e->emit_del(ctx);
        if(!ok) return false;
    }
    return true;
}

void CompExpr::emit_(CodeEmitContext* ctx) {
    ctx->emit_(op0, 0, line);
    iter->emit_(ctx);
    ctx->emit_(OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
    ctx->enter_block(CodeBlockType_FOR_LOOP);
    int curr_iblock = ctx->curr_iblock;
    int for_codei = ctx->emit_(OP_FOR_ITER, curr_iblock, BC_KEEPLINE);
    bool ok = vars->emit_store(ctx);
    // this error occurs in `vars` instead of this line, but...nevermind
    assert(ok);  // this should raise a SyntaxError, but we just assert it
    ctx->try_merge_for_iter_store(for_codei);
    if(cond) {
        cond->emit_(ctx);
        int patch = ctx->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, BC_KEEPLINE);
        expr->emit_(ctx);
        ctx->emit_(op1, BC_NOARG, BC_KEEPLINE);
        ctx->patch_jump(patch);
    } else {
        expr->emit_(ctx);
        ctx->emit_(op1, BC_NOARG, BC_KEEPLINE);
    }
    ctx->emit_(OP_LOOP_CONTINUE, curr_iblock, BC_KEEPLINE);
    ctx->exit_block();
}

void FStringExpr::_load_simple_expr(CodeEmitContext* ctx, Str expr) {
    bool repr = false;
    if(expr.size >= 2 && expr.end()[-2] == '!') {
        switch(expr.end()[-1]) {
            case 'r':
                repr = true;
                expr = expr.slice(0, expr.size - 2);
                break;
            case 's':
                repr = false;
                expr = expr.slice(0, expr.size - 2);
                break;
            default: break;  // nothing happens
        }
    }
    // name or name.name
    bool is_fastpath = false;
    if(is_identifier(expr.sv())) {
        ctx->emit_(OP_LOAD_NAME, StrName(expr.sv()).index, line);
        is_fastpath = true;
    } else {
        int dot = expr.index(".");
        if(dot > 0) {
            std::string_view a = expr.sv().substr(0, dot);
            std::string_view b = expr.sv().substr(dot + 1);
            if(is_identifier(a) && is_identifier(b)) {
                ctx->emit_(OP_LOAD_NAME, StrName(a).index, line);
                ctx->emit_(OP_LOAD_ATTR, StrName(b).index, line);
                is_fastpath = true;
            }
        }
    }

    if(!is_fastpath) {
        int index = ctx->add_const_string(expr.sv());
        ctx->emit_(OP_FSTRING_EVAL, index, line);
    }

    if(repr) { ctx->emit_(OP_REPR, BC_NOARG, line); }
}

static bool is_fmt_valid_char(char c) {
    switch(c) {
            // clang-format off
            case '-': case '=': case '*': case '#': case '@': case '!': case '~':
            case '<': case '>': case '^':
            case '.': case 'f': case 'd': case 's':
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return true;
            default: return false;
            // clang-format on
    }
}

void FStringExpr::emit_(CodeEmitContext* ctx) {
    int i = 0;          // left index
    int j = 0;          // right index
    int count = 0;      // how many string parts
    bool flag = false;  // true if we are in a expression

    while(j < src.size) {
        if(flag) {
            if(src[j] == '}') {
                // add expression
                Str expr = src.slice(i, j);
                // BUG: ':' is not a format specifier in f"{stack[2:]}"
                int conon = expr.index(":");
                if(conon >= 0) {
                    Str spec = expr.substr(conon + 1);
                    // filter some invalid spec
                    bool ok = true;
                    for(char c: spec)
                        if(!is_fmt_valid_char(c)) {
                            ok = false;
                            break;
                        }
                    if(ok) {
                        _load_simple_expr(ctx, expr.slice(0, conon));
                        ctx->emit_(OP_FORMAT_STRING, ctx->add_const_string(spec.sv()), line);
                    } else {
                        // ':' is not a spec indicator
                        _load_simple_expr(ctx, expr);
                    }
                } else {
                    _load_simple_expr(ctx, expr);
                }
                flag = false;
                count++;
            }
        } else {
            if(src[j] == '{') {
                // look at next char
                if(j + 1 < src.size && src[j + 1] == '{') {
                    // {{ -> {
                    j++;
                    ctx->emit_(OP_LOAD_CONST, ctx->add_const_string("{"), line);
                    count++;
                } else {
                    // { -> }
                    flag = true;
                    i = j + 1;
                }
            } else if(src[j] == '}') {
                // look at next char
                if(j + 1 < src.size && src[j + 1] == '}') {
                    // }} -> }
                    j++;
                    ctx->emit_(OP_LOAD_CONST, ctx->add_const_string("}"), line);
                    count++;
                } else {
                    // } -> error
                    // throw std::runtime_error("f-string: unexpected }");
                    // just ignore
                }
            } else {
                // literal
                i = j;
                while(j < src.size && src[j] != '{' && src[j] != '}')
                    j++;
                Str literal = src.slice(i, j);
                ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(literal.sv()), line);
                count++;
                continue;  // skip j++
            }
        }
        j++;
    }

    if(flag) {
        // literal
        Str literal = src.slice(i, src.size);
        ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(literal.sv()), line);
        count++;
    }

    ctx->emit_(OP_BUILD_STRING, count, line);
}

void SubscrExpr::emit_(CodeEmitContext* ctx) {
    lhs->emit_(ctx);
    rhs->emit_(ctx);
    Bytecode last_bc = c11__getitem(Bytecode, &ctx->co->codes, ctx->co->codes.count-1);
    if(rhs->is_name() && last_bc.op == OP_LOAD_FAST) {
        ctx->revert_last_emit_();
        ctx->emit_(OP_LOAD_SUBSCR_FAST, last_bc.arg, line);
    } else if(rhs->is_literal() && last_bc.op == OP_LOAD_SMALL_INT) {
        ctx->revert_last_emit_();
        ctx->emit_(OP_LOAD_SUBSCR_SMALL_INT, last_bc.arg, line);
    } else {
        ctx->emit_(OP_LOAD_SUBSCR, BC_NOARG, line);
    }
}

bool SubscrExpr::emit_store(CodeEmitContext* ctx) {
    lhs->emit_(ctx);
    rhs->emit_(ctx);
    Bytecode last_bc = c11__getitem(Bytecode, &ctx->co->codes, ctx->co->codes.count-1);
    if(rhs->is_name() && last_bc.op == OP_LOAD_FAST) {
        ctx->revert_last_emit_();
        ctx->emit_(OP_STORE_SUBSCR_FAST, last_bc.arg, line);
    } else {
        ctx->emit_(OP_STORE_SUBSCR, BC_NOARG, line);
    }
    return true;
}

void SubscrExpr::emit_inplace(CodeEmitContext* ctx) {
    lhs->emit_(ctx);
    rhs->emit_(ctx);
    ctx->emit_(OP_DUP_TOP_TWO, BC_NOARG, line);
    ctx->emit_(OP_LOAD_SUBSCR, BC_NOARG, line);
}

bool SubscrExpr::emit_store_inplace(CodeEmitContext* ctx) {
    // [a, b, val] -> [val, a, b]
    ctx->emit_(OP_ROT_THREE, BC_NOARG, line);
    ctx->emit_(OP_STORE_SUBSCR, BC_NOARG, line);
    return true;
}

bool SubscrExpr::emit_del(CodeEmitContext* ctx) {
    lhs->emit_(ctx);
    rhs->emit_(ctx);
    ctx->emit_(OP_DELETE_SUBSCR, BC_NOARG, line);
    return true;
}

void AttribExpr::emit_(CodeEmitContext* ctx) {
    child->emit_(ctx);
    ctx->emit_(OP_LOAD_ATTR, name.index, line);
}

bool AttribExpr::emit_del(CodeEmitContext* ctx) {
    child->emit_(ctx);
    ctx->emit_(OP_DELETE_ATTR, name.index, line);
    return true;
}

bool AttribExpr::emit_store(CodeEmitContext* ctx) {
    child->emit_(ctx);
    ctx->emit_(OP_STORE_ATTR, name.index, line);
    return true;
}

void AttribExpr::emit_method(CodeEmitContext* ctx) {
    child->emit_(ctx);
    ctx->emit_(OP_LOAD_METHOD, name.index, line);
}

void AttribExpr::emit_inplace(CodeEmitContext* ctx) {
    child->emit_(ctx);
    ctx->emit_(OP_DUP_TOP, BC_NOARG, line);
    ctx->emit_(OP_LOAD_ATTR, name.index, line);
}

bool AttribExpr::emit_store_inplace(CodeEmitContext* ctx) {
    // [a, val] -> [val, a]
    ctx->emit_(OP_ROT_TWO, BC_NOARG, line);
    ctx->emit_(OP_STORE_ATTR, name.index, line);
    return true;
}

void CallExpr::emit_(CodeEmitContext* ctx) {
    bool vargs = false;
    bool vkwargs = false;
    for(auto& arg: args)
        if(arg->is_starred()) vargs = true;
    for(auto& item: kwargs)
        if(item.second->is_starred()) vkwargs = true;

    // if callable is a AttrExpr, we should try to use `fast_call` instead of use `boundmethod` proxy
    if(callable->is_attrib()) {
        auto p = static_cast<AttribExpr*>(callable);
        p->emit_method(ctx);  // OP_LOAD_METHOD
    } else {
        callable->emit_(ctx);
        ctx->emit_(OP_LOAD_NULL, BC_NOARG, BC_KEEPLINE);
    }

    if(vargs || vkwargs) {
        for(auto& item: args)
            item->emit_(ctx);
        ctx->emit_(OP_BUILD_TUPLE_UNPACK, (uint16_t)args.size(), line);

        if(!kwargs.empty()) {
            for(auto& item: kwargs) {
                if(item.second->is_starred()) {
                    assert(item.second->star_level() == 2);
                    item.second->emit_(ctx);
                } else {
                    // k=v
                    int index = ctx->add_const_string(item.first.sv());
                    ctx->emit_(OP_LOAD_CONST, index, line);
                    item.second->emit_(ctx);
                    ctx->emit_(OP_BUILD_TUPLE, 2, line);
                }
            }
            ctx->emit_(OP_BUILD_DICT_UNPACK, (int)kwargs.size(), line);
            ctx->emit_(OP_CALL_TP, 1, line);
        } else {
            ctx->emit_(OP_CALL_TP, 0, line);
        }
    } else {
        // vectorcall protocol
        for(auto& item: args)
            item->emit_(ctx);
        for(auto& item: kwargs) {
            i64 _val = StrName(item.first.sv()).index;
            ctx->emit_int(_val, line);
            item.second->emit_(ctx);
        }
        int KWARGC = kwargs.size();
        int ARGC = args.size();
        ctx->emit_(OP_CALL, (KWARGC << 8) | ARGC, line);
    }
}

bool BinaryExpr::is_compare() const {
    switch(op) {
        case TK_LT:
        case TK_LE:
        case TK_EQ:
        case TK_NE:
        case TK_GT:
        case TK_GE: return true;
        default: return false;
    }
}

void BinaryExpr::_emit_compare(CodeEmitContext* ctx, small_vector_2<int, 8>& jmps) {
    if(lhs->is_compare()) {
        static_cast<BinaryExpr*>(lhs)->_emit_compare(ctx, jmps);
    } else {
        lhs->emit_(ctx);  // [a]
    }
    rhs->emit_(ctx);                           // [a, b]
    ctx->emit_(OP_DUP_TOP, BC_NOARG, line);    // [a, b, b]
    ctx->emit_(OP_ROT_THREE, BC_NOARG, line);  // [b, a, b]
    switch(op) {
        case TK_LT: ctx->emit_(OP_COMPARE_LT, BC_NOARG, line); break;
        case TK_LE: ctx->emit_(OP_COMPARE_LE, BC_NOARG, line); break;
        case TK_EQ: ctx->emit_(OP_COMPARE_EQ, BC_NOARG, line); break;
        case TK_NE: ctx->emit_(OP_COMPARE_NE, BC_NOARG, line); break;
        case TK_GT: ctx->emit_(OP_COMPARE_GT, BC_NOARG, line); break;
        case TK_GE: ctx->emit_(OP_COMPARE_GE, BC_NOARG, line); break;
        default: PK_UNREACHABLE()
    }
    // [b, RES]
    int index = ctx->emit_(OP_SHORTCUT_IF_FALSE_OR_POP, BC_NOARG, line);
    jmps.push_back(index);
}

void BinaryExpr::emit_(CodeEmitContext* ctx) {
    small_vector_2<int, 8> jmps;
    if(is_compare() && lhs->is_compare()) {
        // (a < b) < c
        static_cast<BinaryExpr*>(lhs)->_emit_compare(ctx, jmps);
        // [b, RES]
    } else {
        // (1 + 2) < c
        if(inplace) {
            lhs->emit_inplace(ctx);
        } else {
            lhs->emit_(ctx);
        }
    }

    rhs->emit_(ctx);
    switch(op) {
        case TK_ADD: ctx->emit_(OP_BINARY_ADD, BC_NOARG, line); break;
        case TK_SUB: ctx->emit_(OP_BINARY_SUB, BC_NOARG, line); break;
        case TK_MUL: ctx->emit_(OP_BINARY_MUL, BC_NOARG, line); break;
        case TK_DIV: ctx->emit_(OP_BINARY_TRUEDIV, BC_NOARG, line); break;
        case TK_FLOORDIV: ctx->emit_(OP_BINARY_FLOORDIV, BC_NOARG, line); break;
        case TK_MOD: ctx->emit_(OP_BINARY_MOD, BC_NOARG, line); break;
        case TK_POW: ctx->emit_(OP_BINARY_POW, BC_NOARG, line); break;

        case TK_LT: ctx->emit_(OP_COMPARE_LT, BC_NOARG, line); break;
        case TK_LE: ctx->emit_(OP_COMPARE_LE, BC_NOARG, line); break;
        case TK_EQ: ctx->emit_(OP_COMPARE_EQ, BC_NOARG, line); break;
        case TK_NE: ctx->emit_(OP_COMPARE_NE, BC_NOARG, line); break;
        case TK_GT: ctx->emit_(OP_COMPARE_GT, BC_NOARG, line); break;
        case TK_GE: ctx->emit_(OP_COMPARE_GE, BC_NOARG, line); break;

        case TK_IN: ctx->emit_(OP_CONTAINS_OP, 0, line); break;
        case TK_NOT_IN: ctx->emit_(OP_CONTAINS_OP, 1, line); break;
        case TK_IS: ctx->emit_(OP_IS_OP, BC_NOARG, line); break;
        case TK_IS_NOT: ctx->emit_(OP_IS_NOT_OP, BC_NOARG, line); break;

        case TK_LSHIFT: ctx->emit_(OP_BITWISE_LSHIFT, BC_NOARG, line); break;
        case TK_RSHIFT: ctx->emit_(OP_BITWISE_RSHIFT, BC_NOARG, line); break;
        case TK_AND: ctx->emit_(OP_BITWISE_AND, BC_NOARG, line); break;
        case TK_OR: ctx->emit_(OP_BITWISE_OR, BC_NOARG, line); break;
        case TK_XOR: ctx->emit_(OP_BITWISE_XOR, BC_NOARG, line); break;

        case TK_DECORATOR: ctx->emit_(OP_BINARY_MATMUL, BC_NOARG, line); break;
        default: PK_FATAL_ERROR("unknown binary operator: %s\n", pk_TokenSymbols[op]);
    }

    for(int i: jmps)
        ctx->patch_jump(i);
}

void TernaryExpr::emit_(CodeEmitContext* ctx) {
    cond->emit_(ctx);
    int patch = ctx->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, cond->line);
    true_expr->emit_(ctx);
    int patch_2 = ctx->emit_(OP_JUMP_FORWARD, BC_NOARG, true_expr->line);
    ctx->patch_jump(patch);
    false_expr->emit_(ctx);
    ctx->patch_jump(patch_2);
}

}  // namespace pkpy