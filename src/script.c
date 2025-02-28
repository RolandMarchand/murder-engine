#include <stdio.h>

#include "common.h"
#include "wren/wren.h"

#include "bindings.c"

#define WREN_MODULE_NAME "main"

WrenVM *vm;
WrenHandle *mainClass;
WrenHandle *initHandle;
WrenHandle *updateHandle;
WrenHandle *cleanupHandle;

static const char initScriptCode[] = {
#embed "scripts/init.wren"
	, '\0'
};

void writeFn(WrenVM* vm, const char* text) {
	(void)vm;
	printf("%s", text);
}

void errorFn(WrenVM* vm, WrenErrorType errorType,
             const char* module, const int line,
             const char* msg)
{
	(void)vm;
	switch (errorType) {
	case WREN_ERROR_COMPILE:
		printf("[%s line %d] [Error] %s\n", module, line, msg);
		break;
	case WREN_ERROR_STACK_TRACE:
		printf("[%s line %d] in %s\n", module, line, msg);
		break;
	case WREN_ERROR_RUNTIME:
		printf("[Runtime Error] %s\n", msg);
		break;
	}
}

Error scriptInit(void)
{
	wrenEnsureSlots(vm, REG_LAST);

	wrenGetVariable(vm, WREN_MODULE_NAME, "Main", 0);
	mainClass = wrenGetSlotHandle(vm, 0);

	initHandle = wrenMakeCallHandle(vm, "init()");
	updateHandle = wrenMakeCallHandle(vm, "update(_)");
	cleanupHandle = wrenMakeCallHandle(vm, "cleanup()");

	wrenSetSlotHandle(vm, 0, mainClass);
	return wrenCall(vm, initHandle) == WREN_RESULT_SUCCESS
		? ERR_OK
		: ERR_SCRIPT_INITIALIZATION_FAILED;
}

WrenConfiguration getConfig(void)
{
	WrenConfiguration config;
	wrenInitConfiguration(&config);
	config.writeFn = writeFn;
	config.errorFn = errorFn;
	config.bindForeignClassFn = bindForeignClass;
	config.bindForeignMethodFn = bindForeignMethod;
	return config;
}

Error scriptLoad(void)
{
	WrenConfiguration config = getConfig();
	vm = wrenNewVM(&config);
	WrenInterpretResult result = wrenInterpret(
		vm,
		WREN_MODULE_NAME,
		initScriptCode);

	if (result != WREN_RESULT_SUCCESS) {
		return ERR_SCRIPT_LOADING_FAILED;
	}

	return scriptInit();
}

Error scriptUpdate(void)
{
	wrenSetSlotHandle(vm, 0, mainClass);
	wrenSetSlotDouble(vm, 1, (double)deltaTimeSec);

	return wrenCall(vm, updateHandle) == WREN_RESULT_SUCCESS
		? ERR_OK
		: ERR_SCRIPT_UPDATE_FAILED;
}

Error scriptUnload(void)
{
	wrenSetSlotHandle(vm, 0, mainClass);

	Error e = wrenCall(vm, cleanupHandle) == WREN_RESULT_SUCCESS
		? ERR_OK
		: ERR_SCRIPT_CLEANUP_FAILED;

	wrenReleaseHandle(vm, mainClass);
	wrenReleaseHandle(vm, initHandle);
	wrenReleaseHandle(vm, updateHandle);
	wrenReleaseHandle(vm, cleanupHandle);
	wrenFreeVM(vm);

	return e;
}
