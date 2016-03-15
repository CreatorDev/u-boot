/*
 * Hash accelerator driver code for MIPS32 CPU-core SPL
 *
 * Copyright (c) 2015 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <config.h>
#include <common.h>
#include <div64.h>
#include <errno.h>
#include <fuse.h>
#include <image.h>
#include <img_hash.h>
#include <malloc.h>
#include <mapmem.h>

#define BIT(nr)				(1UL << (nr))

#define CR_RESET			0
#define CR_RESET_SET			1
#define CR_RESET_UNSET			0

#define CR_MESSAGE_LENGTH_H		0x4
#define CR_MESSAGE_LENGTH_L		0x8

#define CR_CONTROL			0xc
#define CR_CONTROL_BYTE_ORDER_3210	0
#define CR_CONTROL_BYTE_ORDER_0123	1
#define CR_CONTROL_BYTE_ORDER_2310	2
#define CR_CONTROL_BYTE_ORDER_1032	3
#define CR_CONTROL_BYTE_ORDER_SHIFT	8
#define CR_CONTROL_ALGO_MD5	0
#define CR_CONTROL_ALGO_SHA1	1
#define CR_CONTROL_ALGO_SHA224	2
#define CR_CONTROL_ALGO_SHA256	3

#define CR_INTSTAT			0x10
#define CR_INTENAB			0x14
#define CR_INTCLEAR			0x18
#define CR_INT_RESULTS_AVAILABLE	BIT(0)
#define CR_INT_NEW_RESULTS_SET		BIT(1)
#define CR_INT_RESULT_READ_ERR		BIT(2)
#define CR_INT_MESSAGE_WRITE_ERROR	BIT(3)
#define CR_INT_STATUS			BIT(8)

#define CR_RESULT_QUEUE			0x1c
#define CR_RSD0				0x40
#define CR_CORE_REV			0x50
#define CR_CORE_DES1			0x60
#define CR_CORE_DES2			0x70

#define DRIVER_FLAGS_SHA1		BIT(18)
#define DRIVER_FLAGS_SHA224		BIT(19)
#define DRIVER_FLAGS_SHA256		BIT(20)
#define DRIVER_FLAGS_MD5		BIT(21)

#ifdef __LITTLE_ENDIAN
#define IMG_HASH_BYTE_ORDER		CR_CONTROL_BYTE_ORDER_3210
#else
#define IMG_HASH_BYTE_ORDER		CR_CONTROL_BYTE_ORDER_0123
#endif

static inline u32 img_hash_read(u32 base, u32 offset)
{
	return __raw_readl(base + offset);
}

static inline void img_hash_write(u32 base, u32 offset, u32 value)
{
	__raw_writel(value, base + offset);
}

/*
 * img_hash_hw_init software resets the hash accelerator and sets
 * the message length register.
 */
static int img_hash_hw_init(u64 len)
{
	u64 nbits;
	u32 u, l;

	img_hash_write(HASH_BASE_ADDRESS, CR_RESET, CR_RESET_SET);
	img_hash_write(HASH_BASE_ADDRESS, CR_RESET, CR_RESET_UNSET);
	nbits = len << 3;
	u = nbits >> 32;
	debug("upper message length: %u\n", u);
	l = (u32)nbits;
	debug("lower message length: %u\n", l);
	img_hash_write(HASH_BASE_ADDRESS, CR_MESSAGE_LENGTH_H, u);
	img_hash_write(HASH_BASE_ADDRESS, CR_MESSAGE_LENGTH_L, l);
	debug("hw initialized\n");
	return 0;
}

/*
 * img_hash_start sets the control bit in the hash accelerator. This will
 * also make the hash accelerator to read from its write port
 */
