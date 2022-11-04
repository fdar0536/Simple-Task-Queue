package cli

import (
	"STQ/model"
	"fmt"
	"os"
	"runtime"
)

func getCommand(prefix string) (string, int) {

	fmt.Printf("%s: ", prefix)
	var command, err = model.CLIReader.ReadString('\n')
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d %s\n", file, line, err.Error())
		return "", 1
	}

	if runtime.GOOS == "windows" {
		command = command[:len(command)-2] // crlf
	} else {
		command = command[:len(command)-1] // lf
	}

	return command, 0
}
