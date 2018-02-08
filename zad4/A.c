#include "sort.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		perror("Usage: ./A n i\n");
		exit(0);
	}
	int n = atoi(argv[1]), i = atoi(argv[2]);
	printf("A. i=%d, n=%d\n", i, n);
	// while (true) {

	// }
	return 0;
}