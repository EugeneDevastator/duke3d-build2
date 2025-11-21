#ifndef PRAGMAS_H
#define PRAGMAS_H

//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle, Ken Silverman
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#include <stdint.h>
static long dmval;



static inline int32_t scale(int32_t a, int32_t b, int32_t c) {
	return (int32_t)(((int64_t)a * b) / c);
}

static inline int32_t mulscale(int32_t a, int32_t b, int shift) {
	return (int32_t)(((int64_t)a * b) >> shift);
}

// Use the generic version for all cases
#define mulscale1(a,b)  mulscale(a,b,1)
#define mulscale2(a,b)  mulscale(a,b,2)
#define mulscale3(a,b)  mulscale(a,b,3)
#define mulscale4(a,b)  mulscale(a,b,4)
#define mulscale5(a,b)  mulscale(a,b,5)
#define mulscale6(a,b)  mulscale(a,b,6)
#define mulscale7(a,b)  mulscale(a,b,7)
#define mulscale8(a,b)  mulscale(a,b,8)
#define mulscale9(a,b)  mulscale(a,b,9)
#define mulscale10(a,b) mulscale(a,b,10)
#define mulscale11(a,b) mulscale(a,b,11)
#define mulscale12(a,b) mulscale(a,b,12)
#define mulscale13(a,b) mulscale(a,b,13)
#define mulscale14(a,b) mulscale(a,b,14)
#define mulscale15(a,b) mulscale(a,b,15)
#define mulscale16(a,b) mulscale(a,b,16)
#define mulscale17(a,b) mulscale(a,b,17)
#define mulscale18(a,b) mulscale(a,b,18)
#define mulscale19(a,b) mulscale(a,b,19)
#define mulscale20(a,b) mulscale(a,b,20)
#define mulscale21(a,b) mulscale(a,b,21)
#define mulscale22(a,b) mulscale(a,b,22)
#define mulscale23(a,b) mulscale(a,b,23)
#define mulscale24(a,b) mulscale(a,b,24)
#define mulscale25(a,b) mulscale(a,b,25)
#define mulscale26(a,b) mulscale(a,b,26)
#define mulscale27(a,b) mulscale(a,b,27)
#define mulscale28(a,b) mulscale(a,b,28)
#define mulscale29(a,b) mulscale(a,b,29)
#define mulscale30(a,b) mulscale(a,b,30)
#define mulscale31(a,b) mulscale(a,b,31)
#define mulscale32(a,b) mulscale(a,b,32)


// Base function for dmulscale operations
static inline int32_t dmulscale_base(int32_t a, int32_t b, int32_t c, int32_t d, int shift) {
	int64_t result1 = (int64_t)a * b;
	int64_t result2 = (int64_t)c * d;
	int64_t sum = result1 + result2;
	return (int32_t)(sum >> shift);
}

// Special case for dmulscale32 (returns high 32 bits)
static inline int32_t dmulscale32_base(int32_t a, int32_t b, int32_t c, int32_t d) {
	int64_t result1 = (int64_t)a * b;
	int64_t result2 = (int64_t)c * d;
	int64_t sum = result1 + result2;
	return (int32_t)(sum >> 32);
}

// Macro for generating dmulscale functions
#define DMULSCALE_FUNC(n) static inline int32_t dmulscale##n(int32_t a, int32_t b, int32_t c, int32_t d) { return dmulscale_base(a, b, c, d, n); }

// Variable shift version
static inline int32_t dmulscale(int32_t a, int32_t b, int32_t c, int32_t d, int shift) {
	return dmulscale_base(a, b, c, d, shift);
}

