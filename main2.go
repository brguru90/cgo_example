package main

/*
// #cgo LDFLAGS: -lc++
#cgo CFLAGS: -x c++ -fpermissive
// #cgo LDFLAGS: -lstdc++
#cgo linux pkg-config: libcurl
#cgo linux pkg-config: libuv
#cgo darwin LDFLAGS: -lcurl
#cgo windows LDFLAGS: -lcurl
#include "test2.h"
#cgo CFLAGS: -g -Wall
#cgo LDFLAGS: -lssl -lcrypto -lpthread -lm 
*/
import "C"
import (
	"fmt"
)


func main() {
	fmt.Println("---------")
	C.call_class()
}
