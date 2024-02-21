package main

import (
	"c_in_go/c_modules"
	"time"
)

// https://codeahoy.com/learn/libuv/ch5/

// since pointer from fork not working
// only way left is json serialization
// https://github.com/bk192077/struct_mapping/blob/master/example/struct_to_json/struct_to_json.cpp

func main() {
	c_modules.Call_api()
	time.Sleep(time.Second*100)
}
