#include "pocketpy/expr.h"

namespace pkpy{

    int CodeEmitContext::get_loop() const {
        int index = curr_block_i;
        while(index >= 0){
            if(co->blocks[index].type == FOR_LOOP) break;
            if(co->blocks[index].type == WHILE_LOOP) break;
            index = co->blocks[index].parent;
        }
        return index;
    }

    CodeBlock* CodeEmitContext::enter_block(CodeBlockType type){
        if(type == FOR_LOOP) for_loop_depth++;
        co->blocks.push_back(CodeBlock(
            type, curr_block_i, for_loop_depth, (int)co->codes.size()
        ));
        curr_block_i = co->blocks.size()-1;
        return &co->blocks[curr_block_i];
    }

    void CodeEmitContext::exit_block(){
        auto curr_type = co->blocks[curr_block_i].type;
        if(curr_type == FOR_LOOP) for_loop_depth--;
        co->blocks[curr_block_i].end = co->codes.size();
        curr_block_i = co->blocks[curr_block_i].parent;
        if(curr_block_i < 0) FATAL_ERROR();

        if(curr_type == FOR_LOOP){
            // add a no op here to make block check work
            emit_(OP_NO_OP, BC_NOARG, BC_KEEPLINE);
        }
    }

    // clear the expression stack and generate bytecode
    void CodeEmitContext::emit_expr(){
        if(s_expr.size() != 1){
            throw std::runtime_error("s_expr.size() != 1\n" + _log_s_expr());
        }
        Expr_ expr = s_expr.popx();
        expr->emit_(this);
    }

    std::string CodeEmitContext::_log_s_expr(){
        std::stringstream ss; // debug
        for(auto& e: s_expr.data()) ss << e->str() << " ";
        return ss.str();
    }

    int CodeEmitContext::emit_(Opcode opcode, uint16_t arg, int line) {
        co->codes.push_back(Bytecode{(uint8_t)opcode, arg});
        co->iblocks.push_back(curr_block_i);
        co->lines.push_back(line);
        int i = co->codes.size() - 1;
        if(line==BC_KEEPLINE){
            if(i>=1) co->lines[i] = co->lines[i-1];
            else co->lines[i] = 1;
        }
        return i;
    }

    void CodeEmitContext::patch_jump(int index) {
        int target = co->codes.size();
        co->codes[index].arg = target;
    }

    bool CodeEmitContext::add_label(StrName name){
        if(co->labels.contains(name)) return false;
        co->labels.set(name, co->codes.size());
        return true;
    }

    int CodeEmitContext::add_varname(StrName name){
        // PK_MAX_CO_VARNAMES will be checked when pop_context(), not here
        int index = co->varnames_inv.try_get(name);
        if(index >= 0) return index;
        co->varnames.push_back(name);
        index = co->varnames.size() - 1;
        co->varnames_inv.set(name, index);
        return index;
    }

    int CodeEmitContext::add_const(PyObject* v){
        // simple deduplication, only works for int/float
        for(int i=0; i<co->consts.size(); i++){
            if(co->consts[i] == v) return i;
        }
        // string deduplication
        if(is_non_tagged_type(v, vm->tp_str)){
            const Str& v_str = PK_OBJ_GET(Str, v);
            for(int i=0; i<co->consts.size(); i++){
                if(is_non_tagged_type(co->consts[i], vm->tp_str)){
                    if(PK_OBJ_GET(Str, co->consts[i]) == v_str) return i;
                }
            }
        }
        co->consts.push_back(v);
        return co->consts.size() - 1;
    }

    int CodeEmitContext::add_func_decl(FuncDecl_ decl){
        co->func_decls.push_back(decl);
        return co->func_decls.size() - 1;
    }

    void CodeEmitContext::emit_store_name(NameScope scope, StrName name, int line){
        switch(scope){
            case NAME_LOCAL:
                emit_(OP_STORE_FAST, add_varname(name), line);
                break;
            case NAME_GLOBAL:
                emit_(OP_STORE_GLOBAL, StrName(name).index, line);
                break;
            case NAME_GLOBAL_UNKNOWN:
                emit_(OP_STORE_NAME, StrName(name).index, line);
                break;
            default: FATAL_ERROR(); break;
        }
    }


