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
	"fmt"
	"io"
	"io/ioutil"
	"math"
	"net/http"
	"reflect"
	"runtime"
	"runtime/debug"

	"strings"
	"unsafe"
)

func check_error(err error) {
	if err != nil {
		panic(err)
	}
}

func carray2slice(array *C.struct_ResponseData, len int) []C.struct_ResponseData {
	var list []C.struct_ResponseData
	sliceHeader := (*reflect.SliceHeader)((unsafe.Pointer(&list)))
	sliceHeader.Cap = len
	sliceHeader.Len = len
	sliceHeader.Data = uintptr(unsafe.Pointer(array))
	return list
	// return (*[1 << 30]C.struct_ResponseData)(unsafe.Pointer(array))[:len:len]
}

type ResponseDataCMap struct {
	Debug                         int
	Uid                           string
	Response_header               string
	Response_body                 string
	Before_connect_time_microsec  int64
	After_response_time_microsec  int64
	Connected_at_microsec         int64
	First_byte_at_microsec        int64
	Finish_at_microsec            int64
	Connect_time_microsec         int64
	Time_to_first_byte_microsec   int64
	Total_time_from_curl_microsec int64
	Total_time_microsec           int64
	Status_code                   int
	Err_code                      int
}

type thread_data_to_json_type struct {
	Data  []ResponseDataCMap
	Start int
	End   int
}

//export thread_data_to_json
func thread_data_to_json(td *C.struct_ResponseData, _len C.int, start C.int, end C.int) C.struct_StringType {	
	td_arr := carray2slice(td, int(_len))
	td_slice := []ResponseDataCMap{}

	for _, td_item := range td_arr {
		// println("serialized Response_body",int(td_item.Status_code))
		td_slice = append(td_slice, ResponseDataCMap{
			Debug:                         int(td_item.Debug),
			Uid:                           C.GoString(td_item.Uid),
			Status_code:                   int(td_item.Status_code),
			Response_header:               C.GoString(td_item.Response_header),
			Response_body:                 C.GoString(td_item.Response_body),
			Before_connect_time_microsec:  int64(td_item.Before_connect_time_microsec),
			After_response_time_microsec:  int64(td_item.After_response_time_microsec),
			Connected_at_microsec:         int64(td_item.Connect_time_microsec),
			First_byte_at_microsec:        int64(td_item.First_byte_at_microsec),
			Finish_at_microsec:            int64(td_item.Finish_at_microsec),
			Connect_time_microsec:         int64(td_item.Connect_time_microsec),
			Time_to_first_byte_microsec:   int64(td_item.Time_to_first_byte_microsec),
			Total_time_from_curl_microsec: int64(td_item.Total_time_from_curl_microsec),
			Total_time_microsec:           int64(td_item.Total_time_microsec),
			Err_code:                      int(td_item.Err_code),
		})
	}

	println("thread_data_to_json 1",_len,len(td_arr))
	serialize_to := thread_data_to_json_type{
		Data:  td_slice,
		Start: int(start),
		End:   int(end),
	}
	println("thread_data_to_json",_len)
	// json.Marshal freezes on large data due to GC
	// var struct_in_bytes bytes.Buffer 
	// enc:=gob.NewEncoder(&struct_in_bytes) 
	// err := enc.Encode(serialize_to)
	_json_bytes, err := json.Marshal(serialize_to)
	println("thread_data_to_json2",len(_json_bytes))
	if err != nil {
		return C.struct_StringType{}
	}
	// err2 := os.WriteFile(fmt.Sprintf("./json_bytes_%d.json",int(start)), _json_bytes, 0644)
	// check_error(err2)
	// string(_json_bytes) freezes on some data
	// fmt.Println(string(_json_bytes))
	// println("thread_data_to_json2",len(string(_json_bytes)))
	// return C.CString(string(_json_bytes))
	// return C.CString("")
	// cb:=C.CBytes(struct_in_bytes.Bytes())
	cb:=C.CBytes(_json_bytes)
	// defer C.free(unsafe.Pointer(cb))
	// return  (*C.char)(cb)	
	return  C.struct_StringType{
		ch:(*C.char)(cb),
		length:C.ulong(len(_json_bytes)),
	}
}

