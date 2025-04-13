package main

import (
	"fmt"
	"log"
	"os"

	"github.com/Tech-Arch1tect/rkllm-go/bindings"
)

func main() {
	modelPath := os.Getenv("RKLLM_MODEL_PATH")
	if modelPath == "" {
		log.Fatalf("RKLLM_MODEL_PATH is not set")
	}

	if err := bindings.Init(modelPath, 4096, 4096); err != nil {
		log.Fatalf("Failed to initialise RKLLM: %v", err)
	}
	defer bindings.Destroy()

	prompt := "Hello, How are you?"

	output, err := bindings.RunInference(prompt)
	if err != nil {
		log.Fatalf("Inference error: %v", err)
	}
	fmt.Println("LLM Output:")
	fmt.Println(output)
}
