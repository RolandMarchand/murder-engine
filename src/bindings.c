#include <string.h>

#include "cglm/cglm.h"
#include "common.h"
#include "wren/wren.h"

#define WREN_ALLOCATE(vm, type)							\
	wrenSetSlotNewForeign((vm), REG_ACC, REG_ACC, sizeof(type))
#define WREN_ALLOCATE_EMPTY(vm)					\
	wrenSetSlotNewForeign((vm), REG_ACC, REG_ACC, 0)

extern float deltaTimeSec;
extern vec3 cameraPosition;

void bindPlayerAllocate(WrenVM *vm)
{
	WREN_ALLOCATE_EMPTY(vm);
}

void bindPlayerFinalize(void *data)
{
	(void)data;
}

void bindPlayerGetPos(WrenVM* vm)
{
	wrenSetSlotNewList(vm, REG_ACC);

	for (int i = 0; i < 3; i++) {
		wrenSetSlotDouble(vm, REG_TMP1, cameraPosition[i]);
		wrenInsertInList(vm, REG_ACC, i, REG_TMP1);
	}
}

void bindPlayerSetPos(WrenVM* vm)
{
	if (wrenGetSlotType(vm, REG_ARG1) != WREN_TYPE_LIST
	    || wrenGetListCount(vm, 1) < 3) {
		wrenSetSlotString(vm, REG_ACC, "Cannot set a position that is not [num, num, num].");
		wrenAbortFiber(vm, REG_ACC);
		return;
	}

	wrenGetListElement(vm, REG_ARG1, 0, REG_TMP1);
	wrenGetListElement(vm, REG_ARG1, 1, REG_TMP2);
	wrenGetListElement(vm, REG_ARG1, 2, REG_TMP3);

	if (wrenGetSlotType(vm, REG_TMP1) != WREN_TYPE_NUM
	    || wrenGetSlotType(vm, REG_TMP2) != WREN_TYPE_NUM
	    || wrenGetSlotType(vm, REG_TMP3) != WREN_TYPE_NUM) {
		wrenSetSlotString(vm, REG_ACC,
				  "Cannot set a position that is not [num, num, num].");
		wrenAbortFiber(vm, REG_ACC);
		return;
	}

	cameraPosition[0] = (float)wrenGetSlotDouble(vm, REG_TMP1);
	cameraPosition[1] = (float)wrenGetSlotDouble(vm, REG_TMP2);
	cameraPosition[2] = (float)wrenGetSlotDouble(vm, REG_TMP3);
}

WrenForeignMethodFn bindPlayer(bool isStatic, const char* signature)
{
	if (!isStatic) {
		return nullptr;
	}

	if (strcmp(signature, "getPos") == 0) {
		return bindPlayerGetPos;
	}

	if (strcmp(signature, "setPos=(_)") == 0) {
		return bindPlayerSetPos;
	}
	
	return nullptr;
}

void bindVec3New(WrenVM *vm)
{
	vec3 *v = wrenGetSlotForeign(vm, REG_ACC);

	(*v)[0] = (float)wrenGetSlotDouble(vm, REG_ARG1);
	(*v)[1] = (float)wrenGetSlotDouble(vm, REG_ARG2);
	(*v)[2] = (float)wrenGetSlotDouble(vm, REG_ARG3);
}

void bindVec3Allocate(WrenVM *vm)
{
	WREN_ALLOCATE(vm, vec3);
}

void bindVec3Finalize(void *data)
{
	(void)data;
}

void bindVec3AccessElement(WrenVM *vm)
{
	vec3 *v = wrenGetSlotForeign(vm, REG_ACC);
	int idx = (int)wrenGetSlotDouble(vm, REG_ARG1);

	if (idx < 0 || idx >= 3) {
		wrenSetSlotString(vm, REG_ACC,
				  "Cannot access vec3 outside of bound.");
		wrenAbortFiber(vm, REG_ACC);
	}

	wrenSetSlotDouble(vm, REG_ACC, (*v)[idx]);
}

void bindVec3ModifyElement(WrenVM *vm)
{
	vec3 *v = wrenGetSlotForeign(vm, REG_ACC);
	int idx = (int)wrenGetSlotDouble(vm, REG_ARG1);
	float newVal = (float)wrenGetSlotDouble(vm, REG_ARG2);

	if (idx < 0 || idx >= 3) {
		wrenSetSlotString(vm, REG_ACC,
				  "Cannot access vec3 outside of bound.");
		wrenAbortFiber(vm, REG_ACC);
	}

	if (wrenGetSlotType(vm, REG_ARG2) != WREN_TYPE_NUM) {
		wrenSetSlotString(vm, REG_ACC, "Cannot assign a non-number to a vector element.");
		wrenAbortFiber(vm, REG_ACC);
	}

	(*v)[idx] = newVal;

	wrenSetSlotDouble(vm, REG_ACC, newVal);
}

void bindVec3Set(WrenVM *vm)
{
	vec3 *v = wrenGetSlotForeign(vm, REG_ACC);

	float x = (float)wrenGetSlotDouble(vm, REG_ARG1);
	float y = (float)wrenGetSlotDouble(vm, REG_ARG2);
	float z = (float)wrenGetSlotDouble(vm, REG_ARG3);
	
	if (wrenGetSlotType(vm, REG_ARG1) != WREN_TYPE_NUM
	    || wrenGetSlotType(vm, REG_ARG2) != WREN_TYPE_NUM
	    || wrenGetSlotType(vm, REG_ARG3) != WREN_TYPE_NUM) {
		wrenSetSlotString(vm, REG_ACC, "Cannot assign a non-number to a vector element.");
		wrenAbortFiber(vm, REG_ACC);
	}

	(*v)[0] = x;
	(*v)[1] = y;
	(*v)[2] = z;
}

WrenForeignMethodFn bindVec3(bool isStatic, const char* signature)
{
	if (isStatic) {
		return nullptr;
	}

	if (strcmp(signature, "init new(_,_,_)") == 0) {
		return bindVec3New;
	}

	if (strcmp(signature, "[_]") == 0) {
		return bindVec3AccessElement;
	}

	if (strcmp(signature, "[_]=(_)") == 0) {
		return bindVec3ModifyElement;
	}

	if (strcmp(signature, "set(_,_,_)") == 0) {
		return bindVec3Set;
	}

	return nullptr;
}

WrenForeignMethodFn bindForeignMethod(WrenVM* vm, const char* module,
    const char* className, bool isStatic, const char* signature)
{
	(void)vm;
	(void)module;

	if (strcmp(className, "Player") == 0) {
		return bindPlayer(isStatic, signature);
	}

	if (strcmp(className, "Vec3") == 0) {
		return bindVec3(isStatic, signature);
	}

	return nullptr;
}



WrenForeignClassMethods bindForeignClass(
    WrenVM* vm, const char* module, const char* className)
{
	(void)vm;
	(void)module;
	(void)className;

	WrenForeignClassMethods methods = {};

	if (strcmp(className, "Player") == 0) {
		methods.allocate = bindPlayerAllocate;
		methods.finalize = bindPlayerFinalize;
	}

	if (strcmp(className, "Vec3") == 0) {
		methods.allocate = bindVec3Allocate;
		methods.finalize = bindVec3Finalize;
	}

	return methods;
}
