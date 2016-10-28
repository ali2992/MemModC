/*
 ============================================================================
 Name        : Example2.c
 Author      : Alastair Sumpter
 ============================================================================
 */

#include <Python.h>
#include <stdio.h>
#include <stdlib.h>

//used for debugging
#define errorCheck() if(PyErr_Occurred()){ printf("Error at line %d\n", __LINE__); PyErr_Print(); }

/**
* Given a python code object, desired offset and
* new value to be set, this function will set the value at offset to the new value.
* No new code object is created.
*/
static PyObject* modifyByteAtOffset(PyObject *self, PyObject *args)
{
	PyObject *code;
	long newValue;
	int offset;
	
	puts("Trigger hit, modifying loop bytecode..");

	//parse the arguments to the python function into C equivalents
	if(!PyArg_ParseTuple(args, "Oil", &code, &offset, &newValue))
	{
		PyErr_Print();
	}

	char *target;
	Py_buffer view;

	//borrowed from python bytesobject.c
	PyBufferProcs *buffer = Py_TYPE(code)->tp_as_buffer;
	if (buffer == NULL || buffer->bf_getbuffer == NULL)
	{
		PyErr_Print();
	}
	//

	//initialise buffer
	if (buffer->bf_getbuffer(code, &view, PyBUF_SIMPLE) < 0)
		printf("\nERROR\n");

	printf("Buffer pointer: %p\n",view.buf);
	
	target = view.buf + offset;
	
	printf("Offset: %d\n", offset);
	
	printf("Offset pointer: %p\n", (target));
	printf("Value at offset: %d\n", *(target));
	
	printf("Changing Value to OPCODE for INPLACE_SUBTRACT..\n");
	
	//reassign value at target
	*(target) = newValue;
	
	printf("New value at offset: %d\n", *(target));

	PyBuffer_Release(&view);
	
	puts("Resuming loop..");

	Py_RETURN_NONE;
}

/**
* Used for mapping Python module calls to the corresponding C library calls
*/
static PyMethodDef MemModMethods[] =
{
	{ "modifyByte",modifyByteAtOffset, METH_VARARGS | METH_KEYWORDS,"Modify Byte at Offset" },
	{ NULL, NULL, 0, NULL }
};

/**
* Used for compiling the python module
*/
static struct PyModuleDef memModule =
{
		PyModuleDef_HEAD_INIT,
		"Example2",	//module name
		NULL, 	//documentation
		-1,
		MemModMethods
};

//Python module initialiser
PyMODINIT_FUNC PyInit_Example2(void)
{
	return PyModule_Create(&memModule);
}
