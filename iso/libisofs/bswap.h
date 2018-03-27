/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * From the mplayer project (www.mplayerhq.hu)                               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef BSWAP_H
#define BSWAP_H

/* this file doesn't exist
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
*/

#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#else

#ifdef ARCH_X86
inline static unsigned short ByteSwap16(unsigned short x)
{
__asm("xchgb %b0,%h0" :
          "=q"(x) :
                  "0"(x));
    return x;
}
#define bswap_16(x) ByteSwap16(x)

inline static unsigned int ByteSwap32(unsigned int x)
{
#if __CPU__ > 386
__asm("bswap %0":
          "=r"(x)     :
#else
                  __asm("xchgb %b0,%h0\n"
                        " rorl $16,%0\n"
                        " xchgb %b0,%h0":
                        "=q"(x)  :
#endif
                        "0"(x));
                  return x;
              }
#define bswap_32(x) ByteSwap32(x)

      inline static unsigned long long int ByteSwap64(unsigned long long int x)
{
    register union { __extension__ unsigned long long int __ll;
        unsigned int __l[2];
    } __x;
asm("xchgl %0,%1":
        "=r"(__x.__l[0]), "=r"(__x.__l[1]):
                "0"(bswap_32((unsigned long)x)), "1"(bswap_32((unsigned long)(x >> 32))));
    return __x.__ll;
}
#define bswap_64(x) ByteSwap64(x)

#else

#define bswap_16(x) (((x) & 0x00ff) << 8 | ((x) & 0xff00) >> 8)


/* code from bits/byteswap.h (C) 1997, 1998 Free Software Foundation, Inc. */
#define bswap_32(x) \
    ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
     (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define bswap_64(x) \
    (__extension__      \
    ({ union { __extension__ unsigned long long int __ll; \
            unsigned int __l[2]; } __w, __r;  \
        __w.__ll = (x);     \
        __r.__l[0] = bswap_32 (__w.__l[1]);   \
        __r.__l[1] = bswap_32 (__w.__l[0]);   \
        __r.__ll; }))
#endif /* !ARCH_X86 */

#endif /* !HAVE_BYTESWAP_H */

/*
 be2me ... BigEndian to MachineEndian
 le2me ... LittleEndian to MachineEndian
*/

#ifdef WORDS_BIGENDIAN
#define be2me_16(x) (x)
#define be2me_32(x) (x)
#define be2me_64(x) (x)
#define le2me_16(x) bswap_16(x)
#define le2me_32(x) bswap_32(x)
#define le2me_64(x) bswap_64(x)
#else
#define be2me_16(x) bswap_16(x)
#define be2me_32(x) bswap_32(x)
#define be2me_64(x) bswap_64(x)
#define le2me_16(x) (x)
#define le2me_32(x) (x)
#define le2me_64(x) (x)
#endif

#endif
