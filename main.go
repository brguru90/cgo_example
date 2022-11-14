package main

/*
#include <stdlib.h>
#include <string.h>
#include "api_req.h"
#cgo CFLAGS: -g -Wall
#cgo LDFLAGS: -L${SRCDIR} api_req.so
*/
import "C"
import (
	"fmt"
	"unsafe"
)

var gchan chan int

//export goCallback
func goCallback(myid C.int){
	// fmt.Printf("go api call from c %d\n",int(myid))
	gchan <- int(myid)
	if int(myid)==-1{
		close(gchan)
	}
}

func callCAPI(s string) {
	cs := C.CString(s)
	defer C.free(unsafe.Pointer(cs))
	C.run_bulk_api_request(cs)
}

func main() {
	gchan=make(chan int)
	str1 := "hi how are you\n"
	go callCAPI(str1)

	for v:=range gchan{
		println(v);
	}
	fmt.Println("channel closed")
}