// Generate all fixed-shift versions
DMULSCALE_FUNC(1)
DMULSCALE_FUNC(2)
DMULSCALE_FUNC(3)
DMULSCALE_FUNC(4)
DMULSCALE_FUNC(5)
DMULSCALE_FUNC(6)
DMULSCALE_FUNC(7)
DMULSCALE_FUNC(8)
DMULSCALE_FUNC(9)
DMULSCALE_FUNC(10)
DMULSCALE_FUNC(11)
DMULSCALE_FUNC(12)
DMULSCALE_FUNC(13)
DMULSCALE_FUNC(14)
DMULSCALE_FUNC(15)
DMULSCALE_FUNC(16)
DMULSCALE_FUNC(17)
DMULSCALE_FUNC(18)
DMULSCALE_FUNC(19)
DMULSCALE_FUNC(20)
DMULSCALE_FUNC(21)
DMULSCALE_FUNC(22)
DMULSCALE_FUNC(23)
DMULSCALE_FUNC(24)
DMULSCALE_FUNC(25)
DMULSCALE_FUNC(26)
DMULSCALE_FUNC(27)
DMULSCALE_FUNC(28)
DMULSCALE_FUNC(29)
DMULSCALE_FUNC(30)
DMULSCALE_FUNC(31)

// Special case for dmulscale32
static inline int32_t dmulscale32(int32_t a, int32_t b, int32_t c, int32_t d) {
	return dmulscale32_base(a, b, c, d);
}

// Base function for triple multiply and scale
static inline int32_t tmulscale_base(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int shift) {
	int64_t result1 = (int64_t)a * b;
	int64_t result2 = (int64_t)c * d;
	int64_t result3 = (int64_t)e * f;

	int64_t sum = result1 + result2 + result3;

	if (shift == 32) {
		return (int32_t)(sum >> 32);
	} else {
		return (int32_t)(sum >> shift);
	}
}

// Macro to generate all tmulscale functions
#define TMULSCALE_FUNC(n) \
static inline int32_t tmulscale##n(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f) { return tmulscale_base(a, b, c, d, e, f, n); }

// Generate all functions from 1 to 32
TMULSCALE_FUNC(1)
TMULSCALE_FUNC(2)
TMULSCALE_FUNC(3)
TMULSCALE_FUNC(4)
TMULSCALE_FUNC(5)
TMULSCALE_FUNC(6)
TMULSCALE_FUNC(7)
TMULSCALE_FUNC(8)
TMULSCALE_FUNC(9)
TMULSCALE_FUNC(10)
TMULSCALE_FUNC(11)
TMULSCALE_FUNC(12)
TMULSCALE_FUNC(13)
TMULSCALE_FUNC(14)
TMULSCALE_FUNC(15)
TMULSCALE_FUNC(16)
TMULSCALE_FUNC(17)
TMULSCALE_FUNC(18)
TMULSCALE_FUNC(19)
TMULSCALE_FUNC(20)
TMULSCALE_FUNC(21)
TMULSCALE_FUNC(22)
TMULSCALE_FUNC(23)
TMULSCALE_FUNC(24)
TMULSCALE_FUNC(25)
TMULSCALE_FUNC(26)
TMULSCALE_FUNC(27)
TMULSCALE_FUNC(28)
TMULSCALE_FUNC(29)
TMULSCALE_FUNC(30)
TMULSCALE_FUNC(31)

// Special case for tmulscale32 - returns high 32 bits
static inline int32_t tmulscale32(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f) {
	return tmulscale_base(a, b, c, d, e, f, 32);
}

// Generic divscale implementation using 64-bit arithmetic
static inline int32_t divscale_generic(int32_t a, int32_t b, int scale) {
	return (int32_t)(((int64_t)a << scale) / b);
}

// Optimized implementations for common cases
static inline int32_t divscale1_impl(int32_t a, int32_t b) {
    return (int32_t)(((int64_t)a << 1) / b);
}

static inline int32_t divscale16_impl(int32_t a, int32_t b) {
    return (int32_t)(((int64_t)a << 16) / b);
}

static inline int32_t divscale32_impl(int32_t a, int32_t b) {
    return (int32_t)((int64_t)a / b);
}

// Macro generator for all divscale functions
// Macro generator for all divscale functions
#define DIVSCALE_FUNC(n) static inline int32_t divscale##n(int32_t a, int32_t b) { 	return divscale_generic(a, b, n); }

