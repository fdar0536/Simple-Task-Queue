package grpc

import (
	pb "STQ/protos"
	"context"
)

type accessImpl struct {
	pb.UnimplementedAccessServer
}

func (a *accessImpl) Echo(_ context.Context, _ *pb.Empty) (*pb.EchoRes, error) {
	return &pb.EchoRes{}, nil
}

func (a *accessImpl) Stop(_ context.Context, _ *pb.Empty) (*pb.EchoRes, error) {
	global.Logger.Info().Msg("Received stop signal from grpc. stopping...")
	global.StopChan <- true
	return &pb.EchoRes{}, nil
}
