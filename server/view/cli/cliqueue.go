package cli

import (
	"STQ/model"
	"fmt"
	"os"
	"runtime"
	"strings"
	"time"
)

type cliQueue struct {
	queue   *model.TaskQueue
	cmdList map[string]func()
}

func cliQueueInit(in *cliQueue) int {
	if in == nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d invalid input\n", file, line)
		return 1
	}

	// cmd list
	in.cmdList = make(map[string]func())

	in.cmdList["help"] = in.help
	in.cmdList["add"] = in.add
	in.cmdList["delete"] = in.delete
	in.cmdList["listPanding"] = in.listPending
	in.cmdList["listFinished"] = in.listFinished
	in.cmdList["clearPanding"] = in.clearPanding
	in.cmdList["clearFinished"] = in.clearFinished
	in.cmdList["console"] = in.console
	in.cmdList["start"] = in.start
	in.cmdList["stop"] = in.stop

	return 0
}

func (c *cliQueue) run(queue *model.TaskQueue, name string) int {
	if queue == nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d queue is nil\n", file, line)
		return 1
	}

	c.queue = queue
	var prefix = "QueueList/" + name
	var command = ""
	var err = 0
	var cmd func() = nil
	var ok bool = true

	for { // main loop
		command, err = getCommand(prefix)
		if err != 0 {
			var _, file, line, _ = runtime.Caller(0)
			fmt.Fprintf(os.Stderr, "%s:%d getCommand failed\n", file, line)
			return err
		}

		cmd, ok = c.cmdList[command]
		if !ok {
			if command == "exit" {
				break
			}

			if command == "" {
				continue
			}

			fmt.Fprintln(os.Stderr, "Invalid command: "+command)
			fmt.Fprintln(os.Stderr, "Please type \"help\" for more details")
			continue
		}

		cmd()
	} // end main loop

	return 0
}

func (c *cliQueue) help() {
	fmt.Println("help: print this message")
	fmt.Println("add: add new task")
	fmt.Println("delete: delete(cancel) a task by id")
	fmt.Println("listPanding: list all tasks in panding list in id")
	fmt.Println("listFinished: list all tasks in finished list in id")
	fmt.Println("clearPanding: clear(cancel) all task in panding list")
	fmt.Println("clearFinished: clear all task in finished list")
	fmt.Println("console: print current child process' stdout")
	fmt.Println("start: start the queue")
	fmt.Println("stop: stop the queue")
}

func (c *cliQueue) add() {
	var args = ""
	var task = new(model.Task)
	var err = model.TaskInit(task)
	var code = 0
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d model.TaskInit failed\n", file, line)
		return
	}

	task.ExecName, code = getCommand("Exec name")
	if code != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d getCommand failed\n", file, line)
		return
	}

	task.WorkDir, code = getCommand("Working dir")
	if code != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d getCommand failed\n", file, line)
		return
	}

	args, code = getCommand("Arguments")
	if code != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d getCommand failed\n", file, line)
		return
	}
	task.Args = strings.Split(args, " ")

	fmt.Print("Priority: ")
	fmt.Scanf("%d\n", &task.Priority)

	if task.Priority > 4 {
		task.Priority = 4
	}

	if task.Priority < 0 {
		task.Priority = 0
	}

	var id uint32 = 0
	id, err = c.queue.AddTask(task)
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d c.queue.AddTask failed\n", file, line)
		return
	}

	fmt.Printf("Done, id is: %d\n", id)
}

func (c *cliQueue) delete() {
	var id uint32 = 0
	fmt.Print("id: ")
	fmt.Scanf("%d\n", &id)

	var err = c.queue.RemoveTask(id)
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d c.queue.RemoveTask failed\n", file, line)
		return
	}

	fmt.Println("Done")
}

func (c *cliQueue) listPending() {
	var res, err = c.queue.ListPending()
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d c.queue.ListPending failed\n", file, line)
		return
	}

	for i := res.Front(); i != nil; i = i.Next() {
		fmt.Printf("%d\n", i.Value.(uint))
	}
}

func (c *cliQueue) listFinished() {
	var res, err = c.queue.ListFinished()
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d c.queue.ListFinished failed\n", file, line)
		return
	}

	for i := res.Front(); i != nil; i = i.Next() {
		fmt.Printf("%d\n", i.Value.(uint))
	}
}

func (c *cliQueue) clearPanding() {
	c.queue.ClearPanding()
	fmt.Println("Done")
}

func (c *cliQueue) clearFinished() {
	c.queue.ClearFinished()
	fmt.Println("Done")
}

func (c *cliQueue) start() {
	var err = c.queue.Start()
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d c.queue.Start failed\n", file, line)
		return
	}

	fmt.Println("Done")
}

func (c *cliQueue) stop() {
	var err = c.queue.Stop()
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d c.queue.Stop failed\n", file, line)
		return
	}

	fmt.Println("Done")
}

func (c *cliQueue) console() {
	fmt.Print("In the console, press \"Enter\" to stop\n")
	fmt.Print("Press enter to continue.")
	model.CLIReader.ReadString('\n')

	quit := make(chan bool, 2)

	go func() {
		var err error = nil
		var out = ""

		for {
			out, err = c.queue.ReadCurrentOutput()
			if err != nil {
				var _, file, line, _ = runtime.Caller(0)
				fmt.Fprintf(os.Stderr, "%s:%d c.queue.ReadCurrentOutput failed\n", file, line)
				return
			}

			if out != "" {
				fmt.Print(out)
			}

			select {
			case <-time.After(time.Microsecond * 500):
				continue
			case <-quit:
				fmt.Println("Console exited")
				return
			default:
				continue
			}
		}
	}()

	fmt.Println("Console start")
	model.CLIReader.ReadString('\n')
	quit <- true
}
