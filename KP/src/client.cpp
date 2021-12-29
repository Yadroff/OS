#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include "funcs.hpp"
#include <thread>

#define SEND_TO_SERVER(FD) send_message_to_server(FD, login, command, data)

//функция приёма сообщений (для потока)
void func(int fd_respond, std::string &login) {
    while (true) {
        std::string respond = recieve_message_client(fd_respond);
        std::cout << "\n" << respond << "\n";
        std::cout.flush();
        if (respond == "SERVER CLOSED")
            exit(0);
        std::cout << login << "> ";
        std::cout.flush();
    }
}

inline void write_intro() {
    std::cout
            << "Добро пожаловать в игру Быки и Коровы.\nЧтобы создать аккаунт запустите ./server и введите там свой логи\n";
    std::cout << "Затем перезапустите клиент и впишите свой логин\n";
    std::cout << "Введите свой логин: ";
    std::cout.flush();
}

inline void write_menu(std::string &login) {
    std::cout << "Соединение установлено, можете отдавать команды\n";
    std::cout << "Список команд:\n";
    std::cout << "1) create @название игрового стола@ @игровое слово@\n";
    std::cout << "2) connect @название игры@\n";
    std::cout << "3) leave\n";
    if (login != "admin")std::cout << "4) quit\n";
    if (login == "admin") std::cout << "5) shut_down - выключение сервера\n";
    std::cout.flush();
}

inline int server_main_input_fd() {

    int fd = open("main_input", O_RDWR);
    if (fd == -1) {
        std::cout << "ERROR: MAIN FIFO WAS NOT OPENED\n";
        exit(1);
    }
    return fd;
}

int main() {
    int client_main_out_fd = server_main_input_fd();

    write_intro();
    std::string login;
    std::cin >> login;
    send_message_to_server(client_main_out_fd, login, "login", "");
    std::cout << "Устанавливаю соединение\n";
    sleep(1);
    int fd_respond = open(login.c_str(), O_RDWR);
    if (fd_respond == -1) {
        std::cout << "RESPOND FIFO WAS NOT OPENED";
        exit(1);
    }

    write_menu(login);
    std::thread thr_respond(func, fd_respond, login);

    std::string command, data;
    std::string game_word, game_name;
    int game_fd;

    while (true) {
        std::cout << login << "> ";
        std::cin >> command;

        if (command == "create") {
            std::cin >> game_name >> game_word;
            data = game_name + "$" + game_word;
            SEND_TO_SERVER(client_main_out_fd);
        } else if (command == "connect") {
            std::cin >> game_name;
            game_fd = open(("game_%" + game_name).c_str(), O_RDWR);
            if (game_fd == -1) {
                std::cout << "ERROR: GAME NOT FOUND\n";
                std::cout.flush();
            } else {
                data = "";
                SEND_TO_SERVER(game_fd);
                std::cout << login << "> ";
                std::cout.flush();
                while (true) {
                    std::cin >> command;

                    if (command == "maybe") {
                        std::cin >> data;
                        SEND_TO_SERVER(game_fd);
                    } else if (command == "leave") {
                        data = "";
                        SEND_TO_SERVER(game_fd);
                        break;
                    } else {
                        std::cout << login << "> ";
                        std::cout.flush();
                    }
                }
            }
        } else if (command == "quit" && login != "admin") {
            data = "";
            SEND_TO_SERVER(client_main_out_fd);
            thr_respond.detach();
            return 0;
        } else if (command == "shut_down" && login == "admin") {
            data = "";
            SEND_TO_SERVER(client_main_out_fd);
            thr_respond.detach();
            return 0;
        }
    }
    return 0;
}