/*
 ============================================================================
 Name        : fakeplatform.c
 Author      : Alastair Sumpter
 ============================================================================
 */

#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <frameobject.h>

#define errorCheck() if(PyErr_Occurred()){ printf("Error at line %d\n", __LINE__); PyErr_Print(); }

#define appendList(list, item) appendToList(list, item);errorCheck()

//would be an idea to set this dynamically in case /tmp is unavilable.
static char *LIBFILENAME = "/tmp/platform.so";

static char *BPLAYFILENAME = "/tmp/byteplay3.py";

static PyObject *bPlay = NULL;

static PyObject *LOAD_GLOBAL = NULL;

static PyObject *POP_TOP = NULL;

static PyObject *LOAD_CONST = NULL;

static PyObject *LOAD_FAST = NULL;

static PyObject *LOAD_ATTR = NULL;

static PyObject *CALL_FUNCTION = NULL;

static PyObject *RETURN_VALUE = NULL;

static PyObject *STORE_MAP = NULL;

static PyObject *STORE_FAST = NULL;

static PyObject *STORE_SUBSCR = NULL;

static PyObject *BUILD_MAP = NULL;

static PyObject *BUILD_LIST = NULL;

static PyObject *SetLineno = NULL;

static PyObject *inspect = NULL;

/**
* Used for debugging
*/
static void objType(PyObject *obj, char *name)
{
	printf("Type of %s: %s\n", name, obj->ob_type->tp_name);
}

/**
* Used for debugging
*/
static void refcnt(PyObject *obj, char *name)
{
	printf("Refcnt of %s: %d\n", name, obj->ob_refcnt);
}

static printObj(PyObject *obj)
{
	PyObject_Print(obj, stdout, 0);
	puts("");	//why make a print function with no embedded newline guiddo?
}

