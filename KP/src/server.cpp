#include "functions.h"
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <map>
#include <thread>

void check_data_base() {
    std::ifstream file(DATA_BASE_FILE_NAME);
    file.open(DATA_BASE_FILE_NAME);
    if (file.is_open()) {
        std::cout << "OK: DATA BASE" << std::endl;
        file.close();
    } else {
        std::ofstream f(DATA_BASE_FILE_NAME);
        std::string empty_string;
        auto pair = std::make_pair(empty_string, SECRET_KEY + ADMIN_PASSWORD);
        std::string pass = std::to_string(transform(pair).second);
        pair.first = ADMIN_NAME;
        pair.second = pass;
        auto log_pas = transform(pair);
        f << log_pas.first << " " << log_pas.second << std::endl;
        f.close();
        std::cout << "OK: DATA BASE" << std::endl;
    }
}

void add_results(const std::string &game_name, const std::string &game_word, const std::string &winner) {
    std::ifstream inp;
    std::ofstream out;
    inp.open(RESULTS_FILE_NAME);
    if (inp.is_open()){
        inp.close();
        out.open(RESULTS_FILE_NAME, std::ios::app);
        out << game_name << "\t\t" << game_word << "\t\t" << winner << std::endl;
        out.close();
    } else{
        out.open(RESULTS_FILE_NAME);
        out << "Game name\t\tGame word\t\tWinner" << std::endl;
        out << game_name << "\t\t" << game_word << "\t\t" << winner << std::endl;
        out.close();
    }
    std::cout << "RESULTS UPDATED" << std::endl;
}

bool check_in_data_base(std::pair<std::string, std::string> &login_password) {
    std::pair<int, int> pair = transform(login_password);
    std::ifstream file;
    file.open(DATA_BASE_FILE_NAME);
    int log, pas;
    while (file >> log >> pas) {
        if (log == pair.first) {
            if (pas == pair.second) {
                file.close();
                return true;
            }
            file.close();
            return false;
        }
    }
    file.close();
    return false;
}

bool add_in_data_base(std::pair<std::string, std::string> &login_password) {
    std::pair<int, int> pair = transform(login_password);
    std::ifstream file;
    file.open(DATA_BASE_FILE_NAME, std::ios::in);
    int log, pas;
    while (file >> log >> pas) {
        if (log == pair.first) {
            file.close();
            return false;
        }
    }
    file.close();
    std::ofstream f(DATA_BASE_FILE_NAME, std::ios::app);
    f << pair.first << " " << pair.second << std::endl;
    f.close();
    return true;
}

