#include "sort.h"

int* get_shared_mem(int n) {
	int fd_mem = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd_mem == -1) {
		perror("shm_open in sort\n");
		exit(0);
	}
	if (ftruncate(fd_mem, n * sizeof(int)) == -1) {
		perror("ftruncate in sort\n");
		exit(0);
	}
	int *mapped_mem = (int *) mmap(NULL, n * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, 0);
	if (close(fd_mem) == -1) {
		perror("close in sort\n");
	}
	if (shm_unlink(SHM_NAME) == -1) {
		perror("shm_unlink in sort\n");
	}
	if (mapped_mem == MAP_FAILED) {
		perror("mmap in sort\n");
		exit(0);
	}
	return mapped_mem;
}

void fork_processes(int n, int *mem, sem_t **mutexes, sem_t **sems_a, sem_t **sems_b, sem_t *sorting,
					char **mutex_names, char **sem_a_names, char **sem_b_names) {
	// A processes
	for (int i=0; i<n/2; i++) {
		switch(fork()) {
			case -1:
				perror("fork in sort\n");
				exit(0);
			case 0:
				mutexes[i] = sem_open(mutex_names[i], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1);
				sems_a[i] = sem_open(sem_a_names[i], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
				sems_b[i] = sem_open(sem_b_names[i], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
				sorting = sem_open(SEM_SORTING_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
				// TODO: kazdy otwiera swoje semafory jednak, tablice z nazwami
				while (!is_sorted(mem, n, mutexes)) {
					shared_mem_swap(mem, mutexes, 2*i, 2*i + 1);
					if (i <= n/2 - 1) {
						if (sem_post(sems_b[i]))
							perror("sem_post in A\n");
					}
					if (i > 0) {
						if (sem_post(sems_b[i]))
							perror("sem_post in A\n");
					}
					if (i < n/2 - 1) {
						if (sem_wait(sems_a[i]))
							perror("sem_wait in A\n");
					}
					if (i > 0) {
						if (sem_wait(sems_a[i]))
							perror("sem_wait in A\n");
					}
				}
				//printf("SKONCZYLEM A PETLE\n");
				if (sem_post(sorting))
					perror("sem_post in A\n");
				if (sem_close(sorting))
					perror("sem_close in A\n");
				if (sem_close(mutexes[i]) || sem_close(sems_a[i]) || sem_close(sems_b[i]))
					perror("sem_close in A\n");
				exit(0);
		}
	}
	
	// B processes
	for (int i=0; i<(n/2) - 1; i++) {

		switch(fork()) {
			case -1:
				perror("fork in sort\n");
				exit(0);
			case 0:
				while (!is_sorted(mem, n, mutexes)) {
					if (sem_wait(sems_b[i]))
						perror("sem_wait in B\n");
					if (sem_wait(sems_b[i+1]))
						perror("sem_wait in B\n");
					shared_mem_swap(mem, mutexes, 2*i + 1, 2*i + 2);
					if (sem_post(sems_a[i]))
						perror("sem_post in B\n");
					if (sem_post(sems_a[i+1]))
						perror("sem_post in B\n");
				}
				exit(0);
		}
	}
}

int main() {
	setbuf(stdout, NULL);
	int n;
	scanf("%d", &n);
	n *= 2;
	int *arr = (int *) malloc(n*sizeof(int));
	sem_t **mutexes = (sem_t **) malloc(n*sizeof(sem_t));
	sem_t **sems_a = (sem_t **) malloc(n*sizeof(sem_t));
	sem_t **sems_b = (sem_t **) malloc(n*sizeof(sem_t));
	sem_t *sorting = sem_open(SEM_SORTING_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
	char **mutex_names = calloc(n, sizeof(char*));
	char **sem_a_names = calloc(n, sizeof(char*));
	char **sem_b_names = calloc(n, sizeof(char*));
	for (int i=0; i<n; i++) {
		mutex_names[i] = (char *) calloc(1, LEN);
		sem_a_names[i] = (char *) calloc(1, LEN);
		sem_b_names[i] = (char *) calloc(1, LEN);
	}
	for (int i=0; i<n; i++) {
		scanf("%d", &arr[i]);
		char i_str[LEN];
		sprintf(i_str, "%d", i);

		strcpy(mutex_names[i], SEM_MUTEX_NAME);
		strcat(mutex_names[i], i_str);

		strcpy(sem_a_names[i], SEM_A_NAME);
		strcat(sem_a_names[i], i_str);

		strcpy(sem_b_names[i], SEM_B_NAME);
		strcat(sem_b_names[i], i_str);

		mutexes[i] = sem_open(mutex_names[i], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1);
		sems_a[i] = sem_open(sem_a_names[i], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
		sems_b[i] = sem_open(sem_b_names[i], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
		if (mutexes[i] == SEM_FAILED || sems_a[i] == SEM_FAILED || sems_b[i] == SEM_FAILED) {
			perror("sem_open in sort\n");
			exit(0);
		}
	}
	int *mapped_mem = get_shared_mem(n);
	memcpy(mapped_mem, arr, n * sizeof(int));

	fork_processes(n, mapped_mem, mutexes, sems_a, sems_b, sorting, mutex_names, sem_a_names, sem_b_names);
	//printf("waiting here...\n");
	if (sem_wait(sorting)) {
		perror("sem_wait in sort\n");
		exit(0);
	}
	//printf("RESULT\n");
	for (int i=0; i<n; i++) {
		printf("%d\n", mapped_mem[i]);
	}

	// cleaning up
	if (sem_close(sorting))
		perror("sem_close in sort\n");
	if (sem_unlink(SEM_SORTING_NAME))
		perror("sem_unlink in sort\n");
	for (int i=0; i<n; i++) {
		if (sem_close(mutexes[i]) || sem_close(sems_a[i]) || sem_close(sems_b[i]))
			perror("sem_close in sort\n");
		if (sem_unlink(mutex_names[i]) || sem_unlink(sem_a_names[i]) || sem_unlink(sem_b_names[i]))
			perror("sem_unlink in sort\n");
		free(mutex_names[i]);
		free(sem_a_names[i]);
		free(sem_b_names[i]);
	}
	munmap(mapped_mem, n * sizeof(int));
	free(mutex_names); 
	free(sem_a_names); free(sem_b_names);
	free(mutexes);
	free(sems_a); free(sems_b);
	free(arr);
	return 0;
}