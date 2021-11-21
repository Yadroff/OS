#include "realization.h"

#include <malloc.h>
#include <math.h>

float Derivative(float A, float deltaX) {
  return (cosf(A + deltaX) - cosf(A)) / deltaX;
}

char *Translation(long x) {
  char *res = (char *)malloc(sizeof(char));
  int n = 0;
  while (x > 0) {
	res[n++] = x % 2 + '0';
	x /= 2;
	res = realloc(res, (n + 1) * sizeof(char));
  }
  res[n] = '\0';
  char t;
  for (int i = 0; i < n / 2; ++i) {
	t = res[i];
	res[i] = res[n - i - 1];
	res[n - i - 1] = t;
  }
  return res;
}
