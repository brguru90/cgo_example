#include <stdio.h>
#include <functional>

typedef int (*func_type)(int *);
typedef void *(*func_any_type)(void *);

typedef struct TestLambda
{
    func_type func;
} lambda_t;

int run_callback(func_type func)
{
    int n = 77;
    printf("&n=%d\n", &n);
    return (func)(&n);
}

struct Closure
{
    template <typename Any, typename RETURN_TYPE>
    static Any lambda_ptr_exec(void *data)
    {
        return (Any)(*(RETURN_TYPE *)callback<RETURN_TYPE>())(data);
    }

    template <typename Any = void, typename CALLER_TYPE = Any (*)(void *), typename RETURN_TYPE>
    static CALLER_TYPE create(RETURN_TYPE &t)
    {
        callback<RETURN_TYPE>(&t);
        return (CALLER_TYPE)lambda_ptr_exec<Any, RETURN_TYPE>;
    }

    template <typename T>
    static void *callback(void *new_callback = nullptr)
    {
        static void *callback;
        if (new_callback != nullptr)
            callback = new_callback;
        return callback;
    }
};

template <class Type>
struct member_function;

template <class Type, class Ret, class... Args>
struct member_function<Ret (Type::*)(Args...)>
{
    template <Ret (Type::*Func)(Args...)>
    static Ret adapter(Type &obj, Args &&...args)
    {
        return (obj.*Func)(std::forward<Args>(args)...);
    }
};

template <class Type, class Ret, class... Args>
struct member_function<Ret (Type::*)(Args...) const>
{
    template <Ret (Type::*Func)(Args...) const>
    static Ret adapter(const Type &obj, Args &&...args)
    {
        return (obj.*Func)(std::forward<Args>(args)...);
    }
};

class ATest
{
public:
    int b = -1;
    int (ATest::*add_ptr)(int *);
    int add(int *a)
    {
        printf("a=%d,b=%d\n", *a, b);
        return *a + b;
    }
};

int main()
{
    int k = 4;
    int b = 2;
    // auto lambda = [&](void *a)
    // {
    //     printf("lambda got %d\n", *(int*)a);
    //     return ( *(int *)a) + 1;
    // };

    ATest atest;
    atest.b = b;
    atest.add_ptr = &ATest::add;

    auto func = &member_function<decltype(&ATest::add)>::adapter<&ATest::add>;

    printf("atest.add_pt=%d,func_type=%d\n",sizeof(atest.add_ptr),sizeof(func_type));

    // run_callback((func_type)atest.add_ptr);

    // printf("direct call,  from lambda=%d\n", atest.add(&k));
    // printf("direct call,  from lambda=%d\n", (func)(atest, &k));
    // run_callback(func);
    printf("run_callback call,  from lambda=%d\n", run_callback((func_type)(atest.add_ptr)));
    // auto f = [](int *n) -> int
    // {
    //     printf("n=%d\n", *n);
    //     return *n;
    // };
    // printf("run_callback call,  from lambda=%d\n", run_callback((func_type)(*f)));

    // auto cls=Closure::create<int>(lambda);
    // printf("direct call,  from lambda=%d\n", cls(&k));

    // printf("run_callback call,  from lambda=%d\n", run_callback((func_type)fp));

    // int a = 100;
    // auto b2 = [&](void *data)
    // { return *(int *)(data) + a; };
    // auto f4 = Lambda::ptr<int>(b2);
    // printf("%d\n", f4(&k)); // 108

    // https://stackoverflow.com/questions/7852101/c-lambda-with-captures-as-a-function-pointer

    // auto *lambda = new std::function<int(int a)>([=](int a) -> int
    //                                              {
    //     printf("lambda got %d,%d\n",a,b);
    //     return a+b;
    // });

    // auto fp= Lambda::ptr<int>(lambda);

    // printf("direct call,  from lambda=%d\n", fp(&k));

    // printf("run_callback call,  from lambda=%d\n", run_callback((func_type)fp));

    // // auto t=reinterpret_cast<void *>(lambda);
    // printf("direct call,  from lambda=%d\n", (*lambda)(2));
    // // printf("direct call,  from lambda=%d\n", (t)(2));

    // printf("s=%d,func_type=%d,t=%p\n",sizeof(*lambda),sizeof(func_type), *lambda);

    // auto *test_cast=(func_type)lambda;

    // printf("s=%d,t=%p\n",sizeof(*test_cast),*test_cast);
    // (*test_cast)(2);

    //  run_callback((int (*)(int))(lambda));

    // printf("from lambda=%d\n",  run_callback(  reinterpret_cast<func_type&>(*lambda)  )   );
}