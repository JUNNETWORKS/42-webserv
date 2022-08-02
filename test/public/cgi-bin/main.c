#include <stdio.h>

int main(void) {
	printf("Content-type: text/plain; charset=iso-8859-1\n");
	printf("\n");

	for (int i = 0; i < 10000000; i++) {
		printf("%d\n", i);
	}
	return 0;
}
