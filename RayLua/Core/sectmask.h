#ifndef SECTMASK_H
#define SECTMASK_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static unsigned char *sector_bits = NULL;
static size_t sector_bits_size = 0;

static void sectmask_ensure_capacity(long id) {
	size_t needed_bytes = (id / 8) + 1;
	if (needed_bytes > sector_bits_size) {
		size_t new_size = (needed_bytes + 1023) & ~1023;
		sector_bits = realloc(sector_bits, new_size);
		memset(sector_bits + sector_bits_size, 0, new_size - sector_bits_size);
		sector_bits_size = new_size;
	}
}

static void sectmask_mark_sector(long id) {
	sectmask_ensure_capacity(id);
	sector_bits[id / 8] |= (1 << (id % 8));
}

static bool sectmask_was_marked(long id) {
	if (id / 8 >= sector_bits_size) return false;
	return (sector_bits[id / 8] >> (id % 8)) & 1;
}

static void sectmask_reset(void) {
	if (sector_bits) {
		free(sector_bits);
		sector_bits = NULL;
	}
	sector_bits_size = 0;
}

#endif
