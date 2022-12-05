using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using pkpy;

public class A : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        var vm = new pkpy.VM();
        vm.exec("print('Hello world')");
        PyOutput o = vm.read_output();
        Debug.Log(o.stdout.Trim());
        //Debug.Log(o.stderr);
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
