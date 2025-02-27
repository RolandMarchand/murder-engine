#include <stdio.h>

#include "common.h"
#include "wren/wren.h"

WrenVM *vm;
WrenHandle *mainClass;
WrenHandle *initHandle;
WrenHandle *updateHandle;
WrenHandle *cleanupHandle;

extern float deltaTimeSec;

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

Error scriptLoad(void)
{
	WrenConfiguration config;
	wrenInitConfiguration(&config);
	config.writeFn = &writeFn;
	config.errorFn = errorFn;
	vm = wrenNewVM(&config);
	WrenInterpretResult result = wrenInterpret(
		vm,
		"main",
		initScriptCode);

	if (result != WREN_RESULT_SUCCESS) {
		return ERR_SCRIPT_LOADING_FAILED;
	}

	wrenEnsureSlots(vm, 1);

	wrenGetVariable(vm, "main", "Main", 0);
	mainClass = wrenGetSlotHandle(vm, 0);

	initHandle = wrenMakeCallHandle(vm, "init()");
	updateHandle = wrenMakeCallHandle(vm, "update(_)");
	cleanupHandle = wrenMakeCallHandle(vm, "cleanup()");

	wrenSetSlotHandle(vm, 0, mainClass);
	result = wrenCall(vm, initHandle);

	if (result != WREN_RESULT_SUCCESS) {
		return ERR_SCRIPT_INITIALIZATION_FAILED;
	}

	return ERR_OK;
}

Error scriptUpdate(void)
{
	wrenEnsureSlots(vm, 2);
	wrenSetSlotHandle(vm, 0, mainClass);
	wrenSetSlotDouble(vm, 1, (double)deltaTimeSec);

	return wrenCall(vm, updateHandle) == WREN_RESULT_SUCCESS
		? ERR_OK
		: ERR_SCRIPT_UPDATE_FAILED;
}

Error scriptUnload(void)
{
	wrenEnsureSlots(vm, 1);
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
