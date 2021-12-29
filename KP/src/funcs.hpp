#include <string>

//отправить сообщение серверу в удобной форме - логин$команда$сообщение
void send_message_to_server(int fd, std::string curlogin, std::string command, std::string data)
{
    std::string text = curlogin + "$" + command + "$" + data;
    int k = text.size();
    write(fd, &k, sizeof(k));
    char messagec[k];
    for (int i = 0; i < k; ++i)
        messagec[i] = text[i];

    write(fd, messagec, k);
}
//отправить сообщение клиенту
void send_message_to_client(int fd, std::string message)
{
    std::string text = message;
    int k = text.size();
    write(fd, &k, sizeof(k));
    char messagec[k];
    for (int i = 0; i < k; ++i)
        messagec[i] = text[i];
    write(fd, messagec, k);
}

//получить сообщение в удобной для клиента форме
std::string recieve_message_client(int fd)
{
    int size;
    read(fd, &size, sizeof(size));

    char messagec[size];
    read(fd, messagec, size);

    std::string recv;
    for (int i = 0; i < size; ++i)
    {
        if (messagec[i] != '$')
            recv.push_back(messagec[i]);
        else
            recv = recv + ": ";
    }
    return recv;
}

//получить логин из сообщения для сервера
std::string extract_login(std::string message)
{
    std::string login;
    int i = 0;
    while (message[i] != '$')
    {
        login.push_back(message[i]);
        ++i;
    }
    return login;
}

//получить сообщение для клиента
std::string extract_command(std::string message)
{
    std::string command;
    int i = 0;
    while (message[i] != '$')
        ++i;
    ++i;
    while (message[i] != '$')
    {
        command.push_back(message[i]);
        ++i;
    }
    return command;
}

std::string extract_data(std::string message)
{
    std::string data;
    size_t i = 0;
    while (message[i] != '$')
        ++i;
    ++i;
    while (message[i] != '$')
        ++i;
    ++i;
    while (i < message.size())
    {
        data.push_back(message[i]);
        ++i;
    }
    return data;
}

void recieve_message_server(int fd, std::string *rcvd_name, std::string *rcvd_command, std::string *rcvd_data)
{
    int size;
    read(fd, &size, sizeof(size));

    char messagec[size];
    read(fd, messagec, size);

    std::string recv;
    for (int i = 0; i < size; ++i)
        recv.push_back(messagec[i]);
    
    *rcvd_name = extract_login(recv);
    *rcvd_command = extract_command(recv);
    *rcvd_data = extract_data(recv);
}

inline void extract_game_data(std::string message, std::string *game_name, std::string *game_word)
{
    size_t i = 0;
    std::string recv1;
    std::string recv2;
    while (message[i] != '$') 
    {
        recv1.push_back(message[i]);
        ++i;
    }
    ++i;
    while (i < message.size())
    {
        recv2.push_back(message[i]);
        ++i;
    }
    *game_name = recv1;
    *game_word = recv2;
}