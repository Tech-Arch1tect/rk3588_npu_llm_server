g++ -c -fPIC rkllm_wrapper.cpp -o rkllm_wrapper.o
g++ -shared -o librkllm_wrapper.so rkllm_wrapper.o  -lrkllmrt -lstdc++ -lpthread

cp rkllm_wrapper.h /usr/include/
cp librkllm_wrapper.so /usr/lib/
