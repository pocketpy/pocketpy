using PlasticPipe.PlasticProtocol.Messages;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace pkpy
{
    public abstract class JsonRpcServer
    {
        protected abstract Task<string> dispatch(ThreadedVM vm, string request);

        public async Task attach(ThreadedVM vm)
        {
            while (vm.state <= ThreadState.running) await Task.Yield();
            switch (vm.state)
            {
                case ThreadState.suspended:
                    string request = vm.read_jsonrpc_request();
                    string response = await dispatch(vm, request);
                    vm.write_jsonrpc_response(response);
                    await attach(vm);
                    break;
                case ThreadState.finished:
                    break;
                default:
                    throw new Exception("Unexpected state");
            }
        }
    }
}