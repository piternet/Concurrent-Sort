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
#define SEM_MUTEX_NAME	 "/pw_sort_mutex"
#define SEM_SORTING_NAME "/pw_sort_sorting"
#define LEN 			 100

void shared_mem_swap(int *mem, sem_t **mutexes, int i, int j) {
	// P mutexes
	//printf("mem swap %d %d...\n", i, j);
	if (sem_wait(mutexes[i]))
		perror("sem_wait on mutex\n");
	if (sem_wait(mutexes[j]))
		perror("sem_wait on mutex\n");
	if (i < j && mem[i] > mem[j]) {
		int tmp = mem[i];
		mem[i] = mem[j];
		mem[j] = tmp;
	}
	// V mutexes
	if (sem_post(mutexes[i]))
		perror("sem_post on mutex\n");
	if (sem_post(mutexes[j]))
		perror("sem_post on mutex\n");
}

bool is_sorted(int *mem, int n, sem_t **mutexes) {
	//printf("spr czy sorted: ");
	for (int i=0; i<n; i++) {
		if (sem_wait(mutexes[i]))
			perror("sem_wait on mutex\n");
	}
	bool result = true;
	for (int i=0; i<n-1; i++) {
		if (mem[i] > mem[i+1])
			result = false;
		//printf("%d, ", mem[i]);
	}
	//printf("%d ", mem[n-1]);
	for (int i=0; i<n; i++) {
		if (sem_post(mutexes[i]))
			perror("sem_wait on mutex\n");
	}
	// if (result)
	// 	printf("SORTED\n");
	return result;
}

#endif // SORT_H_