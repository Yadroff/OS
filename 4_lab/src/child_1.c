//
// Created by Temi4 on 17.10.2021.
//
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "shrmem.h"

int main(int argc, char **argv) {
  int map_fd = shm_open(BackingFile, O_RDWR, AccessPerms);
  if (map_fd < 0) {
	perror("SHM_OPEN");
	exit(EXIT_FAILURE);
  }
  struct stat statbuf;
  fstat(map_fd, &statbuf);
  const size_t map_size = statbuf.st_size;
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
  char *out = (char *)calloc(1, sizeof(char));
  size_t m_size = 0;
  for (int i = 0; i + 1 < map_size; ++i) {// преобразование
	if (memptr[i] == ' ' && memptr[i + 1] == ' ') {
	  ++i;
	  continue;//
	}
	out[m_size] = memptr[i];
	out = (char *)realloc(out, (++m_size + 1) * sizeof(char));
  }
  out[m_size++] = '\0';
  ftruncate(map_fd, (off_t)m_size);
  memset(memptr, '\0', m_size);
  sprintf(memptr, "%s", out);
  free(out);
  close(map_fd);
  usleep(00150000); // one iteration of cycle while (true) at main.c may be longer than this program
  sem_post(semptr);
  sem_close(semptr);
  return EXIT_SUCCESS;
}