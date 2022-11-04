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
