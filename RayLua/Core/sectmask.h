#ifndef SECTMASK_H
#define SECTMASK_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
	unsigned char *bits;
	size_t size;
} sectmask_t;

static sectmask_t* sectmask_create(void) {
	sectmask_t *mask = malloc(sizeof(sectmask_t));
	mask->bits = NULL;
	mask->size = 0;
	return mask;
}

static void sectmask_ensure_capacity(sectmask_t *mask, long id) {
	size_t needed_bytes = (id / 8) + 1;
	if (needed_bytes > mask->size) {
		size_t new_size = (needed_bytes + 1023) & ~1023;
		mask->bits = realloc(mask->bits, new_size);
		memset(mask->bits + mask->size, 0, new_size - mask->size);
		mask->size = new_size;
	}
}

static void sectmask_mark_sector(sectmask_t *mask, long id) {
	sectmask_ensure_capacity(mask, id);
	mask->bits[id / 8] |= (1 << (id % 8));
}

static bool sectmask_was_marked(sectmask_t *mask, long id) {
	if (id / 8 >= mask->size) return false;
	return (mask->bits[id / 8] >> (id % 8)) & 1;
}

static void sectmask_destroy(sectmask_t *mask) {
	if (mask) {
		free(mask->bits);
		mask->bits = NULL;
		free(mask);
	}
}

#endif
