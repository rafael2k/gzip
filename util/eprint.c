#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <utf.h>
#include "../util.h"

void venprintf (int c, const char *fmt, va_list ap) {
	vfprintf (stderr, fmt, ap);
	
	if (fmt[0] && fmt[strlen (fmt) - 1] == ':') {
		fputc (' ', stderr);
		perror (0);
	}
	
	if (c) exit (c);
}

void enprintf (int c, const char *fmt, ...) {
	va_list ap;
	
	va_start (ap, fmt);
	venprintf (c, fmt, ap);
	va_end (ap);
}

void eprintf (const char *fmt, ...) {
	va_list ap;
	
	va_start (ap, fmt);
	venprintf (1, fmt, ap);
	va_end (ap);
}
