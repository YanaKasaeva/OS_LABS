#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_prime(int n) {
    if (n < 1) return 0;
    if (n == 1) return 1;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    FILE *file = fopen(argv[1], "w");
    char line[100];

    while (fgets(line, 100, stdin)) {
        int num = atoi(line);

        if (num < 0 || is_prime(num)) {
            fprintf(file, "EXIT: %d\n", num);
            fclose(file);
            printf("EXIT\n");
            fflush(stdout);
            return 0;
        } else {
            fprintf(file, "%d\n", num);
            fflush(file);
            printf("OK\n");
            fflush(stdout);
        }
    }
    fclose(file);
    return 0;
}