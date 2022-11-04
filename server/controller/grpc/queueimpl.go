package grpc

import (
	pb "STQ/protos"
	"context"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

type queueImpl struct {
	pb.UnimplementedQueueServer
}

func (q *queueImpl) CreateQueue(_ context.Context, req *pb.QueueReq) (*pb.Empty, error) {
	var err = global.QueueList.CreateQueue(req.GetName())
	if err != nil {
		return nil, status.Error(codes.AlreadyExists, err.Error())
	}

	return &pb.Empty{}, nil
}

func (q *queueImpl) RenameQueue(_ context.Context, req *pb.RenameQueueReq) (*pb.Empty, error) {
	var err = global.QueueList.RenameQueue(req.GetOldname(), req.GetNewName())
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	return &pb.Empty{}, nil
}
func (q *queueImpl) DeleteQueue(_ context.Context, req *pb.QueueReq) (*pb.Empty, error) {
	var err = global.QueueList.DeleteQueue(req.GetName())
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	return &pb.Empty{}, nil
}
func (q *queueImpl) ListQueue(req *pb.Empty, res pb.Queue_ListQueueServer) error {
	var ret, err = global.QueueList.ListQueue()
	if err != nil {
		return status.Error(codes.InvalidArgument, err.Error())
	}

	var listRes = pb.ListQueueRes{}
	for _, name := range ret {
		listRes.Name = name
		err = res.Send(&listRes)
		if err != nil {
			return status.Error(codes.Internal, err.Error())
		}
	}

	return nil
}
