#include "rkllm_wrapper.h"
#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <model_path>\n";
        return 1;
    }

    const char* modelPath = argv[1];

    int ret = rkllm_init_simple(modelPath, 256, 512);
    if (ret != 0) {
        std::cerr << "rkllm_init_simple failed with error: " << ret << "\n";
        return ret;
    }

    const char* prompt = "Hello, How are you?";
    constexpr int outputBufferSize = 8192;
    char output[outputBufferSize];
    std::memset(output, 0, outputBufferSize);

    ret = rkllm_run_simple(prompt, output, outputBufferSize);
    if (ret != 0) {
        std::cerr << "rkllm_run_simple failed with error: " << ret << "\n";
        rkllm_destroy_simple();
        return ret;
    }

    std::cout << "LLM Output:\n" << output << "\n";

    rkllm_destroy_simple();
    return 0;
}
