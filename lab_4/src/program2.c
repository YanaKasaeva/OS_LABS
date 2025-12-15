#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

typedef float (*FuncPi)(int);
typedef float (*FuncSquare)(float, float);

int main() {
    void *lib = dlopen("./libmath_impl1.so", RTLD_LAZY);
    FuncPi Pi = (FuncPi)dlsym(lib, "Pi");
    FuncSquare Square = (FuncSquare)dlsym(lib, "Square");
    int lib_num = 1;
    
    char cmd[100];
    printf("1 K - Pi\n2 A B - Square\n0 - switch lib\n3 - exit\n> ");
    
    while (fgets(cmd, 100, stdin)) {

        if (cmd[0] == '3') break;

        if (cmd[0] == '0') {
            dlclose(lib);
            lib_num = (lib_num == 1) ? 2 : 1;
            char lib_name[30];
            sprintf(lib_name, "./libmath_impl%d.so", lib_num);
            lib = dlopen(lib_name, RTLD_LAZY);
            Pi = (FuncPi)dlsym(lib, "Pi");
            Square = (FuncSquare)dlsym(lib, "Square");
            printf("Switched to lib %d\n> ", lib_num);
            continue;
        }
        
        if (cmd[0] == '1') {
            int K = atoi(cmd + 2);
            printf("Pi: = %f (lib %d)\n", Pi(K), lib_num);
        }
        else if (cmd[0] == '2') {
            float A, B;
            sscanf(cmd + 2, "%f %f", &A, &B);
            printf("Square: %f (lib %d)\n", Square(A, B), lib_num);
        }
        printf("> ");
    }
    
    dlclose(lib);
    return 0;
}