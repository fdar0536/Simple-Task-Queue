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
	"errors"
	"fmt"
	"runtime"
)

type TaskPriority int

const (
	High TaskPriority = iota
	AbroveNormal
	Normal
	BelowNormal
	Low
)

type Task struct {
	ID        uint32
	ExecName  string
	Args      []string
	WorkDir   string
	Priority  TaskPriority
	ExitCode  int
	IsSuccess bool
}

func TaskInit(input *Task) error {
	if input == nil {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d input is nil\n", file, line)
		return errors.New("input is nil")
	}

	input.ID = 0
	input.ExecName = ""
	input.Args = make([]string, 0)
	input.WorkDir = ""
	input.Priority = Normal
	input.ExitCode = 0
	input.IsSuccess = false

	return nil
}

func (t *Task) Print() {
	fmt.Printf("ID: %d\n", t.ID)
	fmt.Printf("ExecName: %s\n", t.ExecName)
	var out string = ""
	for _, arg := range t.Args {
		out += arg
		out += " "
	}

	fmt.Printf("Args: %s\n", out)
	fmt.Printf("WorkDir: %s\n", t.WorkDir)
	fmt.Printf("Priority: %d\n", t.Priority)
	fmt.Printf("ExitCode: %d\n", t.ExitCode)
	fmt.Printf("IsSuccess: %t\n", t.IsSuccess)
}

func (t *Task) DeepCopy(other *Task) error {
	if other == nil {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d other is mil\n", file, line)
		return errors.New("other is mil")
	}

	other.ID = t.ID
	other.ExecName = t.ExecName
	other.Args = make([]string, len(t.Args))
	copy(other.Args, t.Args)
	other.WorkDir = t.WorkDir
	other.Priority = t.Priority
	other.ExitCode = t.ExitCode
	other.IsSuccess = t.IsSuccess

	return nil
}
