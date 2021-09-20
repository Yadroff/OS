#include <unistd.h>
#include <stdio.h>
#include <cctype>
#include <algorithm>

int main() {
    int err = 0;
    int fd[2];
    int fd_1[2];
    int fd_2[2];
    pipe(fd); // pipe между дочерними потоками
    pipe(fd_1); // pipe между родительским и первым
    pipe(fd_2); // pipe между родительским и вторым
    pid_t pid_1 = 0; // первый поток
    pid_t pid_2 = 0; // второй поток
    if ((pid_1 = fork()) > 0) { // создаем первый процесс
        if ((pid_2 = fork()) > 0) { //создаем второй процесс
            // Parent
            char *in = (char *) malloc(sizeof(char) * 2);
            in[0] = 0;
            char c;
            while ((c = getchar()) != EOF) {
                in[0] += 1;
                in[in[0]] = c;
                in = (char *) realloc(in, (in[0] + 2) * sizeof(char));
            }
            in[in[0]] = '\0';
            err = write(fd_1[1], in, (in[0] + 2) * sizeof(char)); // кидаем в pipe fd_1
            if (err == -1){
                printf("Write error\n");
                exit(-1);
            }
            char *out = (char *) malloc(sizeof(char));
            err = read(fd_2[0], &out[0], sizeof(char)); // вытаскиваем из pipe fd_2
            if (err == -1){
                printf("Read error\n");
                exit(-1);
            }
            out = (char *) realloc(out, (out[0] + 2) * sizeof(char));
            for (int i = 1; i < out[0] + 1; ++i) {
                err = read(fd_2[0], &out[i], sizeof(char));
                if (err == -1){
                    printf("Read error\n");
                    exit(-1);
                }
                printf("%c", out[i]);
            }
            printf("\n");
            close(fd_2[0]);
            close(fd_1[1]);
            free(in);
            free(out);
        } else if (pid_2 == 0) { //Child_2
            fflush(stdin);
            fflush(stdout);
            char *in = (char *) malloc(sizeof(char));
            err = read(fd[0], &in[0], sizeof(char)); // считываем из pipe fd
            if (err == -1){
                printf("Read error\n");
                exit(-1);
            }
            in = (char *) realloc(in, (in[0] + 2) * sizeof(char));
            for (int i = 1; i < in[0] + 1; i++) {
                err = read(fd[0], &in[i], sizeof(char));
                if (err == -1){
                    printf("Read error\n");
                    exit(-1);
                }
            }
            char *out = (char *) malloc(2 * sizeof(char));
            out[0] = 0;
            for (int i = 1; i < in[0]; i++) { // преобразование
                if (in[i] == ' ' && in[i + 1] == ' ') {
                    i++;
                    continue; //
                }
                out[0]++;
                out[out[0]] = in[i];
                out = (char *) realloc(out, (out[0] + 2) * sizeof(out));
            }
            out[0]++;
            out[out[0]] = '\0';
            err = write(fd_2[1], out, (out[0] + 2) * (sizeof(char))); // кидаем в pipe fd_2
            if (err == -1){
                printf("Write error\n");
                exit(-1);
            }
            fflush(stdout);
            close(fd_2[1]); // закрываем вход и выход pipe'ов
            close(fd[0]);
            free(in);
            free(out);
        } else { //Parent
            printf("Fork error\n");
            exit(-1);
        }
    } else if (pid_1 == 0) { //Child_1
        char *in = (char *) malloc(sizeof(char));
        err = read(fd_1[0], &in[0], sizeof(char));
        if (err == -1){
            printf("Read error\n");
            exit(-1);
        }
        in = (char *) realloc(in, (in[0] + 2) * sizeof(char));
        char *out = (char *) malloc((in[0] + 2) * sizeof(char));
        out[0] = in[0];
        for (int i = 1; i < in[0] + 1; i++) { // преобразование
            err = read(fd_1[0], &in[i], sizeof(char));
            if (err == -1){
                printf("Read error\n");
                exit(-1);
            }
            out[i] = toupper(in[i]);
        }
        err = write(fd[1], out, (out[0] + 2) * sizeof(char)); // в pipe между дочерними процессами
        if (err == -1){
            printf("Read error\n");
            exit(-1);
        }
        close(fd_1[0]);
        close(fd[1]);
        free(in);
        free(out);
    } else {
        printf("Fork error\n");
        exit(-1);
    }
    return 0;
}