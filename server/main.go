package main

import (
	"STQ/controller"
	"STQ/model"
	"STQ/utils"
	"STQ/view/cli"
	"fmt"
	"os"
	"os/signal"
	"runtime"
	"syscall"
	"time"
)

func main() {
	if utils.UserIsAdmin() {
		if runtime.GOOS == "windows" {
			fmt.Fprintln(os.Stderr, "Refuse to run as administrator!")
		} else {
			fmt.Fprintln(os.Stderr, "Refuse to run as super user!")
		}

		os.Exit(1)
	}

	var config = utils.ConfigParam{}
	var ret = model.Init(&os.Args, &config)
	if ret != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d model.Init failed\n", file, line)
		os.Exit(ret)
	}

	defer model.Global.Fin()

	if config.Debug && config.CLI {
		var cliMain = cli.CLIQueueList{}
		ret = cli.CLIQueueListInit(&cliMain)
		if ret != 0 {
			var _, file, line, _ = runtime.Caller(0)
			fmt.Fprintf(os.Stderr, "%s:%d cli.CLIQueueListInit failed\n", file, line)
			os.Exit(ret)
		}

		ret = cliMain.Run()
		if ret != 0 {
			var _, file, line, _ = runtime.Caller(0)
			fmt.Fprintf(os.Stderr, "%s:%d cliMain.Run failed\n", file, line)
			os.Exit(ret)
		}

		os.Exit(0)
	}

	var ctrler = controller.Controller{}
	ret = controller.Init(&config, &ctrler)
	if ret != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d controller.Init failed\n", file, line)
		os.Exit(ret)
	}

	go func() {
		var err = ctrler.Grpc.Server.Serve(ctrler.Grpc.Listener)
		if err != nil {
			model.Global.Logger.Error().Msgf("Grpc server failed: %s", err.Error())
			model.Global.StopChan <- true
		}
	}()

	fmt.Printf("gRPC server is listen on %s:%d\n", config.IP, config.Port)
	var cancelChan = make(chan os.Signal, 2)
	signal.Notify(cancelChan, os.Interrupt, syscall.SIGTERM, syscall.SIGINT)

	for {
		select {
		case <-model.Global.StopChan:
			goto exit
		case sig := <-cancelChan:
			if sig == syscall.SIGTERM || sig == os.Interrupt || sig == syscall.SIGINT {
				goto exit
			}
		case <-time.After(time.Millisecond * 500):
			continue
		default:
			continue
		}
	}

exit:
	ctrler.Grpc.Server.GracefulStop()
}