/**
* Import opcodes
*/
static void importOps()
{
	PyObject *byteplay = PyImport_ImportModule("byteplay3");
	Py_XINCREF(byteplay);
	bPlay = byteplay;

	LOAD_GLOBAL = PyObject_GetAttrString(bPlay, "LOAD_GLOBAL");
	Py_XINCREF(LOAD_GLOBAL);

	LOAD_CONST = PyObject_GetAttrString(bPlay, "LOAD_CONST");
	Py_XINCREF(LOAD_CONST);
	
	POP_TOP = PyObject_GetAttrString(bPlay, "POP_TOP");
	Py_XINCREF(POP_TOP);
	
	LOAD_ATTR = PyObject_GetAttrString(bPlay, "LOAD_ATTR");
	Py_XINCREF(LOAD_ATTR);

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

/**
* Import necessary libraries if not already present
*/
static void checkImports()
{
	if(!inspect)
	{
		PyObject *insp = PyImport_ImportModule("inspect");
		if(PyErr_Occurred())
		{
			PyErr_Print();
		}
		inspect = insp;
	}
	
	if(!bPlay)
	{
		importOps();
	}
}

static PyObject* getCurrentFrame()
{
	checkImports();
	
	PyObject *currentFrame = PyObject_GetAttrString(inspect, "currentframe");
	
	PyObject *frame = PyObject_Call(currentFrame, Py_BuildValue("()"), NULL);
	
	return frame;
}

/*
 * remove the fake module entry from global namespace and invalidate the cache entry.
 * this causes the import process to search the 'syspath' again, rather than reloading
 * the module from cache
 */
static PyObject *loadRealPlatform()
{
	PyObject *frame = getCurrentFrame();
	PyObject *globals = ((PyFrameObject *)frame)->f_globals;
	PyObject *locals = ((PyFrameObject *)frame)->f_locals;
	PyObject *sysModule = PyDict_GetItemString(globals, "sys");
	PyObject *sysDict = PyModule_GetDict(sysModule);	
	PyObject *sysPath = PyDict_GetItemString(sysDict, "path");
	PyDict_SetItemString(sysDict, "path", PyList_GetSlice(sysPath, 1, PyList_Size(sysPath)));
	
	PyDict_DelItemString(locals, "platform");
	errorCheck()
	PyDict_DelItemString(PyDict_GetItemString(sysDict, "modules"), "platform");
	errorCheck()
	PyObject *platformModule = PyImport_ImportModule("platform");
	PyDict_SetItemString(locals, "platform", platformModule);
	
	//reload the local variables to persist the changes past this function
	PyFrame_LocalsToFast(frame, 0);
	
	return platformModule;
}

/**
* Remove all traces of the code.
* Note, unlinks the library from filesystem - does not guarantee deletion
*/
static void cleanUp()
{
	PyObject *key, *value;
	Py_ssize_t index = 0;
	
	PyObject *globals = ((PyFrameObject *)getCurrentFrame())->f_globals;
	PyObject *locals = ((PyFrameObject *)getCurrentFrame())->f_locals;
	
	Py_INCREF(locals);
	Py_INCREF(globals);
	
	PyObject *currentModule = NULL;

	while(PyDict_Next(globals, &index, &key, &value))
	{
		errorCheck()
		if(PyModule_Check(value))
		{
			errorCheck()
			if(PyDict_Contains(PyModule_GetDict(value), PyUnicode_FromString("__docb__")))
			{
				errorCheck()
				currentModule = value;
				break;
			}
		}
	}
	//if no match found then module must still be in original place
	if(currentModule == NULL)
	{
		loadRealPlatform();
		errorCheck()
	}
	else
	{
		PyObject *realModule = PyDict_GetItemString(PyModule_GetDict(currentModule), "__doca__");
		PyDict_SetItemString(globals, PyModule_GetName(realModule), realModule);
		errorCheck()
		
		//remove reference to old dict
		if(PyDict_Contains(PyModule_GetDict(currentModule), PyUnicode_FromString("__tm__")))
		{
			PyObject *fakeModule = PyDict_GetItemString(globals, "__tm__");
			Py_DECREF(fakeModule);
			PyDict_DelItemString(globals, "__tm__");
		}
	}
	
	
	//remove library file
	if(access(LIBFILENAME, F_OK ) != -1) {
    	if(remove(LIBFILENAME))
    		printf("Error removing library: %s\n", strerror(errno));
	}
	
	if(access(BPLAYFILENAME, F_OK ) != -1) {
    	if(remove(BPLAYFILENAME))
    		printf("Error removing byteplay: %s\n", strerror(errno));
	}
}

/**
* Walk back up the call stack looking for function calls originating from pdb or bdb
* Does not catch custom debuggers or C debuggers
* EXTENDED TO ALSO LOOK FOR OCCURENCES OF PDB IN LOCALS
*/
static unsigned char debuggerCheck()
{
	checkImports();
	
	PyObject *frames = PyObject_Call(PyObject_GetAttrString(inspect, "stack"), Py_BuildValue("()"), NULL);
	
	unsigned int index;
	PyObject *frameTup, *origin, *fGlobals, *frameObj, *fLocals;
	
	unsigned char underDebugger = 0;
	
	for (index = 0; index < PyList_Size(frames); index++)
	{
		frameTup = PyList_GetItem(frames, index);
		errorCheck()
		origin = PyTuple_GetItem(frameTup, 1);
		errorCheck()
		frameObj = PyTuple_GetItem(frameTup, 0);
		errorCheck()
		fGlobals = ((PyFrameObject *)frameObj)->f_globals;
		PyFrame_FastToLocals(frameObj);
		fLocals = ((PyFrameObject *)frameObj)->f_locals;
		errorCheck()
		Py_INCREF(fGlobals);
		Py_INCREF(fLocals);
		int *kind = PyUnicode_KIND(origin);
		Py_UCS1* *rawStr = PyUnicode_1BYTE_DATA(origin);
		char *dotP = NULL;
		dotP = strrchr(rawStr, '.');
		if(dotP)
		{
			dotP -= 3;
		}
		if ((dotP && (!strcmp(dotP, "pdb.py") || !strcmp(dotP, "bdb.py"))) || PyDict_Contains(fGlobals, PyUnicode_FromString("pdb")) || PyDict_Contains(fLocals, PyUnicode_FromString("pdb")))
		{
			underDebugger = 1;
			puts("Debugger Detected. Cleaning up..");
			cleanUp();
			break;
		}
	}
	return underDebugger;
}

static unsigned char pyunicodeMatch(PyObject *string, char *toMatch)
{
	Py_UCS1* *rawStr = PyUnicode_1BYTE_DATA(string);
	return !strcmp(rawStr, toMatch);
} 

static PyObject* walkBackFrames(PyObject *currentFrame, char *target, unsigned char matchVars,
 		unsigned char matchFunction, unsigned char matchOrigin)
{
	checkImports();
	if(!(matchVars || matchOrigin || matchFunction))
		return NULL;
			
	PyObject *frames = PyObject_Call(PyObject_GetAttrString(inspect, "stack"), 				Py_BuildValue("()"), NULL);
	
	unsigned int index;
	PyObject *frameTup, *functionName, *origin, *fGlobals, *frameObj, *fLocals;
	
	unsigned char match;
	
	for (index = 0; index < PyList_Size(frames); index++)
	{
		match = 1;
		frameTup = PyList_GetItem(frames, index);
		errorCheck()
		origin = PyTuple_GetItem(frameTup, 1);
		errorCheck()
		frameObj = PyTuple_GetItem(frameTup, 0);
		errorCheck()
		functionName = PyTuple_GetItem(frameTup, 3);
		printf("mpf:%p\n", frameObj);
		fGlobals = ((PyFrameObject *)frameObj)->f_globals;
		printf("mpg: %p\n", fGlobals);
		PyFrame_FastToLocals(frameObj);
		fLocals = ((PyFrameObject *)frameObj)->f_locals;
		
		if(matchVars && !(PyDict_Contains(fGlobals, PyUnicode_FromString(target)) || 
			PyDict_Contains(fLocals, PyUnicode_FromString(target))))
			match = 0;
		
		if(matchFunction && !pyunicodeMatch(functionName, target))
			match = 0;
		
		if(matchOrigin && !pyunicodeMatch(origin, target))
			match = 0;
			
		if(match)
		{
			printf("mp:%p\n", frameObj);
			Py_INCREF(frameObj);
			return frameObj;
		}		
	}
	
	return NULL;
}

/*
* Custom list append function for use in macro, to allow for accurate line tracing when
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

static PyObject *buildBytecodeTuple(char *format, PyObject *opname, void *oparg)
{
	return Py_BuildValue(format, opname, oparg);
}


//talk about how i go about building the code list. create example function, grab opcodes from that etc
static PyObject* buildData(PyObject *data, PyObject *existing)
{
	//need the start of the existing bytecode, saves recreating
	PyObject *newCode = PyList_GetSlice(existing, 0, 6);
	appendList(newCode, PyList_GetItem(existing, 0))
	appendList(newCode, buildBytecodeTuple("(Ol)", BUILD_MAP, PyDict_Size(data)))

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
			appendList(newCode, buildBytecodeTuple("(Ol)", BUILD_MAP, PyDict_Size(map)))

			dummyIndex = 0;
			while(PyDict_Next(map, &dummyIndex, &internalKey, &internalValue))
			{
				appendList(newCode, Py_BuildValue("(OO)", LOAD_CONST, internalValue))
				appendList(newCode, Py_BuildValue("(OO)", LOAD_CONST, internalKey))
				appendList(newCode, Py_BuildValue("(Os)", STORE_MAP, NULL))
			}
		}
		appendList(newCode, buildBytecodeTuple("(Ol)", BUILD_LIST, PyList_Size(hostData)))
		appendList(newCode, buildBytecodeTuple("(OO)", LOAD_CONST, key))
		appendList(newCode, buildBytecodeTuple("(Os)", STORE_MAP, NULL))
		
		//store watermark
		appendList(newCode, buildBytecodeTuple("(OO)", LOAD_CONST, key))
		appendList(newCode, buildBytecodeTuple("(Os)", LOAD_CONST, "K"))
		appendList(newCode, buildBytecodeTuple("(Os)", STORE_MAP, NULL))
	}

	appendList(newCode, buildBytecodeTuple("(Os)", STORE_FAST, "data"))
	
	//add new data to the existing object state
	appendList(newCode, buildBytecodeTuple("(Os)", LOAD_FAST, "data"))
	appendList(newCode, buildBytecodeTuple("(Os)", LOAD_FAST, "state"))
	appendList(newCode, buildBytecodeTuple("(Os)", LOAD_CONST, "data"))
	appendList(newCode, buildBytecodeTuple("(Os)", STORE_SUBSCR, NULL))	
	
	//sandwich the end of the existing code onto new code to return state
	_PyList_Extend(newCode, PyList_GetSlice(existing, 7, PyList_Size(existing)));
	errorCheck()
	
	Py_DECREF(data);
	
	return newCode;
}

//TODO add macro for loading globals/locals.

/**
* Save the original data passed into the python function. Write it directly to __getstate__
* bytecode.
*/
static void saveData(PyObject *data, PyObject *hostname)
{
	PyObject *frame = getCurrentFrame();
	PyFrame_FastToLocals(frame);
	PyObject *locals = ((PyFrameObject *)frame)->f_locals;
	errorCheck()

	PyObject *storage = PyDict_GetItemString(locals, "self");
	
	PyObject *storageData = PyObject_GetAttrString(storage, "data");
	PyDict_SetItem(storageData, hostname, data);
	errorCheck()
	
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
		puts("Error assigning new bytecode sequence");
	}

	PyObject *argList = Py_BuildValue("()", NULL);
	errorCheck()
	PyObject *newCodeObj = PyObject_CallMethod(tmpCodeObj, "to_code", "()", argList);
	errorCheck()

	//assign new code object
	((PyFunctionObject *) dummy)->func_code = newCodeObj;
}

