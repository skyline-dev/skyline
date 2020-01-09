/**
 * @file smc.c
 * @copyright libnx Authors
 */

#include <stddef.h>

#include "mem.h"

#include "skyline/nx/result.h"
#include "skyline/nx/kernel/svc.h"
#include "skyline/nx/smc.h"

static Result _smcWriteAddress(void *dst_addr, u64 val, u32 size)
{
    SecmonArgs args;
    args.X[0] = 0xF0000003;     /* smcAmsWriteAddress */
    args.X[1] = (u64)dst_addr;  /* DRAM address */
    args.X[2] = val;            /* value */
    args.X[3] = size;           /* Amount to write */
    Result rc = svcCallSecureMonitor(&args);
    if (rc == 0)
    {
        if (args.X[0] != 0)
        {
            /* SPL result n = SMC result n */
            rc = (26u | ((u32)args.X[0] << 9u));
        }
    }
    return rc;
}

#define UNALIGNED(X, Y) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

#define BIGBLOCKSIZE    (sizeof (long) << 2)
#define LITTLEBLOCKSIZE (sizeof (long))
#define LBLOCKSIZE (sizeof(long))

#undef TOO_SMALL
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

void* smcMemCpy(void* dst_addr, void* src_addr, size_t size) {
    char *dst = dst_addr;
    const char *src = src_addr;
    long *aligned_dst;
    const long *aligned_src;

  /* If the size is small, or either SRC or DST is unaligned,
     then punt into the byte copy loop.  This should be rare.  */
    if (!TOO_SMALL(size) && !UNALIGNED (src, dst)) 
    {
        aligned_dst = (long*)dst;
        aligned_src = (long*)src;

        /* Copy 4X long words at a time if possible.  */
        while (size >= BIGBLOCKSIZE)
        {
            smcWriteAddress64(aligned_dst++, *aligned_src++);
            smcWriteAddress64(aligned_dst++, *aligned_src++);
            smcWriteAddress64(aligned_dst++, *aligned_src++);
            smcWriteAddress64(aligned_dst++, *aligned_src++);
            size -= BIGBLOCKSIZE;
        }

        /* Copy one long word at a time if possible.  */
        while (size >= LITTLEBLOCKSIZE)
        {
            smcWriteAddress64(aligned_dst++, *aligned_src++);
            size -= LITTLEBLOCKSIZE;
        }

        /* Pick up any residual with a byte copier.  */
        dst = (char*)aligned_dst;
        src = (char*)aligned_src;
    }

    while (size--)
        smcWriteAddress8(dst++, *src++);


    return dst_addr;
}

#undef LBLOCKSIZE
#undef UNALIGNED
#undef TOO_SMALL

#define LBLOCKSIZE (sizeof(long))
#define UNALIGNED(X)   ((long)X & (LBLOCKSIZE - 1))
#define TOO_SMALL(LEN) ((LEN) < LBLOCKSIZE)

void *smcMemSet(void* dst_addr, u32 value, size_t size) {
    char *s = (char *) dst_addr;

    unsigned int i;
    unsigned long buffer;
    unsigned long *aligned_addr;
    unsigned int d = value & 0xff;	/* To avoid sign extension, copy C to an unsigned variable.  */

    while (UNALIGNED(s))
    {
        if (size--)
            smcWriteAddress8(s++, (char)value);
        else
            return dst_addr;
    }

    if (!TOO_SMALL(size))
    {
        /* If we get this far, we know that n is large and s is word-aligned. */
        aligned_addr = (unsigned long *) s;

        /* Store D into each char sized location in BUFFER so that
            we can set large blocks quickly.  */
        buffer = (d << 8) | d;
        buffer |= (buffer << 16);
        for (i = 32; i < LBLOCKSIZE * 8; i <<= 1)
        buffer = (buffer << i) | buffer;

        /* Unroll the loop.  */
        while (size >= LBLOCKSIZE*4)
        {
            smcWriteAddress64(aligned_addr++, buffer);
            smcWriteAddress64(aligned_addr++, buffer);
            smcWriteAddress64(aligned_addr++, buffer);
            smcWriteAddress64(aligned_addr++, buffer);
            size -= 4*LBLOCKSIZE;
        }

        while (size >= LBLOCKSIZE)
        {
            smcWriteAddress64(aligned_addr++, buffer);
            size -= LBLOCKSIZE;
        }
        /* Pick up the remainder with a bytewise loop.  */
        s = (char*)aligned_addr;
    }

    while (size--)
    smcWriteAddress8(s++, (char) value);

    return dst_addr;
}

Result smcWriteAddress8(void *dst_addr, u8 val)
{
    return _smcWriteAddress(dst_addr, val, 1);
}

Result smcWriteAddress16(void *dst_addr, u16 val)
{
    return _smcWriteAddress(dst_addr, val, 2);
}

Result smcWriteAddress32(void *dst_addr, u32 val)
{
    return _smcWriteAddress(dst_addr, val, 4);
}

Result smcWriteAddress64(void *dst_addr, u64 val)
{
    return _smcWriteAddress(dst_addr, val, 8);
}
