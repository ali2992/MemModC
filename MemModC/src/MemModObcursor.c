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

unsigned char* readIntAsString(unsigned int *target)
{
	unsigned char result[8];	//64 bits - no null bytes, temp storage!
	unsigned int bufferIndex = 0;
	
	char *bytes = ((char *)(target))+3;	//no standard pointer arithmetic as "target" is of type long!
	
	printf("Target pointer: %p, Target + offset: %p\n", target, bytes);
	
	for(; bytes>=&target; bufferIndex++)
	{
		printf("Current bytes pointer: %p\n", bytes);
		unsigned char c = *bytes;
		printf("Char at pointer: %c\n", bytes);
		bytes-=sizeof(char);
		if(!((c>='a'&& c<='z') || (c>='A' && c<='Z') || c == '_'))
		{
			c = '\0';
		}
		result[bufferIndex] = c;
	}
	
	return result;
}

static PyObject* test(PyObject *self, PyObject *args)
{
	unsigned int stream1, stream2;

	if(!PyArg_ParseTuple(args, "kkk", &stream1, &stream2))
	{
		PyErr_Print();
	}
	
	printf("Stream1 loc: %p, Stream2 loc: %p\n", &stream1, &stream2);
	
	unsigned long mask = 0xFFFFFFFFFFFFFFFF;
	
	//stream1 = stream1 ^ mask;
	//stream2 = stream2 ^ mask;
	
	char combined[17];
	
	snprintf(combined, sizeof combined, "%s%s", readIntAsString(&stream1), readIntAsString(&stream2));
	combined[16] = '\0';
	
	printf("Combined: %s\n", combined);
	
	return PyUnicode_FromString(combined);
}

static PyObject *inspect = NULL;

static PyObject* play(PyObject *self, PyObject *args)
{
	PyObject *data;

	/*
	if(!PyArg_ParseTuple(args, "O", &data))
	{
		PyErr_Print();
	}
	*/

	PyObject *insp = PyImport_ImportModule("inspect");
	if(PyErr_Occurred())
	{
		PyErr_Print();
	}
	Py_XINCREF(insp);
	inspect = insp;

	PyObject *currentFrame = PyObject_GetAttrString(inspect, "currentframe");

	PyObject *frame = PyObject_Call(currentFrame, Py_BuildValue("()"), NULL);

	PyObject *globals = ((PyFrameObject *)frame)->f_globals;

	PyObject *function = PyDict_GetItemString(globals, "b");
	Py_XINCREF(function);

	PyObject *code = ((PyFunctionObject *) function)->func_code;
	PyObject *locals = ((PyFrameObject *)frame)->f_localsplus;

	return ((PyCodeObject *) code)->co_code;

	//printf("Locals type : %s\n", locals->ob_type->tp_name);

	//Py_RETURN_NONE;

}

static void walkBackFrames(PyObject *currentFrame)
{
	printf("Pointer at start: %p\n", currentFrame);


	PyObject *ptr = currentFrame;
	int i;
	for(i=0; i<3;i++)
	{
		PyObject *prev = ((PyFrameObject *) ptr)->f_back;

		if(prev != NULL)
		{
			printf("%d frame back\n", i+1);
			PyObject *code2 = ((PyFrameObject *) prev)->f_code;

			printf("code first lineno: %d\n", ((PyCodeObject *) code2)->co_firstlineno);

			PyObject *consts = ((PyCodeObject *) code2)->co_consts;

			int numConsts = PyTuple_Size(consts);
			if(PyErr_Occurred())
			{
				PyErr_Print();
			}
			int j;
			for(j = 0; j<numConsts; j++)
			{
				PyObject *obj = PyTuple_GetItem(consts, i);
				if(PyErr_Occurred())
				{
					PyErr_Print();
				}

				printf("%s\n", obj->ob_type->tp_name);
			}

			ptr = prev;
		}
		else
		{
			puts("no back frame");
			break;
		}
	}

	printf("Pointer at end: %p\n", currentFrame);
}

