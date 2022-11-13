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