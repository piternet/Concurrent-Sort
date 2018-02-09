#include "sort2.h"

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

void fork_processes(int n, int *mem, sem_t **sems_a, sem_t **sems_b,
					char **sem_a_names, char **sem_b_names) {
	// A processes
	for (int i=0; i<n/2; i++) {
		switch(fork()) {
			case -1:
				perror("fork in sort\n");
				exit(0);
			case 0:
				sems_a[i] = sem_open(sem_a_names[i], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
				sems_b[i] = sem_open(sem_b_names[i], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
				int j = 0;
				while (j++ < 2*n) {
					sem_wait(sems_a[i]);
					if (i != 0 && i != n/2 - 1) {
						printf("tu\n");
						sem_wait(sems_a[i]);
					}
					printf("A nr %d zamienia\n", i);
					shared_mem_swap(mem, 2*i, 2*i + 1);
					if (i != 0) {
						sem_post(sems_b[i-1]);
					}
					if (i != n/2 -1) {
						sem_post(sems_b[i]);
					}
				}
				if (sem_close(sems_a[i]) + sem_close(sems_b[i]))
					perror("sem_close in A\n");
				exit(0);
			default: ;
				pid_t wpid;
				int status = 0;
				while ((wpid = wait(&status)) > 0);
		}
	}
	
	// B processes
	for (int i=0; i<(n/2) - 1; i++) {
		switch(fork()) {
			case -1:
				perror("fork in sort\n");
				exit(0);
			case 0:
				sems_a[i] = sem_open(sem_a_names[i], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
				sems_b[i] = sem_open(sem_b_names[i], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
				int j = 0;
				while (j++ < 2*n) {
					sem_wait(sems_b[i]);
					sem_wait(sems_b[i]);
					printf("A nr %d zamienia\n", i);
					shared_mem_swap(mem, 2*i + 1, 2*i + 2);
					sem_post(sems_a[i]);
					sem_post(sems_a[i+1]);
				}
				if (sem_close(sems_a[i]) + sem_close(sems_b[i]))
					perror("sem_close in A\n");
				exit(0);
			default: ;
				pid_t wpid;
				int status = 0;
				while ((wpid = wait(&status)) > 0);
		}
	}
}

int main() {
	setbuf(stdout, NULL);
	int n;
	scanf("%d", &n);
	int *arr = (int *) malloc(2*n*sizeof(int));
	sem_t **sems_a = (sem_t **) malloc(n*sizeof(sem_t));
	sem_t **sems_b = (sem_t **) malloc(n*sizeof(sem_t));
	char **sem_a_names = calloc(n, sizeof(char*));
	char **sem_b_names = calloc(n, sizeof(char*));
	for (int i=0; i<n; i++) {
		sem_a_names[i] = (char *) calloc(1, LEN);
		sem_b_names[i] = (char *) calloc(1, LEN);
	}
	for (int i=0; i<2*n; i++) {
		scanf("%d", &arr[i]);
	}
	for (int i=0; i<n; i++) {
		char i_str[LEN];
		sprintf(i_str, "%d", i);

		strcpy(sem_a_names[i], SEM_A_NAME);
		strcat(sem_a_names[i], i_str);

		strcpy(sem_b_names[i], SEM_B_NAME);
		strcat(sem_b_names[i], i_str);

		sems_a[i] = sem_open(sem_a_names[i], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
		sems_b[i] = sem_open(sem_b_names[i], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
		if (sems_a[i] == SEM_FAILED || sems_b[i] == SEM_FAILED) {
			perror("sem_open in sort\n");
			exit(0);
		}
	}
	int *mapped_mem = get_shared_mem(n);
	memcpy(mapped_mem, arr, n * sizeof(int));
	printf("everything ok so far\n");
	for (int i=0; i<n; i++) {
		sem_post(sems_a[i]);
		if (i != 0 && i != n/2 - 1) {
			sem_post(sems_a[i]);
		}
	}
	fork_processes(n, mapped_mem, sems_a, sems_b, sem_a_names, sem_b_names);
	printf("waiting here...\n");
	//printf("RESULT\n");
	for (int i=0; i<n; i++) {
		printf("%d\n", mapped_mem[i]);
	}

	// cleaning up
	for (int i=0; i<n; i++) {
		if (sem_close(sems_a[i]) + sem_close(sems_b[i]))
			perror("sem_close in sort\n");
		if (sem_unlink(sem_a_names[i]) + sem_unlink(sem_b_names[i]))
			perror("sem_unlink in sort\n");
		free(sem_a_names[i]);
		free(sem_b_names[i]);
	}
	munmap(mapped_mem, n * sizeof(int));
	free(sem_a_names); free(sem_b_names);
	free(sems_a); free(sems_b);
	free(arr);
	return 0;
}