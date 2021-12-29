#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <fcntl.h>
#include <thread>
#include "funcs.hpp"

#define CLIENT_ID(name) in(logins,name)
#define PLAYER_ID(name) in(curr_playrs_name, name)

inline int create_game_pipe(std::string game_name) {
    if (mkfifo(("game_%" + game_name).c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        std::cout << "GAME " << ("game_%" + game_name).c_str() << " FIFO WAS NOT CREATED";
        exit(1);
    }
    int game_input_fd = open(("game_%" + game_name).c_str(), O_RDWR);
    if (game_input_fd == -1) {
        std::cout << "MAIN INPUT FIFO WAS NOT OPENED";
        exit(1);
    }
    return game_input_fd;
}

inline int in(std::vector<std::string> logins, std::string str) {
    for (int i = 0; i < logins.size(); ++i) {
        if (logins[i] == str)
            return i;
    }
    return -1;
}

inline int create_main_pipe() {
    if (mkfifo("main_input", S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        std::cout << "MAIN INPUT FIFO WAS NOT CREATED";
        exit(1);
    }
    int fd_recv = open("main_input", O_RDWR);
    if (fd_recv == -1) {
        std::cout << "MAIN INPUT FIFO WAS NOT OPENED";
        exit(1);
    }
    return fd_recv;
}

inline int create_admin_pipe() {
    if (mkfifo("admin", S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        std::cout << "ADMIN INPUT FIFO WAS NOT CREATED";
        exit(1);
    }
    int admin_fd = open("admin", O_RDWR);
    if (admin_fd == -1) {
        std::cout << "ADMIN INPUT FIFO WAS NOT OPENED";
        exit(1);
    }
    return admin_fd;
}

inline int create_client_pipe(std::string rcvd_name) {
    if (mkfifo(rcvd_name.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        std::cout << "CLIENT INPUT FIFO WAS NOT CREATED";
        exit(1);
    }
    int fd = open(rcvd_name.c_str(), O_RDWR);
    if (fd == -1) {
        std::cout << "CLIENT INPUT FIFO WAS NOT OPENED";
        exit(1);
    }
    return fd;
}

int hit_check(std::string game_word, std::string try_word, int *cows, int *bulls) {
    if (try_word.size() != game_word.size())
        return -1;
    if (try_word == game_word)
        return -2;

    *cows = 0;
    *bulls = 0;

    for (size_t i = 0; i < try_word.size(); ++i)
        for (size_t j = 0; j < game_word.size(); ++j)
            if (try_word[i] == game_word[j])
                *cows = *cows + 1;
    for (size_t i = 0; i < try_word.size(); ++i)
        if (try_word[i] == game_word[i])
            *bulls = *bulls + 1;
    return 0;
}

void game_funk(std::string game_name, std::string game_word) {
    std::vector<std::string> curr_playrs_name;
    std::vector<int> curr_playrs_fd;
    auto iter_fd = curr_playrs_fd.cbegin();
    auto iter_log = curr_playrs_name.cbegin();
    int game_input_fd = create_game_pipe(game_name);
    int cows, bulls;
    std::string game_respond;
    int game_status;

    std::cout << "START GAME: " << game_name << std::endl;
    std::cout.flush();

    std::string rcvd_name, rcvd_command, rcvd_data;
    while (1) {
        recieve_message_server(game_input_fd, &rcvd_name, &rcvd_command, &rcvd_data);
        if (rcvd_command == "connect") {
            curr_playrs_name.push_back(rcvd_name);
            curr_playrs_fd.push_back(open(rcvd_name.c_str(), O_RDWR));

            std::cout << "CLIENT: " << rcvd_name << " JOIN GAME: " << game_name << std::endl;
            std::cout.flush();

            game_respond = "Добро пожаловать за стол " + game_name;
            send_message_to_client(curr_playrs_fd[PLAYER_ID(rcvd_name)], game_respond);
            game_respond = "Делайте свои предполодения с помощью команды maybe @слово@";
            send_message_to_client(curr_playrs_fd[PLAYER_ID(rcvd_name)], game_respond);
        } else if (rcvd_command == "maybe") {
            game_status = hit_check(game_word, rcvd_data, &cows, &bulls);

            if (game_status == -1) {
                game_respond = "Размеры слов не совподают";
                send_message_to_client(curr_playrs_fd[PLAYER_ID(rcvd_name)], game_respond);
            } else if (game_status == -2) {
                game_respond = "Вы выйграли";
                send_message_to_client(curr_playrs_fd[PLAYER_ID(rcvd_name)], game_respond);

                for (int i = 0; i < curr_playrs_name.size(); i++) {
                    game_respond = "Игру выиграл: " + rcvd_name + "\nЗагаданное слово: " + game_word;
                    send_message_to_client(curr_playrs_fd[i], game_respond.c_str());
                    do {
                        game_respond = "Выйдите из-за стола (команда leave)";
                        send_message_to_client(curr_playrs_fd[i], game_respond.c_str());
                        recieve_message_server(game_input_fd, &rcvd_name, &rcvd_command, &rcvd_data);
                    } while (rcvd_command != "leave");
                }

                close(game_input_fd);

                std::cout << "TEST" << std::endl;

                std::cout << "FINISH GAME: " << game_name << std::endl;
                int mainFD = open("main_input", O_RDWR);
                game_respond = "finish";
                send_message_to_server(mainFD, game_name, game_respond, "");
                std::cout << "TEST" << std::endl;
                return;
            } else if (game_status == 0) {
                game_respond = "Коровы: " + std::to_string(cows) + " Быки: " + std::to_string(bulls);
                send_message_to_client(curr_playrs_fd[PLAYER_ID(rcvd_name)], game_respond);
            }
        } else if (rcvd_command == "leave") {
            iter_fd = curr_playrs_fd.cbegin();
            curr_playrs_fd.erase(iter_fd + PLAYER_ID(rcvd_name));
            iter_log = curr_playrs_name.cbegin();
            curr_playrs_name.erase(iter_log + PLAYER_ID(rcvd_name));
            std::cout << "CLIENT: " << rcvd_name << " LEFT GAME: " << game_name << std::endl;
        }
    }
}

int main() {
    std::vector<std::string> logins;
    std::vector<int> client_pipe_fd;
    std::vector<std::thread> games_threads;
    std::vector<std::string> games_name;
    std::string game_name_table, game_word;

    int fd_recv = create_main_pipe();
    int admin_fd = create_admin_pipe();

    std::string login;
    std::string rcvd_name, rcvd_command, rcvd_data;
    auto iter_fd = client_pipe_fd.cbegin();
    auto iter_log = logins.cbegin();
    auto iter_game_thread = games_threads.cbegin();
    auto iter_game_name = games_name.cbegin();
    while (1) {
        recieve_message_server(fd_recv, &rcvd_name, &rcvd_command, &rcvd_data);

        if (rcvd_command == "login" && rcvd_name != "admin") {
            std::cout << "New client: " << rcvd_name << std::endl;
            client_pipe_fd.push_back(create_client_pipe(rcvd_name));
            logins.push_back(rcvd_name);
        } else if (rcvd_command == "create") {
            extract_game_data(rcvd_data, &game_name_table, &game_word);
            games_name.push_back(game_name_table);
            games_threads.emplace_back(game_funk, game_name_table, game_word);
        } else if (rcvd_command == "finish") {
            std::remove(("game_%" + rcvd_name).c_str());
            std::cout << "TEST\n";
            std::cout.flush();
            games_threads[in(games_name, rcvd_name)].detach();
            std::cout << "TEST\n";
            std::cout.flush();
            iter_game_thread = games_threads.cbegin();
            games_threads.erase(iter_game_thread + in(games_name, rcvd_name));
            std::cout << "TEST\n";
            std::cout.flush();
            iter_game_name = games_name.cbegin();
            games_name.erase(iter_game_name + in(games_name, rcvd_name));
            std::cout << "TEST\n";
            std::cout.flush();

        } else if (rcvd_command == "quit") {
            close(client_pipe_fd[CLIENT_ID(rcvd_name)]);
            std::remove(rcvd_name.c_str());

            iter_fd = client_pipe_fd.cbegin();
            iter_log = logins.cbegin();

            client_pipe_fd.erase(iter_fd + CLIENT_ID(rcvd_name));
            logins.erase(iter_log + CLIENT_ID(rcvd_name));
            std::cout << "CLIENT: " << rcvd_name << " LEFT\n";

        } else if (rcvd_command == "shut_down" && rcvd_name == "admin") {
            for (int i = 0; i < logins.size(); i++) {
                send_message_to_client(client_pipe_fd[i], "SERVER CLOSED");
                close(client_pipe_fd[i]);
                std::remove(logins[i].c_str());
            }
            for (int i = 0; i < games_threads.size(); i++) {
                std::remove(games_name[i].c_str());
                games_threads[i].detach();
            }

            close(admin_fd);
            std::remove("admin");
            std::remove("main_input");
            std::cout << "SERVER OFF\n";

            return 0;
        }
    }
}