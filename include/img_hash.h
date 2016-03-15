/*
 * Hash accelerator driver header file for MIPS32 CPU-core SPL
 *
 * Copyright (c) 2015 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _IMG_HASH_H_
#define _IMG_HASH_H_

#include <crypto/md5.h>
#include <crypto/sha.h>

#define CHUNKSZ_DUMMY	0

int img_hash_digest(u64 addr, u64 len, u32 digsize, u32 *digest);
void imgtec_hwsha1(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz);
void imgtec_hwsha256(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz);
void imgtec_hwsha224(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz);
void imgtec_hwmd5(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz);

#endif	/* _IMG_HASH_H_ */
