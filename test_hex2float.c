#include <stdio.h>
#include <unistd.h>

int main(int argc, char**argv) {
	unsigned int i;
	float *f;

	printf("input hex number: ");
	scanf("%X", &i);
	f = (float *)&i;
	printf("%X = %f\n", i, *f); 
}
