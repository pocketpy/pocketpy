using System;
using System.Runtime.InteropServices;
using UnityEngine;

namespace pkpy
{
    public static class Bindings
    {
#if UNITY_IOS
        private const string _libName = "__Internal";
#else
        private const string _libName = "pocketpy";
#endif
        [DllImport(_libName)]
        internal static extern void pkpy_delete(IntPtr p);
        [DllImport(_libName)]
        internal static extern IntPtr pkpy_new_repl(IntPtr vm);
        [DllImport(_libName)]
        internal static extern int pkpy_repl_input(IntPtr r, string line);
        [DllImport(_libName)]
        internal static extern IntPtr pkpy_new_tvm(bool use_stdio);
        [DllImport(_libName)]
        internal static extern bool pkpy_tvm_exec_async(IntPtr vm, string source);
        [DllImport(_libName)]
        internal static extern int pkpy_tvm_get_state(IntPtr vm);
        [DllImport(_libName)]
        internal static extern string pkpy_tvm_read_jsonrpc_request(IntPtr vm);
        [DllImport(_libName)]
        internal static extern void pkpy_tvm_reset_state(IntPtr vm);
        [DllImport(_libName)]
        internal static extern void pkpy_tvm_terminate(IntPtr vm);
        [DllImport(_libName)]
        internal static extern void pkpy_tvm_write_jsonrpc_response(IntPtr vm, string value);
        [DllImport(_libName)]
        internal static extern IntPtr pkpy_new_vm(bool use_stdio);
        [DllImport(_libName)]
        internal static extern bool pkpy_vm_add_module(IntPtr vm, string name, string source);
        [DllImport(_libName)]
        internal static extern string pkpy_vm_eval(IntPtr vm, string source);
        [DllImport(_libName)]
        internal static extern bool pkpy_vm_exec(IntPtr vm, string source);
        [DllImport(_libName)]
        internal static extern string pkpy_vm_get_global(IntPtr vm, string name);
        [DllImport(_libName)]
        internal static extern string pkpy_vm_read_output(IntPtr vm);
    }
}

namespace pkpy
{
    public struct PyOutput
    {
        public string stdout;
        public string stderr;

        public PyOutput(string stdout, string stderr)
        {
            this.stdout = stdout;
            this.stderr = stderr;
        }
    }

    public class VM
    {
        public IntPtr pointer { get; private set; }

        public VM()
        {
            if(this is ThreadedVM)
            {
                pointer = Bindings.pkpy_new_tvm(false);
            }
            else
            {
                pointer = Bindings.pkpy_new_vm(false);
            }
        }

        public PyOutput read_output()
        {
            var _o = Bindings.pkpy_vm_read_output(pointer);
            return JsonUtility.FromJson<PyOutput>(_o);
        }

        public void dispose()
        {
            Bindings.pkpy_delete(pointer);
        }

        /// <summary>
        /// Add a source module into a virtual machine.  Return `true` if there is no complie error.
        /// </summary>
        public bool add_module(string name, string source)
        {
            return Bindings.pkpy_vm_add_module(pointer, name, source);
        }

        /// <summary>
        /// Evaluate an expression.  Return a json representing the result. If there is any error, return `nullptr`.
        /// </summary>
        public string eval(string source)
        {
            return Bindings.pkpy_vm_eval(pointer, source);
        }

        /// <summary>
        /// Run a given source on a virtual machine.  Return `true` if there is no compile error.
        /// </summary>
        public bool exec(string source)
        {
            return Bindings.pkpy_vm_exec(pointer, source);
        }

        /// <summary>
        /// Get a global variable of a virtual machine.  Return a json representing the result. If the variable is not found, return `nullptr`.
        /// </summary>
        public string get_global(string name)
        {
            return Bindings.pkpy_vm_get_global(pointer, name);
        }

    }

    public enum ThreadState
    {
        ready = 0,
        running,
        suspended,
        finished
    }

    public class ThreadedVM : VM
    {
        public ThreadState state => (ThreadState)Bindings.pkpy_tvm_get_state(pointer);
        
        /// <summary>
        /// Run a given source on a threaded virtual machine. The excution will be started in a new thread.  Return `true` if there is no compile error.
        /// </summary>
        public bool exec_async(string source)
        {
            return Bindings.pkpy_tvm_exec_async(pointer, source);
        }

        /// <summary>
        /// Read the current JSONRPC request from shared string buffer.
        /// </summary>
        public string read_jsonrpc_request()
        {
            return Bindings.pkpy_tvm_read_jsonrpc_request(pointer);
        }

        /// <summary>
        /// Set the state of a threaded virtual machine to `THREAD_READY`. The current state should be `THREAD_FINISHED`.
        /// </summary>
        public void reset_state()
        {
            Bindings.pkpy_tvm_reset_state(pointer);
        }

        /// <summary>
        /// Emit a KeyboardInterrupt signal to stop a running threaded virtual machine. 
        /// </summary>
        public void terminate()
        {
            Bindings.pkpy_tvm_terminate(pointer);
        }

        /// <summary>
        /// Write a JSONRPC response to shared string buffer.
        /// </summary>
        public void write_jsonrpc_response(string value)
        {
            Bindings.pkpy_tvm_write_jsonrpc_response(pointer, value);
        }

    }
}

namespace pkpy
{
    public class REPL
    {
        public IntPtr pointer { get; private set; }
        public VM vm { get; private set; }

        public REPL(VM vm)
        {
            this.vm = vm;
            pointer = Bindings.pkpy_new_repl(vm.pointer);
        }

        public void dispose()
        {
            Bindings.pkpy_delete(pointer);
        }

        /// <summary>
        /// Input a source line to an interactive console.  Return `0` if need more lines, `1` if execution happened, `2` if execution skipped (compile error or empty input).
        /// </summary>
        public int input(string line)
        {
            return Bindings.pkpy_repl_input(pointer, line);
        }

    }
}

