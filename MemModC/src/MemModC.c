/*
 ============================================================================
 Name        : MemModC.c
 Author      : Ali
 ============================================================================
 */

#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <frameobject.h>

//todo AT THE MOMENT, PASSING CONSTS RETURNS A TUPLE. NEED TO HAVE MODIFY TUPLE METHOD
//todo set up dynamic check for object type
static PyObject* modifyConsts(PyObject *self, PyObject *args)
{
	PyObject *consts;
	long index, newValue;

	if(!PyArg_ParseTuple(args, "Oll", &consts, &index, &newValue))
	{
		PyErr_Print();
	}

	printf("\nrefcnt = %d\n", consts->ob_refcnt);

	PyObject *obj = PyLong_FromLong(newValue);
	if(PyErr_Occurred())
	{
		PyErr_Print();
	}
	//todo - check this as ref count error checking not present for this call
	//PyTuple_SET_ITEM(consts, index, obj);
	if(PyErr_Occurred())
	{
		PyErr_Print();
	}

	Py_RETURN_NONE;
}

static PyObject* test3(PyObject *self, PyObject *args)
{
	PyObject *func, *code, *consts, *item;

	if(!PyArg_ParseTuple(args, "O", &func))
	{
		PyErr_Print();
	}
		
	code = ((PyFunctionObject *)func)->func_code;
	
	consts = ((PyCodeObject *)code)->co_consts;
	
	item = PyTuple_GetItem(consts, 1);
	
	char *buffer = ((char *)(PyUnicode_DATA(item)));
	
	printf("String buffer: %p\n", buffer);
	printf("String: %s\n", buffer);
	printf("String in compact base: %s\n", ((PyUnicodeObject *)item)->_base._base.wstr);
	//printf("String in ascii base: %s\n", buffer);
	printf("String length: %d\n", PyUnicode_GET_LENGTH(item));
	
	int kind = PyUnicode_KIND(item);
	
	switch(kind){
		case PyUnicode_WCHAR_KIND:
			puts("WCHAR_KIND");
			break;
		case PyUnicode_1BYTE_KIND:
			puts("1BYTE_KIND");
			break;
		case PyUnicode_2BYTE_KIND:
			puts("2BYTE_KIND");
			break;
		case PyUnicode_4BYTE_KIND:
			puts("4BYTE_KIND");
			break;
	}

	
	buffer = "This as a test";
	
	PyUnicode_READY(item);
	
	//((PyUnicodeObject *)item)->_base._base.wstr = t;
	
	char *t = "This was a test";
	
	PyObject *newString = PyUnicode_FromKindAndData(kind, t, strlen(t));
	//Py_INCREF(newString);
	//Py_DECREF(item);
	
	//newString->_ob_next = item->_ob_next;
	//newString->_ob_prev = item->_ob_prev;
	
	//*item = *newString;
	
	printf("New String buffer: %p\n", newString);
	printf("New String: %s\n", ((char *)(PyUnicode_DATA(newString))));
	printf("New String length: %d\n", PyUnicode_GET_LENGTH(newString));
	
	Py_RETURN_NONE;
}

/**
 * Note - I have to recreate the tuple object rather than just resizing as the API resize function removes the reference to
 * the original tuple (fair enough) and doesn't return new pointer.
 */
static PyObject* addIntConst(PyObject *self, PyObject *args)
{
	PyObject *codeObj, *consts, *newConsts, *frame, *func;
	PyObject* *constsPtr;
	long newConst;
	int index;

	if(!PyArg_ParseTuple(args, "Ol", &func, &newConst))
	{
		PyErr_Print();
	}
	
	printf("Func type: %s\n", func->ob_type->tp_name);
	
	//frame = ((PyFunctionObject *)func)->func_code;
	
	codeObj = ((PyFunctionObject *)func)->func_code;
	
	printf("Code type: %s\n", codeObj->ob_type->tp_name);
	
	//printf("Consts refcnt: %d\n", ((PyCodeObject *)codeObj)->co_consts->ob_refcnt);

	consts =  ((PyCodeObject *)codeObj)->co_consts;
	constsPtr = &consts;
	
	printf("Code Consts pointer: %p\n", ((PyCodeObject *)codeObj)->co_consts);
	printf("Code pointer: %p\n", ((PyCodeObject *)codeObj)->co_code);
	printf("Consts pointer: %p\n", consts);
	printf("ConstsPtr pointer: %p\n", *(constsPtr));
	//puts("SLEEP FOR OLD CONSTS");
	//sleep(10);

	Py_ssize_t oldConstsSize = PyTuple_Size(consts);
	long t = 55;
	PyObject *newPythonValue = PyLong_FromLong(t);
	Py_INCREF(newPythonValue);
	
	if(_PyTuple_Resize(constsPtr, oldConstsSize+1)<0)
	{
		puts("Resize didn't work. Aborting..");
		Py_RETURN_NONE;
	}
	
	printf("Resized Consts pointer: %p\n", consts);
	printf("Resized ConstsPtr pointer: %p\n", *(constsPtr));
	
	if(!PyTuple_SET_ITEM(consts, oldConstsSize, newPythonValue))
	{
		PyErr_Print();
	}
	
	Py_ssize_t resizedConstsSize = PyTuple_Size(consts);
	
	printf("Old consts size: %d, New consts size: %d\n", oldConstsSize, resizedConstsSize);

/*
	newConsts = PyTuple_New(oldConstsSize+1);
	
	//printf("Old consts size: %d\n", oldConstsSize);
	for(index = 0; index<oldConstsSize;index++)
	{
		if(!PyTuple_SET_ITEM(newConsts, index, PyTuple_GetItem(consts, index)))
		{
			PyErr_Print();
		}
	}

	//add new value
	//todo talk about ownership in report -
	if(!PyTuple_SET_ITEM(newConsts, oldConstsSize, newPythonValue))
	{
		PyErr_Print();
	}
*/

	/*printf("Consts pointer: %p\n", consts);
	printf("New Consts pointer: %p\n", newConsts);
	printf("Index: %d\n", index);
	printf("New value pointer: %p\n", newPythonValue);
	*/

	//point consts at new tuple and decrement reference to old tuple to allow for garbage collection
	//PyObject *oldConsts = consts; //2 references to original consts
	//((PyCodeObject *)codeObj)->co_consts = newConsts;	//1 reference to original consts

	//Py_INCREF(newConsts);
	
	/*if(!PyTuple_SET_ITEM(newConsts, 1, PyLong_FromLong(t)))
	{
		PyErr_Print();
	}*/
	
	((PyCodeObject *)codeObj)->co_consts = consts;
	
	((PyFunctionObject *)func)->func_code = codeObj;
	
	printf("func pointer: %p\n", func);
	printf("New Code Obj within func pointer: %p\n", ((PyFunctionObject *)func)->func_code);
	printf("New Code Consts pointer: %p\n", ((PyCodeObject *)codeObj)->co_consts);
	printf("Code Consts ob repr: %p\n", ((PyCodeObject *)codeObj)->co_consts->ob_type->tp_repr);

	return PyLong_FromLong(resizedConstsSize);
}

