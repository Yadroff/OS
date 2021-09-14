#include <iostream>
#include <unistd.h>
#include <cctype>
#include <algorithm>

using namespace std;

int main() {
    int fd[2];
    int fd_1[2];
    int fd_2[2];
    pipe(fd); // pipe между дочерними потоками
    pipe(fd_1); // pipe между родительским и первым
    pipe(fd_2); // pipe между родительским и вторым
    int pid_1 = 0; // первый поток
    int pid_2 = 0; // второй поток
    if ((pid_1 = fork()) > 0) { // создаем первый поток
        if ((pid_2 = fork()) > 0) { //создаем второй поток
            string s; // Parent
            auto *in = new char[2];
            in[0] = 0;
            char c;
            while ((c = getchar()) != EOF) {
                in[0] += 1;
                in[in[0]] = c;
                in = (char *) realloc(in, (in[0] + 2) * sizeof(char));
            }
            in[in[0]] = '\0';
            write(fd_1[1], in, (in[0] + 2) * sizeof(char)); // кидаем в pipe fd_1
            auto *out = (char *) malloc(sizeof(char));
            read(fd_2[0], &out[0], sizeof(char)); // вытаскиваем из pipe fd_2
            out = (char *) realloc(out, (out[0] + 2) * sizeof(char));
            for (int i = 1; i < out[0] + 1; ++i) {
                read(fd_2[0], &out[i], sizeof(char));
                cout << out[i];
            }
            cout << endl;
            close(fd_2[0]);
            close(fd_1[1]);
            free(in);
            free(out);
        } else if (pid_2 == 0) { //Child_2
            fflush(stdin);
            fflush(stdout);
            char *in = (char *) malloc(sizeof(char));
            read(fd[0], &in[0], sizeof(char)); // считываем из pipe fd
            in = (char *) realloc(in, (in[0] + 2) * sizeof(char));
            for (int i = 1; i < in[0] + 1; i++) {
                read(fd[0], &in[i], sizeof(char));
            }
            char *out = (char *) malloc(2 * sizeof(char));
            out[0] = 0;
            for (int i = 1; i < in[0]; i++) {
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
            write(fd_2[1], out, (out[0] + 2) * (sizeof(char))); // кидаем в pipe fd_2
            fflush(stdout);
            close(fd_2[1]); // закрываем вход и выход pipe'ов
            close(fd[0]);
            free(in);
            free(out);
        } else { //Parent
            cout << "fork error" << endl;
            exit(-1);
        }
    } else if (pid_1 == 0) { //Child_1
        fflush(stdin);
        fflush(stdout);
        char *in = (char *) malloc(sizeof(char));
        read(fd_1[0], &in[0], sizeof(char));
        in = (char *) realloc(in, (in[0] + 2) * sizeof(char));
        char *out = (char *) malloc((in[0] + 2) * sizeof(char));
        out[0] = in[0];
        for (int i = 1; i < in[0] + 1; i++) {
            read(fd_1[0], &in[i], sizeof(char));
            out[i] = toupper(in[i]);
        }
        write(fd[1], out, (out[0] + 2) * sizeof(char));
        close(fd_1[0]);
        close(fd[1]);
        delete in;
        delete out;
    } else {
        cout << "Fork error" << endl;
        exit(-1);
    }
    return 0;
}
