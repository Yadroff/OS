#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

int R;
int *N;
typedef struct arguments {
    int points;
    int i;
} Arg;

double double_rand(double min, double max) { // return random double from min to max
    srand((unsigned int) time(NULL));
    double scale = (double) rand() / (double) RAND_MAX;
    return min + scale * (max - min);
}

void *thread_function(void *args) { // create n random points of square size 2*R and check if point at circle
    Arg *arg = (Arg *) args;
    int n = arg->points;
    int i = arg->i;
    for (int j = 0; j < n; j++) {
        double x = double_rand(-R, R);
        double y = double_rand(-R, R);
        if (x * x + y * y <= R * R) {
            N[i]++;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Syntax: ./*executable_file_name* Radius Number_of_points Number_of_threads\n");
        exit(1);
    }
    R = atoi(argv[1]);
    int points_num = atoi(argv[2]), threads_num = atoi(argv[3]);
    N = (int *) calloc(threads_num, sizeof(int)); // array of number points at circle
    pthread_t *threads = (pthread_t *) calloc(threads_num, sizeof(pthread_t));
    if (threads == NULL) {
        printf("Can't allocate memory for threads\n");
        exit(1);
    }
    int points_for_thread = points_num / threads_num;
    Arg a;
    for (int i = 0; i < threads_num; i++) {
        a.points = points_for_thread + (i < (points_num % threads_num));
        a.i = i;
        pthread_create(&threads[i], NULL, thread_function, &a);
    }
    for (int i = 0; i < threads_num; i++) {
        pthread_join(threads[i], NULL);
    }
    int n = 0;
    for (int i = 0; i < threads_num; i++) { // calculate points at circle
        n += N[i];
    }
    printf("Circle square is %.5f\n", (double) 4 * R * R * n /
                                      points_num); // (Circle Square)/(Square of square with size 2*R) = N/M, where M = points_num
    free(threads);
    return 0;
}
