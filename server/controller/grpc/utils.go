package grpc

import (
	"STQ/model"
	pb "STQ/protos"
	"errors"
	"runtime"
)

func buildTaskDetailsRes(in *model.Task, out *pb.TaskDetailsRes) error {
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
