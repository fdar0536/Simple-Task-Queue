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

type pendingImpl struct {
	pb.UnimplementedPendingServer
}

func (p *pendingImpl) List(req *pb.QueueReq, res pb.Pending_ListServer) error {
	var name = req.GetName()
	var tq, err = global.QueueList.GetQueue(name)
	if err != nil {
		return status.Error(codes.InvalidArgument, err.Error())
	}

	var listRes *list.List
	listRes, err = tq.ListPending()
	if err != nil {
		return status.Error(codes.InvalidArgument, err.Error())
	}

	var pendingRes = pb.ListTaskRes{}
	for i := listRes.Front(); i != nil; i = i.Next() {
		pendingRes.ID = i.Value.(uint32)
		err = res.Send(&pendingRes)
		if err != nil {
			status.Error(codes.Internal, err.Error())
		}
	}

	return nil
}

func (p *pendingImpl) Details(_ context.Context, req *pb.TaskDetailsReq) (*pb.TaskDetailsRes, error) {
	var tq, err = global.QueueList.GetQueue(req.GetQueueName())
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	var task = model.Task{}
	err = tq.PendingDetails(req.GetID(), &task)
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

func (p *pendingImpl) Current(_ context.Context, req *pb.QueueReq) (*pb.TaskDetailsRes, error) {
	var tq, err = global.QueueList.GetQueue(req.GetName())
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	var task = model.Task{}
	err = tq.CurrentTask(&task)
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

func (p *pendingImpl) Add(_ context.Context, req *pb.AddTaskReq) (*pb.ListTaskRes, error) {
	var tq, err = global.QueueList.GetQueue(req.GetQueueName())
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	var task = new(model.Task)
	err = model.TaskInit(task)
	if err != nil {
		return nil, status.Error(codes.Internal, err.Error())
	}

	task.Args = make([]string, len(req.GetArgs()))
	copy(task.Args, req.GetArgs())
	task.ExecName = req.GetProgramName()
	task.Priority = model.TaskPriority(req.GetPriority())
	task.WorkDir = req.WorkDir
	var id uint32
	id, err = tq.AddTask(task)
	if err != nil {
		return nil, status.Error(codes.Internal, err.Error())
	}

	return &pb.ListTaskRes{ID: id}, nil
}

func (p *pendingImpl) Remove(_ context.Context, req *pb.TaskDetailsReq) (*pb.Empty, error) {
	var tq, err = global.QueueList.GetQueue(req.GetQueueName())
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	err = tq.RemoveTask(req.GetID())
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	return &pb.Empty{}, nil
}

func (p *pendingImpl) Start(_ context.Context, req *pb.QueueReq) (*pb.Empty, error) {
	var tq, err = global.QueueList.GetQueue(req.GetName())
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	err = tq.Start()
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	return &pb.Empty{}, nil
}

func (p *pendingImpl) Stop(_ context.Context, req *pb.QueueReq) (*pb.Empty, error) {
	var tq, err = global.QueueList.GetQueue(req.GetName())
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	err = tq.Stop()
	if err != nil {
		return nil, status.Error(codes.InvalidArgument, err.Error())
	}

	return &pb.Empty{}, nil
}
