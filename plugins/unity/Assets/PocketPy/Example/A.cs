using UnityEngine;
using TMPro;

public class A : MonoBehaviour
{
    TextMeshProUGUI text;
    pkpy.VM vm;

    // Start is called before the first frame update
    void Start()
    {
        text = GetComponent<TextMeshProUGUI>();
        Application.targetFrameRate = 60;

        try
        {
            vm = new pkpy.VM();
            vm.exec("a = 0");
        }
        catch (System.Exception e)
        {
            text.text = e.StackTrace.ToString() + e.GetType().ToString()+ ": " + e.Message.ToString();
            throw;
        }
    }

    // Update is called once per frame
    void Update()
    {
        if(vm == null)
            return;
        vm.exec("a += 1");
        text.text = vm.eval("a");
    }
}
