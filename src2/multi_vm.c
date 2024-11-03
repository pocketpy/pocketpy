#include "pocketpy.h"

int main()
{
    py_initialize();

    bool ok = py_exec("print('Hello world from VM0!')", "<string1>", EXEC_MODE, NULL);
    if(!ok){
        py_printexc();
        return 1;
    }

	//This line will cause assert error in Debug build and crash (exception) in Release build
    py_switchvm(1); 

    ok = py_exec("print('Hello world from VM1!')", "<string2>", EXEC_MODE, NULL);
    if(!ok){
        py_printexc();
        return 1;
    }

    py_switchvm(0);
    ok = py_exec("print('Hello world from VM0 again!')", "<string3>", EXEC_MODE, NULL);
    if(!ok){
        py_printexc();
        return 1;
    }

    py_finalize();
	
	return 0;
}