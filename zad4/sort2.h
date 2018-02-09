#ifndef SORT_H_
#define SORT_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <mqueue.h>
#include <sys/wait.h>

#define SHM_NAME		 "/pw_sort"
#define SEM_A_NAME 		 "/pw_sort_asem"
#define SEM_B_NAME 		 "/pw_sort_bsem"
#define LEN 			 100

void shared_mem_swap(int *mem, int i, int j) {
	//printf("mem swap %d %d...\n", i, j);
	if (i < j && mem[i] > mem[j]) {
		int tmp = mem[i];
		mem[i] = mem[j];
		mem[j] = tmp;
	}
}

#endif // SORT_H_