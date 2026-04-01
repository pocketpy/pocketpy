@echo off
REM Batch script to build pocketpy with GCC on Windows

cd /d "%~dp0"

echo Compiling pocketpy...

set "FLAGS=-std=c11 -Iinclude -O2 -DPK_ENABLE_OS=1"
set "LIBS=-lm -lws2_32 -lkernel32 -liphlpapi -lstdc++"

REM Compile all source files
gcc %FLAGS% ^
    src/bindings/py_array.c ^
    src/bindings/py_mappingproxy.c ^
    src/bindings/py_method.c ^
    src/bindings/py_number.c ^
    src/bindings/py_object.c ^
    src/bindings/py_property.c ^
    src/bindings/py_range.c ^
    src/bindings/py_str.c ^
    src/common/algorithm.c ^
    src/common/chunkedvector.c ^
    src/common/dmath.c ^
    src/common/memorypool.c ^
    src/common/name.c ^
    src/common/serialize.c ^
    src/common/smallmap.c ^
    src/common/socket.c ^
    src/common/sourcedata.c ^
    src/common/sstream.c ^
    src/common/str.c ^
    src/common/threads.c ^
    src/common/vector.c ^
    src/common/_generated.c ^
    src/compiler/compiler.c ^
    src/compiler/lexer.c ^
    src/debugger/core.c ^
    src/debugger/dap.c ^
    src/interpreter/ceval.c ^
    src/interpreter/dll.c ^
    src/interpreter/frame.c ^
    src/interpreter/generator.c ^
    src/interpreter/heap.c ^
    src/interpreter/line_profiler.c ^
    src/interpreter/objectpool.c ^
    src/interpreter/py_compile.c ^
    src/interpreter/typeinfo.c ^
    src/interpreter/vm.c ^
    src/interpreter/vmx.c ^
    src/modules/array2d.c ^
    src/modules/base64.c ^
    src/modules/builtins.c ^
    src/modules/colorcvt.c ^
    src/modules/conio.c ^
    src/modules/dis.c ^
    src/modules/easing.c ^
    src/modules/enum.c ^
    src/modules/gc.c ^
    src/modules/importlib.c ^
    src/modules/inspect.c ^
    src/modules/json.c ^
    src/modules/lz4.c ^
    src/modules/math.c ^
    src/modules/os.c ^
    src/modules/pickle.c ^
    src/modules/picoterm.c ^
    src/modules/pkpy.c ^
    src/modules/random.c ^
    src/modules/stdc.c ^
    src/modules/time.c ^
    src/modules/traceback.c ^
    src/modules/unicodedata.c ^
    src/modules/vmath.c ^
    src/objects/bintree.c ^
    src/objects/codeobject.c ^
    src/objects/codeobject_ser.c ^
    src/objects/container.c ^
    src/objects/namedict.c ^
    src/objects/object.c ^
    src/public/Bindings.c ^
    src/public/CodeExecution.c ^
    src/public/DictSlots.c ^
    src/public/FrameOps.c ^
    src/public/GlobalSetup.c ^
    src/public/Inspection.c ^
    src/public/ModuleSystem.c ^
    src/public/PyDict.c ^
    src/public/PyException.c ^
    src/public/PyList.c ^
    src/public/PySlice.c ^
    src/public/PythonOps.c ^
    src/public/PyTuple.c ^
    src/public/StackOps.c ^
    src/public/TypeSystem.c ^
    src/public/ValueCast.c ^
    src/public/ValueCreation.c ^
    src2/main.c ^
    %LIBS% ^
    -o pocketpy.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful! Created pocketpy.exe
    exit /b 0
) else (
    echo Build failed!
    exit /b 1
)
