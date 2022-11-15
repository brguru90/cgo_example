package main

/*
#cgo linux pkg-config: libcurl
#cgo darwin LDFLAGS: -lcurl
#cgo windows LDFLAGS: -lcurl
#include <stdlib.h>
#include <string.h>
#include "api_req.h"
#cgo CFLAGS: -g -Wall
#cgo LDFLAGS: -L${SRCDIR} api_req.so
*/
import "C"
import (
	"fmt"
	"net/http"
	"net/http/httputil"
	"strings"
	"unsafe"
)

var gchan chan int

//export goCallback
func goCallback(myid C.int) {
	fmt.Printf("go api call from c %d\n", int(myid))
	// gchan <- int(myid)
	// if int(myid)==-1{
	// 	close(gchan)
	// }
}

func callCAPI(s string) {
	cs := C.CString(s)
	defer C.free(unsafe.Pointer(cs))
	C.run_bulk_api_request(cs)
}

func check_error(err error) {
	if err != nil {
		panic(err)
	}
}

func call_api() {
	// url := "http://localhost:8000/api/test"
	url:="http://guruinfo.epizy.com/edu.php"
	// url:="https://jsonplaceholder.typicode.com/posts"
	req, err := http.NewRequest("GET", url, nil)
	check_error(err)
	reqDump, err := httputil.DumpRequestOut(req, true)
	s_port := req.URL.Port()
	if s_port == "" {
		if strings.HasPrefix(req.URL.String(), "http://") {
			s_port = "80"
		} else if strings.HasPrefix(req.URL.String(), "https://") {
			s_port = "443"
		}
	}
	// port, err := strconv.Atoi(s_port)
	// check_error(err)

	// host := C.CString(req.URL.Hostname())
	c_url := C.CString(url)
	defer C.free(unsafe.Pointer(c_url))

	raw_req := C.CString(string(reqDump))
	defer C.free(unsafe.Pointer(raw_req))

	C.send_raw_request(c_url, req.URL.Scheme == "https", raw_req, 1)
}

func main() {
	gchan = make(chan int)
	// str1 := "hi how are you\n"
	// callCAPI(str1)

	call_api()

	// for v:=range gchan{
	// 	println(v);
	// }
	// fmt.Println("channel closed")
}
