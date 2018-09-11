#include "array.h"
#include <stdio.h>

int main() {

	Array a = array_new(5);

	for (long i = 0; i < 100*1000*1000; i++) {
		array_insertBack(&a, i);
	}
	array_destroy(a);

	printf("Finished main()\r\n");
	
	return 0;
}