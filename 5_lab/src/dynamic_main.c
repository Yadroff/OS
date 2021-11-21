//
// Created by Temi4 on 21.11.2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef enum{
  FIRST,
  SECOND,
} CONTRACT;

CONTRACT cont = FIRST;

const char* libName1 = "libFirst.so";
const char* libName2 = "libSecond.so";

float (*Derivative)(float, float) = NULL;
char* (*Translation)(long) = NULL;

void *libHandle = NULL;

void loadLibs(CONTRACT contract){
  const char* name;
  switch (contract) {
  case FIRST:{
	name = libName1;
	break;
  }
  case SECOND:{
	name = libName2;
	break;
  }
  }
  libHandle = dlopen(name, RTLD_LAZY);
  if (libHandle == NULL){
	perror("dlopen");
	exit(EXIT_FAILURE);
  }
}

void loadContract(){
  loadLibs(cont);
  Derivative = dlsym(libHandle, "Derivative");
  Translation = dlsym(libHandle, "Translation");
}

void changeContract(){
  dlclose(libHandle);
  switch (cont) {
	case FIRST:{
		cont = SECOND;
		break;
	}
	case SECOND:{
	  cont = FIRST;
	  break;
	}
  }
  loadContract();
}

int main(){
  loadContract();
  int cmd = 0;
  while (scanf("%d", &cmd) != EOF){
	switch (cmd) {
	  case 0:{
	  changeContract();
	  printf("Contract has been changed\n");
	  switch (cont) {
		case FIRST: {
		  printf("Contract is First\n");
		  break;
		}
		case SECOND:{
		  printf("Contract is Second\n");
		}
	  }
	  break;
	}
	case 1:{
	  float A, deltaX;
	  if (scanf("%f %f", &A, &deltaX) == 2){
		printf("%.6f\n", Derivative(A, deltaX));
	  }
	  break;
	}
	case 2:{
	  long x;
	  if (scanf("%ld", &x) == 1){
		printf("Translation from 10 to ");
		switch (cont) {
		  case FIRST:{
			printf("2");
			break;
		  }
		  case SECOND:{
			printf("3");
		  }
		}
		printf(" base %s\n", Translation(x));
	  }
	  break;
	}
	default:{
	  printf("Wrong answer\n");
	}
	}
  }
  return 0;
}