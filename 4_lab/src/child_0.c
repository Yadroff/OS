//
// Created by Temi4 on 17.10.2021.
//
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "shrmem.h"

int main(int argc, char **argv) {
  assert(argc == 2);
  const size_t map_size = atoi(argv[1]);
  int map_fd = shm_open(BackingFile, O_RDWR, AccessPerms);
  if (map_fd < 0) {
	perror("SHM_OPEN");
	exit(EXIT_FAILURE);
  }
  caddr_t memptr = mmap(
	  NULL,
	  map_size,
	  PROT_READ | PROT_WRITE,
	  MAP_SHARED,
	  map_fd,
	  0);
  if (memptr == MAP_FAILED) {
	perror("MMAP");
	exit(EXIT_FAILURE);
  }
  sem_t *semptr = sem_open(SemaphoreName, O_CREAT, AccessPerms, 2);
  if (semptr == SEM_FAILED) {
	perror("SEM_OPEN");
	exit(EXIT_FAILURE);
  }
  if (sem_wait(semptr) != 0) {
	perror("SEM_WAIT");
	exit(EXIT_FAILURE);
  }
  char *string = (char *)malloc(strlen(memptr) * sizeof(char));
  strcpy(string, memptr);
  for (int i = 0; i < map_size; ++i) {// преобразование
	string[i] = toupper(string[i]);
  }
  memset(memptr, '\0', map_size);
  sprintf(memptr, "%s", string);
  free(string);
  pid_t pid = fork();
  if (pid == 0) {
	munmap(memptr, map_size);
	close(map_fd);
	sem_close(semptr);
	char str[10];
	sprintf(str, "%lu", map_size);
	execl("4_lab_child_1", "4_lab_child_1", str, NULL);
	perror("EXECL");
	exit(EXIT_FAILURE);
  } else if (pid == -1) {
	perror("FORK");
	exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}