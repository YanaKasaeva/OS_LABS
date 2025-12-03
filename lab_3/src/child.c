#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

int is_prime(int n) {
    if (n < 1) return 0;
    if (n == 1) return 1;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 1;
    }

    FILE *file = fopen(argv[1], "w");
    if (file == NULL) {
        return 1;
    }

    sem_t *sem_parent = sem_open("/sem_parent", 0);
    sem_t *sem_child = sem_open("/sem_child", 0);

    int input_fd = shm_open("/input_shm", O_RDWR, 0666);
    int output_fd = shm_open("/output_shm", O_RDWR, 0666);

    if (input_fd == -1 || output_fd == -1) {
        return 1;
    }

    char *input_data = mmap(0, 100, PROT_READ | PROT_WRITE, MAP_SHARED, input_fd, 0);
    char *output_data = mmap(0, 100, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd, 0);

    while (1) {
        sem_wait(sem_parent);

        if (strlen(input_data) > 0) {
            char line[100];
            strcpy(line, input_data);
            memset(input_data, 0, 100);

            int num = atoi(line);

            if (num < 0 || is_prime(num)) {
                fprintf(file, "EXIT: %d\n", num);
                fclose(file);
                strcpy(output_data, "EXIT");
                sem_post(sem_child);
                break;
            } else {
                fprintf(file, "%d\n", num);
                fflush(file);
                strcpy(output_data, "OK");
                sem_post(sem_child);
            }
        }
    }

    munmap(input_data, 100);
    munmap(output_data, 100);
    close(input_fd);
    close(output_fd);
    sem_close(sem_parent);
    sem_close(sem_child);

    return 0;
}