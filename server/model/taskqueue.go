package model

import (
	"bytes"
	"container/heap"
	"container/list"
	"errors"
	"fmt"
	"io"
	"os/exec"
	"runtime"
	"sync"
	"sync/atomic"
	"time"

	"github.com/shirou/gopsutil/v3/process"
)

type TaskQueue struct {
	pendingMutex sync.Mutex
	pending      TaskHeap

	finishedMutex sync.Mutex
	finished      *list.List

	currentMutex sync.Mutex
	current      *Task

	stopFlag  uint32
	isRunning uint32

	childOutMutex *sync.Mutex
	childOut      string

	id         uint32
	currentPid int32
}

func taskQueueInit(t *TaskQueue, name string) error {
	if t == nil {
		var _, file, line, _ = runtime.Caller(0)
		var msg = fmt.Sprintf("%s:%d invalid input", file, line)
		Global.Logger.Error().Msg(msg)
		return errors.New(msg)
	}

	t.pending = make(TaskHeap, 0)

	t.finished = list.New()

	t.stopFlag = 0
	t.isRunning = 0

	t.childOut = ""
	t.childOutMutex = new(sync.Mutex)

	t.id = 0
	t.currentPid = 0

	return nil
}

func (t *TaskQueue) fin() {
	t.ClearPanding()
	t.Stop()
}

func (t *TaskQueue) ListPending() (*list.List, error) {
	var ret *list.List = nil
	var err error = nil

	t.pendingMutex.Lock()
	if t.pending.Len() != 0 {
		ret = list.New()
		for _, task := range t.pending {
			_ = ret.PushBack(task.ID)
		}
	} else {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d pending list is empty", file, line)
		err = errors.New("pending list is empty")
	}

	t.pendingMutex.Unlock()
	return ret, err
}

func (t *TaskQueue) ListFinished() (*list.List, error) {
	var ret *list.List = nil
	var err error = nil

	t.finishedMutex.Lock()
	if t.finished.Len() != 0 {
		ret = list.New()
		for e := t.finished.Front(); e != nil; e = e.Next() {
			ret.PushBack((e.Value.(*Task)).ID)
		}
	} else {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d finished list is empty", file, line)
		err = errors.New("finished list is empty")
	}

	t.finishedMutex.Unlock()
	return ret, err
}

func (t *TaskQueue) PendingDetails(id uint32, out *Task) error {
	var file string
	var line int
	if out == nil {
		_, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d out is nil", file, line)
		return errors.New("out is nil")
	}

	var err error = nil
	t.pendingMutex.Lock()
	if t.pending.Len() <= 0 {
		_, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d pending list is empty", file, line)
		err = errors.New("pending list is empty")
	} else {
		for _, task := range t.pending {
			if task.ID == id {
				task.DeepCopy(out)
				goto exit
			}
		}

		_, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d no such id: %d", file, line, id)
		err = fmt.Errorf("no such id: %d", id)
	}
exit:
	t.pendingMutex.Unlock()

	return err
}

func (t *TaskQueue) FinishedDetails(id uint32, out *Task) error {
	if out == nil {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d out is nil", file, line)
		return errors.New("out is nil")
	}

	var err error = nil

	t.finishedMutex.Lock()
	if t.finished.Len() <= 0 {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d pending list is empty", file, line)
		err = errors.New("pending list is empty")
	} else {
		for e := t.finished.Front(); e != nil; e = e.Next() {
			if e.Value.(*Task).ID == id {
				e.Value.(*Task).DeepCopy(out)
				goto exit
			}
		}

		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d no such id: %d", file, line, id)
		err = fmt.Errorf("no such id: %d", id)
	}
exit:
	t.finishedMutex.Unlock()

	return err
}

func (t *TaskQueue) ClearPanding() {
	t.pendingMutex.Lock()
	t.pending = t.pending[:0]
	t.pendingMutex.Unlock()
}

func (t *TaskQueue) ClearFinished() {
	t.finishedMutex.Lock()
	t.finished = list.New()
	t.finishedMutex.Unlock()
}

func (t *TaskQueue) CurrentTask(out *Task) error {
	if out == nil {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d out is nil", file, line)
		return errors.New("out is nil")
	}

	var err error = nil
	t.currentMutex.Lock()
	if t.current == nil {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d this queue is not running", file, line)
		err = errors.New("this queue is not running")
	} else {
		t.current.DeepCopy(out)
	}
	t.currentMutex.Unlock()

	return err
}

func (t *TaskQueue) AddTask(in *Task) (uint32, error) {
	if in == nil {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d in is nil", file, line)

		return 0, errors.New("in is nil")
	}

	t.pendingMutex.Lock()
	in.ID = t.id
	heap.Push(&t.pending, in)
	t.id += 1
	t.pendingMutex.Unlock()

	return in.ID, nil
}

func (t *TaskQueue) RemoveTask(id uint32) error {
	var err error = nil
	var file string = ""
	var line int = 0
	t.pendingMutex.Lock()
	for i, task := range t.pending {
		if task.ID == id {
			heap.Remove(&t.pending, i)
			goto exit
		}
	}

	_, file, line, _ = runtime.Caller(0)
	Global.Logger.Error().Msgf("%s:%d no such id: %d", file, line, id)
	err = fmt.Errorf("no such id: %d", id)
exit:
	t.pendingMutex.Unlock()

	return err
}

func (t *TaskQueue) IsRunning() bool {
	return atomic.LoadUint32(&t.isRunning) > 0
}

