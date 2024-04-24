#include "pocketpy/expr.h"

namespace pkpy{

    inline bool is_identifier(std::string_view s){
        if(s.empty()) return false;
        if(!isalpha(s[0]) && s[0] != '_') return false;
        for(char c: s) if(!isalnum(c) && c != '_') return false;
        return true;
    }

    int CodeEmitContext::get_loop() const {
        int index = curr_block_i;
        while(index >= 0){
            if(co->blocks[index].type == CodeBlockType::FOR_LOOP) break;
            if(co->blocks[index].type == CodeBlockType::WHILE_LOOP) break;
            index = co->blocks[index].parent;
        }
        return index;
    }

    CodeBlock* CodeEmitContext::enter_block(CodeBlockType type){
        if(type==CodeBlockType::FOR_LOOP || type==CodeBlockType::CONTEXT_MANAGER) base_stack_size++;
        co->blocks.push_back(CodeBlock(
            type, curr_block_i, base_stack_size, (int)co->codes.size()
        ));
        curr_block_i = co->blocks.size()-1;
        return &co->blocks[curr_block_i];
    }

    void CodeEmitContext::exit_block(){
        auto curr_type = co->blocks[curr_block_i].type;
        if(curr_type == CodeBlockType::FOR_LOOP || curr_type==CodeBlockType::CONTEXT_MANAGER) base_stack_size--;
        co->blocks[curr_block_i].end = co->codes.size();
        curr_block_i = co->blocks[curr_block_i].parent;
        if(curr_block_i < 0) PK_FATAL_ERROR();

        if(curr_type == CodeBlockType::FOR_LOOP){
            // add a no op here to make block check work
            emit_(OP_NO_OP, BC_NOARG, BC_KEEPLINE, true);
        }
    }

    // clear the expression stack and generate bytecode
    void CodeEmitContext::emit_expr(){
        if(s_expr.size() != 1) throw std::runtime_error("s_expr.size() != 1");
        Expr_ expr = s_expr.popx();
        expr->emit_(this);
    }

    int CodeEmitContext::emit_(Opcode opcode, uint16_t arg, int line, bool is_virtual) {
        co->codes.push_back(Bytecode{(uint8_t)opcode, arg});
        co->iblocks.push_back(curr_block_i);
        co->lines.push_back(CodeObject::LineInfo{line, is_virtual});
        int i = co->codes.size() - 1;
        if(line == BC_KEEPLINE){
            if(i >= 1) co->lines[i].lineno = co->lines[i-1].lineno;
            else co->lines[i].lineno = 1;
        }
        return i;
    }

    void CodeEmitContext::revert_last_emit_(){
        co->codes.pop_back();
        co->iblocks.pop_back();
        co->lines.pop_back();
    }

    void CodeEmitContext::try_merge_for_iter_store(int i){
        // [FOR_ITER, STORE_?, ]
        if(co->codes[i].op != OP_FOR_ITER) return;
        if(co->codes.size() - i != 2) return;
        uint16_t arg = co->codes[i+1].arg;
        if(co->codes[i+1].op == OP_STORE_FAST){
            revert_last_emit_();
            co->codes[i].op = OP_FOR_ITER_STORE_FAST;
            co->codes[i].arg = arg;
            return;
        }
        if(co->codes[i+1].op == OP_STORE_GLOBAL){
            revert_last_emit_();
            co->codes[i].op = OP_FOR_ITER_STORE_GLOBAL;
            co->codes[i].arg = arg;
            return;
        }
    }

