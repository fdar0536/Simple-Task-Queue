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

package cli

import (
	"STQ/model"
	"fmt"
	"os"
	"runtime"
)

type CLIQueueList struct {
	Char      byte
	cmdList   map[string]func()
	queueList *model.TaskQueueList
	queue     cliQueue
}

func CLIQueueListInit(in *CLIQueueList) int {
	if in == nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d invalid input\n", file, line)
		return 1
	}

	in.Char = 0x1
	var err = cliQueueInit(&in.queue)
	if err != 0 {
		return err
	}

	in.queueList = &model.Global.QueueList
	in.cmdList = make(map[string]func())

	// function lists
	in.cmdList["help"] = in.help
	in.cmdList["list"] = in.list
	in.cmdList["add"] = in.add
	in.cmdList["rename"] = in.rename
	in.cmdList["delete"] = in.delete

	return 0
}

func (c *CLIQueueList) Run() int {
	var command string = ""
	var err error = nil
	var code = 0
	var ok bool = false
	var cmd func() = nil
	for { // main loop
		command, code = getCommand("QueueList")
		if code != 0 {
			return code
		}

		cmd, ok = c.cmdList[command]
		if !ok {
			if command == "exit" {
				break
			}

			if command == "" {
				continue
			}

			var queue *model.TaskQueue = nil
			queue, err = c.queueList.GetQueue(command)
			if err != nil {
				var _, file, line, _ = runtime.Caller(0)
				fmt.Fprintf(os.Stderr, "%s:%d %s\n", file, line, err.Error())
				fmt.Fprintf(os.Stderr, "Please type \"help\" for more details\n")
				continue
			}

			code = c.queue.run(queue, command)
			if code != 0 {
				var _, file, line, _ = runtime.Caller(0)
				fmt.Fprintf(os.Stderr, "%s:%d c.queue.run failed\n", file, line)
			}
			continue
		}

		cmd()
	} // end main loop

	return 0
}

func (c *CLIQueueList) help() {
	fmt.Println("help: print this message")
	fmt.Println("list: print all list")
	fmt.Println("add: add a new queue")
	fmt.Println("rename: rename the queue")
	fmt.Println("delete: delete the queue")
	fmt.Println("exit: exit this program")
	fmt.Println("Or just input queue's name for enter queue.")
}

func (c *CLIQueueList) list() {
	var res, err = c.queueList.ListQueue()
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d c.queueList.ListQueue failed\n", file, line)
		return
	}

	for _, name := range res {
		fmt.Println(name)
	}
}

func (c *CLIQueueList) add() {
	var name, code = getCommand("Enter name")
	if code != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d getCommand failed\n", file, line)
		return
	}

	var err = c.queueList.CreateQueue(name)
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d c.queueList.CreateQueu failed\n", file, line)
		return
	}

	fmt.Println("Done")
}

func (c *CLIQueueList) rename() {
	var oldName string = ""
	var newName string = ""
	var err error = nil
	var code = 0

	oldName, code = getCommand("Enter old name")
	if code != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d getCommand failed\n", file, line)
		return
	}

	newName, code = getCommand("Enter new name")
	if code != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d c.queueList.CreateQueu failed\n", file, line)
		return
	}

	err = c.queueList.RenameQueue(oldName, newName)
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d c.queueList.RenameQueue failed\n", file, line)
		return
	}

	fmt.Println("Done")
}

func (c *CLIQueueList) delete() {
	var code = 0
	var err error = nil
	var name string = ""
	name, code = getCommand("Enter name")
	if code != 0 {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d getCommand failed\n", file, line)
		return
	}

	err = c.queueList.DeleteQueue(name)
	if err != nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d c.queueList.DeleteQueue failed\n", file, line)
		return
	}

	fmt.Println("Done")
}
