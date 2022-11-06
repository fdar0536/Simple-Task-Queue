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

package types

import (
	"STQ/model"
	"errors"
	"runtime"
)

var global = &model.Global

type ErrMsg struct {
	Msg string `json:"msg"`
}

type QueueReq struct {
	Name string `json:"name"`
}

type RenameQueueReq struct {
	Old string `json:"oldName"`
	New string `json:"newName"`
}

type TaskDetailsReq struct {
	QueueName string `json:"queueName"`
	ID        uint32 `json:"id"`
}

type TaskDetailsRes struct {
	WorkDir     string   `json:"workDir"`
	ProgramName string   `json:"programName"`
	Args        []string `json:"args"`
	ExitCode    uint32   `json:"exitCode"`
	Priority    uint32   `json:"priority"`
}

type AddTaskReq struct {
	QueueName   string   `json:"queueName"`
	WorkDir     string   `json:"workDir"`
	ProgramName string   `json:"programName"`
	Args        []string `json:"args"`
	Priority    uint32   `json:"priority"`
}

type ListQueueRes struct {
	Name []string `json:"name"`
}

type ListTaskRes struct {
	ID []uint32 `json:"id"`
}

func BuildTaskDetailsRes(in *model.Task, out *TaskDetailsRes) error {
	if in == nil || out == nil {
		var _, file, line, _ = runtime.Caller(0)
		global.Logger.Error().Msgf("%s:%d invalid input", file, line)
		return errors.New("invalid input")
	}

	out.Args = in.Args
	out.ExitCode = uint32(in.ExitCode)
	out.Priority = uint32(in.Priority)
	out.ProgramName = in.ExecName
	out.WorkDir = in.WorkDir
	return nil
}
