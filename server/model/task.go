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
	fmt.Printf("Priority: %d", t.Priority)
	fmt.Printf("ExitCode: %d", t.ExitCode)
	fmt.Printf("IsSuccess: %t", t.IsSuccess)
}

func (t *Task) DeepCopy(other *Task) error {
	if other == nil {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d other is mil\n", file, line)
		return errors.New("other is mil")
	}

	other.ID = t.ID
	other.ExecName = t.ExecName
	other.Args = t.Args
	other.WorkDir = t.WorkDir
	other.Priority = t.Priority
	other.ExitCode = t.ExitCode
	other.IsSuccess = t.IsSuccess

	return nil
}
