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
