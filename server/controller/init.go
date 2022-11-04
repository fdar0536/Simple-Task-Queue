package controller

import (
	"STQ/controller/grpc"
	"STQ/utils"
	"fmt"
	"os"
	"runtime"
)

type Controller struct {
	Grpc grpc.GPRCController
}

func Init(config *utils.ConfigParam, c *Controller) int {
	if config == nil || c == nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d invalid input\n", file, line)
		return 1
	}

	var code = grpc.Init(config, &c.Grpc)
	if code != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d grpc.Init failed\n", file, line)
		return 1
	}

	return 0
}
