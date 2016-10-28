/*
 ============================================================================
 Name        : Example1.c
 Author      : Alastair Sumpter
 ============================================================================
 */

#include <Python.h>
#include <stdio.h>
#include <stdlib.h>

//used for debugging
#define errorCheck() if(PyErr_Occurred()){ printf("Error at line %d\n", __LINE__); PyErr_Print(); }


/**
* Given the Python object representing local constansts to a function, increment the value held
* at the location pointed to by the second item. Specifically assumes that the second object pointer
* in the tuple is a number object (PyLong) and thus is NOT A GENERIC MODIFICATION FUNCTION.
* Tailored specifically to be used with example1-memoryaddresschange.py, see there for more detail.
*
* Note, this could have easily been done in the Python script, however it is a good introductory example
* as to how C library calls work in Python.
*/
static PyObject* modifyList(PyObject *self, PyObject *args)
{
	PyObject *consts;

	if(!PyArg_ParseTuple(args, "O", &consts))
	{
		PyErr_Print();
	}

	PyObject *obj = PyTuple_GetItem(consts, 1);
	errorCheck()
	
	((PyLongObject *) obj)->ob_digit[0]+=1;

	Py_RETURN_NONE;
}

/**
* Used for mapping Python module calls to the corresponding C library calls
*/
static PyMethodDef MemModMethods[] =
{
	{ "modifyList",modifyList, METH_VARARGS | METH_KEYWORDS,"Modify List Values" },
	{ NULL, NULL, 0, NULL }
};

/**
* Used for compiling the python module
*/
static struct PyModuleDef memModule =
{
		PyModuleDef_HEAD_INIT,
		"Example1",	//module name
		NULL, 	//documentation
		-1, //state kept in global variables TODO-what!?
		MemModMethods
};

//Python module initialiser
PyMODINIT_FUNC PyInit_Example1(void)
{
	return PyModule_Create(&memModule);
}