static PyObject* cursor(PyObject *self, PyObject *args)
{
	PyObject *insp = PyImport_ImportModule("inspect");
	if(PyErr_Occurred())
	{
		PyErr_Print();
	}
	Py_XINCREF(insp);
	inspect = insp;
	
	PyObject *currentFrame = PyObject_GetAttrString(inspect, "currentframe");
	
	PyObject *frame = PyObject_Call(currentFrame, Py_BuildValue("()"), NULL);
	
	PyObject *info = PyObject_Call(PyObject_GetAttrString(inspect, "getframeinfo"), Py_BuildValue("(O)", frame), NULL);

	printf("info size %d\n", PyTuple_Size(info));
	int j;
	PyObject *obj = PyTuple_GetItem(info, 0);
	if(PyErr_Occurred())
	{
		PyErr_Print();
	}

	PyFrame_FastToLocals(frame);
	PyObject *locals = ((PyFrameObject *)frame)->f_locals;

	printf("Locals type: %s\n", locals->ob_type->tp_name);
	PyObject *code = ((PyFrameObject *) frame)->f_code;
	PyObject *names = ((PyCodeObject *)code)->co_names;

	PyObject *tmp = PyDict_GetItemString(locals, "storage");

	PyObject *inM = PyDict_GetItemString(locals, "inspect");
	Py_INCREF(tmp);
	Py_INCREF(inM);
	PyObject *keys = PyDict_Keys(locals);

	int e = PyDict_SetItemString(locals, "storage", inM);
	printf("STATUS CODE %d\n", e);
	PyFrame_LocalsToFast(frame, 0);
	//return tmp;

	//Py_INCREF(locals);
	//PyObject *tmp = &locals[0];
	//return locals;
	Py_RETURN_NONE;
	/*for(j = 0; j<PyTuple_Size(info); j++)
	{
		PyObject *obj = PyTuple_GetItem(info, j);
		if(PyErr_Occurred())
		{
			PyErr_Print();
		}

		printf("%s\n", obj->ob_type->tp_name);
	}/*

	/*


	PyObject *globals = ((PyFrameObject *) frame)->f_globals;
	PyObject *function = PyDict_GetItemString(globals, "test");
	Py_XINCREF(function);
	
	PyObject *code = ((PyFunctionObject *) function)->func_code;
	//PyObject *code = ((PyFrameObject *) frame)->f_code;

	printf("this code first lineno: %d\n", ((PyCodeObject *) code)->co_firstlineno);


	PyObject *consts = ((PyCodeObject *) code)->co_consts;
	int i;

	int numConsts = PyTuple_Size(consts);
	if(PyErr_Occurred())
	{
		PyErr_Print();
	}
	printf("CONSTS SIZE: %d\n, numConsts");
	for(j = 0; i<numConsts; i++)
	{
		PyObject *obj = PyTuple_GetItem(consts, i);
		if(PyErr_Occurred())
		{
			PyErr_Print();
		}

		printf("%s\n", obj->ob_type->tp_name);
	}


	//walk backwards over the frames


	//walkBackFrames(frame);

	//PyObject *globals = ((PyFrameObject *) frame)->f_globals;


	//PyObject *byteplay = PyImport_ImportModule("byteplay3");
	//Py_XINCREF(byteplay);
	//bPlay = byteplay;
	 *
	 */
}

static PyObject *bPlay = NULL;

static PyObject *LOAD_GLOBAL = NULL;

static PyObject *LOAD_CONST = NULL;

static PyObject *CALL_FUNCTION = NULL;

static PyObject *RETURN_VALUE = NULL;

