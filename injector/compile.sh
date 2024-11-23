#!/bin/bash

COMPILE_INJ="g++ -fPIC -shared -g -std=c++23 -o injector.so Injector.cpp -lfmt -I/home/yvesw/miniconda3/envs/ggml/include/"
COMPILE_INJ_DBG="g++ -fPIC -shared -O0 -g -std=c++20 -o injector.so Injector.cpp -lfmt -I/home/yvesw/miniconda3/envs/ggml/include/"
# COMPILE_INJ="g++ -fPIC -shared -g -std=c++23 -o injector.so Injector.cpp AddrToLine.cpp -lfmt -L ./libbfd/ -lbfd_fpic -I/home/yvesw/miniconda3/envs/ggml/include/"
# COMPILE_INJ_DBG="g++ -fPIC -shared -O0 -g -std=c++20 -o injector.so Injector.cpp AddrToLine.cpp -lfmt -lbfd -I/home/yvesw/miniconda3/envs/ggml/include/"

COMPILE_TST="g++ uftrace-func-trace.cpp -g -finstrument-functions"

RUN="./a.out"

ARG=$1

case "$ARG" in
  "dbg")
    ${COMPILE_INJ_DBG} && ${COMPILE_TST} && \
    LD_PRELOAD=$(pwd)/injector.so gdb -tui ${RUN}
    ;;
  "py")
    c++ -O3 -Wall -shared -std=c++11 -fPIC $(python3 -m pybind11 --includes) pyinjector.cpp -o pyinjector$(python3-config --extension-suffix)
    ;;
  *)
    ${COMPILE_INJ} && ${COMPILE_TST} && \
    LD_PRELOAD=$(pwd)/injector.so ${RUN} && \
    echo "done."
    ;;
esac

