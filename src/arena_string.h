/* String Arena - A simple string management system
 *
 * OVERVIEW: - The user only has to care about calling `arenaStoreString()` to
 * store strings and manage the returned ArenaChar *references.
 *
 * - Do not reallocate or grow ArenaStrings. If modifications are needed,
 *   copy them, modify the copy, and store the result using `arenaStoreString()`.
 *
 * - ArenaStrings can be safely cast to char*, as long as the string isn't grown
 *   or reallocated.
 *
 * - `arenaStoreString()` returns nullptr on allocation errors. It should always
 *   be checked.
 *
 * - Call arenaFree() at the end of your program to release all allocated memory.
 *   You can still store strings after calling arenaFree(), but you must call
 *   arenaFree() again later to prevent memory leaks.
 *
 * - This library is meant for single-threaded use.
 *
 * USAGE:
 * - ArenaChar *str = arenaStoreString("example"); // Store a string
 * - // Use str as needed
 * - arenaFree(); // Free all strings when done
 */
#pragma once

#include "common.h"
#include "stb_ds.h"

typedef const char ArenaChar;

struct StringArena {
	ArenaChar **buffer;	/* stb_ds.h array */
};

struct StringArena *arenaGetAddress(void)
{
	static struct StringArena stringArena = {};
	return &stringArena;
}

void arenaFree(void) {
	struct StringArena *arena = arenaGetAddress();

	/* Uninitialized */
	if (arena->buffer == nullptr) {
		return;
	}
	
	for (int i = 0; i < arrlen(arena->buffer); i++) {
		free((void*)arena->buffer[i]);
	}

	arrfree(arena->buffer);
	arena->buffer = nullptr;
}

ArenaChar *arenaStoreString(const char *str) {
	if (str == nullptr) {
		return nullptr;
	}

	struct StringArena *arena = arenaGetAddress();

	ArenaChar *s = strdup(str);

	/* Out of memory */
	if (unlikely(s == nullptr)) {
		return nullptr;
	}

	arrput(arena->buffer, s);
	/* Out of memory */
	if (unlikely(arena->buffer == nullptr)) {
		return nullptr;
	}

	return s;
}
