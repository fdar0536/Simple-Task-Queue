package model

import (
	"STQ/utils"
	"fmt"
	"os"
	"runtime"
)

func Init(args *[]string, config *utils.ConfigParam) int {
	if args == nil || config == nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d invalid input\n", file, line)
		return 1
	}

	if len(*args) != 2 {
		fmt.Printf("%s usage:\n", (*args)[0])
		fmt.Printf("%s <path to config file(json)>\n", (*args)[0])
		return 1
	}

	var code = utils.ParseConfigFile(&(*args)[1], config)
	if code != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d ParseConfigFile failed\n", file, line)
		return 1
	}

	code = globalInit(config)
	if code != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d globalInit failed\n", file, line)
		return 1
	}

	return 0
}