static PyObject* ini(PyObject *self, PyObject *args)
{
	/*PyObject* funcCode;

	if(!PyArg_ParseTuple(args, "O", &funcCode))
	{
		PyErr_Print();
	}
	*/
	
	PyObject *byteplay = PyImport_ImportModule("byteplay3");
	Py_XINCREF(byteplay);
	bPlay = byteplay;
	
	PyObject *bPlayCodeObj = PyObject_GetAttrString(bPlay, "Code");
	
	PyObject *fromCode = PyObject_GetAttrString(bPlayCodeObj, "from_code");
	
	PyObject *insp = PyImport_ImportModule("inspect");
	if(PyErr_Occurred())
	{
		PyErr_Print();
	}
	Py_XINCREF(insp);
	inspect = insp;
	
	PyObject *currentFrame = PyObject_GetAttrString(inspect, "currentframe");
	
	PyObject *frame = PyObject_Call(currentFrame, Py_BuildValue("()"), NULL);
	
	PyObject *globals = ((PyFrameObject *) frame)->f_globals;
	PyObject *function = PyDict_GetItemString(globals, "test");
	Py_XINCREF(function);
	
	PyObject *code = ((PyFunctionObject *) function)->func_code;
	
	PyObject *result = PyObject_Call(fromCode, Py_BuildValue("(O)", code), NULL);
	
	PyObject *tmpCode = PyObject_GetAttrString(result, "code");
	
	LOAD_GLOBAL = PyObject_GetAttrString(bPlay, "LOAD_GLOBAL");
	Py_XINCREF(LOAD_GLOBAL);

	LOAD_CONST = PyObject_GetAttrString(bPlay, "LOAD_CONST");
	Py_XINCREF(LOAD_CONST);

	CALL_FUNCTION = PyObject_GetAttrString(bPlay, "CALL_FUNCTION");
	Py_XINCREF(CALL_FUNCTION);

	RETURN_VALUE = PyObject_GetAttrString(bPlay, "RETURN_VALUE");
	Py_XINCREF(RETURN_VALUE);
	
	PyObject *item1 = Py_BuildValue("(Os)", LOAD_GLOBAL, "print");
	PyObject *item2 = Py_BuildValue("(Os)", LOAD_CONST, "DUM DUM DUMMMMMM");
	PyObject *item3 = Py_BuildValue("(Oi)", CALL_FUNCTION, 1);
	PyObject *item4 = Py_BuildValue("(Os)", LOAD_CONST, NULL);
	PyObject *item5 = Py_BuildValue("(Os)", RETURN_VALUE, NULL);

	PyObject *newCode = PyList_New(0);
	PyList_Append(newCode, item1);
	PyList_Append(newCode, item2);
	PyList_Append(newCode, item3);
	PyList_Append(newCode, item4);
	PyList_Append(newCode, item5);

	int i = PyObject_SetAttrString(result, "code", newCode);
	//TODO check error codes and status for everything!!

	PyObject *toCode = PyObject_GetAttrString(result, "to_code");
	
	printf("%d\n", 1);
	Py_XINCREF(result);
	PyObject *argList = Py_BuildValue("()", NULL);
	if(PyErr_Occurred())
	{
		PyErr_Print();
	}
	Py_XINCREF(argList);
	PyObject *test = PyObject_CallMethod(result, "to_code", "()", argList);
	if(PyErr_Occurred())
	{
		PyErr_Print();
	}
	Py_XDECREF(argList);
	((PyFunctionObject *) function)->func_code = test;

	/*	
	//puts("SLEEP FOR byteplay");
	//sleep(10);
	*/
	//return test;
	Py_RETURN_NONE;
}
	
static PyObject* fcreate(PyObject *self, PyObject *args)
{
	char *name;

	if(!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_Print();
	}
	
	FILE *fp;
	
	fp = fopen(name, "w+");
	fprintf(fp, "%s", "TEST");
	
	fclose(fp);
	
	
	Py_RETURN_NONE;
}

static PyMethodDef MemModMethods[] =
{
	{ "test",test, METH_VARARGS | METH_KEYWORDS,"TEST" },
	{ "fcreate",fcreate, METH_VARARGS | METH_KEYWORDS,"TEST" },
	{ "ini",ini, METH_VARARGS | METH_KEYWORDS,"TEST" },
	{ "play",play, METH_VARARGS | METH_KEYWORDS,"TEST" },
	{ "cursor",cursor, METH_VARARGS | METH_KEYWORDS,"TEST" },
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef memModule =
{
		PyModuleDef_HEAD_INIT,
		"MemModOb",	//module name
		NULL, 	//documentation
		-1, //state kept in global variables TODO-what!?
		MemModMethods
};

PyMODINIT_FUNC PyInit_MemModOb(void)
{
	return PyModule_Create(&memModule);
}
