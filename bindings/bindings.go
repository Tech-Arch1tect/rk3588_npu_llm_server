package bindings

/*
#cgo LDFLAGS: -L/usr/lib/ -lrkllm_wrapper -lrkllmrt -lstdc++
#cgo CXXFLAGS: -std=c++11
#include "rkllm_wrapper.h"
#include <stdlib.h>
*/
import "C"
import (
	"errors"
	"unsafe"
)

func Init(modelPath string, maxNewTokens, maxContextLen int) error {
	cModelPath := C.CString(modelPath)
	defer C.free(unsafe.Pointer(cModelPath))
	ret := C.rkllm_init_simple(cModelPath, C.int(maxNewTokens), C.int(maxContextLen))
	if ret != 0 {
		return errors.New("failed to initialise RKLLM")
	}
	return nil
}

func RunInference(prompt string) (string, error) {
	cPrompt := C.CString(prompt)
	defer C.free(unsafe.Pointer(cPrompt))

	const bufSize = 8192
	outBuffer := C.malloc(C.size_t(bufSize))
	defer C.free(outBuffer)

	ret := C.rkllm_run_simple(cPrompt, (*C.char)(outBuffer), C.int(bufSize))
	if ret != 0 {
		return "", errors.New("LLM inference error")
	}

	result := C.GoString((*C.char)(outBuffer))
	return result, nil
}

func Destroy() {
	C.rkllm_destroy_simple()
}
