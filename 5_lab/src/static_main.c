#include "realization.h"
#include <stdio.h>

int main(){
  int cmd = 0;
  while (scanf("%d", &cmd) != EOF){
	switch (cmd) {
	  case 1: {
		float A, deltaX;
		if (scanf("%f %f", &A, &deltaX) == 2) {
		  printf("%.6f\n", Derivative(A, deltaX));
		}
		break;
	  }
	  case 2:{
		long x;
		if (scanf("%ld", &x) == 1){
		  printf("%s\n", Translation(x));
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