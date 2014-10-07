/*
 ============================================================================
 Name        : MemModC.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <Python.h>
#include <frameobject.h>

static PyObject* test_call(PyObject *self, PyObject *args)
{
	int i;
	for (i = 0; i < 12; i++)
	{
		printf("%d", i);

		PyThreadState *threadstate = PyThreadState_Get();

		PyCodeObject *code = NULL;//TODO add this, in evalcode it is passed in to the method, has to come from somewhere

		PyObject *globals, *locals; //TODO also provided by caller to evalcode

		PyFrameObject *frame = PyFrame_New(PyThreadState_Get(), code, globals, locals);

	}

	return PyLong_FromLong(i);
}

static PyMethodDef MemModMethods[] =
{
	{ "test",test_call, METH_VARARGS,"Test Me" },
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef memModule =
{
		PyModuleDef_HEAD_INIT,
		"MemMod",	//module name
		NULL, 	//documentation
		-1, //state kept in global variables TODO-what!?
		MemModMethods
};

PyMODINIT_FUNC PyInit_MemModC(void)
{
	return PyModule_Create(&memModule);
}
