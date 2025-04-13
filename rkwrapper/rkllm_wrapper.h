#ifndef RKLLM_WRAPPER_H
#define RKLLM_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

int rkllm_init_simple(const char* model_path, int max_new_tokens, int max_context_len);

int rkllm_run_simple_with_fifo(const char* prompt, const char* fifo, char* output, int output_size);

int rkllm_run_simple(const char* prompt, char* output, int output_size);

void rkllm_destroy_simple();

#ifdef __cplusplus
}
#endif

#endif
