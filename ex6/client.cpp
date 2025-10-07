#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "graph.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port>" << std::endl;
        std::cerr << "Examples:" << std::endl;
        std::cerr << "  " << argv[0] << " localhost 8080" << std::endl;
        std::cerr << "  " << argv[0] << " 127.0.0.1 8080" << std::endl;
        return 1;
    }

    std::string server_ip = argv[1];
    int port = atoi(argv[2]);

    if (port <= 0 || port > 65535)
    {
        std::cerr << "Error: Invalid port number" << std::endl;
        return 1;
    }

    // יצירת socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket");
        return 1;
    }

    // הגדרת כתובת השרת
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    

    if (server_ip == "localhost" || server_ip == "127.0.0.1")
    {
        server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }
    else if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0)
    {
        std::cerr << "Error: Invalid IP address" << std::endl;
        close(sock_fd);
        return 1;
    }

    // conect to sever
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    std::cout << "Connected to server " << server_ip << ":" << port << std::endl;

    std::cout << "Connected to server " << server_ip << ":" << port << std::endl;
    std::cout << "Type messages to send to server, or 'quit' to exit." << std::endl;
    std::cout << "Note: Server shutdown messages will be detected automatically." << std::endl;

    // sending messages loop
    std::string message;
    while (true)
    {
        // small timeout to check if there is new message
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock_fd, &readfds);
        
        // timeout 
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 10000; // 10ms
        
        int activity = select(sock_fd + 1, &readfds, NULL, NULL, &tv);
        
        if (activity < 0) {
            perror("select");
            break;
        }
        
        // there is message need handeling
        if (activity > 0 && FD_ISSET(sock_fd, &readfds)) {
            char buffer[1024];
            ssize_t bytes_received = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                std::cout << "\nServer response: " << buffer << std::endl;
                
                // check if server sent closing message
                if (std::string(buffer) == "SERVER_SHUTDOWN") {
                    std::cout << "Server is shutting down, disconnecting..." << std::endl;
                    break;
                }
            } else if (bytes_received == 0) {
                std::cout << "\nServer disconnected" << std::endl;
                break;
            } else {
                perror("recv");
                break;
            }
        }
        
        //check is there input drom keyboard
        if (activity == 0) { 
            fd_set stdin_fds;
            FD_ZERO(&stdin_fds);
            FD_SET(STDIN_FILENO, &stdin_fds);
            
            struct timeval quick_tv;
            quick_tv.tv_sec = 0;
            quick_tv.tv_usec = 0; // לא מחכים בכלל
            
            if (select(STDIN_FILENO + 1, &stdin_fds, NULL, NULL, &quick_tv) > 0) {
                std::cout << "\nEnter graph request (e.g., '-e 5 -v 4') or 'quit' to exit: ";
                std::getline(std::cin, message);
                
                if (message == "quit") {
                    break;
                }
                
                if (message.empty()) {
                    continue;
                }
                
                // send to server a message
                if (send(sock_fd, message.c_str(), message.length(), 0) < 0) {
                    perror("send");
                    break;
                }
                
                std::cout << "graph request sent successfully!" << std::endl;
            }
        }
    }

    close(sock_fd);
    std::cout << "Disconnected from server" << std::endl;
    return 0;
}
