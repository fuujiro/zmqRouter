#include <iostream>  
#include <Python.h>  
  
using namespace std;  

void testClient();
  
int main()  
{  
    cout << "Starting Test..." << endl;  

    testClient();  
  
    system("pause");  
    return 0;  
}  

void testClient() {
	Py_Initialize();              //使用python之前，要调用Py_Initialize();这个函数进行初始化  
	PyObject * pModule = NULL;    //声明变量  
    PyObject * pFunc = NULL;      //声明变量  
	pModule =PyImport_ImportModule("client");      //client:Python文件名  
    pFunc= PyObject_GetAttrString(pModule, "test");  //test:Python文件中的函数名  
	PyObject *pArgs = PyTuple_New(1);               //函数调用的参数传递均是以元组的形式打包的,1表示参数个数  
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("i", 1));//0---序号  i表示创建int型变量    
    
	PyEval_CallObject(pFunc, pArgs);                         //调用函数,NULL表示参数为空  

    Py_DECREF(pModule);
    Py_DECREF(pFunc);
    Py_DECREF(pArgs);

    Py_Finalize();                //调用Py_Finalize,这个和Py_Initialize相对应的.  
}