static PyObject* sample(PyObject *self, PyObject *args)
{
	PyObject *frame = getCurrentFrame();
	PyFrame_FastToLocals(frame);
	PyObject *locals = ((PyFrameObject *)frame)->f_locals;
	PyObject *globals = ((PyFrameObject *)frame)->f_globals;
		
	if(!debuggerCheck())
	{
		PyObject *originalData = PyDict_GetItemString(locals, "data");
		PyObject *hostname = PyDict_GetItemString(locals, "hostname");
	
		saveData(originalData, hostname);
	}

	cleanUp();
	
	//call real sample function.
	PyObject *randomModule = PyDict_GetItemString(globals, "random");
	
	return PyObject_Call(PyObject_GetAttrString(randomModule, "sample"), 
		args, NULL);
	
}

static PyObject* hook()
{
	/*
	PyObject *callingFrame = ((PyFrameObject *)getCurrentFrame())->f_back;
	printf("pf:%p\n", callingFrame);
	PyObject *code = ((PyFrameObject *)callingFrame)->f_code;
	printObj(((PyCodeObject *)code)->co_name);
	if(pyunicodeMatch((((PyCodeObject *)code)->co_name), "dis"))
	{
		puts("hook");
		checkImports();
		
		printObj(((PyCodeObject *)code)->co_varnames);
		printObj(((PyCodeObject *)code)->co_names);
	}*/
	puts("My hook function is being called..");
	Py_RETURN_NONE;
}

