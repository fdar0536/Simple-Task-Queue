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
	"STQ/utils"
	"fmt"
	"net"
	"os"
	"runtime"

	gRPC "google.golang.org/grpc"
)

type GPRCController struct {
	Server   *gRPC.Server
	Listener net.Listener
}

var global = &model.Global

func Init(config *utils.ConfigParam, controller *GPRCController) int {
	if config == nil || controller == nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d invalid input\n", file, line)
		return 1
	}

	var err error
	controller.Listener, err = net.Listen("tcp", fmt.Sprintf("%s:%d", config.IP, config.Port))
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d %s\n", file, line, err.Error())
		return 1
	}

	controller.Server = gRPC.NewServer()
	pb.RegisterAccessServer(controller.Server, &accessImpl{})
	pb.RegisterQueueServer(controller.Server, &queueImpl{})
	pb.RegisterPendingServer(controller.Server, &pendingImpl{})
	pb.RegisterConsoleServer(controller.Server, &consoleImpl{})
	pb.RegisterDoneServer(controller.Server, &doneImpl{})

	return 0
}