// Generate all functions 1-31
DIVSCALE_FUNC(1)
DIVSCALE_FUNC(2)
DIVSCALE_FUNC(3)
DIVSCALE_FUNC(4)
DIVSCALE_FUNC(5)
DIVSCALE_FUNC(6)
DIVSCALE_FUNC(7)
DIVSCALE_FUNC(8)
DIVSCALE_FUNC(9)
DIVSCALE_FUNC(10)
DIVSCALE_FUNC(11)
DIVSCALE_FUNC(12)
DIVSCALE_FUNC(13)
DIVSCALE_FUNC(14)
DIVSCALE_FUNC(15)
DIVSCALE_FUNC(16)
DIVSCALE_FUNC(17)
DIVSCALE_FUNC(18)
DIVSCALE_FUNC(19)
DIVSCALE_FUNC(20)
DIVSCALE_FUNC(21)
DIVSCALE_FUNC(22)
DIVSCALE_FUNC(23)
DIVSCALE_FUNC(24)
DIVSCALE_FUNC(25)
DIVSCALE_FUNC(26)
DIVSCALE_FUNC(27)
DIVSCALE_FUNC(28)
DIVSCALE_FUNC(29)
DIVSCALE_FUNC(30)
DIVSCALE_FUNC(31)

// Special case for divscale32
static inline int32_t divscale32(int32_t a, int32_t b) {
    return a / b;
}

// Generic divscale with runtime scale parameter
static inline int32_t divscale(int32_t a, int32_t b, int scale) {
    if (scale == 0) return a / b;
    if (scale > 31) scale = 31;
    return divscale_generic(a, b, scale);
}

// Function pointer array for runtime dispatch (optional)
typedef int32_t (*divscale_func_t)(int32_t, int32_t);

static const divscale_func_t divscale_funcs[33] = {
    [0] = divscale32,
    [1] = divscale1, [2] = divscale2, [3] = divscale3, [4] = divscale4,
    [5] = divscale5, [6] = divscale6, [7] = divscale7, [8] = divscale8,
    [9] = divscale9, [10] = divscale10, [11] = divscale11, [12] = divscale12,
    [13] = divscale13, [14] = divscale14, [15] = divscale15, [16] = divscale16,
    [17] = divscale17, [18] = divscale18, [19] = divscale19, [20] = divscale20,
    [21] = divscale21, [22] = divscale22, [23] = divscale23, [24] = divscale24,
    [25] = divscale25, [26] = divscale26, [27] = divscale27, [28] = divscale28,
    [29] = divscale29, [30] = divscale30, [31] = divscale31, [32] = divscale32
};

// Runtime dispatch function
static inline int32_t divscale_dispatch(int32_t a, int32_t b, int scale) {
    if (scale < 0 || scale > 32) return divscale_generic(a, b, scale);
    return divscale_funcs[scale](a, b);
}

// Clear buffer with 32-bit value
static inline void clearbuf(void* dest, size_t count, uint32_t value) {
    uint32_t* ptr = (uint32_t*)dest;
    for (size_t i = 0; i < count; i++) {
        ptr[i] = value;
    }
}

// Clear buffer int8_t-wise with optimized alignment
static inline void clearbufbyte(void* dest, size_t count, uint8_t value) {
    uint8_t* ptr = (uint8_t*)dest;

    if (count < 4) {
        // Handle small buffers
        if (count & 1) {
            *ptr++ = value;
            count--;
        }
        uint16_t value16 = value | (value << 8);
        while (count >= 2) {
            *(uint16_t*)ptr = value16;
            ptr += 2;
            count -= 2;
        }
        return;
    }

    // Align to 4-int8_t boundary
    uint32_t value32 = value | (value << 8) | (value << 16) | (value << 24);

    while ((uintptr_t)ptr & 1) {
        *ptr++ = value;
        count--;
    }

    while ((uintptr_t)ptr & 2) {
        *(uint16_t*)ptr = value32;
        ptr += 2;
        count -= 2;
    }

    // Fill 4-int8_t chunks
    while (count >= 4) {
        *(uint32_t*)ptr = value32;
        ptr += 4;
        count -= 4;
    }

    // Handle remaining bytes
    if (count & 2) {
        *(uint16_t*)ptr = value32;
        ptr += 2;
    }
    if (count & 1) {
        *ptr = value;
    }
}

// Copy buffer 32-bit aligned
static inline void copybuf(const void* src, void* dest, size_t count) {
    const uint32_t* src32 = (const uint32_t*)src;
    uint32_t* dest32 = (uint32_t*)dest;
    for (size_t i = 0; i < count; i++) {
        dest32[i] = src32[i];
    }
}

