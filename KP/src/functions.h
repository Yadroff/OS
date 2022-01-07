#ifndef KP_FUNCTIONS_H
#define KP_FUNCTIONS_H

#include <valarray>
#include <string>
#include <vector>
#include <unistd.h>

const char *SERVER_INPUT_FILE_NAME = "server_in";
const char *DATA_BASE_FILE_NAME = "data_base";
const char *RESULTS_FILE_NAME = "results";
const char *SECRET_KEY = "$";
const std::string ADMIN_NAME("Yadroff");
const std::string ADMIN_PASSWORD("qwerty");
const int INF = (int) pow(10, 9) + 7;
#define ACCESS_PERMS S_IRWXO | S_IRWXG | S_IRWXU
std::pair<int, int> transform(std::pair<std::string, std::string> &pair) {
    /* Преобразование пары строк в пару чисел происходит следующим образом:
     * первая строка проходится в прямом порядке от начала к концу и хэшируется
     * вторая строка проходится в обратном порядке*/
    std::string first_string = pair.first, second_string = pair.second;
    int first = 0, second = 0;
    for (auto i: first_string) {
        first = (first * 10 + i) % INF;
    }
    for (auto it = second_string.rbegin(); it != second_string.rend(); ++it) {
        second = (second * 10 + *it) % INF;
    }
    return std::make_pair(first, second);
}

// сообщение отправляется в виде строки: login#cmd#data
void send_to_server(int fd, std::string &login, std::string &cmd, std::string &data) {
    std::string str = login + "#" + cmd + "#" + data;
    size_t size = str.size();
    write(fd, &size, sizeof(size));
    write(fd, str.c_str(), sizeof(char) * size);
}

void send_to_client(int fd, std::string &message) {
    size_t size = message.size();
    write(fd, &size, sizeof(size));
    write(fd, message.c_str(), sizeof(char) * size);
}

std::string receive(int fd) {
    size_t size;
    read(fd, &size, sizeof(size_t));
    std::string recv;
    char c;
    for (size_t i = 0; i < size; ++i) {
        read(fd, &c, sizeof(char));
        if (c == '#') {
            recv += ": ";
        } else {
            recv.push_back(c);
        }
    }
    return recv;
}


void receive_from_client(int fd, std::string *login, std::string *cmd, std::string *data) {
    size_t size;
    *login = *cmd = *data = "";
    read(fd, &size, sizeof(size));
    int k = 0;
    char c;
    for (size_t i = 0; i < size; ++i){
        read(fd, &c, sizeof(char));
        if (c == '#'){
            k++;
            continue;
        }
        switch (k) {
            case 0: {
                *login += c;
                break;
            }
            case 1: {
                *cmd += c;
                break;
            }
            case 2:{
                *data += c;
                break;
            }
        }
    }
}

void parse_game_word(std::string& message, std::string &game, std::string &word){
    int i = 0;
    for (auto item: message){
        if (item == '%'){
            ++i;
            continue;
        }
        (i == 0) ? game.push_back(item) : word.push_back(item);
    }
}
#endif //KP_FUNCTIONS_H
