#include <utils/endian.h>
#include <endian.h>

static int *get_crt_endianess(void) {
	static int crt_endian = -1;
	if (crt_endian == -1) {
		unsigned int i = 1;
		char *c = (char *) &i;
		crt_endian = (*c == 1 ? LITTLE_ENDIAN : BIG_ENDIAN);
	}
	return &crt_endian;
}

int get_endian(void)
{
	return *get_crt_endianess();
}