    int CodeEmitContext::emit_int(i64 value, int line){
        if(value >= 0 && value <= 16){
            uint8_t op = OP_LOAD_INT_0 + (uint8_t)value;
            return emit_((Opcode)op, BC_NOARG, line);
        }else{
            return emit_(OP_LOAD_CONST, add_const(VAR(value)), line);
        }
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

    int CodeEmitContext::add_const_string(std::string_view key){
        auto it = _co_consts_string_dedup_map.find(key);
        if(it != _co_consts_string_dedup_map.end()){
            return it->second;
        }else{
            co->consts.push_back(VAR(key));
            int index = co->consts.size() - 1;
            _co_consts_string_dedup_map[std::string(key)] = index;
            return index;
        }
    }

    int CodeEmitContext::add_const(PyObject* v){
        if(is_type(v, vm->tp_str)){
            // warning: should use add_const_string() instead
            return add_const_string(PK_OBJ_GET(Str, v).sv());
        }else{
            // non-string deduplication
            auto it = _co_consts_nonstring_dedup_map.find(v);
            if(it != _co_consts_nonstring_dedup_map.end()){
                return it->second;
            }else{
                co->consts.push_back(v);
                int index = co->consts.size() - 1;
                _co_consts_nonstring_dedup_map[v] = index;
                return index;
            }
        }
        PK_UNREACHABLE()
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
            default: PK_FATAL_ERROR(); break;
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
            default: PK_FATAL_ERROR(); break;
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
            default: PK_FATAL_ERROR();
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
        if(std::holds_alternative<i64>(value)){
            i64 _val = std::get<i64>(value);
            ctx->emit_int(_val, line);
            return;
        }
        if(std::holds_alternative<f64>(value)){
            f64 _val = std::get<f64>(value);
            ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(_val)), line);
            return;
        }
        if(std::holds_alternative<Str>(value)){
            std::string_view key = std::get<Str>(value).sv();
            ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(key), line);
            return;
        }
    }

    void NegatedExpr::emit_(CodeEmitContext* ctx){
        VM* vm = ctx->vm;
        // if child is a int of float, do constant folding
        if(child->is_literal()){
            LiteralExpr* lit = static_cast<LiteralExpr*>(child.get());
            if(std::holds_alternative<i64>(lit->value)){
                i64 _val = -std::get<i64>(lit->value);
                ctx->emit_int(_val, line);
                return;
            }
            if(std::holds_alternative<f64>(lit->value)){
                f64 _val = -std::get<f64>(lit->value);
                ctx->emit_(OP_LOAD_CONST, ctx->add_const(VAR(_val)), line);
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
        ctx->enter_block(CodeBlockType::FOR_LOOP);
        int for_codei = ctx->emit_(OP_FOR_ITER, BC_NOARG, BC_KEEPLINE);
        bool ok = vars->emit_store(ctx);
        // this error occurs in `vars` instead of this line, but...nevermind
        if(!ok) throw std::runtime_error("SyntaxError");
        ctx->try_merge_for_iter_store(for_codei);
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
        bool is_fastpath = false;
        if(is_identifier(expr.sv())){
            ctx->emit_(OP_LOAD_NAME, StrName(expr.sv()).index, line);
            is_fastpath = true;
        }else{
            int dot = expr.index(".");
            if(dot > 0){
                std::string_view a = expr.sv().substr(0, dot);
                std::string_view b = expr.sv().substr(dot+1);
                if(is_identifier(a) && is_identifier(b)){
                    ctx->emit_(OP_LOAD_NAME, StrName(a).index, line);
                    ctx->emit_(OP_LOAD_ATTR, StrName(b).index, line);
                    is_fastpath = true;
                }
            }
        }
        
        if(!is_fastpath){
            int index = ctx->add_const_string(expr.sv());
            ctx->emit_(OP_FSTRING_EVAL, index, line);
        }

        if(repr){
            ctx->emit_(OP_REPR, BC_NOARG, line);
        }
    }

    void FStringExpr::emit_(CodeEmitContext* ctx){
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
                            ctx->emit_(OP_FORMAT_STRING, ctx->add_const_string(spec.sv()), line);
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
                        ctx->emit_(OP_LOAD_CONST, ctx->add_const_string("{"), line);
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
                        ctx->emit_(OP_LOAD_CONST, ctx->add_const_string("}"), line);
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
                    ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(literal.sv()), line);
                    count++;
                    continue;   // skip j++
                }
            }
            j++;
        }

        if(flag){
            // literal
            Str literal = src.substr(i, src.size-i);
            ctx->emit_(OP_LOAD_CONST, ctx->add_const_string(literal.sv()), line);
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
        ctx->emit_(OP_LOAD_ATTR, b.index, line);
    }

    bool AttribExpr::emit_del(CodeEmitContext* ctx) {
        a->emit_(ctx);
        ctx->emit_(OP_DELETE_ATTR, b.index, line);
        return true;
    }

    bool AttribExpr::emit_store(CodeEmitContext* ctx){
        a->emit_(ctx);
        ctx->emit_(OP_STORE_ATTR, b.index, line);
        return true;
    }

    void AttribExpr::emit_method(CodeEmitContext* ctx) {
        a->emit_(ctx);
        ctx->emit_(OP_LOAD_METHOD, b.index, line);
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
                        PK_ASSERT(item.second->star_level() == 2)
                        item.second->emit_(ctx);
                    }else{
                        // k=v
                        int index = ctx->add_const_string(item.first.sv());
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
            // vectorcall protocol
            for(auto& item: args) item->emit_(ctx);
            for(auto& item: kwargs){
                i64 _val = StrName(item.first.sv()).index;
                ctx->emit_int(_val, line);
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

    void BinaryExpr::_emit_compare(CodeEmitContext* ctx, pod_vector<int>& jmps){
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
            default: PK_UNREACHABLE()
        }
        // [b, RES]
        int index = ctx->emit_(OP_SHORTCUT_IF_FALSE_OR_POP, BC_NOARG, line);
        jmps.push_back(index);
    }

    void BinaryExpr::emit_(CodeEmitContext* ctx) {
        pod_vector<int> jmps;
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
            default: PK_FATAL_ERROR();
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