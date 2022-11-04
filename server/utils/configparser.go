/*
 * Simple Task Queue
 * Copyright (c) 2022 fdar0536
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
