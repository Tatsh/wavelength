/* gzip (GNU zip) -- compress files with zip algorithm and 'compress' interface
 * Copyright (C) 1992-1993 Jean-loup Gailly
 * The unzip code was written and put in the public domain by Mark Adler.
 * Portions of the lzw code are derived from the public domain 'compress'
 * written by Spencer Thomas, Joe Orost, James Woods, Jim McKie, Steve Davies,
 * Ken Turkowski, Dave Mack and Peter Jannesen.
 *
 * See the file COPYING for the software license.
 * See the file algorithm.doc for the compression algorithms and file formats.
 */

#ifdef RCSID
static char rcsid[] = "$Id: gzip.c,v 0.24 1993/06/24 10:52:07 jloup Exp $";
#endif

#include <ctype.h>
#include <sys/types.h>
#include <errno.h>

#include "tailor.h"
#include "gzip.h"
#include "lzw.h"

#ifndef MAX_PATH_LEN
#  define MAX_PATH_LEN   1024 /* max pathname length */
#endif

		/* global buffers */

DECLARE(uch, inbuf,  INBUFSIZ +INBUF_EXTRA);
DECLARE(uch, outbuf, OUTBUFSIZ+OUTBUF_EXTRA);
DECLARE(ush, d_buf,  DIST_BUFSIZE);
DECLARE(uch, window, 2L*WSIZE);
#ifndef MAXSEG_64K
    DECLARE(ush, tab_prefix, 1L<<BITS);
#else
    DECLARE(ush, tab_prefix0, 1L<<(BITS-1));
    DECLARE(ush, tab_prefix1, 1L<<(BITS-1));
#endif

		/* local variables */

int method = DEFLATED;/* compression method */
int level = 6;        /* compression level */
int exit_code = OK;   /* program exit code */
local int decompress = 0;   /* decompress (-d) */
local int no_time = -1;     /* don't save or restore the original file time */
long time_stamp;      /* original time stamp (modification time) */

long bytes_in;             /* number of input bytes */
long bytes_out;            /* number of output bytes */
char ifname[MAX_PATH_LEN]; /* input file name */
int  ifd;                  /* input file descriptor */
int  ofd;                  /* output file descriptor */
unsigned insize;           /* valid bytes in inbuf */
unsigned inptr;            /* index of next byte to be processed in inbuf */
unsigned outcnt;           /* bytes in output buffer */

const uch *gzipInSrcBuff;  /* start of the memory source */
const uch *gzipInSrcPtr;   /* next byte of the memory source */
int gzipInSrcSize;         /* length of the memory source */

/* local functions */

local int  get_method   OF((int in));

/* ========================================================================
 * Check the magic number of the input file.
 * Return the compression method, -1 for error.
 * Set inptr to the offset of the next byte to be processed.
 * Updates time_stamp if there is one and --no-time is not used.
 * IN assertions: there is at least one remaining compressed member.
 * NTSC-U/C: 0x00282fb0, PAL: 0x0028c860
 */