    void NameExpr::emit_(CodeEmitContext* ctx) {
        int index = ctx->co->varnames_inv.try_get(name);
        if(scope == NAME_LOCAL && index >= 0){
            ctx->emit_(OP_LOAD_FAST, index, line);
        }else{
            Opcode op = ctx->level <= 1 ? OP_LOAD_GLOBAL : OP_LOAD_NONLOCAL;
            if(ctx->is_compiling_class && scope == NAME_GLOBAL){
                // if we are compiling a class, we should use OP_LOAD_ATTR_GLOBAL instead of OP_LOAD_GLOBAL
                // this supports @property.setter
                op = OP_LOAD_CLASS_GLOBAL;
                // exec()/eval() won't work with OP_LOAD_ATTR_GLOBAL in class body
            }else{
                // we cannot determine the scope when calling exec()/eval()
                if(scope == NAME_GLOBAL_UNKNOWN) op = OP_LOAD_NAME;
            }
            ctx->emit_(op, StrName(name).index, line);
        }
    }

    bool NameExpr::emit_del(CodeEmitContext* ctx) {
        switch(scope){
            case NAME_LOCAL:
                ctx->emit_(OP_DELETE_FAST, ctx->add_varname(name), line);
                break;
            case NAME_GLOBAL:
                ctx->emit_(OP_DELETE_GLOBAL, StrName(name).index, line);
                break;
            case NAME_GLOBAL_UNKNOWN:
                ctx->emit_(OP_DELETE_NAME, StrName(name).index, line);
                break;
            default: FATAL_ERROR(); break;
        }
        return true;
    }

    bool NameExpr::emit_store(CodeEmitContext* ctx) {
        if(ctx->is_compiling_class){
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

    void Literal0Expr::emit_(CodeEmitContext* ctx){
        switch (token) {
            case TK("None"):    ctx->emit_(OP_LOAD_NONE, BC_NOARG, line); break;
            case TK("True"):    ctx->emit_(OP_LOAD_TRUE, BC_NOARG, line); break;
            case TK("False"):   ctx->emit_(OP_LOAD_FALSE, BC_NOARG, line); break;
            case TK("..."):     ctx->emit_(OP_LOAD_ELLIPSIS, BC_NOARG, line); break;
            default: FATAL_ERROR();
        }
    }

    void LongExpr::emit_(CodeEmitContext* ctx) {
        VM* vm = ctx->vm;
        ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(s)), line);
        ctx->emit_(OP_BUILD_LONG, BC_NOARG, line);
    }

