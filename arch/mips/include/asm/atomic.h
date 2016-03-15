/* MIPS atomic operations
 *
 * (C) Copyright 2015 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_MIPS_ATOMIC_H_
#define _ASM_MIPS_ATOMIC_H_

#ifdef CONFIG_SMP
#error SMP not supported
#endif

typedef struct { volatile int counter; } atomic_t;

#define atomic_read(v)  ((v)->counter)
#define atomic_set(v,i) (((v)->counter) = (i))

/*
 * The atomic operations are used for budgeting etc which is not
 * needed for the read-only U-Boot implementation:
 */
static inline void atomic_add(int i, volatile atomic_t *v) { }

static inline void atomic_sub(int i, volatile atomic_t *v) { }

static inline void atomic_inc(volatile atomic_t *v) { }

static inline void atomic_dec(volatile atomic_t *v) { }

#endif	/* _ASM_MIPS_ATOMIC_H_ */
