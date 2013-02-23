/* This work is free. I, the authour, release this work into the public domain.
 * 
 * This work comes AS IS, WITH ALL FAULTS, WITH NO WARRANTY OF ANY KIND WHATSOEVER, not even the implied warranties of MERCHANTABILITY, FITNESS, and TITLE.
 * In NO CASE SHALL THE AUTHOUR BE LIABLE for ANY DAMAGES OR HARMS CAUSED BY OR LINKED TO THIS WORK.
 */

#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "miniz.h"
#include "util.h"

typedef struct {
	int l;
	uint32_t crc32;
} St;

enum {
	cFlag = 1,
};

static int chunkSize = 1 << 20;

char *selfName;

St go (int level /* Zip level; 0 to mean unzip */, int ifd, int ofd) {
	uint8_t x[chunkSize], y[chunkSize];
	z_stream s;
	
	memset (&s, 0, sizeof (z_stream));
	int windowBits = -MZ_DEFAULT_WINDOW_BITS;
	if (level == 0 ? inflateInit2 (&s,                     windowBits)
	               : deflateInit2 (&s, level, MZ_DEFLATED, windowBits, 6, MZ_DEFAULT_STRATEGY)) {
		eprintf ("%s: failed\n", selfName);
	}
	
	for (;;) {
		int n, fin = 0;
		n = read (ifd, x + s.avail_in, chunkSize - s.avail_in);
		if (n < 0) eprintf ("%s:", selfName);
		s.next_in   = x;
		s.next_out  = y;
		s.avail_in += n;
		s.avail_out = chunkSize;
retry:
		switch (level == 0 ? mz_inflate (&s, MZ_SYNC_FLUSH) : mz_deflate (&s, n > 0 ? MZ_FINISH : MZ_SYNC_FLUSH)) {
		int n;
		case MZ_STREAM_END:
			fin = 1;
		case MZ_OK:
			write (ofd, y, chunkSize - s.avail_out);
			s.avail_in = 0;
			if (fin) return (St) { .l = level == 0 ? s.total_out : s.total_in, .crc32 = s.crc32 };
			break;
		case MZ_BUF_ERROR:
			continue;
		case MZ_DATA_ERROR:
			eprintf ("%s: not flated data\n", selfName);
		case MZ_PARAM_ERROR:
			eprintf ("%s: failed\n", selfName);
		}
	}
}

void skipString (int fd) {
	uint8_t _[1];
	for (_[0] = 1; _[0]; readn (fd, _, 1));
}

void ungz (int ifd, int ofd) {
	uint8_t x[10];
	switch (readn (ifd, x, 10)) {
	case  0: return;
	case 10: break;
	default: eprintf ("%s: not in gz format\n", selfName);
	}
	if (x[0] != 0x1F || x[1] != 0x8B) eprintf ("%s: not in gz format\n", selfName);
	if (x[2] != 8) eprintf ("%s: unknown z-algorithm: 0x%0hhX\n", selfName, x[2]);
	if (x[3] & 1 << 2) /* FEXTRA */ {
		uint16_t n;
		uint8_t n_[2], _[1];
		readn (ifd, n_, 2);
		for (n = n_[0] << 0 | n_[1] << 8; n-- > 0; readn (ifd, _, 1));
	}
	if (x[3] & 1 << 3) /* FNAME    */ skipString (ifd);
	if (x[3] & 1 << 4) /* FCOMMENT */ skipString (ifd);
	if (x[3] & 1 << 1) /* FCRC */ {
		uint16_t _[1];
		readn (ifd, _, 2);
	}
	go (0, ifd, ofd);
}

void storLE32 (uint8_t *p, uint32_t n) {
	p[0] = n >>  0;
	p[1] = n >>  8;
	p[2] = n >> 16;
	p[3] = n >> 24;
}

void gz (int level, int ifd, int ofd) {
	uint8_t hdr[10] = {
		0x1F, 0x8B,	/* magic */
		8,		/* z method */
		0,		/* flags */
		0,0,0,0,	/* mtime */
		0,		/* xfl */
		0xFF,		/* OS */
	};
	uint8_t ftr[8];
	St st;
	write (ofd, hdr, 10);
	st = go (level, ifd, ofd);
	storLE32 (ftr + 0, st.crc32);
	storLE32 (ftr + 4, st.l);
	write (ofd, ftr, 8);
}

int main (int argc, char *argu[]) {
	int level = 6; // zip level; 0 to mean unzip
	int flags = 0;
	char **ss, **ts;
#include "argPrae.c"
	case 'c':
		flags |= cFlag;
		break;
	case 'd':
		level = 0;
		break;
	default:
		if (argu[ii][jj] <= '9' && argu[ii][jj] >= '0') level = (int)(argu[ii][jj] - '0');
		break;
#include "argPost.c"
	selfName = argu[0];
	ss = argu;
	if (!flags & cFlag) {
		ts = malloc (sizeof (char *)*(argc - 1));
		if (!ts) eprintf ("%s:", argu[0]);
		for (int ii = 1; ii < argc; ii++) {
			if (level == 0) {
				char *p;
				ts[ii] = strdup (ss[ii]);
				if (!ts[ii]) eprintf ("%s:", argu[0]);
				p = strrchr (ts[ii], '.');
				if (!p) eprintf ("%s: %s: no \".gz\" suffix\n", argu[0], ts[ii]);
				p[0] = 0;
			}
			else {
				ts[ii] = malloc (strlen (ss[ii]) + 4);
				strcpy (ts[ii], ss[ii]);
				strcat (ts[ii], ".gz");
			}
		}
	}

	for (int ii = 1; ii < argc; ii++) {
		int ifd, ofd;
		struct stat st;
		if (stat (ss[ii], &st) < 0) eprintf ("%s:", argu[0]);
		ifd = open (ss[ii], O_RDONLY);
		ofd = flags & cFlag ? 1 : open (ts[ii], O_WRONLY | O_CREAT, st.st_mode);
		if (ifd < 0 || ofd < 0) eprintf ("%s:", argu[0]);
		if (level == 0) ungz (ifd, ofd);
		else gz (level, ifd, ofd);
	}
	
	if (argc <= 1) {
		if (level == 0) ungz (0, 1);
		else gz (level, 0, 1);
	}
	
	return 0;
}
