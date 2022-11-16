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
	"bytes"
	"fmt"
	"io/ioutil"
	"net/http"
	"unsafe"
)

// https://stackoverflow.com/questions/65562170/go-pointer-stored-into-non-go-memory
// https://stackoverflow.com/questions/41492071/how-do-i-convert-a-go-array-of-strings-to-a-c-array-of-strings

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
	url := "http://localhost:8000/api/hello/1?query=text"
	// url := "http://guruinfo.epizy.com/edu.php"
	// url:="https://jsonplaceholder.typicode.com/posts"
	req, err := http.NewRequest("POST", url, bytes.NewBuffer([]byte("some string")))
	req.Header.Add("some header", "its value")
	req.Header.Add("some header2", "its value2")

	check_error(err)
	// s_port := req.URL.Port()
	// if s_port == "" {
	// 	if strings.HasPrefix(req.URL.String(), "http://") {
	// 		s_port = "80"
	// 	} else if strings.HasPrefix(req.URL.String(), "https://") {
	// 		s_port = "443"
	// 	}
	// }
	c_url := C.CString(url)
	defer C.free(unsafe.Pointer(c_url))

	c_request_input := C.malloc(C.size_t(5) * C.sizeof_struct_SingleRequestInput)
	defer C.free(unsafe.Pointer(c_request_input))
	request_input := (*[1<<30 - 1]C.struct_SingleRequestInput)(c_request_input)

	c_headers := C.malloc(C.size_t(len(req.Header)) * C.sizeof_struct_Headers)
	defer C.free(unsafe.Pointer(c_headers))
	headers_data := (*[1<<30 - 1]C.struct_Headers)(c_headers)
	var i int = 0
	for name, values := range req.Header {
		for _, value := range values {
			headers_data[i] = C.struct_Headers{
				header: C.CString(name + ": " + value),
			}
			i++
		}
	}

	Body, _ := req.GetBody()
	body, _ := ioutil.ReadAll(Body)

	request_input[0] = C.struct_SingleRequestInput{
		url:         c_url,
		method:      C.CString(req.Method),
		headers:     (*C.struct_Headers)(c_headers),
		headers_len: C.int(len(req.Header)),
		body: C.CString(string(body)),
	}

	var response_data C.struct_ResponseData
	C.send_raw_request(c_url, req.URL.Scheme == "https", &(request_input[0]),&response_data, 1)
	fmt.Println(response_data.status_code)
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