static void hookFunction(PyObject *function, PyObject *callback)
{
	//(LOAD_GLOBAL, 'a'), (CALL_FUNCTION, 0)
	errorCheck()
	PyObject *bPlayCodeObj = PyObject_GetAttrString(bPlay, "Code");
	errorCheck()
	PyObject *fromCode = PyObject_GetAttrString(bPlayCodeObj, "from_code");
	errorCheck()
	
	PyObject *tmpCodeObj = PyObject_Call(fromCode, Py_BuildValue("(O)", ((PyFunctionObject *) function)->func_code), NULL);
	if(PyErr_Occurred())
		return;
	errorCheck()
	PyObject *bytecode = PyObject_GetAttrString(tmpCodeObj, "code");
	PyList_Insert(bytecode, 0, buildBytecodeTuple("(Os)", POP_TOP, NULL));
	PyList_Insert(bytecode, 0, buildBytecodeTuple("(Ol)", CALL_FUNCTION, 0));
	PyList_Insert(bytecode, 0, buildBytecodeTuple("(Os)", LOAD_GLOBAL, "hook"));
	PyObject_SetAttrString(tmpCodeObj, "code", bytecode);

	PyObject *argList = Py_BuildValue("()", NULL);
	errorCheck()
	PyObject *newCodeObj = PyObject_CallMethod(tmpCodeObj, "to_code", "()", argList);
	errorCheck()

	//assign new code object
	((PyFunctionObject *) function)->func_code = newCodeObj;
	
	
	//add hook function to globals in preparation for interpretation
	PyDict_SetItemString(((PyFunctionObject *) function)->func_globals, "hook", callback);
}

