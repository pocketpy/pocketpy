// #include "pocketpy/interpreter/vm.h"
// #include "pocketpy/objects/base.h"

// void pkpy_VM__ctor(pkpy_VM* self){
//     self->True = (PyVar){
//         .type=tp_bool,
//         .is_ptr=true,
//         .extra=1,
//         ._obj=pkpy_VM__gcnew(self, tp_bool)
//     };

//     self->False = (PyVar){
//         .type=tp_bool,
//         .is_ptr=true,
//         .extra=0,
//         ._obj=pkpy_VM__gcnew(self, tp_bool)
//     };

//     self->None = (PyVar){
//         .type=tp_none_type,
//         .is_ptr=true,
//         ._obj=pkpy_VM__gcnew(self, tp_none_type)
//     };

//     self->NotImplemented = (PyVar){
//         .type=tp_not_implemented_type,
//         .is_ptr=true,
//         ._obj=pkpy_VM__gcnew(self, tp_not_implemented_type)
//     };

//     self->Ellipsis = (PyVar){
//         .type=tp_ellipsis,
//         .is_ptr=true,
//         ._obj=pkpy_VM__gcnew(self, tp_ellipsis)
//     };
// }

// void pkpy_VM__dtor(pkpy_VM* self){

// }