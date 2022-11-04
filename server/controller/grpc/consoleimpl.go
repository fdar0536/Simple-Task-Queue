package grpc

import (
	"STQ/model"
	pb "STQ/protos"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

type consoleImpl struct {
	pb.UnimplementedConsoleServer
}

func (c *consoleImpl) Output(req pb.Console_OutputServer) error {
	var flag bool = false
	var msg *pb.Msg
	var err error
	var tq *model.TaskQueue
	var out string
	for { // loop 1
		msg, err = req.Recv()
		if err != nil {
			status.Error(codes.Internal, err.Error())
		}

		if flag {
			if msg.GetMsg() != "c" {
				break
			}
		} else {
			flag = true
			tq, err = global.QueueList.GetQueue(msg.GetMsg())
			if err != nil {
				return status.Error(codes.InvalidArgument, err.Error())
			}
		}

		out, err = tq.ReadCurrentOutput()
		if err != nil {
			return status.Error(codes.InvalidArgument, err.Error())
		}

		msg.Msg = out
		err = req.Send(msg)
		if err != nil {
			return status.Error(codes.Internal, err.Error())
		}
	} // end loop 1

	return nil
}
