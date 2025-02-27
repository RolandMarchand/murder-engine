#include <stdio.h>
#include <string.h>

#include "cglm/cglm.h"
#include "common.h"
#include "wren/wren.h"

#define WREN_MODULE_NAME "main"

enum {
	SLOT_COUNT = 8,
};

WrenVM *vm;
WrenHandle *mainClass;
WrenHandle *initHandle;
WrenHandle *updateHandle;
WrenHandle *cleanupHandle;

extern float deltaTimeSec;
extern vec3 cameraPos;

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

void getPlayerPosition(WrenVM* vm)
{
	wrenSetSlotNewList(vm, 0);

	for (int i = 0; i < 3; i++) {
		wrenSetSlotDouble(vm, 1, cameraPos[i]);
		wrenInsertInList(vm, 0, i, 1);
	}
}

void setPlayerPosition(WrenVM* vm)
{
	if (wrenGetSlotType(vm, 1) != WREN_TYPE_LIST
	    || wrenGetListCount(vm, 1) < 3) {
		wrenSetSlotString(vm, 0, "Cannot set a position that is not [num, num, num].");
		wrenAbortFiber(vm, 0);
		return;
	}

	wrenGetListElement(vm, 1, 0, 2);
	wrenGetListElement(vm, 1, 1, 3);
	wrenGetListElement(vm, 1, 2, 4);

	if (wrenGetSlotType(vm, 2) != WREN_TYPE_NUM
	    || wrenGetSlotType(vm, 3) != WREN_TYPE_NUM
	    || wrenGetSlotType(vm, 4) != WREN_TYPE_NUM) {
		wrenSetSlotString(vm, 0, "Cannot set a position that is not [num, num, num].");
		wrenAbortFiber(vm, 0);
		return;
	}

	cameraPos[0] = (float)wrenGetSlotDouble(vm, 2);
	cameraPos[1] = (float)wrenGetSlotDouble(vm, 3);
	cameraPos[2] = (float)wrenGetSlotDouble(vm, 4);
}

void playerAllocate(WrenVM *vm)
{
	wrenSetSlotNewForeign(vm, 0, 0, 0);
}

void playerFinalize(void *data)
{
	(void)data;
}

WrenForeignClassMethods bindForeignClass(
    WrenVM* vm, const char* module, const char* className)
{
	(void)vm;
	(void)module;
	(void)className;

	WrenForeignClassMethods methods = {};

	if (strcmp(className, "Player") == 0) {
		methods.allocate = playerAllocate;
		methods.finalize = playerFinalize;
	}

	return methods;
}

WrenForeignMethodFn bindForeignMethod(WrenVM* vm, const char* module,
    const char* className, bool isStatic, const char* signature)
{
	(void)vm;
	(void)module;

	if (strcmp(className, "Player") != 0) {
		return nullptr;
	}

	if (isStatic && strcmp(signature, "getPos") == 0) {
		return getPlayerPosition;
	}

	if (isStatic && strcmp(signature, "setPos=(_)") == 0) {
		return setPlayerPosition;
	}

	return nullptr;
}

Error scriptInit(void)
{
	wrenEnsureSlots(vm, SLOT_COUNT);

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
