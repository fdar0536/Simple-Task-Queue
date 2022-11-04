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
