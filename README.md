# A simplified C++ api for RKLLM + CGO bindings

Tested with RKLLM v1.2.0

This is a simple POC to run NPU accelerated LLM's on RK3588 using RKLLM through Go (using CGO).

Initially I was testing with c-for-go, generating CGO bindings for rkllm.h however I couldn't get this working (due to rkllm's use of unions). To 'solve' this issue I have put together a very small rkllm wrapper (in ./rkwrapper) and written some basic CGO bindings for the wrapper (./bindings).

- `example/main.go` has an example of how the simplified wrapper bindings can be used.
- `bindings/` contains the CGO bindings
- `rkwrapper` containers the simple rkllm wrapper
- `rkwrapper/test_wrapper.cpp` <-- this isn't really useful anymore but was useful to test the simplified api until I got it working in CGO.

### original project / inspiration: https://github.com/av1d/rk3588_npu_llm_server

A special note, on the old branch (main) I updated av1d's server.cpp program to work with rkllm 1.2.0 in case anyone is looking for that.
