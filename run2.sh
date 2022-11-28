export LD_LIBRARY_PATH=${PWD}
# export GOFLAGS="-count=1"

rm -rf ./*.so ./*.o ./*.a
# gcc  -static  -Wall curl.c -o libcurl.a  -nostartfiles -g -DCURL_STATICLIB -lssl -lcrypto -lpthread -L$PWD/curl/lib -I$PWD/curl/include 
# gcc -fPIC  -shared curl.c -o libcurl.a -lssl -lcrypto -lpthread -L$PWD/curl/lib -I$PWD/curl/include 
# gcc -fPIC  -Wall -shared test22.c -o test22.so -lssl -lcrypto -lpthread -lcurl

gcc -fPIC -shared test22.c -o test22.so -lssl -lcrypto -lpthread  -I$PWD/libffcall/
# gcc -fPIC -c *.c
# gcc -shared -o test22.so test22.o
go run main4.go
# go build -o a.bin && ./a.bin