#include <unistd.h>

ssize_t readn (int fd, void *x, size_t n) {
	int n0 = n;
	while (n > 0) {
		int m = read (fd, x, n);
		if (m < 0) return m;
		n -= m;
		x += m;
	}
	return n0;
}
