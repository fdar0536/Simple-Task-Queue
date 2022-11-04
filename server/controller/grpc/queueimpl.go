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
