package utils

import (
	"encoding/json"
	"fmt"
	"os"
	"runtime"
)

// ConfigParam stores data from config file(json)
type ConfigParam struct {
	FileSavePath      string `json:"file save path"`
	OutputFilePath    string `json:"output file path"`
	LogPath           string `json:"log path"`
	IP                string `json:"ip"`
	Port              uint16 `json:"port"`
	RestfulServer     bool   `json:"restful server"`
	RestfulHostName   string `json:"restful host name"`
	ResrfulServerPort uint16 `json:"restful server port"`
	WebContextPath    string `json:"web context path"`
	Debug             bool   `json:"debug"`
	CLI               bool   `json:"cli"`
}

func verifyPath(path *string) int {
	var testString string
	if runtime.GOOS == "windows" {
		testString = *path + "\\test_file.tmp"
	} else {
		testString = *path + "/test_file.tmp"
	}

	var testFile, err = os.OpenFile(testString,
		os.O_APPEND|os.O_CREATE|os.O_RDWR,
		0644)
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d %s", file, line, err.Error())
		return 1
	}

	testFile.Close()
	return 0
}

func ParseConfigFile(path *string, config *ConfigParam) int {
	if path == nil || config == nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d invalid input\n", file, line)
		return 1
	}

	var file, err = os.Open(*path)
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d %s\n", file, line, err.Error())
		return 1
	}
	defer file.Close()

	var decoder = json.NewDecoder(file)
	err = decoder.Decode(config)
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d %s\n", file, line, err.Error())
		return 1
	}

	var exitCode = verifyPath(&config.FileSavePath)
	if exitCode != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d verifyPath failed\n", file, line)
		return 1
	}

	exitCode = verifyPath(&config.OutputFilePath)
	if exitCode != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d verifyPath failed\n", file, line)
		return 1
	}

	exitCode = verifyPath(&config.LogPath)
	if exitCode != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d verifyPath failed\n", file, line)
		return 1
	}

	exitCode = verifyPath(&config.WebContextPath)
	if exitCode != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d verifyPath failed\n", file, line)
		return 1
	}

	if config.Port == config.ResrfulServerPort {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d verifyPath failed\n", file, line)
		return 1
	}

	return 0
}
