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
    
    // בדיקה אם זה localhost או כתובת IP
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

    // חיבור לשרת
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

    // לולאה לשליחת הודעות
    std::string message;
    while (true)
    {
        // בדיקה מהירה אם יש הודעות מהשרת (timeout קצר מאוד)
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock_fd, &readfds);
        
        // timeout קצר מאוד - 10 מילישניות
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 10000; // 10ms
        
        int activity = select(sock_fd + 1, &readfds, NULL, NULL, &tv);
        
        if (activity < 0) {
            perror("select");
            break;
        }
        
        // יש הודעות מהשרת - נטפל בהן מיד!
        if (activity > 0 && FD_ISSET(sock_fd, &readfds)) {
            char buffer[1024];
            ssize_t bytes_received = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                std::cout << "\nServer response: " << buffer << std::endl;
                
                // בדיקה אם השרת שולח הודעת סגירה
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
        
        // בדיקה מהירה אם יש קלט מהמקלדת
        if (activity == 0) { // timeout - אין הודעות מהשרת
            // בדיקה מהירה אם יש קלט מהמקלדת
            fd_set stdin_fds;
            FD_ZERO(&stdin_fds);
            FD_SET(STDIN_FILENO, &stdin_fds);
            
            struct timeval quick_tv;
            quick_tv.tv_sec = 0;
            quick_tv.tv_usec = 0; // לא מחכים בכלל
            
            if (select(STDIN_FILENO + 1, &stdin_fds, NULL, NULL, &quick_tv) > 0) {
                std::cout << "\nEnter graph parameters (e.g., '-e 5 -v 4 -s 43') or 'quit' to exit: ";
                std::cout << "\nServer will run ALL algorithms on the generated graph." << std::endl;
                std::getline(std::cin, message);
                
                if (message == "quit") {
                    break;
                }
                
                if (message.empty()) {
                    continue;
                }
                
                // שליחת ההודעה לשרת
                if (send(sock_fd, message.c_str(), message.length(), 0) < 0) {
                    perror("send");
                    break;
                }
                
                std::cout << "Graph parameters sent successfully! Server will run all algorithms..." << std::endl;
            }
        }
    }

    close(sock_fd);
    std::cout << "Disconnected from server" << std::endl;
    return 0;
}
