package main

/*
#cgo linux pkg-config: libcurl
#cgo linux pkg-config: libuv
#cgo darwin LDFLAGS: -lcurl
#cgo windows LDFLAGS: -lcurl
#include "test22.h"
#cgo CFLAGS: -g -Wall
#cgo LDFLAGS: -lssl -lcrypto -lpthread -lm  -lffcall
*/
import "C"
import (
	"fmt"
)


func main() {
	fmt.Println("---------")
}
