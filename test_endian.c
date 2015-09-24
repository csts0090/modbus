#include <stdio.h>
#include <unistd.h>

int main() {
	unsigned int i = 0x11223344;
	char *c;
	
	c = (char *)&i;
	printf("%X %X %X %X\n", c[0], c[1], c[2], c[3]);
}
