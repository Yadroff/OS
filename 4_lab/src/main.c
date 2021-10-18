#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "shrmem.h"

int main() {
  size_t map_size = 0;
  char *in = (char *)calloc(1, sizeof(char));
  char c;
  while ((c = getchar()) != EOF) {
	in[map_size] = c;
	in = (char *)realloc(in, (++map_size + 1) * sizeof(char));
  }
  in[map_size] = '\0';
  //read string stream
  int fd = shm_open(BackingFile, O_RDWR | O_CREAT, AccessPerms);
  if (fd == -1) {
	perror("OPEN");
	exit(EXIT_FAILURE);
  }
  sem_t *semptr = sem_open(SemaphoreName, O_CREAT, AccessPerms, 2);
  if (semptr == SEM_FAILED) {
	perror("SEM_OPEN");
	exit(EXIT_FAILURE);
  }
  int val;

  ftruncate(fd, (off_t)map_size);
  caddr_t memptr = mmap(
	  NULL,
	  map_size,
	  PROT_READ | PROT_WRITE,
	  MAP_SHARED,
	  fd,
	  0);
  if (memptr == MAP_FAILED) {
	perror("MMAP");
	exit(EXIT_FAILURE);
  }
  memset(memptr, '\0', map_size);
  sprintf(memptr, "%s", in);
  free(in);
  if (sem_getvalue(semptr, &val) != 0) {
	perror("SEM_GETVALUE");
	exit(EXIT_FAILURE);
  }
  while (val++ < 2) {
	sem_post(semptr);
  }
  int pid_0 = 0;
  if ((pid_0 = fork()) == 0) {
	munmap(memptr, map_size);
	close(fd);
	sem_close(semptr);
	execl("4_lab_child_0", "4_lab_child_0", NULL);
	perror("EXECL");
	exit(EXIT_FAILURE);
  } else if (pid_0 < 0) {
	perror("FORK");
	exit(EXIT_FAILURE);
  }
  while (true) {
	if (sem_getvalue(semptr, &val) != 0) {
	  perror("SEM_GETVALUE");
	  exit(EXIT_FAILURE);
	}
	if (val == 0) {
	  if (sem_wait(semptr) == -1) {
		perror("SEM_WAIT");
		exit(EXIT_FAILURE);
	  }
	  printf("%s\n", memptr);
	  return EXIT_SUCCESS;
	}
  }
}