static PyObject* check(PyObject *self, PyObject *args)
{
	PyObject *consts;
	if(!PyArg_ParseTuple(args, "O", &consts))
	{
		PyErr_Print();
	}
	
	int index;
	
	printf("Consts size in check: %d\n", PyTuple_Size(consts));
	printf("Consts pointer in check: %p\n", consts);
	
	PyObject *obj;
	
	for(index = 0; index<PyTuple_Size(consts);index++)
	{
		obj = PyTuple_GetItem(consts, index);
		printf("Object at index %d in consts: Type: %s, repr: %p\n", index, obj->ob_type->tp_name, obj->ob_type->tp_repr);
	}

	Py_RETURN_NONE;
}

static PyObject* modifyByteAtOffset(PyObject *self, PyObject *args)
{
	PyObject *code;
	unsigned char newValue;
	unsigned int offset;

	if(!PyArg_ParseTuple(args, "Oib", &code, &offset, &newValue))
	{
		PyErr_Print();
	}

	char *target;

	Py_buffer view;

	//python bytesobject.c code
	PyBufferProcs *buffer = Py_TYPE(code)->tp_as_buffer;

	if (buffer == NULL || buffer->bf_getbuffer == NULL)
	{
		PyErr_Print();
	}

	if (buffer->bf_getbuffer(code, &view, PyBUF_SIMPLE) < 0)
		PyErr_Print();
	
	//index to desired byte in the buffer
	target = view.buf + offset;
	
	printf("Value at offset (-1 0 +1): %d %d %d\n", *(target-1), *(target), *(target+1));
	
	printf("Value at offset HEX (-1 0 +1): %x %x %x\n", *(target-1), *(target), *(target+1));
	
	*(target) = newValue;
	
	printf("New value at offset (-1 0 +1): %d %d %d\n", *(target-1), *(target), *(target+1));
	
	printf("New Value at offset HEX (-1 0 +1): %0x %0x %0x\n", *(target-1), *(target), *(target+1));

	PyBuffer_Release(buffer);

	Py_RETURN_NONE;
}


static PyObject* modifyInt(PyObject *self, PyObject *args)
{
	PyObject *num;

	if(!PyArg_ParseTuple(args, "O", &num))
	{
		PyErr_Print();
	}

	PyLongObject *l = (PyLongObject *) num;

	//Throw error if new value requires addition of overflow checks etc
	if(((PyVarObject *)num)->ob_size > 30000)//todo check limit, 64bit long limit?
	{
		//ERROR YO
	}
	else
	{
		l->ob_digit[0]++;
	}

	Py_RETURN_NONE;

}

static PyMethodDef MemModMethods[] =
{
	{ "modifyInt",modifyInt, METH_VARARGS | METH_KEYWORDS,"Modify Int Value" },
	{ "modifyConsts",modifyConsts, METH_VARARGS | METH_KEYWORDS,"Modify List Values" },
	{ "modifyByte",modifyByteAtOffset, METH_VARARGS | METH_KEYWORDS,"Modify Byte at Offset" },
	{ "addInt",addIntConst, METH_VARARGS | METH_KEYWORDS,"Add new integer constant" },
	{ "constsCheck",check, METH_VARARGS | METH_KEYWORDS,"Add new integer constant" },
	{ "test3",test3, METH_VARARGS | METH_KEYWORDS,"Add new integer constant" },
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef memModule =
{
		PyModuleDef_HEAD_INIT,
		"MemModC",	//module name
		NULL, 	//documentation
		-1, //state kept in global variables TODO-what!?
		MemModMethods
};

PyMODINIT_FUNC PyInit_MemModC(void)
{
	return PyModule_Create(&memModule);
}
