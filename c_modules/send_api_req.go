package c_modules

/*
#cgo CXXFLAGS: -std=gnu++17
#cgo linux pkg-config: libcurl
#cgo linux pkg-config: libuv
#cgo darwin LDFLAGS: -lcurl
#cgo windows LDFLAGS: -lcurl
#include <stdlib.h>
#include <string.h>
#include "api_req.h"
#cgo CFLAGS: -g -Wall
#cgo LDFLAGS: -lssl -lcrypto -lpthread -lm
*/
import "C"
import (
	"bufio"
	"bytes"
	"encoding/json"
	"reflect"

	// "encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"math"
	"net/http"

	// "reflect"
	"strconv"

	// "strconv"
	"strings"
	"unsafe"
)

func carray2slice(array *C.struct_ResponseData, len int) []C.struct_ResponseData {
	var list []C.struct_ResponseData
	sliceHeader := (*reflect.SliceHeader)((unsafe.Pointer(&list)))
	sliceHeader.Cap = len
	sliceHeader.Len = len
	sliceHeader.Data = uintptr(unsafe.Pointer(array))
	return list	
	// return (*[1 << 30]C.struct_ResponseData)(unsafe.Pointer(array))[:len:len]
}


type ResponseDataCMap struct 
{
    Debug int
    Uid string
    Response_header string
    Response_body string
    Before_connect_time_microsec int64
    After_response_time_microsec int64
    Connected_at_microsec int64
    First_byte_at_microsec int64
    Finish_at_microsec int64
    Connect_time_microsec int64
    Time_to_first_byte_microsec int64
    Total_time_from_curl_microsec int64
    Total_time_microsec int64
    Status_code int
    Err_code int
}

type thread_data_to_json_type struct{
	Data []ResponseDataCMap
	Start int
	End int
}

//export thread_data_to_json
func thread_data_to_json(td C.struct_ResponseData, _len C.int,start C.int,end C.int) *C.char {
	td_arr := carray2slice(&td, int(_len))
	td_slice:=[]ResponseDataCMap{}

	for _, td_item := range td_arr {
		td_slice=append(td_slice, ResponseDataCMap{
			Debug: int(td_item.Debug),
			Uid: C.GoString(td_item.Uid),
			Response_header: C.GoString(td_item.Response_header),
			Response_body:  C.GoString(td_item.Response_body),
			Before_connect_time_microsec: int64(td_item.Before_connect_time_microsec),
			After_response_time_microsec: int64(td_item.After_response_time_microsec),
			Connected_at_microsec: int64(td_item.Connect_time_microsec),
			First_byte_at_microsec: int64(td_item.First_byte_at_microsec),
			Finish_at_microsec: int64(td_item.Finish_at_microsec),
			Connect_time_microsec: int64(td_item.Connect_time_microsec),
			Time_to_first_byte_microsec: int64(td_item.Time_to_first_byte_microsec),
			Total_time_from_curl_microsec: int64(td_item.Total_time_from_curl_microsec),
			Total_time_microsec: int64(td_item.Total_time_microsec),
			Status_code: int(td_item.Status_code),
			Err_code: int(td_item.Status_code),
		})
		println("Response_body=>", C.GoString(td_item.Response_body), "<=")
		println("Response_status=>", int(td_item.Status_code), "<=")
		// td_item.Resp_body=C.CString(C.GoString(td_item.Resp_body))
	}
	serialize_to:=thread_data_to_json_type{
		Data: td_slice,
		Start: int(start),
		End:int(end),
	}
	_json_bytes,err := json.Marshal(serialize_to)
	if err!=nil{
		return  C.CString("")
	}
	// fmt.Println(string(_json_bytes))
	return C.CString(string(_json_bytes))
}

func check_error(err error) {
	if err != nil {
		panic(err)
	}
}

func parseHttpResponse(header string, _body string, req *http.Request) (*http.Response, error) {
	skip_string := "Transfer-Encoding: chunked\r\n"
	pos := strings.Index(header, skip_string)
	if pos >= 0 {
		pos_end := pos + len(skip_string)
		header = header[0:pos] + header[pos_end:]
	}
	r := header + _body
	body := bytes.NewBuffer([]byte(r))
	prefix := make([]byte, 7)
	n, err := io.ReadFull(body, prefix)
	if err != nil {
		panic("handler err")
	}
	// fmt.Println(n, err, string(prefix))
	if string(prefix[:n]) == "HTTP/2 " {
		// fix HTTP/2 proto
		return http.ReadResponse(bufio.NewReader(io.MultiReader(bytes.NewBufferString("HTTP/2.0 "), body)), req)
	} else {
		// other proto
		// return http.ReadResponse(bufio.NewReader(bytes.NewBuffer([]byte(r))), req)
		return http.ReadResponse(bufio.NewReader(io.MultiReader(bytes.NewBuffer(prefix[:n]), body)), req)
	}
}

func Call_api() {
	total_requests := 2
	// url := "http://localhost:8000/api/hello/1?query=text"
	url := "http://localhost:8000/api/user/"
	// url := "http://guruinfo.epizy.com/edu.php"
	// url:="https://jsonplaceholder.typicode.com/posts"
	// url:="https://google.com/"
	req, err := http.NewRequest("GET", url, bytes.NewBuffer([]byte("some string")))
	req.Header.Add("some-header", "its value")
	req.Header.Add("some-header2", "its value2")
	check_error(err)
	c_url := C.CString(url)
	defer C.free(unsafe.Pointer(c_url))

	request_input := make([]C.struct_SingleRequestInput, total_requests)

	c_headers := C.malloc(C.size_t(len(req.Header)+4) * C.sizeof_struct_Headers)
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

	for i = 0; i < total_requests; i++ {
		request_input[i] = C.struct_SingleRequestInput{
			url: C.CString("http://localhost:8000/api/test/" + strconv.Itoa(i)),
			// url:         C.CString(req.URL.String()),
			method:      C.CString(req.Method),
			headers:     (*C.struct_Headers)(c_headers),
			headers_len: C.int(len(req.Header)),
			body:        C.CString(string(body)),
			cookies:     C.CString(""),
		}
	}

	bulk_response_data := make([]C.struct_ResponseData, total_requests)
	ram_size_in_GB := float64(C.sysconf(C._SC_PHYS_PAGES)*C.sysconf(C._SC_PAGE_SIZE)) / (1024 * 1024)
	nor_of_thread := math.Ceil(ram_size_in_GB / 70)
	fmt.Println("go Nor of threads", nor_of_thread)

	C.send_request_in_concurrently(&(request_input[0]), &(bulk_response_data[0]), C.int(total_requests), C.int(2), 0)

	// for i = 0; i < total_requests; i++ {
	// 	// fmt.Println(i,C.GoString(bulk_response_data[i].response_body))
	// 	fmt.Println("status=", int(bulk_response_data[i].Status_code))
	// }
	// for i = 0; i < total_requests; i++ {
	// 	// fmt.Println(i,C.GoString(bulk_response_data[i].response_body))
	// 	fmt.Println(int(bulk_response_data[i].status_code),C.GoString(bulk_response_data[i].response_body))
	// }

	// fmt.Println(int(response_data.status_code))

	// C.send_raw_request(&(request_input[0]), &response_data, 0)
	// fmt.Println(int(response_data.status_code))
	// resp, err := parseHttpResponse(C.GoString(response_data.response_header), C.GoString(response_data.response_body), nil)
	// body2, err := ioutil.ReadAll(resp.Body)
	// fmt.Println(string(body2), err)
}
