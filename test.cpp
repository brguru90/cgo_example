#include <stdio.h>
#include <functional>

typedef int (*func_type)(int);
typedef void* (*func_any_type)(void*);

int run_callback(func_type func)
{
    (func)(2);
    // printf("%p\n",(func)(2));
    // // printf("%d\n",(func)(2));
    // // return func(2);
    return 1;
}

int main()
{
    int b = 2;
    // auto lambda = [](int a)
    // {
    //     printf("lambda got %d\n", a);
    //     return a + 1;
    // };

    auto *lambda = new std::function<int(int a)>([=](int a) -> int
    { 
        printf("lambda got %d\n",a);
        return a+b; 
    });
    auto t=reinterpret_cast<void *>(lambda);
    // printf("direct call,  from lambda=%d\n", (*lambda)(2));
    printf("direct call,  from lambda=%d\n", (t)(2));

   
    // printf("from lambda=%d\n",  run_callback(  reinterpret_cast<void*>(lambda)  )   );
}