package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strings"
	"syscall"
	"time"

	"github.com/Tech-Arch1tect/rkllm-go/bindings"
)

func main() {
	fifoPath := "/tmp/llm_output_123.fifo"

	if _, err := os.Stat(fifoPath); os.IsNotExist(err) {
		if err := syscall.Mkfifo(fifoPath, 0666); err != nil {
			log.Fatalf("Failed to create FIFO: %v", err)
		}
	}

	file, err := os.OpenFile(fifoPath, os.O_RDWR, os.ModeNamedPipe)
	if err != nil {
		log.Fatalf("Failed to open FIFO: %v", err)
	}
	defer file.Close()

	var fifoClosed bool

	go func() {
		reader := bufio.NewReader(file)
		for {
			line, err := reader.ReadString('\n')
			if err != nil {
				fifoClosed = true
				break
			}
			trimmed := strings.TrimSpace(line)
			if trimmed == "[[EOS]]" {
				log.Println("Received EOS marker, ending stream")
				fifoClosed = true
				break
			}
			fmt.Printf("Received chunk: %s", line)
		}
	}()

	modelPath := os.Getenv("RKLLM_MODEL_PATH")
	if modelPath == "" {
		log.Fatalf("RKLLM_MODEL_PATH environment variable is not set")
	}

	if err := bindings.Init(modelPath, 4096, 4096); err != nil {
		log.Fatalf("Failed to initialise RKLLM: %v", err)
	}
	defer bindings.Destroy()

	prompt := "Hello, How are you?"

	output, err := bindings.RunInferenceWithFifo(prompt, fifoPath)
	if err != nil {
		log.Fatalf("Inference error: %v", err)
	}

	fmt.Println("LLM Final Output:")
	fmt.Println(output)

	for !fifoClosed {
		time.Sleep(1 * time.Second)
	}
}
