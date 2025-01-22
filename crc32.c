#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//const uint32_t divisor = 0xEDB88320;
// static uint32_t crc = ~0;

static uint32_t crc = ~0;
const uint32_t divisor = 0xEDB88320;

uint32_t crc32_gzip(const uint8_t *p, size_t len)
{
	// uint32_t crc = ~0;

	for (size_t i = 0; i < len * 8; i++)
	{
		int bit;
		uint32_t multiple;

		bit = (p[i / 8] >> (i % 8)) & 1;
		crc ^= bit;
		if (crc & 1)
			multiple = divisor;
		else
			multiple = 0;
		crc >>= 1;
		crc ^= multiple;
	}

	uint8_t o[4];
	o[0] = crc >>  0;
	o[1] = crc >>  8;
	o[2] = crc >> 16;
	o[3] = crc >> 24;

	printf("%02x%02x%02x%02x\n", o[3], o[2], o[1], o[0]);
	return ~crc;
}

#define BLOCK_SIZE 128

int main(int argc, char *argv[])
{
	uint8_t memory[BLOCK_SIZE];
	uint32_t crc32;

	FILE *fin = fopen(argv[1], "r");

	int fd = fileno(fin); //if you have a stream (e.g. from fopen), not a file descriptor.
	struct stat buf;
	fstat(fd, &buf);
	off_t size = buf.st_size;

#if 0

	uint8_t *memory = malloc(size);
	fread (memory, 1, size, fin);

	crc32 = crc32_gzip(memory, size);
#endif

#if 1
	while (size)
	{
		if (size > BLOCK_SIZE)
		{
			fread (memory, 1, BLOCK_SIZE, fin);
			crc32 = crc32_gzip(memory, BLOCK_SIZE);
			size -= BLOCK_SIZE;
		}
		else
		{
			fread (memory, 1, size, fin);
			crc32 = crc32_gzip(memory, size);
			size = 0;
		}
		printf("size %d\n", size);
	}
#endif

	uint8_t p[4];
	p[0] = crc32 >>  0;
	p[1] = crc32 >>  8;
	p[2] = crc32 >> 16;
	p[3] = crc32 >> 24;

	printf("%02x%02x%02x%02x\n", p[3], p[2], p[1], p[0]);

}