local int get_method(in)
    int in;        /* input file descriptor */
{
    uch flags;     /* compression flags */
    char magic[2]; /* magic header */
    ulg stamp;     /* time stamp */

    magic[0] = (char)get_byte();
    magic[1] = (char)get_byte();
    method = -1;                 /* unknown yet */
    header_bytes = 0;

    if (memcmp(magic, GZIP_MAGIC, 2) == 0) {

	method = (int)get_byte();
	if (method != DEFLATED) {
	    fprintf(stderr,
		    "%s: unknown method %d -- get newer version of gzip\n",
		    ifname, method);
	    exit_code = ERROR;
	    return -1;
	}
	flags  = (uch)get_byte();

	if ((flags & (ENCRYPTED | CONTINUATION | RESERVED)) != 0) {
	    fprintf(stderr,
		    "%s has flags it shouldn't have for this version of gzip\n",
		    ifname);
	    exit_code = ERROR;
	    return -1;
	}
	stamp  = (ulg)get_byte();
	stamp |= ((ulg)get_byte()) << 8;
	stamp |= ((ulg)get_byte()) << 16;
	stamp |= ((ulg)get_byte()) << 24;
	if (stamp != 0 && !no_time) time_stamp = stamp;

	(void)get_byte();  /* Ignore extra flags for the moment */
	(void)get_byte();  /* Ignore OS type for the moment */

	if ((flags & EXTRA_FIELD) != 0) {
	    unsigned len = (unsigned)get_byte();
	    len |= ((unsigned)get_byte())<<8;
	    while (len--) (void)get_byte();
	}

	/* Discard the original file name if any */
	if ((flags & ORIG_NAME) != 0) {
	    char c; /* dummy used for NeXTstep 3.0 cc optimizer bug */
	    do {c=get_byte();} while (c != 0);
	}

	/* Discard file comment if any */
	if ((flags & COMMENT) != 0) {
	    while (get_char() != 0) /* null */ ;
	}
	header_bytes = inptr + 2*sizeof(long); /* include crc and size */
    }
    if (method >= 0) return method;

    fprintf(stderr, "\n%s: not in gzip format\n", ifname);
    exit_code = ERROR;
    return -1;
}

/* ========================================================================
 * Mark the module initialized.
 * NTSC-U/C: 0x002834b8, PAL: 0x0028cd68
 */
void GzipInit()
{
    static int bInit = 0;

    if (!bInit) {
	bInit = 1;
    }
}

/* ========================================================================
 * Decompress or compress the input to a buffer and return the length of the
 * output, or -1 on error.
 * NTSC-U/C: 0x002834d8, PAL: 0x0028cd88
 */
int GzipFdToRam(int in, void *out, int compress)
{
    int (*work) OF((void)); /* function to call */

    GzipInit();
    ifd = in;
    init_outdest((uch *)out);
    clear_bufs();
    if (!compress) {
	decompress = 1;
	work = unzip;
	method = get_method(ifd);
	if (method < 0) return -1;
    } else {
	decompress = 0;
	method = DEFLATED;
	work = zip;
    }
    if ((*work)() != OK) return -1;
    return size_outdest();
}

/* ========================================================================
 * Decompress a gzip member in memory to a buffer.
 * NTSC-U/C: 0x002835b0, PAL: 0x0028ce60
 */
int GzipDecompressRamToRam(const void *src, int len, void *out)
{
    gzipInSrcSize = len;
    gzipInSrcPtr = (const uch *)src;
    gzipInSrcBuff = (const uch *)src;
    strcpy(ifname, "(in-memory)");
    return GzipFdToRam(NO_FILE, out, 0);
}

/* ========================================================================
 * Return the uncompressed length that ends a gzip member in memory.
 * NTSC-U/C: 0x00283620, PAL: 0x0028ced0
 */
int GzipSizeDecompressed(const void *src, int len)
{
    int size;

    memcpy(&size, (const char *)src + len - sizeof(size), sizeof(size));
    return size;
}

/* ========================================================================
 * Compress bytes in memory to a gzip member in a buffer.
 * NTSC-U/C: 0x00283648, PAL: 0x0028cef8
 */
int GzipCompressRamToRam(const void *src, int len, void *out)
{
    gzipInSrcSize = len;
    gzipInSrcPtr = (const uch *)src;
    gzipInSrcBuff = (const uch *)src;
    strcpy(ifname, "(in-memory)");
    return GzipFdToRam(NO_FILE, out, 1);
}

/* ========================================================================
 * Write the name of the compressed copy of a file, either under gen beside
 * the file or the file name with .gz appended.
 * NTSC-U/C: 0x002836b8, PAL: 0x0028cf68
 */
void GzipFileName(const char *name, int gen, char *out)
{
    if (gen) {
	sprintf(out, "%s/gen/%s.%s.gz", FileGetPath(name),
		FileGetBaseName(name), FileGetExt(name));
    } else {
	sprintf(out, "%s.gz", name);
    }
}
