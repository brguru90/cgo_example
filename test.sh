GODEBUG=cgocheck=11,gctrace=1
UV_THREADPOOL_SIZE=100
GOTRACEBACK=crash
go run main.go  -gcflags=all="-N -l" -race -msan -fsanitize=memory