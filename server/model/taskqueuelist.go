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

package model

import (
	"errors"
	"fmt"
	"os"
	"runtime"
	"sync"
)

type TaskQueueList struct {
	queueListMutex sync.Mutex
	queueList      map[string]*TaskQueue
}

func taskQueueListInit(in *TaskQueueList) int {
	if in == nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d invalid input\n", file, line)
		return 1
	}

	in.queueList = make(map[string]*TaskQueue)
	return 0
}

func (t *TaskQueueList) fin() {
	if len(t.queueList) > 0 {
		for _, queue := range t.queueList {
			queue.fin()
		}
	}
}

func (t *TaskQueueList) CreateQueue(name string) error {
	t.queueListMutex.Lock()
	var err error = nil
	var _, ok = t.queueList[name]
	if ok {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d %s", file, line, name+" is already exist")
		err = errors.New(name + " is already exist")
	}

	if err == nil {
		var toInsert *TaskQueue = new(TaskQueue)
		err = taskQueueInit(toInsert, name)
		if err == nil {
			t.queueList[name] = toInsert
		}
	}
	t.queueListMutex.Unlock()
	return err
}

func (t *TaskQueueList) ListQueue() ([]string, error) {
	var ret []string
	t.queueListMutex.Lock()
	if len(t.queueList) != 0 {
		ret = make([]string, len(t.queueList))
		ret = ret[:0]
		for name := range t.queueList {
			ret = append(ret, name)
		}
	}

	t.queueListMutex.Unlock()

	if len(ret) <= 0 {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d list is empty", file, line)
		return nil, errors.New("list is empty")
	}

	return ret, nil
}

func (t *TaskQueueList) DeleteQueue(name string) error {
	var err error = nil
	t.queueListMutex.Lock()
	var task, ok = t.queueList[name]
	if ok {
		task.Stop()
		delete(t.queueList, name)
	} else {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d %s", file, line, "no such queue: "+name)
		err = errors.New("no such queue: " + name)
	}

	t.queueListMutex.Unlock()

	return err
}

func (t *TaskQueueList) RenameQueue(old string, newName string) error {
	var err error = nil
	var file string
	var line int
	t.queueListMutex.Lock()

	// find the new one
	var task, ok = t.queueList[newName]
	if ok {
		_, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d new name is duplicated", file, line)
		err = errors.New("new name is duplicated")
		goto exit
	}

	// find the old name
	task, ok = t.queueList[old]
	if ok {
		delete(t.queueList, old)
		t.queueList[newName] = task
	} else {
		_, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d %s", file, line, "no such queue: "+old)
		err = errors.New("no such queue: " + old)
	}

exit:
	t.queueListMutex.Unlock()

	return err
}

func (t *TaskQueueList) GetQueue(name string) (*TaskQueue, error) {
	var err error = nil
	var ret *TaskQueue = nil
	var ok bool = true
	t.queueListMutex.Lock()
	ret, ok = t.queueList[name]
	if !ok {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d %s", file, line, "no such queue: "+name)
		err = errors.New("no such queue: " + name)
	}
	t.queueListMutex.Unlock()

	return ret, err
}