static int img_hash_start(u32 flag)
{
	u32 cr = IMG_HASH_BYTE_ORDER << CR_CONTROL_BYTE_ORDER_SHIFT;

	if (flag & DRIVER_FLAGS_MD5)
		cr |= CR_CONTROL_ALGO_MD5;
	else if (flag & DRIVER_FLAGS_SHA1)
		cr |= CR_CONTROL_ALGO_SHA1;
	else if (flag & DRIVER_FLAGS_SHA224)
		cr |= CR_CONTROL_ALGO_SHA224;
	else if (flag & DRIVER_FLAGS_SHA256)
		cr |= CR_CONTROL_ALGO_SHA256;

	debug("Control bits set in hash accelerator\n");
	img_hash_write(HASH_BASE_ADDRESS, CR_CONTROL, cr);

	/*
	 * The hardware block requires two cycles between writing the control
	 * register and writing the first word of data
	 */
	cr = img_hash_read(HASH_BASE_ADDRESS, CR_CONTROL);
	return 0;
}

/*
 * img_hash_check_method determines the hashing algorithm according
 * to the digest size and specifies it in the flag
 */
static int img_hash_check_method(u32 digsize, u32 *flag)
{
	switch (digsize) {
	case SHA1_DIGEST_SIZE:
		*flag |= DRIVER_FLAGS_SHA1;
		break;
	case SHA256_DIGEST_SIZE:
		*flag |= DRIVER_FLAGS_SHA256;
		break;
	case SHA224_DIGEST_SIZE:
		*flag |= DRIVER_FLAGS_SHA224;
		break;
	case MD5_DIGEST_SIZE:
		*flag |= DRIVER_FLAGS_MD5;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/*
 * img_hash_xmit takes writes from the image buffer to the hash accelerator
 * write port in the specified byte order
 */
static int img_hash_xmit(u64 len, u64 addr)
{
	u64 writecount;
	u64 readcount = 0;
	u32 buf;
	u32 *bufptr = &buf;
	u8 *bufp = (u8 *)bufptr;
	u64 wordlen;
	u64 remainder = len % 4;
	u64 offset;

	wordlen = len / 4;
	offset = wordlen * 4;
	debug("wordlen: %d\n", (int)wordlen);
	debug("remainder: %d\n", (int)remainder);
	/* run the loop for the maximum number of whole words */
	for (writecount = 0; writecount < wordlen; writecount++) {
		buf = img_hash_read(addr, writecount*4);
#ifdef __LITTLE_ENDIAN
		img_hash_write(HASH_WRITE_PORT, 0, buf);
#else
		img_hash_write(HASH_WRITE_PORT, 0, cpu_to_be32(buf));
#endif
	}
	/* and then write the remaining bytes */
	for (readcount = 0; readcount < remainder; readcount++) {
		*(bufp + readcount) = __raw_readb((u32)(addr +
						readcount + offset));
		debug("in buffer : %u\n", buf);
		debug("readcount : %d, addr: %d, offset: %d\n",
		      (int)readcount, (int)addr, (int)offset);
	}
	if (remainder) {
#ifdef __LITTLE_ENDIAN
		img_hash_write(HASH_WRITE_PORT, 0, buf);
#else
		img_hash_write(HASH_WRITE_PORT, 0, cpu_to_be32(buf));
#endif
	}

	if (len == 0)
		debug("Warning: Image length is zero\n");

	debug("finished writing to write port\n");

	/*
	 * need an udelay between img_hash_xmit and img_hash_poll or else
	 * the digest buffer will be filled by 0
	 */
	udelay(1);
	return 0;
}

/*
 * img_hash_get_result reads the digest from the result queue and loads
 * them into the digest buffer in a reversed order
 */
static int img_hash_get_result(u32 *digest, u32 digsize)
{
	u32 tmp;
	int count;

	for (count = (int)(digsize / 4); count > 0; count--) {
		tmp = img_hash_read(HASH_BASE_ADDRESS, CR_RESULT_QUEUE);
		digest[count-1] = tmp;
	}
	debug("Result obtained!\n");
	return 0;
}

/*
 * img_hash_poll polls from the INSTAT register. It attempts to read the
 * result if the INSTAT register indicates a new result is available
 */
static int img_hash_poll(u32 *digest, u32 digsize)
{
	int err = 1;
	u32 reg = 0;

	debug("polling...  ");
	while (!reg) {
		reg = img_hash_read(HASH_BASE_ADDRESS, CR_INTSTAT);
		img_hash_write(HASH_BASE_ADDRESS, CR_INTCLEAR, reg);
		if (reg & CR_INT_RESULT_READ_ERR) {
			debug("Error! Read on a empty result queue!\n");
			return err;
		}
		if (reg & CR_INT_MESSAGE_WRITE_ERROR) {
			debug("Error! Write attempt before configuring!\n");
			return err;
		}
		if (reg & CR_INT_NEW_RESULTS_SET) {
			debug("new result set in the result queue!\n");
			img_hash_get_result(digest, digsize);
		}
	}
	return 0;
}

/*
 * img_hash_digest_to_array moves the digest from a 32 bit pointer into the 8
 * bits output buffer created by the hash command.
 */
static void img_hash_digest_to_array(unsigned char *output, u32 *digest,
				     int len)
{
	int count;
	u32 *tmpptr = (u32 *) output;

	len = len / 4;
	for (count = 0; count < len; count++)
		tmpptr[count] = be32_to_cpu(digest[count]);
}

int img_hash_digest(u64 addr, u64 len, u32 digsize, u32 *digest)
{
	int err;
	u32 flag = 0;
	u32 *flagptr = &flag;

	/* configures the hardware and write to the write port */
	err = img_hash_check_method(digsize, flagptr);
	if (err != 0) {
		debug("digsize is not valid\n");
		return 1;
	}
	img_hash_hw_init(len);
	img_hash_start(flag);
	img_hash_xmit(len, addr);

	/* now the machine polls and waits for the result to be done */
	err = img_hash_poll(digest, digsize);
	if (err != 0)
		return 1;

	debug("program finished\n");
	return 0;

}

void imgtec_hwsha1(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz)
{
	int ret;
	u64 addr = (u64)map_to_sysmem(input);
	u32 *digest = (u32 *)malloc(SHA1_DIGEST_SIZE * (sizeof(u8)));

	ret = img_hash_digest(addr, (u64)ilen, SHA1_DIGEST_SIZE, digest);
	if (ret) {
		free(digest);
		return;
	}
	img_hash_digest_to_array(output, digest, SHA1_DIGEST_SIZE);
	free(digest);
}

void imgtec_hwsha256(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz)
{
	int ret;
	u64 addr = (u64)map_to_sysmem(input);
	u32 *digest = (u32 *)malloc(SHA256_DIGEST_SIZE * (sizeof(u8)));

	ret = img_hash_digest(addr, (u64)ilen, SHA256_DIGEST_SIZE, digest);
	if (ret) {
		free(digest);
		return;
	}
	img_hash_digest_to_array(output, digest, SHA256_DIGEST_SIZE);
	free(digest);
}
void imgtec_hwsha224(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz)
{
	int ret;
	u64 addr = (u64)map_to_sysmem(input);
	u32 *digest = (u32 *)malloc(SHA224_DIGEST_SIZE * (sizeof(u8)));

	ret = img_hash_digest(addr, (u64)ilen, SHA224_DIGEST_SIZE, digest);
	if (ret) {
		free(digest);
		return;
	}
	img_hash_digest_to_array(output, digest, SHA224_DIGEST_SIZE);
	free(digest);
}
void imgtec_hwmd5(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz)
{
	int ret;
	u64 addr = (u64)map_to_sysmem(input);
	u32 *digest = (u32 *)malloc(MD5_DIGEST_SIZE * (sizeof(u8)));

	ret = img_hash_digest(addr, (u64)ilen, MD5_DIGEST_SIZE, digest);
	if (ret) {
		free(digest);
		return;
	}
	img_hash_digest_to_array(output, digest, MD5_DIGEST_SIZE);
	free(digest);
}