func (t *TaskQueue) ReadCurrentOutput() (string, error) {
	if !t.IsRunning() {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d task is not running", file, line)

		return "", errors.New("task is not running")
	}

	var ret string = ""
	t.childOutMutex.Lock()
	if len(t.childOut) > 0 {
		ret = t.childOut
		t.childOut = ""
	}
	t.childOutMutex.Unlock()

	return ret, nil
}

func (t *TaskQueue) Start() error {
	if t.IsRunning() {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d queue is already running", file, line)
		return errors.New("queue is already running")
	}

	atomic.StoreUint32(&t.isRunning, 1)
	go t.mainLoop()
	return nil
}

func (t *TaskQueue) Stop() error {
	if !t.IsRunning() {
		var _, file, line, _ = runtime.Caller(0)
		Global.Logger.Error().Msgf("%s:%d queue is stopped", file, line)
		return errors.New("queue is stopped")
	}

	var pid = atomic.LoadInt32(&t.currentPid)
	if pid == 0 {
		var _, file, line, _ = runtime.Caller(0)
		var msg = fmt.Sprintf("%s:%d queue is stopped", file, line)
		Global.Logger.Error().Msg(msg)
		return errors.New("queue is stopped")
	}

	atomic.StoreUint32(&t.stopFlag, 1)
	killPid(int(pid), true)
	return nil
}

func (t *TaskQueue) mainLoop() {
	var task *Task = nil

	for { // for loop 1

		var stopFlag = atomic.LoadUint32(&t.stopFlag) > 0

		if stopFlag {
			goto EXIT_LOOP_1
		}

		task = nil
		t.pendingMutex.Lock()
		if t.pending.Len() <= 0 {
			task = nil
		} else {
			task = heap.Pop(&t.pending).(*Task)
		}
		t.pendingMutex.Unlock()

		if task == nil {
			var _, file, line, _ = runtime.Caller(0)
			Global.Logger.Warn().Msgf("%s:%d Panding list is empty", file, line)
			break
		}

		// update current task
		t.currentMutex.Lock()
		t.current = task
		t.currentMutex.Unlock()

		var cmd = exec.Command(task.ExecName, task.Args...)
		if cmd.Err != nil {
			t.mainLoopCleanUp(false)
			continue
		}

		var outBuf bytes.Buffer

		cmd.Dir = task.WorkDir
		cmd.Stdout = &outBuf
		cmd.Stderr = &outBuf

		var err = cmd.Start()
		if err != nil {
			var _, file, line, _ = runtime.Caller(0)
			var msg = fmt.Sprintf("%s:%d %s", file, line, err.Error())
			Global.Logger.Error().Msg(msg)
			t.mainLoopCleanUp(false)
			continue
		}

		err = flagPid(cmd.Process.Pid)
		if err != nil {
			var _, file, line, _ = runtime.Caller(0)
			Global.Logger.Error().Msgf("%s:%d %s", file, line, err.Error())
			killPid(cmd.Process.Pid, false)
			t.mainLoopCleanUp(false)
			continue
		}

		var monitor *process.Process
		monitor, err = process.NewProcess(int32(cmd.Process.Pid))
		if err != nil {
			var _, file, line, _ = runtime.Caller(0)
			Global.Logger.Error().Msgf("%s:%d %s", file, line, err.Error())
			killPid(cmd.Process.Pid, true)
			t.mainLoopCleanUp(false)
			continue
		}

		var isChildRunning bool = true
		var bufChan = make(chan string, 8)
		var quitChan = make(chan uint8, 8)
		atomic.StoreInt32(&t.currentPid, int32(cmd.Process.Pid))

		go func() {
			var buf = make([]byte, 4096)
			var readCount = 0
			for { // loop 1
				isChildRunning, err = monitor.IsRunning()
				if err != nil {
					var _, file, line, _ = runtime.Caller(0)
					Global.Logger.Error().Msgf("%s:%d %s", file, line, err.Error())
					killPid(cmd.Process.Pid, true)
					quitChan <- 2
					return
				}

				if !isChildRunning {
					quitChan <- 2
					return
				}

				stopFlag = atomic.LoadUint32(&t.stopFlag) > 0
				if stopFlag {
					killPid(cmd.Process.Pid, true)
					quitChan <- 1
					return
				}

				readCount, err = outBuf.Read(buf)
				if readCount > 0 {
					bufChan <- string(buf[:])
				}

				if err != nil && err != io.EOF {
					var _, file, line, _ = runtime.Caller(0)
					Global.Logger.Error().Msgf("%s:%d %s", file, line, err.Error())
				}

				time.Sleep(time.Millisecond * 500)
			} // end loop 1
		}()

		for { // for loop 2
			select {
			case code := <-quitChan:
				if code == 1 {
					goto EXIT_LOOP_1
				} else {
					goto EXIT_LOOP_2
				}
			case data := <-bufChan:
				t.childOutMutex.Lock()
				t.childOut = data
				t.childOutMutex.Unlock()
			case <-time.After(time.Millisecond * 500):
				continue
			default:
				continue
			} // end select
		} // end for loop 2

	EXIT_LOOP_2:
		// child process exited
		t.mainLoopCleanUp(false)
	} // end for loop 1

EXIT_LOOP_1:
	t.mainLoopCleanUp(true)
}

func (t *TaskQueue) mainLoopCleanUp(stopped bool) {
	atomic.StoreUint32(&t.stopFlag, 0)

	t.finishedMutex.Lock()
	if t.current != nil {
		t.finished.PushBack(t.current)
	}
	t.finishedMutex.Unlock()

	t.currentMutex.Lock()
	t.current = nil
	t.currentMutex.Unlock()

	if stopped {
		atomic.StoreUint32(&t.isRunning, 0)
		atomic.StoreInt32(&t.currentPid, 0)
	}
}
