#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math_functions.h"

int main() {
    char cmd[100];
    
    printf("1 K - Pi(K)\n2 A B - Square(A,B)\n0 - exit\n");
    
    while (1) {
        printf("> ");
        if (!fgets(cmd, 100, stdin)) break;
        
        if (cmd[0] == '0') break;
        
        if (cmd[0] == '1') {
            int K = atoi(cmd + 2);
            printf("Pi: %f\n", Pi(K));
        }
        else if (cmd[0] == '2') {
            float A, B;
            sscanf(cmd + 2, "%f %f", &A, &B);
            printf("Square: %f\n", Square(A, B));
        }
    }
    
    return 0;
}