    void BytesExpr::emit_(CodeEmitContext* ctx) {
        VM* vm = ctx->vm;
        ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(s)), line);
        ctx->emit_(OP_BUILD_BYTES, BC_NOARG, line);
    }

    void LiteralExpr::emit_(CodeEmitContext* ctx) {
        VM* vm = ctx->vm;
        PyObject* obj = nullptr;
        if(std::holds_alternative<i64>(value)){
            i64 _val = std::get<i64>(value);
            if(_val >= INT16_MIN && _val <= INT16_MAX){
                ctx->emit_(OP_LOAD_INTEGER, (uint16_t)_val, line);
                return;
            }
            obj = VAR(_val);
        }
        if(std::holds_alternative<f64>(value)){
            obj = VAR(std::get<f64>(value));
        }
        if(std::holds_alternative<Str>(value)){
            obj = VAR(std::get<Str>(value));
        }
        if(obj == nullptr) FATAL_ERROR();
        ctx->emit_(OP_LOAD_CONST, ctx->add_const(obj), line);
    }

    void NegatedExpr::emit_(CodeEmitContext* ctx){
        VM* vm = ctx->vm;
        // if child is a int of float, do constant folding
        if(child->is_literal()){
            LiteralExpr* lit = static_cast<LiteralExpr*>(child.get());
            if(std::holds_alternative<i64>(lit->value)){
                i64 _val = -std::get<i64>(lit->value);
                if(_val >= INT16_MIN && _val <= INT16_MAX){
                    ctx->emit_(OP_LOAD_INTEGER, (uint16_t)_val, line);
                }else{
                    ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(_val)), line);
                }
                return;
            }
            if(std::holds_alternative<f64>(lit->value)){
                PyObject* obj = VAR(-std::get<f64>(lit->value));
                ctx->emit_(OP_LOAD_CONST, ctx->add_const(obj), line);
                return;
            }
        }
        child->emit_(ctx);
        ctx->emit_(OP_UNARY_NEGATIVE, BC_NOARG, line);
    }


    void SliceExpr::emit_(CodeEmitContext* ctx){
        if(start){
            start->emit_(ctx);
        }else{
            ctx->emit_(OP_LOAD_NONE, BC_NOARG, line);
        }

        if(stop){
            stop->emit_(ctx);
        }else{
            ctx->emit_(OP_LOAD_NONE, BC_NOARG, line);
        }

        if(step){
            step->emit_(ctx);
        }else{
            ctx->emit_(OP_LOAD_NONE, BC_NOARG, line);
        }

        ctx->emit_(OP_BUILD_SLICE, BC_NOARG, line);
    }

    void DictItemExpr::emit_(CodeEmitContext* ctx) {
        if(is_starred()){
            PK_ASSERT(key == nullptr);
            value->emit_(ctx);
        }else{
            value->emit_(ctx);
            key->emit_(ctx);     // reverse order
            ctx->emit_(OP_BUILD_TUPLE, 2, line);
        }
    }

    bool TupleExpr::emit_store(CodeEmitContext* ctx) {
        // TOS is an iterable
        // items may contain StarredExpr, we should check it
        int starred_i = -1;
        for(int i=0; i<items.size(); i++){
            if(!items[i]->is_starred()) continue;
            if(starred_i == -1) starred_i = i;
            else return false;  // multiple StarredExpr not allowed
        }

        if(starred_i == -1){
            Bytecode& prev = ctx->co->codes.back();
            if(prev.op == OP_BUILD_TUPLE && prev.arg == items.size()){
                // build tuple and unpack it is meaningless
                prev.op = OP_NO_OP;
                prev.arg = BC_NOARG;
            }else{
                ctx->emit_(OP_UNPACK_SEQUENCE, items.size(), line);
            }
        }else{
            // starred assignment target must be in a tuple
            if(items.size() == 1) return false;
            // starred assignment target must be the last one (differ from cpython)
            if(starred_i != items.size()-1) return false;
            // a,*b = [1,2,3]
            // stack is [1,2,3] -> [1,[2,3]]
            ctx->emit_(OP_UNPACK_EX, items.size()-1, line);
        }
        // do reverse emit
        for(int i=items.size()-1; i>=0; i--){
            bool ok = items[i]->emit_store(ctx);
            if(!ok) return false;
        }
        return true;
    }

    bool TupleExpr::emit_del(CodeEmitContext* ctx){
        for(auto& e: items){
            bool ok = e->emit_del(ctx);
            if(!ok) return false;
        }
        return true;
    }

    void CompExpr::emit_(CodeEmitContext* ctx){
        ctx->emit_(op0(), 0, line);
        iter->emit_(ctx);
        ctx->emit_(OP_GET_ITER, BC_NOARG, BC_KEEPLINE);
        ctx->enter_block(FOR_LOOP);
        ctx->emit_(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
        bool ok = vars->emit_store(ctx);
        // this error occurs in `vars` instead of this line, but...nevermind
        PK_ASSERT(ok);  // TODO: raise a SyntaxError instead
        if(cond){
            cond->emit_(ctx);
            int patch = ctx->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, BC_KEEPLINE);
            expr->emit_(ctx);
            ctx->emit_(op1(), BC_NOARG, BC_KEEPLINE);
            ctx->patch_jump(patch);
        }else{
            expr->emit_(ctx);
            ctx->emit_(op1(), BC_NOARG, BC_KEEPLINE);
        }
        ctx->emit_(OP_LOOP_CONTINUE, ctx->get_loop(), BC_KEEPLINE);
        ctx->exit_block();
    }


    void FStringExpr::_load_simple_expr(CodeEmitContext* ctx, Str expr){
        bool repr = false;
        if(expr.size>=2 && expr.end()[-2]=='!'){
            switch(expr.end()[-1]){
                case 'r': repr = true; expr = expr.substr(0, expr.size-2); break;
                case 's': repr = false; expr = expr.substr(0, expr.size-2); break;
                default: break;     // nothing happens
            }
        }
        // name or name.name
        PK_LOCAL_STATIC const std::regex pattern(R"(^[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*){0,1}$)");
        if(std::regex_match(expr.str(), pattern)){
            int dot = expr.index(".");
            if(dot < 0){
                ctx->emit_(OP_LOAD_NAME, StrName(expr.sv()).index, line);
            }else{
                StrName name(expr.substr(0, dot).sv());
                StrName attr(expr.substr(dot+1).sv());
                ctx->emit_(OP_LOAD_NAME, name.index, line);
                ctx->emit_(OP_LOAD_ATTR, attr.index, line);
            }
        }else{
            int index = ctx->add_const(py_var(ctx->vm, expr));
            ctx->emit_(OP_FSTRING_EVAL, index, line);
        }
        if(repr){
            ctx->emit_(OP_REPR, BC_NOARG, line);
        }
    }

    void FStringExpr::emit_(CodeEmitContext* ctx){
        VM* vm = ctx->vm;
        int i = 0;              // left index
        int j = 0;              // right index
        int count = 0;          // how many string parts
        bool flag = false;      // true if we are in a expression

        const char* fmt_valid_chars = "0-=*#@!~" "<>^" ".fds" "0123456789";
        PK_LOCAL_STATIC const std::set<char> fmt_valid_char_set(fmt_valid_chars, fmt_valid_chars + strlen(fmt_valid_chars));

        while(j < src.size){
            if(flag){
                if(src[j] == '}'){
                    // add expression
                    Str expr = src.substr(i, j-i);
                    // BUG: ':' is not a format specifier in f"{stack[2:]}"
                    int conon = expr.index(":");
                    if(conon >= 0){
                        Str spec = expr.substr(conon+1);
                        // filter some invalid spec
                        bool ok = true;
                        for(char c: spec) if(!fmt_valid_char_set.count(c)){ ok = false; break; }
                        if(ok){
                            _load_simple_expr(ctx, expr.substr(0, conon));
                            ctx->emit_(OP_FORMAT_STRING, ctx->add_const(VAR(spec)), line);
                        }else{
                            // ':' is not a spec indicator
                            _load_simple_expr(ctx, expr);
                        }
                    }else{
                        _load_simple_expr(ctx, expr);
                    }
                    flag = false;
                    count++;
                }
            }else{
                if(src[j] == '{'){
                    // look at next char
                    if(j+1 < src.size && src[j+1] == '{'){
                        // {{ -> {
                        j++;
                        ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR("{")), line);
                        count++;
                    }else{
                        // { -> }
                        flag = true;
                        i = j+1;
                    }
                }else if(src[j] == '}'){
                    // look at next char
                    if(j+1 < src.size && src[j+1] == '}'){
                        // }} -> }
                        j++;
                        ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR("}")), line);
                        count++;
                    }else{
                        // } -> error
                        // throw std::runtime_error("f-string: unexpected }");
                        // just ignore
                    }
                }else{
                    // literal
                    i = j;
                    while(j < src.size && src[j] != '{' && src[j] != '}') j++;
                    Str literal = src.substr(i, j-i);
                    ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(literal)), line);
                    count++;
                    continue;   // skip j++
                }
            }
            j++;
        }

        if(flag){
            // literal
            Str literal = src.substr(i, src.size-i);
            ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(literal)), line);
            count++;
        }

        ctx->emit_(OP_BUILD_STRING, count, line);
    }


    void SubscrExpr::emit_(CodeEmitContext* ctx){
        a->emit_(ctx);
        b->emit_(ctx);
        ctx->emit_(OP_LOAD_SUBSCR, BC_NOARG, line);
    }

    bool SubscrExpr::emit_del(CodeEmitContext* ctx){
        a->emit_(ctx);
        b->emit_(ctx);
        ctx->emit_(OP_DELETE_SUBSCR, BC_NOARG, line);
        return true;
    }

    bool SubscrExpr::emit_store(CodeEmitContext* ctx){
        a->emit_(ctx);
        b->emit_(ctx);
        ctx->emit_(OP_STORE_SUBSCR, BC_NOARG, line);
        return true;
    }

    void AttribExpr::emit_(CodeEmitContext* ctx){
        a->emit_(ctx);
        int index = StrName(b).index;
        ctx->emit_(OP_LOAD_ATTR, index, line);
    }

    bool AttribExpr::emit_del(CodeEmitContext* ctx) {
        a->emit_(ctx);
        int index = StrName(b).index;
        ctx->emit_(OP_DELETE_ATTR, index, line);
        return true;
    }

    bool AttribExpr::emit_store(CodeEmitContext* ctx){
        a->emit_(ctx);
        int index = StrName(b).index;
        ctx->emit_(OP_STORE_ATTR, index, line);
        return true;
    }

    void AttribExpr::emit_method(CodeEmitContext* ctx) {
        a->emit_(ctx);
        int index = StrName(b).index;
        ctx->emit_(OP_LOAD_METHOD, index, line);
    }

    void CallExpr::emit_(CodeEmitContext* ctx) {
        bool vargs = false;
        bool vkwargs = false;
        for(auto& arg: args) if(arg->is_starred()) vargs = true;
        for(auto& item: kwargs) if(item.second->is_starred()) vkwargs = true;

        // if callable is a AttrExpr, we should try to use `fast_call` instead of use `boundmethod` proxy
        if(callable->is_attrib()){
            auto p = static_cast<AttribExpr*>(callable.get());
            p->emit_method(ctx);    // OP_LOAD_METHOD
        }else{
            callable->emit_(ctx);
            ctx->emit_(OP_LOAD_NULL, BC_NOARG, BC_KEEPLINE);
        }

        if(vargs || vkwargs){
            for(auto& item: args) item->emit_(ctx);
            ctx->emit_(OP_BUILD_TUPLE_UNPACK, (uint16_t)args.size(), line);

            if(!kwargs.empty()){
                for(auto& item: kwargs){
                    if(item.second->is_starred()){
                        if(item.second->star_level() != 2) FATAL_ERROR();
                        item.second->emit_(ctx);
                    }else{
                        // k=v
                        int index = ctx->add_const(py_var(ctx->vm, item.first));
                        ctx->emit_(OP_LOAD_CONST, index, line);
                        item.second->emit_(ctx);
                        ctx->emit_(OP_BUILD_TUPLE, 2, line);
                    }
                }
                ctx->emit_(OP_BUILD_DICT_UNPACK, (int)kwargs.size(), line);
                ctx->emit_(OP_CALL_TP, 1, line);
            }else{
                ctx->emit_(OP_CALL_TP, 0, line);
            }
        }else{
            // vectorcall protocal
            for(auto& item: args) item->emit_(ctx);
            for(auto& item: kwargs){
                uint16_t index = StrName(item.first.sv()).index;
                ctx->emit_(OP_LOAD_INTEGER, index, line);
                item.second->emit_(ctx);
            }
            int KWARGC = kwargs.size();
            int ARGC = args.size();
            ctx->emit_(OP_CALL, (KWARGC<<8)|ARGC, line);
        }
    }


    bool BinaryExpr::is_compare() const {
        switch(op){
            case TK("<"): case TK("<="): case TK("=="):
            case TK("!="): case TK(">"): case TK(">="): return true;
            default: return false;
        }
    }

    void BinaryExpr::_emit_compare(CodeEmitContext* ctx, std::vector<int>& jmps){
        if(lhs->is_compare()){
            static_cast<BinaryExpr*>(lhs.get())->_emit_compare(ctx, jmps);
        }else{
            lhs->emit_(ctx); // [a]
        }
        rhs->emit_(ctx); // [a, b]
        ctx->emit_(OP_DUP_TOP, BC_NOARG, line);      // [a, b, b]
        ctx->emit_(OP_ROT_THREE, BC_NOARG, line);    // [b, a, b]
        switch(op){
            case TK("<"):   ctx->emit_(OP_COMPARE_LT, BC_NOARG, line);  break;
            case TK("<="):  ctx->emit_(OP_COMPARE_LE, BC_NOARG, line);  break;
            case TK("=="):  ctx->emit_(OP_COMPARE_EQ, BC_NOARG, line);  break;
            case TK("!="):  ctx->emit_(OP_COMPARE_NE, BC_NOARG, line);  break;
            case TK(">"):   ctx->emit_(OP_COMPARE_GT, BC_NOARG, line);  break;
            case TK(">="):  ctx->emit_(OP_COMPARE_GE, BC_NOARG, line);  break;
            default: UNREACHABLE();
        }
        // [b, RES]
        int index = ctx->emit_(OP_SHORTCUT_IF_FALSE_OR_POP, BC_NOARG, line);
        jmps.push_back(index);
    }

    void BinaryExpr::emit_(CodeEmitContext* ctx) {
        std::vector<int> jmps;
        if(is_compare() && lhs->is_compare()){
            // (a < b) < c
            static_cast<BinaryExpr*>(lhs.get())->_emit_compare(ctx, jmps);
            // [b, RES]
        }else{
            // (1 + 2) < c
            lhs->emit_(ctx);
        }

        rhs->emit_(ctx);
        switch (op) {
            case TK("+"):   ctx->emit_(OP_BINARY_ADD, BC_NOARG, line);  break;
            case TK("-"):   ctx->emit_(OP_BINARY_SUB, BC_NOARG, line);  break;
            case TK("*"):   ctx->emit_(OP_BINARY_MUL, BC_NOARG, line);  break;
            case TK("/"):   ctx->emit_(OP_BINARY_TRUEDIV, BC_NOARG, line);  break;
            case TK("//"):  ctx->emit_(OP_BINARY_FLOORDIV, BC_NOARG, line);  break;
            case TK("%"):   ctx->emit_(OP_BINARY_MOD, BC_NOARG, line);  break;
            case TK("**"):  ctx->emit_(OP_BINARY_POW, BC_NOARG, line);  break;

            case TK("<"):   ctx->emit_(OP_COMPARE_LT, BC_NOARG, line);  break;
            case TK("<="):  ctx->emit_(OP_COMPARE_LE, BC_NOARG, line);  break;
            case TK("=="):  ctx->emit_(OP_COMPARE_EQ, BC_NOARG, line);  break;
            case TK("!="):  ctx->emit_(OP_COMPARE_NE, BC_NOARG, line);  break;
            case TK(">"):   ctx->emit_(OP_COMPARE_GT, BC_NOARG, line);  break;
            case TK(">="):  ctx->emit_(OP_COMPARE_GE, BC_NOARG, line);  break;

            case TK("in"):      ctx->emit_(OP_CONTAINS_OP, 0, line);   break;
            case TK("not in"):  ctx->emit_(OP_CONTAINS_OP, 1, line);   break;
            case TK("is"):      ctx->emit_(OP_IS_OP, 0, line);         break;
            case TK("is not"):  ctx->emit_(OP_IS_OP, 1, line);         break;

            case TK("<<"):  ctx->emit_(OP_BITWISE_LSHIFT, BC_NOARG, line);  break;
            case TK(">>"):  ctx->emit_(OP_BITWISE_RSHIFT, BC_NOARG, line);  break;
            case TK("&"):   ctx->emit_(OP_BITWISE_AND, BC_NOARG, line);  break;
            case TK("|"):   ctx->emit_(OP_BITWISE_OR, BC_NOARG, line);  break;
            case TK("^"):   ctx->emit_(OP_BITWISE_XOR, BC_NOARG, line);  break;

            case TK("@"):   ctx->emit_(OP_BINARY_MATMUL, BC_NOARG, line);  break;
            default: FATAL_ERROR();
        }

        for(int i: jmps) ctx->patch_jump(i);
    }

    void TernaryExpr::emit_(CodeEmitContext* ctx){
        cond->emit_(ctx);
        int patch = ctx->emit_(OP_POP_JUMP_IF_FALSE, BC_NOARG, cond->line);
        true_expr->emit_(ctx);
        int patch_2 = ctx->emit_(OP_JUMP_ABSOLUTE, BC_NOARG, true_expr->line);
        ctx->patch_jump(patch);
        false_expr->emit_(ctx);
        ctx->patch_jump(patch_2);
    }

}   // namespace pkpy