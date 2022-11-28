#include<stdio.h>


static inline void test_access(){
    extern int n;
    printf("n=%d\n",n);
}

int main(){
    int n=4;
    test_access();
}