// Copy buffer int8_t-wise with alignment optimization
static inline void copybufbyte(const void* src, void* dest, size_t count) {
    const uint8_t* src_ptr = (const uint8_t*)src;
    uint8_t* dest_ptr = (uint8_t*)dest;

    if (count < 4) {
        // Handle small buffers
        if (count & 1) {
            *dest_ptr++ = *src_ptr++;
            count--;
        }
        while (count >= 2) {
            *(uint16_t*)dest_ptr = *(const uint16_t*)src_ptr;
            dest_ptr += 2;
            src_ptr += 2;
            count -= 2;
        }
        return;
    }

    // Align destination to boundaries
    while ((uintptr_t)dest_ptr & 1) {
        *dest_ptr++ = *src_ptr++;
        count--;
    }

    while ((uintptr_t)dest_ptr & 2) {
        *(uint16_t*)dest_ptr = *(const uint16_t*)src_ptr;
        dest_ptr += 2;
        src_ptr += 2;
        count -= 2;
    }

    // Copy 4-int8_t chunks
    while (count >= 4) {
        *(uint32_t*)dest_ptr = *(const uint32_t*)src_ptr;
        dest_ptr += 4;
        src_ptr += 4;
        count -= 4;
    }

    // Handle remaining bytes
    if (count & 2) {
        *(uint16_t*)dest_ptr = *(const uint16_t*)src_ptr;
        dest_ptr += 2;
        src_ptr += 2;
    }
    if (count & 1) {
        *dest_ptr = *src_ptr;
    }
}

// Copy buffer in reverse with int8_t swapping
static inline void copybufreverse(const void* src, void* dest, size_t count) {
    const uint8_t* src_ptr = (const uint8_t*)src;
    uint8_t* dest_ptr = (uint8_t*)dest;

    // Handle odd int8_t
    if (count & 1) {
        *dest_ptr++ = *src_ptr--;
        count--;
    }

    // Handle 2-int8_t chunks with int8_t swap
    if (count & 2) {
        uint16_t val = *(const uint16_t*)(src_ptr - 1);
        *(uint16_t*)dest_ptr = ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
        dest_ptr += 2;
        src_ptr -= 2;
        count -= 2;
    }

    // Handle 4-int8_t chunks with full reversal
    while (count >= 4) {
        uint32_t val = *(const uint32_t*)(src_ptr - 3);
        // Byte swap 32-bit value
        val = ((val & 0xFF000000) >> 24) |
              ((val & 0x00FF0000) >> 8)  |
              ((val & 0x0000FF00) << 8)  |
              ((val & 0x000000FF) << 24);
        *(uint32_t*)dest_ptr = val;
        dest_ptr += 4;
        src_ptr -= 4;
        count -= 4;
    }
}

// Absolute value
static inline int32_t klabs(long x) {
	return labs(x);
}

// Sign function: returns -1, 0, or 1
static inline int32_t ksgn(int32_t x) {
	return (x > 0) - (x < 0);
}

// Unsigned minimum
static inline uint32_t umin(uint32_t a, uint32_t b) {
	return (a < b) ? a : b;
}

// Unsigned maximum
static inline uint32_t umax(uint32_t a, uint32_t b) {
	return (a > b) ? a : b;
}

// Signed minimum
static inline int32_t kmin(int32_t a, int32_t b) {
	return (a < b) ? a : b;
}

// Signed maximum
static inline int32_t kmax(int32_t a, int32_t b) {
	return (a > b) ? a : b;
}

//static long timeroffs1mhz;


// Convert deltatime1mhz from assembly to C
inline uint32_t deltatime1mhz(uint32_t start_time, uint32_t current_time) {
	uint32_t delta = current_time - start_time;

	// Handle timer overflow (equivalent to jnc skipit + add)
	if (current_time < start_time) {
		delta += 0x0fff0000;
	}

	return delta;
}

// Convert printchrasm from assembly to C
inline void printchrasm(uint16_t* dest, uint32_t count, uint16_t value) {
	for (uint32_t i = 0; i < count; i++) {
		dest[i] = value;
	}
}
#endif
