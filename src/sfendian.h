/*
** Copyright (C) 1999-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "config.h"

#if defined (HAVE_BYTESWAP_H)

#include <byteswap.h>

#define	ENDSWAP_SHORT(x)	bswap_16 (x)
#define	ENDSWAP_INT(x)		bswap_32 (x)

#else

#define	ENDSWAP_SHORT(x)	((((x)>>8)&0xFF)+(((x)&0xFF)<<8))
#define	ENDSWAP_INT(x)		((((x)>>24)&0xFF)+(((x)>>8)&0xFF00)+(((x)&0xFF00)<<8)+(((x)&0xFF)<<24))

#endif

#if defined (SIZEOF_INT64_T) && (SIZEOF_INT64_T == 8)

#define HAVE_SF_INT64_T 1

#else
#define HAVE_SF_INT64_T 0
#endif

/*
** Many file types (ie WAV, AIFF) use sets of four consecutive bytes as a
** marker indicating different sections of the file.
** The following MAKE_MARKER macro allows th creation of integer constants
** for these markers.
*/

#if (CPU_IS_LITTLE_ENDIAN == 1)
	#define	MAKE_MARKER(a,b,c,d)	((a)|((b)<<8)|((c)<<16)|((d)<<24))
#elif (CPU_IS_BIG_ENDIAN == 1)
	#define	MAKE_MARKER(a,b,c,d)	(((a)<<24)|((b)<<16)|((c)<<8)|(d))
#else
	#error "Target CPU endian-ness unknown. May need to hand edit src/config.h"
#endif

/*
** Macros to handle reading of data of a specific endian-ness into host endian
** shorts and ints. The single input is an unsigned char* pointer to the start
** of the object. There are two versions of each macro as we need to deal with
** both big and little endian CPUs.
*/

#if (CPU_IS_LITTLE_ENDIAN == 1)
	#define LES2H_SHORT(x)			(x)
	#define LEI2H_INT(x)			(x)

	#define BES2H_SHORT(x)			ENDSWAP_SHORT(x)
	#define BEI2H_INT(x)			ENDSWAP_INT(x)

#elif (CPU_IS_BIG_ENDIAN == 1)
	#define LES2H_SHORT(x)			ENDSWAP_SHORT(x)
	#define LEI2H_INT(x)			ENDSWAP_INT(x)

	#define BES2H_SHORT(x)			(x)
	#define BEI2H_INT(x)			(x)

#else
	#error "Target CPU endian-ness unknown. May need to hand edit src/config.h"
#endif

#define LET2H_SHORT_PTR(x)		((x) [1] + ((x) [2] << 8))
#define LET2H_INT_PTR(x)		(((x) [0] << 8) + ((x) [1] << 16) + ((x) [2] << 24))

#define BET2H_SHORT_PTR(x)		(((x) [0] << 8) + (x) [1])
#define BET2H_INT_PTR(x)		(((x) [0] << 24) + ((x) [1] << 16) + ((x) [2] << 8))

/*-----------------------------------------------------------------------------------------------
** Generic functions for performing endian swapping on integer arrays.
*/

static inline void
endswap_short_array (short *ptr, int len)
{	short	temp ;

	while (--len >= 0)
	{	temp = ptr [len] ;
		ptr [len] = ENDSWAP_SHORT (temp) ;
		} ;
} /* endswap_short_array */

static inline void
endswap_int_array (int *ptr, int len)
{	int temp ;

	while (--len >= 0)
	{	temp = ptr [len] ;
		ptr [len] = ENDSWAP_INT (temp) ;
		} ;
} /* endswap_int_array */

#if  (defined (HAVE_BYTESWAP_H) && defined (SIZEOF_INT64_T) && (SIZEOF_INT64_T == 8))

static inline void
endswap_long_array (int64_t *ptr, int len)
{	int64_t value ;

	while (--len >= 0)
	{	value = ptr [len] ;
		ptr [len] = bswap_64 (value) ;
		} ;
} /* endswap_long_array */

#else

/*	This function assumes that sizeof (long) == 8, but works correctly even
**	is sizeof (long) == 4.
*/
static inline void
endswap_long_array (void *ptr, int len)
{	unsigned char *ucptr, temp ;

	ucptr = (unsigned char *) ptr + 8 * len ;
	while (--len >= 0)
	{	ucptr -= 8 ;

		temp = ucptr [0] ;
		ucptr [0] = ucptr [7] ;
		ucptr [7] = temp ;

		temp = ucptr [1] ;
		ucptr [1] = ucptr [6] ;
		ucptr [6] = temp ;

		temp = ucptr [2] ;
		ucptr [2] = ucptr [5] ;
		ucptr [5] = temp ;

		temp = ucptr [3] ;
		ucptr [3] = ucptr [4] ;
		ucptr [4] = temp ;
		} ;
} /* endswap_long_array */

#endif

/*========================================================================================
*/

static inline void
endswap_short_copy (short *dest, const short *src, int len)
{
	while (--len >= 0)
	{	dest [len] = ENDSWAP_SHORT (src [len]) ;
		} ;
} /* endswap_short_copy */

static inline void
endswap_int_copy (int *dest, const int *src, int len)
{
	while (--len >= 0)
	{	dest [len] = ENDSWAP_INT (src [len]) ;
		} ;
} /* endswap_int_copy */

#if  (defined (HAVE_BYTESWAP_H) && defined (SIZEOF_INT64_T) && (SIZEOF_INT64_T == 8))

static inline void
endswap_long_copy (int64_t *dest, const int64_t *src, int len)
{	int64_t value ;

	while (--len >= 0)
	{	value = src [len] ;
		dest [len] = bswap_64 (value) ;
		} ;
} /* endswap_long_copy */

#else

/*
**	This function assumes that sizeof (long) == 8, but works correctly even
**	if sizeof (long) == 4.
*/
static inline void
endswap_long_copy (void *dest, const void *src, int len)
{	const unsigned char *psrc ;
	unsigned char *pdest ;

	psrc = ((const unsigned char *) src) + 8 * len ;
	pdest = ((unsigned char *) dest) + 8 * len ;
	while (--len >= 0)
	{	psrc -= 8 ;
		pdest -= 8 ;

		pdest [0] = psrc [7] ;
		pdest [1] = psrc [6] ;
		pdest [2] = psrc [5] ;
		pdest [3] = psrc [4] ;
		pdest [4] = psrc [3] ;
		pdest [5] = psrc [2] ;
		pdest [6] = psrc [1] ;
		pdest [7] = psrc [0] ;
		} ;
} /* endswap_long_copy */

#endif

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: f0c5cd54-42d3-4237-90ec-11fe24995de7
*/
