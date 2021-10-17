#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shrmem.h"

int main() {
  size_t map_size = 0;
  char *in = (char *)calloc(1, sizeof(char));
  char c;
  while ((c = getchar()) != EOF) {
	in[map_size] = c;
	++map_size;
	in = (char *)realloc(in, (map_size + 1) * sizeof(char));
  }
  in[map_size] = '\0';
  //read string stream
  int fd = shm_open(BackingFile, O_RDWR | O_CREAT, AccessPerms);
  if (fd == -1) {
	perror("OPEN");
	exit(EXIT_FAILURE);
  }
  sem_t *semptr = sem_open(SemaphoreName, O_CREAT, AccessPerms, 3);
  if (semptr == SEM_FAILED) {
	perror("SEM_OPEN");
	exit(EXIT_FAILURE);
  }
  int val;

  ftruncate(fd, map_size);
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
  while (val++ < 2) {// if semaphore already was created, just fill it up to 2
	sem_post(semptr);
  }
  int pid_0 = 0;
  int pid_1 = 0;
  if ((pid_0 = fork()) == 0) {
	munmap(memptr, map_size);
	close(fd);
	sem_close(semptr);
	char str[10];
	sprintf(str, "%lu", map_size);
	execl("4_lab_child_0", "4_lab_child_0", str, NULL);
	perror("EXECL");
	exit(EXIT_FAILURE);
  }
  while (true) {
	if (sem_getvalue(semptr, &val) != 0) {
	  perror("SEM_GETVALUE");
	  exit(EXIT_FAILURE);
	}
	if (val == 0) {
	  if (sem_wait(semptr) == -1) {
		perror("SEM_POST");
		exit(EXIT_FAILURE);
	  }
	  char *string = (char *)malloc(strlen(memptr) * sizeof(char));
	  strcpy(string, memptr);
	  printf("%s", string);
	  free(string);
	  return EXIT_SUCCESS;
	}
  }
}