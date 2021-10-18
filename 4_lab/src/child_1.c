//
// Created by Temi4 on 17.10.2021.
//
#include <assert.h>
#include <fcntl.h>
#include <semaphore.h>
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
  char *string = (char *)calloc(map_size, sizeof(char));
  char *out = (char *)calloc(1, sizeof(char));
  size_t m_size = 0;
  strcpy(string, memptr);
  for (int i = 0; i < map_size; ++i) {// преобразование
	if (string[i] == ' ' && string[i + 1] == ' ') {
	  ++i;
	  continue;//
	}
	out[m_size] = string[i];
	++m_size;
	out = (char *)realloc(out, (m_size + 1) * sizeof(out));
  }
  out[m_size] = '\0';
  memset(memptr, '\0', m_size);
  sprintf(memptr, "%s", out);
  out = (char*)calloc(m_size, sizeof(char));
  strcpy(out, memptr);
  close(map_fd);
  sem_post(semptr);
  sem_close(semptr);
  free(out);
  free(string);
  return EXIT_SUCCESS;
}