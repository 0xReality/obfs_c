#include <stdio.h>

int add(int a, int b);

int main(int argc, char** argv){
    if(argc < 2){
        return 1;
    }
    int x = 5;
    int y = 2;
    printf("add: %d\n", add(x, y));
    return 0;
}

int add(int a, int b){
    return a + b;
}
