#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 22222
#define BUF_SIZE 1024

volatile sig_atomic_t sighup_received = 0;

void handle_signal(int sig) {
    if (sig == SIGHUP) {
        sighup_received = 1;
    }
}

void set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl set");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int server_fd, client_fd, active_fd = -1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    fd_set read_fds;
    struct sigaction sa;
    char buffer[BUF_SIZE];
    ssize_t bytes_read;

    // Настройка обработки сигналов
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Блокировка сигнала SIGHUP до вызова pselect
    sigset_t block_mask, all_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGHUP);

    if (sigprocmask(SIG_BLOCK, &block_mask, &all_mask) == -1) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    // Создание серверного сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Сервер запущен на порту %d\n", PORT);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        int max_fd = server_fd;

        if (active_fd != -1) {
            FD_SET(active_fd, &read_fds);
            if (active_fd > max_fd) {
                max_fd = active_fd;
            }
        }

        // Вызов pselect с разблокировкой сигнала
        if (pselect(max_fd + 1, &read_fds, NULL, NULL, NULL, &all_mask) == -1) {
            if (errno == EINTR) {
                // Обработка сигнала
                if (sighup_received) {
                    printf("SIGHUP получен\n");
                    sighup_received = 0;
                }
                continue;
            } else {
                perror("pselect");
                break;
            }
        }

        if (FD_ISSET(server_fd, &read_fds)) {
            client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
            if (client_fd == -1) {
                perror("accept");
                continue;
            }
            printf("Новое подключение с %s:%d\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            if (active_fd == -1) {
                active_fd = client_fd;
                set_non_blocking(active_fd);
            } else {
                printf("Соединение разорвано\n");
                close(client_fd);
            }
        }

        if (active_fd != -1 && FD_ISSET(active_fd, &read_fds)) {
            bytes_read = read(active_fd, buffer, BUF_SIZE);
            if (bytes_read > 0) {
                printf("Получено %zd байтов\n", bytes_read);
            } else if (bytes_read == 0 || (bytes_read == -1 && errno != EAGAIN)) {
                printf("Соединение потеряно\n");
                close(active_fd);
                active_fd = -1;
            }
        }
    }

    close(server_fd);
    return 0;
}
