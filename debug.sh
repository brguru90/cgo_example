
GODEBUG=cgocheck=11,gctrace=1
UV_THREADPOOL_SIZE=100
GOTRACEBACK=crash


PID_LIST=""

function beforeExit() {
    echo;
    echo "statrting Profiling...";
    trap - SIGINT
    kill -s SIGINT $PID_LIST    
    sleep 2
    # analyse generated profiling data
    go tool pprof --http=localhost:8800 ./debug.bin ./mem.pprof
    echo "Benchmark Exited";
}


rm -rf ./debug.bin ./mem.pprof ./cpu.pprof
go build -v -gcflags=all="-N -l" -race -o  ./debug.bin
./debug.bin &

PID_LIST+=" $!";

echo "execution started, press ctrl+c to end execution & view profiling data"

echo "PIDs=$PID_LIST"
trap beforeExit SIGINT
wait $PID_LIST

echo "Exited";
