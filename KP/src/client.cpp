#include "functions.h"
#include <iostream>
#include <fcntl.h>
#include <thread>

inline void intro() {
    std::cout << "Hello! Welcome to the game \"Bulls and cows\"." << std::endl;
    std::cout << "Have you already account in this game?\n Answer: \"yes\" or \"no\"" << std::endl;
    std::cout << "> ";
    std::cout.flush();
}

void func(int fd,const std::string& login){
    while (true){
        std::string respond = receive(fd);
        std::cout << "\n" << respond << std::endl;
        if (respond == "SERVER CLOSED"){
            exit(EXIT_SUCCESS);
        }
        std::cout << login << ">";
        std::cout.flush();
    }
}

std::pair<std::string, std::string> login() {
    std::string login, password;
    std::cout << "Please enter your login:\n> ";
    std::cout.flush();
    std::cin >> login;
    std::cout << "Please enter your password:\n> ";
    std::cout.flush();
    std::cin >> password;
    return std::make_pair(login, SECRET_KEY + password);
}


int connect_to_server() {
    int fd = open(SERVER_INPUT_FILE_NAME, O_RDWR);
    if (fd == -1) {
        std::cout << "Server doesn't exists" << std::endl;
        exit(EXIT_FAILURE);
    }
    return fd;
}

void menu() {
    std::cout << "\t\t Main menu" << std::endl;
    std::cout << "\t List of commands" << std::endl;
    std::cout << "1) rules" << std::endl;
    std::cout << "2) create $table_name $game_word" << std::endl;
    std::cout << "3) connect $table_name" << std::endl;
    std::cout << "4) quit" << std::endl;
}

void loading() {
    std::cout << "\t\tLoading" << std::endl;
    int width = 50;
    int progress = 0;
    while (progress <= 100) {
        int now = progress / 2;
        std::cout << "[";
        for (int i = 0; i < width; ++i) {
            if (i < now) { std::cout << "="; }
            else if (i == now) { std::cout << ">"; }
            else { std::cout << " "; }
        }
        std::cout << "] " << progress << "%" << std::endl;
        progress += 4;
        usleep(50000);
    }
}

void rules() {
    std::cout << "\t\tRules" << std::endl;
    std::cout << "1) One word is made up" << std::endl;
    std::cout << "2) Players try to guess the word" << std::endl;
    std::cout << "3) The server gives response like \"N bulls, M cows\"." << std::endl;
    std::cout << "4) Server's response means N chars are in the answer and are in their place" << std::endl;
    std::cout << "5) M chars are in the answer, but aren't in their place" << std::endl;
}

int main() {
    int client_out_fd = connect_to_server();
    intro();
    std::string answer;
    std::cin >> answer;
    bool ok = (answer == "yes");
    auto pair = login();
    std::string password = std::to_string(transform(pair).second);
    std::string login = pair.first;
    pair.second = ""; // "security"
    std::string cmd;
    ok ? cmd = "log" : cmd = "reg";
    send_to_server(client_out_fd, login, cmd, password);
    std::cout << "Connecting to server" << std::endl;
    loading();
    int fd = open(pair.first.c_str(), O_RDWR);
    if (fd == -1) {
        perror("Open file");
        exit(EXIT_FAILURE);
    }
    menu();
    std::string command, data, game, game_word;
    int game_fd;
    std::thread thread(func, fd, login);
    while (true) {
        std::cout << "> ";
        std::cout.flush();
        std::cin >> command;
        if (command == "rules") {
            rules();
        } else if (command == "create") {
            std::cin >> game >> game_word;
            data = game + "%" + game_word;
            send_to_server(client_out_fd, login, command, data);
        } else if (command == "connect") {
            std::cin >> game;
            data = game;
            loading();
            game_fd = open(("game_" + game).c_str(), O_RDWR);
            if (game_fd == -1) {
                perror("Open file");
                exit(EXIT_FAILURE);
            }
            send_to_server(game_fd, login, command, data);
            while (true) {
                std::cin >> command;
                if (command == "maybe"){
                    std::cin >> data;
                    send_to_server(game_fd, login, command, data);
                } else if (command == "leave"){
                    send_to_server(game_fd, login, command, data);
                    break;
                }
                std::cout << "> ";
                std::cout.flush();
            }
            menu();
        } else if (command == "quit") {
            data = "";
            send_to_server(client_out_fd, login, command, data);
            thread.detach();
            std::cout << "Thank you for game. Come to us later" << std::endl;
            exit(EXIT_SUCCESS);
        } else if (command == "shut_down" and login == ADMIN_NAME){
            data = "";
            send_to_server(client_out_fd, login, command, data);
            thread.detach();
            std::cout << "Server will be turned off" << std::endl;
            exit(EXIT_SUCCESS);
        }
    }
}