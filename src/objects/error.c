// #include "pocketpy/objects/error.h"
// #include "pocketpy/common/strname.h"
// #include "pocketpy/common/sstream.h"

// void pkpy_Exception__ctor(pkpy_Exception* self, py_Name type){
//     self->type = type;
//     self->is_re = true;
//     self->_ip_on_error = -1;
//     self->_code_on_error = NULL;
//     self->self = NULL;

//     py_Str__ctor(&self->msg, "");
//     c11_vector__ctor(&self->stacktrace, sizeof(pkpy_ExceptionFrame));
// }

// void pkpy_Exception__dtor(pkpy_Exception* self){
//     for(int i=0; i<self->stacktrace.count; i++){
//         pkpy_ExceptionFrame* frame = c11__at(pkpy_ExceptionFrame, &self->stacktrace, i);
//         PK_DECREF(frame->src);
//         py_Str__dtor(&frame->name);
//     }
//     py_Str__dtor(&self->msg);
//     c11_vector__dtor(&self->stacktrace);
// }

// void pkpy_Exception__stpush(pkpy_Exception* self, pk_SourceData_ src, int lineno, const char* cursor, const char* name){
//     if(self->stacktrace.count >= 7) return;
//     PK_INCREF(src);
//     pkpy_ExceptionFrame* frame = c11_vector__emplace(&self->stacktrace);
//     frame->src = src;
//     frame->lineno = lineno;
//     frame->cursor = cursor;
//     py_Str__ctor(&frame->name, name);
// }

// py_Str pkpy_Exception__summary(pkpy_Exception* self){
//     c11_sbuf ss;
//     c11_sbuf__ctor(&ss);

//     if(self->is_re){
//         c11_sbuf__write_cstr(&ss, "Traceback (most recent call last):\n");
//     }
//     for(int i=self->stacktrace.count-1; i >= 0; i--) {
//         pkpy_ExceptionFrame* frame = c11__at(pkpy_ExceptionFrame, &self->stacktrace, i);
//         py_Str s = pk_SourceData__snapshot(frame->src, frame->lineno, frame->cursor, py_Str__data(&frame->name));
//         c11_sbuf__write_Str(&ss, &s);
//         py_Str__dtor(&s);
//         c11_sbuf__write_cstr(&ss, "\n");
//     }

//     const char* name = py_Name__rmap(self->type);
//     c11_sbuf__write_cstr(&ss, name);

//     if(self->msg.size > 0){
//         c11_sbuf__write_cstr(&ss, ": ");
//         c11_sbuf__write_Str(&ss, &self->msg);
//     }
//     return c11_sbuf__submit(&ss);
// }