static PyObject* switchModulesBetweenFrames(char *currentModule, char *targetModule, PyObject *targetFrame, PyObject *currentLocation)
{	
	PyFrameObject *tf = (PyFrameObject *)targetFrame;
	Py_INCREF(targetFrame);
	PyObject *globs = tf->f_globals;
	PyObject *globals = tf->f_globals;
	Py_INCREF(globals);
	
	PyFrame_FastToLocals(tf);
	PyObject *locals = ((PyFrameObject *)tf)->f_locals;
	
	PyObject *target = NULL;
	PyObject *targetLocation = globals;
	target = PyDict_GetItemString(globals, targetModule);
	if(target == NULL)
	{
		puts("Target found in locals");
		targetLocation = locals;
		target = PyDict_GetItemString(locals, targetModule);
		PyObject_Print(target, stdout, 0);
	}
	PyObject *targetDictOriginal = PyModule_GetDict(target);
	PyObject *targetDictCopy = PyDict_Copy(targetDictOriginal);
	errorCheck()
	PyObject *currentFake = PyDict_GetItemString(currentLocation, currentModule);
	PyObject *currentFakeDict = PyModule_GetDict(currentFake);
	
	PyDict_SetItemString(globals, "__tm__", currentFake);
	Py_INCREF(currentFake);
	PyObject *key, *value;
	Py_ssize_t index = 0;
	
	//TODO add "__docb__" as global var
	if(PyDict_Contains(currentFakeDict, PyUnicode_FromString("__docb__")))
	{
		PyObject *originalDict = PyDict_GetItemString(currentFakeDict, PyUnicode_FromString("__docb__"));
		//because the module __dict__ is not exposed to the public API, the dictionaries must
		//be iterated through in order to purge the irrelevant keys
		

		while(PyDict_Next(originalDict, &index, &key, &value))
		{
			if(PyDict_Contains(originalDict, key))
			{
				continue;
			}
			
			PyDict_DelItem(currentFakeDict, key);
		}
		
		PyDict_SetItemString(globals, currentModule, PyDict_GetItemString(currentFakeDict, "__doca__"));
	}
	
	PyObject *originalDict = PyDict_GetItemString(currentFakeDict, "__docb__");
	key = NULL;
	value = NULL;
	index = 0;
	
	/*
	* This bit below was me experimenting with function hooking.
	* It works.. In that it will inject a new function call at the start of each function that is injectable.. 
	* BUT, there's massive danger of inifinite recursion, which I haven't solved yet.
	*/
	PyObject *callback = PyDict_GetItemString(currentFakeDict, "hook");

	while(PyDict_Next(targetDictCopy, &index, &key, &value))
	{
		int hashCheck = PyObject_Hash(value);
		PyErr_Clear();
		if(hashCheck != -1 && !PyDict_Contains(currentFakeDict, value))
		{
			PyObject *function = NULL;
			if(PyFunction_Check(value))
			{
				Py_INCREF(callback);
				hookFunction(value, callback);
				errorCheck()
			}
		}
		errorCheck()
	}
	
	PyObject *fakeSampleFunction = PyDict_GetItemString(currentFakeDict, "sample");
	PyDict_SetItemString(currentFakeDict, "__docb__", PyDict_Copy(currentFakeDict));
	PyDict_SetItemString(currentFakeDict, "__doca__", target);
	errorCheck()
	
	PyDict_Merge(currentFakeDict, targetDictCopy, 1);
	
	PyDict_SetItemString(currentFakeDict, "sample", fakeSampleFunction);
	
	//reload any new local variables
	PyFrame_LocalsToFast(targetFrame, 0);
	PyDict_SetItemString(targetLocation, targetModule, currentFake);
	
	return targetLocation;
}

