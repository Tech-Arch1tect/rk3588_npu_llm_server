#include "rkllm_wrapper.h"
#include "rkllm.h"
#include <cstring>
#include <string>
#include <mutex>
#include <condition_variable>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <chrono>
#include <thread>

static LLMHandle llmHandle = nullptr;
static std::mutex mtx;
static std::condition_variable cv;
static bool generation_finished = false;

struct InferenceData {
    std::string output;
    std::string fifo_path;
    int fifo_fd;
};

void writeToPersistentFifo(int fd, const char* text) {
    if (fd < 0) return;
    std::string out(text);
    out.append("\n");
    ssize_t written = write(fd, out.c_str(), out.size());
    if (written != static_cast<ssize_t>(out.size())) {
        perror("Failed to write to persistent FIFO");
    }
}

void simpleCallback(RKLLMResult* result, void* userdata, LLMCallState state) {
    InferenceData* data = static_cast<InferenceData*>(userdata);

    if (state == RKLLM_RUN_FINISH) {
        writeToPersistentFifo(data->fifo_fd, "[[EOS]]");
        {
            std::lock_guard<std::mutex> lock(mtx);
            generation_finished = true;
        }
        cv.notify_one();
    } else if (state == RKLLM_RUN_ERROR) {
        printf("\nLLM run error\n");
        {
            std::lock_guard<std::mutex> lock(mtx);
            generation_finished = true;
        }
        cv.notify_one();
    } else {
        {
            std::lock_guard<std::mutex> lock(mtx);
            data->output += result->text;
        }
        writeToPersistentFifo(data->fifo_fd, result->text);
    }
}

int rkllm_init_simple(const char* model_path, int max_new_tokens, int max_context_len) {
    RKLLMParam param = rkllm_createDefaultParam();
    param.model_path = model_path;
    param.is_async = true;
    param.max_new_tokens = max_new_tokens;
    param.max_context_len = max_context_len;
    
    int ret = rkllm_init(&llmHandle, &param, simpleCallback);
    if (ret != 0) {
        printf("rkllm_init failed with error: %d\n", ret);
    }
    return ret;
}

int rkllm_run_simple(const char* prompt, char* output, int output_size) {
    if (!llmHandle) return -1;

    std::string promptStr(prompt);
    std::string* resultOutput = new std::string();
    {
        std::unique_lock<std::mutex> lock(mtx);
        generation_finished = false;
    }

    RKLLMInput llmInput;
    llmInput.input_type = RKLLM_INPUT_PROMPT;
    llmInput.prompt_input = promptStr.c_str();

    RKLLMInferParam inferParams;
    inferParams.mode = RKLLM_INFER_GENERATE;
    inferParams.lora_params = nullptr;
    inferParams.prompt_cache_params = nullptr;
    inferParams.keep_history = 0;

    int ret = rkllm_run(llmHandle, &llmInput, &inferParams, static_cast<void*>(resultOutput));
    if (ret != 0) {
        delete resultOutput;
        return ret;
    }

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return generation_finished; });
    }

    if (resultOutput->size() >= static_cast<size_t>(output_size)) {
        delete resultOutput;
        return -2;
    }
    std::strcpy(output, resultOutput->c_str());
    delete resultOutput;
    return 0;
}

int rkllm_run_simple_with_fifo(const char* prompt, const char* fifo, char* output, int output_size) {
    if (!llmHandle)
        return -1;

    std::string promptStr(prompt);
    InferenceData* data = new InferenceData();
    data->fifo_path = fifo;

    int persistent_fd = open(fifo, O_WRONLY);
    if (persistent_fd == -1) {
        perror("Failed to open FIFO persistently");
        delete data;
        return -1;
    }
    data->fifo_fd = persistent_fd;

    {
        std::unique_lock<std::mutex> lock(mtx);
        generation_finished = false;
    }

    RKLLMInput llmInput;
    llmInput.input_type = RKLLM_INPUT_PROMPT;
    llmInput.prompt_input = promptStr.c_str();

    RKLLMInferParam inferParams;
    inferParams.mode = RKLLM_INFER_GENERATE;
    inferParams.lora_params = nullptr;
    inferParams.prompt_cache_params = nullptr;
    inferParams.keep_history = 0;

    int ret = rkllm_run(llmHandle, &llmInput, &inferParams, static_cast<void*>(data));
    if (ret != 0) {
        close(data->fifo_fd);
        delete data;
        return ret;
    }

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return generation_finished; });
    }

    if (data->output.size() >= static_cast<size_t>(output_size)) {
        close(data->fifo_fd);
        delete data;
        return -2;
    }
    std::strcpy(output, data->output.c_str());

    close(data->fifo_fd);
    delete data;
    return 0;
}

void rkllm_destroy_simple() {
    if (llmHandle) {
        rkllm_destroy(llmHandle);
        llmHandle = nullptr;
    }
}