//export json_to_thread_data
func json_to_thread_data(json_data *C.char, str_len C.size_t) *C.struct_ResponseDeserialized {
    // defer C.free(unsafe.Pointer(json_data))
	go_str_len := uint64(str_len)
	// println("go_str_len", go_str_len)
	mySlice := (*[1 << 30]byte)(unsafe.Pointer(json_data))[:go_str_len:go_str_len]

	// err2 := os.WriteFile("./json_bytes.json", mySlice, 0644)
	// check_error(err2)

	// json_data_in_bytes := make([]byte,go_str_len )
	// copy(json_data_in_bytes, (*(*[1024]byte)(unsafe.Pointer(json_data)))[:go_str_len:go_str_len])

	returnStruct := (*C.struct_ResponseDeserialized)(C.malloc(C.size_t(unsafe.Sizeof(C.struct_ResponseDeserialized{}))))

	// var struct_in_bytes bytes.Buffer 
	// struct_in_bytes.Write(mySlice)
	// dec := gob.NewDecoder(&struct_in_bytes)

	var response_data thread_data_to_json_type
	// err := dec.Decode(&response_data)

	err := json.Unmarshal(mySlice, &response_data)
	// println("json_to_thread_data",len(response_data.Data))
	if err != nil {
		// println(string(mySlice))
		// fmt.Printf("mystr:\t %02X \n", mySlice)
		// err2 := os.WriteFile("./json_bytes_go.json", mySlice, 0644)
		// check_error(err2)

		check_error(err)
		returnStruct.data = nil
		returnStruct.len = C.int(0)
		returnStruct.start = C.int(0)
		returnStruct.end = C.int(0)
		return returnStruct
	}
	response_data_c_array := C.malloc(C.size_t(len(response_data.Data)) * C.sizeof_struct_ResponseData)
	// defer C.free(unsafe.Pointer(response_data_c_array))
	response_data_c_slice := (*[1<<30 - 1]C.struct_ResponseData)(response_data_c_array)
	// response_data_c_slice := make([]C.struct_ResponseData, len(response_data.Data))
	for i, rd := range response_data.Data {
		// fmt.Println("desrialise Response_body",rd.Status_code)
		response_data_c_slice[i] = C.struct_ResponseData{
			Debug:                         C.int(rd.Debug),
			Uid:                           C.CString(rd.Uid),
			Response_header:               C.CString(rd.Response_header),
			Response_body:                 C.CString(rd.Response_body),
			Before_connect_time_microsec:  C.longlong(rd.Before_connect_time_microsec),
			After_response_time_microsec:  C.longlong(rd.After_response_time_microsec),
			Connected_at_microsec:         C.longlong(rd.Connect_time_microsec),
			First_byte_at_microsec:        C.longlong(rd.First_byte_at_microsec),
			Finish_at_microsec:            C.longlong(rd.Finish_at_microsec),
			Time_to_first_byte_microsec:   C.long(rd.Time_to_first_byte_microsec),
			Connect_time_microsec:         C.long(rd.Connect_time_microsec),
			Total_time_from_curl_microsec: C.long(rd.Total_time_from_curl_microsec),
			Total_time_microsec:           C.long(rd.Total_time_microsec),
			Status_code:                   C.int(rd.Status_code),
			Err_code:                      C.int(rd.Err_code),
		}
	}
	// println("json_to_thread_data2",len(response_data_c_slice))
	returnStruct.data = (*C.struct_ResponseData)(response_data_c_array)
	returnStruct.len = C.int(len(response_data.Data))
	returnStruct.start = C.int(response_data.Start)
	returnStruct.end = C.int(response_data.End)
	return returnStruct
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
	// debug.SetGCPercent(-1)
	total_requests := 10000
	// url := "http://localhost:8000/api/hello/1?query=text"
	url := "http://127.0.0.1:8000/api/user/"
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
			// url: C.CString("http://localhost:8000/api/test/" + strconv.Itoa(i)),

			url: C.CString("http://127.0.0.1:8040/"),
			// url:         C.CString(req.URL.String()),
			method:      C.CString(req.Method),
			headers:     (*C.struct_Headers)(c_headers),
			headers_len: C.int(len(req.Header)),
			body:        C.CString(string(body)),
			cookies:     C.CString(""),
		}
	}



	ram_size_in_GB := float64(C.sysconf(C._SC_PHYS_PAGES)*C.sysconf(C._SC_PAGE_SIZE)) / (1024 * 1024)
	nor_of_thread := math.Ceil(ram_size_in_GB / 70)
	fmt.Println("go Nor of threads", nor_of_thread)


	// c_bulk_response_data := C.malloc(C.size_t(total_requests) * C.sizeof_struct_ResponseData)
	// defer C.free(unsafe.Pointer(c_bulk_response_data))
	// bulk_response_data := (*[1<<30 - 1]C.struct_ResponseData)(c_bulk_response_data)
	// debug.SetGCPercent(-1)
	// C.send_request_in_concurrently(&(request_input[0]),  (*C.struct_ResponseData)(c_bulk_response_data), C.int(total_requests), C.int(runtime.NumCPU()), 0)
	// debug.SetGCPercent(100)


	bulk_response_data := make([]C.struct_ResponseData, total_requests)
	// runtime.KeepAlive(request_input)
	// runtime.KeepAlive(bulk_response_data)
	debug.SetGCPercent(-1)
	C.send_request_in_concurrently(&(request_input[0]), &(bulk_response_data[0]), C.int(total_requests), C.int(runtime.NumCPU()), 0)
	debug.SetGCPercent(100)
	var status_codes=make(map[int]int)
	for i = 0; i < total_requests; i++ {
		// fmt.Println("Response_body=",C.GoString(bulk_response_data[i].Response_body))
		// fmt.Print("go status=", int(bulk_response_data[i].Status_code),",")
		_,ok:=status_codes[int(bulk_response_data[i].Status_code)]
		if ok{
			status_codes[int(bulk_response_data[i].Status_code)]++
		} else{
			status_codes[int(bulk_response_data[i].Status_code)]=1
		}
	}
	for k, v := range status_codes {
		fmt.Printf("status_code=%d,count=%d\n",k,v)
	}
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
	println("\n------------ go end --------------")
}