int create_main_pipe() {
    if (mkfifo(SERVER_INPUT_FILE_NAME, ACCESS_PERMS) == -1) {
        perror("Mkfifo");
        exit(EXIT_FAILURE);
    }
    int fd = open(SERVER_INPUT_FILE_NAME, O_RDWR);
    if (fd == -1) {
        perror("Open file");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int create_client_pipe(std::string &login) {
    if (mkfifo(login.c_str(), ACCESS_PERMS) == -1) {
        perror("Mkfifo");
        exit(EXIT_FAILURE);
    }
    int fd = open(login.c_str(), O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int create_game_pipe(const std::string &game_name) {
    if (mkfifo(("game_" + game_name).c_str(), ACCESS_PERMS) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    int fd = open(("game_" + game_name).c_str(), O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int check(const std::string &game_word, const std::string &try_word, int &cows, int &bulls) {
    bulls = 0;
    cows = 0;
    if (try_word.size() != game_word.size()) {
        return -1;
    }
    if (try_word == game_word) {
        return 1;
    }
    size_t size = game_word.size();
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            if (game_word[i] == try_word[j]) {
                if (i == j) {
                    ++bulls;
                } else {
                    ++cows;
                }
            }
        }
    }
    return 0;
}

void game_func(const std::string &game_name, const std::string &game_word) {
    std::map<std::string, int> players_fd;
    int fd = create_game_pipe(game_name);
    int cows, bulls;
    std::string game_respond;
    int game_status;

    std::cout << "START GAME: " << game_name << std::endl;
    std::string login, cmd, data;
    while (true) {
        login = cmd = data = "";
        receive_from_client(fd, &login, &cmd, &data);
        if (cmd == "connect") {
            players_fd[login] = open(login.c_str(), O_RDWR);
            std::cout << "CLIENT " << login << " JOINED THE TABLE " << game_name << std::endl;
            game_respond =
                    "Welcome to the game " + game_name + "\n" + "Make your guesses with the command \"maybe $word\"";
            send_to_client(players_fd[login], game_respond);
            game_respond = login + " connected to the game";
            for (const auto &it: players_fd) {
                if (it.first != login){
                    send_to_client(it.second, game_respond);
                }
            }
        } else if (cmd == "maybe") {
            game_status = check(game_word, data, cows, bulls);
            if (game_status == -1) {
                game_respond = "Wrong string size";
                send_to_client(players_fd[login], game_respond);
            } else if (game_status == 1) {
                game_respond = "You won the game";
                send_to_client(players_fd[login], game_respond);
                add_results(game_name, game_word, login);
                game_respond = "Game is over. Winner is " + login + ". Game word is " + game_word + ".";
                std::string annoy = "Please leave from the table. Use command \"leave\".";
                for (const auto &it: players_fd) {
                    game_respond += annoy;
                    send_to_client(it.second, game_respond);
                }
                for (const auto &it:players_fd){
                    while (true){
                        receive_from_client(fd, &login, &cmd, &data);
                        if (cmd == "leave"){
                            std::cout << "PLAYER " << login << " LEFT THE TABLE " << game_name << std::endl;
                            break;
                        } else{
                            send_to_client(it.second, annoy);
                        }
                    }
                }
                close(fd);
                std::cout << "FINISH GAME: " << game_name << std::endl;
                int mainFD = open(SERVER_INPUT_FILE_NAME, O_RDWR);
                game_respond = "finish";
                login = game_name;
                send_to_server(mainFD, login, game_respond, data);
                return;
            } else {
                game_respond = "Bulls: " + std::to_string(bulls) + " Cows: " + std::to_string(cows);
                send_to_client(players_fd[login], game_respond);
            }
        } else if (cmd == "leave") {
            players_fd.erase(login);
            std::cout << "CLIENT " << login << " LEFT FROM THE TABLE " << game_name << std::endl;
        }
    }
}


int main() {
    int fd_rec = create_main_pipe();
    std::map<std::string, int> logins_fds;
    std::vector<std::thread> games_threads;
    std::vector<std::string> games_names;
    std::string game_name, game_word;
    std::string login, cmd, data;
    check_data_base();
    while (true) {
        login = cmd = data = "";
        receive_from_client(fd_rec, &login, &cmd, &data);
        if (cmd == "log") {
            std::string password = data;
            std::pair<std::string, std::string> pair = std::make_pair(login, password);
            if (check_in_data_base(pair)) {
                std::cout << "NEW CLIENT: " << login << std::endl;
                logins_fds[login] = create_client_pipe(login);
            }
        } else if (cmd == "reg") {
            std::string password = data;
            std::pair<std::string, std::string> pair = std::make_pair(login, password);
            if (add_in_data_base(pair)) {
                std::cout << "NEW USER: " << login << std::endl;
                logins_fds[login] = create_client_pipe(login);
            }
        } else if (cmd == "create") {
            game_name = game_word = "";
            parse_game_word(data, game_name, game_word);
            games_names.emplace_back("game_" + game_name);
            games_threads.emplace_back(std::thread(game_func, game_name, game_word));
        } else if (cmd == "quit") {
            close(logins_fds[login]);
            std::remove(login.c_str());
            logins_fds.erase(login);
            std::cout << "CLIENT " << login << " LEFT THE GAME" << std::endl;
        } else if (cmd == "shut_down" and login == ADMIN_NAME) {
            std::string message = "SERVER CLOSED";
            for (auto &it: logins_fds) {
                send_to_client(it.second, message);
                close(it.second);
                remove(it.first.c_str());
            }
            for (size_t i = 0; i < games_threads.size(); ++i) {
                std::remove(games_names[i].c_str());
                games_threads[i].detach();
            }
            std::remove(SERVER_INPUT_FILE_NAME);
            std::cout << "SERVER OFF" << std::endl;
            exit(EXIT_SUCCESS);
        }
    }
}