#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <ctype.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

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
    char filename[100];
    char number[100];
    pid_t pid;

    sem_unlink("/sem_parent");
    sem_unlink("/sem_child");
    sem_t *sem_parent = sem_open("/sem_parent", O_CREAT, 0666, 0);
    sem_t *sem_child = sem_open("/sem_child", O_CREAT, 0666, 0);

    shm_unlink("/input_shm");
    shm_unlink("/output_shm");

    int input_fd = shm_open("/input_shm", O_CREAT | O_RDWR, 0666);
    int output_fd = shm_open("/output_shm", O_CREAT | O_RDWR, 0666);

    ftruncate(input_fd, 100);
    ftruncate(output_fd, 100);

    char *input_data = mmap(0, 100, PROT_READ | PROT_WRITE, MAP_SHARED, input_fd, 0);
    char *output_data = mmap(0, 100, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd, 0);

    memset(input_data, 0, 100);
    memset(output_data, 0, 100);

    printf("Введите имя файла: ");
    scanf("%s", filename);

    pid = fork();
    if (pid == 0) {
        close(input_fd);
        close(output_fd);
        sem_close(sem_parent);
        sem_close(sem_child);
        execl("./child", "child", filename, NULL);
        exit(1);
    } else {
        sleep(1);

        while (1) {
            printf("Введите число: ");
            scanf("%s", number);

            if (!is_integer(number)) {
                printf("Введенные данные не типа int, попробуйте еще раз:\n");
                continue;
            }

            if (waitpid(pid, NULL, WNOHANG) == pid) {
                printf("Дочерний процесс завершен\n");
                break;
            }

            strcpy(input_data, number);
            sem_post(sem_parent);

            sem_wait(sem_child);

            char response[10];
            strcpy(response, output_data);

            if (strstr(response, "EXIT")) {
                printf("Получен сигнал завершения\n");
                break;
            }

            memset(output_data, 0, 100);
        }

        wait(NULL);

        munmap(input_data, 100);
        munmap(output_data, 100);
        close(input_fd);
        close(output_fd);
        sem_close(sem_parent);
        sem_close(sem_child);
        sem_unlink("/sem_parent");
        sem_unlink("/sem_child");
        shm_unlink("/input_shm");
        shm_unlink("/output_shm");

        printf("Родительский процесс завершен\n");
    }
    return 0;
}