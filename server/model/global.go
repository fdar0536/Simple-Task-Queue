package model

import (
	"STQ/utils"
	"bufio"
	"fmt"
	"os"
	"runtime"

	"github.com/rs/zerolog"
)

type GlobalData struct {
	QueueList      TaskQueueList
	FileSavePath   string
	OutputFilePath string
	Logger         zerolog.Logger

	StopChan chan bool

	logFile *os.File
}

var (
	Global    = GlobalData{}
	CLIReader = bufio.NewReader(os.Stdin)
)

func globalInit(config *utils.ConfigParam) int {
	if config == nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d invalid input\n", file, line)
		return 1
	}

	Global.QueueList = TaskQueueList{}
	var code = taskQueueListInit(&Global.QueueList)
	if code != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d taskQueueListInit failed\n", file, line)
		return 1
	}

	Global.FileSavePath = config.FileSavePath
	Global.OutputFilePath = config.OutputFilePath

	code = zerologInit(config)
	if code != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d zerologInit failed\n", file, line)
		return 1
	}

	// channel
	Global.StopChan = make(chan bool, 2)

	return 0
}

func (g *GlobalData) Fin() {
	g.QueueList.fin()
	g.logFile.Close()
}

func zerologInit(config *utils.ConfigParam) int {
	if config == nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d invalid input\n", file, line)
		return 1
	}

	var logPath string
	if runtime.GOOS == "windows" {
		logPath = config.LogPath + "\\STQServer.log"
	} else {
		logPath = config.LogPath + "/STQServer.log"
	}

	var err error = nil
	Global.logFile, err = os.OpenFile(logPath, os.O_APPEND|os.O_CREATE|os.O_RDWR, 0644)
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d %s\n", file, line, err.Error())
		return 1
	}

	if config.Debug {
		Global.Logger = zerolog.New(Global.logFile).Level(zerolog.DebugLevel)
	} else {
		Global.Logger = zerolog.New(Global.logFile).Level(zerolog.WarnLevel)
	}

	return 0
}
