#include <stdio.h>
#include <unistd.h>

int main(int argc, char**argv) {
	float f;
	char *c;
	printf("input floating number: ");
	scanf("%f", &f);
	c = (char *)&f;
	printf("%f = %X %X %X %X\n", f, c[0], c[1], c[2], c[3]);
}
