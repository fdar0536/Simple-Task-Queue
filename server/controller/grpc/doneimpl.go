package grpc

import (
	"STQ/model"
	pb "STQ/protos"
	"container/list"
	"context"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

type doneImpl struct {
	pb.UnimplementedDoneServer
}

func (d *doneImpl) List(req *pb.QueueReq, res pb.Done_ListServer) error {
	var tq, err = global.QueueList.GetQueue(req.GetName())
	if err != nil {
		return status.Error(codes.InvalidArgument, err.Error())
	}

	var resList *list.List
	resList, err = tq.ListFinished()
	if err != nil {
		return status.Error(codes.InvalidArgument, err.Error())
	}

	var msg = pb.ListTaskRes{}
	for i := resList.Front(); i != nil; i = i.Next() {
		msg.ID = i.Value.(uint32)
		err = res.Send(&msg)
		if err != nil {
			return status.Error(codes.Internal, err.Error())
		}
	}

	return nil
}
func (d *doneImpl) Details(_ context.Context, req *pb.TaskDetailsReq) (*pb.TaskDetailsRes, error) {
	var tq, err = global.QueueList.GetQueue(req.GetQueueName())
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	var task = model.Task{}
	err = tq.FinishedDetails(req.GetID(), &task)
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	var res = pb.TaskDetailsRes{}
	err = buildTaskDetailsRes(&task, &res)
	if err != nil {
		return nil, status.Error(codes.Internal, err.Error())
	}

	return &res, nil
}
func (d *doneImpl) Clear(_ context.Context, req *pb.QueueReq) (*pb.Empty, error) {
	var tq, err = global.QueueList.GetQueue(req.GetName())
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	tq.ClearFinished()
	return &pb.Empty{}, nil
}