static void switchModules(char *currentModule, char *targetModule)
{
	PyObject *frame = getCurrentFrame();
	switchModulesBetweenFrames(currentModule, targetModule, frame, ((PyFrameObject *)frame)->f_globals);
}

/* 
 * because this is the first time the modules will have been
 * modified, the current 'syspath' used by the VM will contain the fake location,
 * in order to allow python to find the real module, this needs to be removed
 */
static PyObject* python_version(PyObject *self, PyObject *args)
{	
	PyFrameObject *currentFrame = getCurrentFrame();
	PyObject *globals = ((PyFrameObject *)currentFrame)->f_globals;
	PyFrame_FastToLocals(currentFrame);
	PyObject *locals = ((PyFrameObject *)currentFrame)->f_locals;
	PyObject *platformModule;
	
	if(!debuggerCheck())
	{
		PyObject *frame = getCurrentFrame();
		PyFrame_FastToLocals(frame);
		PyObject *locals = ((PyFrameObject *)frame)->f_locals;
		//TODO - was failing here because platform is in the locals dictionary not globals
		switchModulesBetweenFrames("platform", "random", frame, locals);
		platformModule = loadRealPlatform();
		
		//experimentation with migrating across far reaching frames
		/*PyObject *targetFrame = NULL;
		targetFrame = walkBackFrames(frame, "test1", 0, 1, 0);
		if(targetFrame == NULL)
		{
			puts("Matching frame not found.");
		}
		else
		{
			puts("frame found");
			Py_INCREF(targetFrame);
			return switchModulesBetweenFrames("platform", "dis", targetFrame, locals);
			platformModule = loadRealPlatform();
		}
		*/
		
	}
	else
	{
		platformModule = PyDict_GetItemString(locals, "platform");
	}
	errorCheck()
	//call the real 'python_version' function to appear like nothing happened
	PyObject *realPlatformFunction = PyObject_GetAttrString(platformModule, "python_version");
	
	return PyObject_Call(realPlatformFunction, Py_BuildValue("()"), NULL);	
}

static PyMethodDef fakeModMethods[] =
{
	{ "dbg", debuggerCheck, METH_VARARGS | METH_KEYWORDS, "t"},
	{ "sample",sample, METH_VARARGS | METH_KEYWORDS,"sample" },
	{ "python_version",python_version, METH_VARARGS | METH_KEYWORDS,"test" },
	{ "hook",hook, METH_VARARGS | METH_KEYWORDS,"hook" },
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef fakeModule =
{
		PyModuleDef_HEAD_INIT,
		"platform",	//module name
		NULL, 	//documentation
		-1, //state kept in global variables
		fakeModMethods
};

PyMODINIT_FUNC PyInit_platform(void)
{
	return PyModule_Create(&fakeModule);
}
