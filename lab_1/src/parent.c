#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <ctype.h>
#include <limits.h>

// Функция проверки что строка - целое число в границах int
int is_integer(const char *str) {
    if (str == NULL || *str == '\0') return 0;

    int i = 0;

    if (str[0] == '-') {
        if (str[1] == '\0') return 0;
        i = 1;
    }

    for (; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) return 0;
    }

    long num = atol(str);
    if (num < -2147483648 || num > 2147483647) {
        return 0;
    }

    return 1;
}

int main() {
    int pipe1[2], pipe2[2];
    char filename[100];
    char number[100];
    pid_t pid;

    pipe(pipe1);
    pipe(pipe2);

    printf("Введите имя файла: ");
    scanf("%s", filename);

    pid = fork();
    if (pid == 0) {
        // Дочерний процесс
        close(pipe1[1]);
        close(pipe2[0]);
        dup2(pipe1[0], 0);
        close(pipe1[0]);
        dup2(pipe2[1], 1);
        close(pipe2[1]);
        execl("./child", "child", filename, NULL);
        exit(1);
    } else {
        // Родительский процесс
        close(pipe1[0]);
        close(pipe2[1]);

        while (1) {
            printf("Введите число: ");
            scanf("%s", number);

            if (!is_integer(number)) {
                printf("Введенные данные не типа int, попробуйте еще раз:\n");
                continue;
            }

            // Проверяем, жив ли дочерний процесс
            if (waitpid(pid, NULL, WNOHANG) == pid) {
                printf("Дочерний процесс завершен\n");
                break;
            }

            write(pipe1[1], number, strlen(number));
            write(pipe1[1], "\n", 1);

            char response[10];
            if (read(pipe2[0], response, 10) > 0) {
                if (strstr(response, "EXIT")) {
                    printf("Получен сигнал завершения\n");
                    break;
                }
            }
        }

        close(pipe1[1]);
        close(pipe2[0]);
        wait(NULL);
        printf("Родительский процесс завершен\n");
    }
    return 0;
}