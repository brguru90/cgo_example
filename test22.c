
#include"test22.h"

typedef void *func_t(void *);
// https://www.gnu.org/software/libffcall/callback.html


void add(void* a,va_alist alist)
{
    int* temp=(int*)a;
    va_start_int(alist);
    int b = va_arg_int(alist);
    printf("a=%d,b=%d\n",*temp,b);

    char* s=(char*)va_arg_char(alist);
    printf("c=%s\n",*s);
    // va_list ap;
    // va_start(ap, 1);

    // int b = va_arg(ap, int);

    // printf("a=%d,b=%d\n",a,b);

    // va_end(ap);

    // return a + b;
}

// int run_callback(func_t f)
// {
//     int m=4;
//     return f(&m);
// }

int main()
{
    int v=2,w=4,r=-1;
    char* y="guru\0";
    printf("Entered C\n");
    // r=add(2,4);
    int (*callback)() = alloc_callback(&add, &v);
    callback(w,y);
    // r=callback(w,y);
    // r=run_callback(callback);
    // printf("v=%d\n",r);
    // free_callback(callback);
    return 0;
}