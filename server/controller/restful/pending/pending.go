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

package pending

import (
	"STQ/controller/restful/types"
	"STQ/model"
	"container/list"
	"net/http"

	"github.com/labstack/echo/v4"
)

var global = &model.Global

func List(c echo.Context) error {
	var req = types.QueueReq{}
	var err = c.Bind(&req)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var tq *model.TaskQueue
	tq, err = global.QueueList.GetQueue(req.Name)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var ret *list.List
	ret, err = tq.ListPending()
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var res = types.ListTaskRes{}
	res.ID = make([]uint32, ret.Len())
	res.ID = res.ID[:0]
	for i := ret.Front(); i != nil; i = i.Next() {
		res.ID = append(res.ID, i.Value.(uint32))
	}

	return c.JSON(http.StatusOK, &res)
}

func Details(c echo.Context) error {
	var req = types.TaskDetailsReq{}
	var err = c.Bind(&req)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var tq *model.TaskQueue
	tq, err = global.QueueList.GetQueue(req.QueueName)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var task = model.Task{}
	err = tq.PendingDetails(req.ID, &task)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var res = types.TaskDetailsRes{}
	err = types.BuildTaskDetailsRes(&task, &res)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusInternalServerError, &errMsg)
	}

	return c.JSON(http.StatusOK, &res)
}

func Current(c echo.Context) error {
	var req = types.QueueReq{}
	var err = c.Bind(&req)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var tq *model.TaskQueue
	tq, err = global.QueueList.GetQueue(req.Name)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var task = model.Task{}
	err = tq.CurrentTask(&task)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var res = types.TaskDetailsRes{}
	err = types.BuildTaskDetailsRes(&task, &res)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusInternalServerError, &errMsg)
	}

	return c.JSON(http.StatusOK, &res)
}

func Add(c echo.Context) error {
	var req = types.AddTaskReq{}
	var err = c.Bind(&req)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var tq *model.TaskQueue
	tq, err = global.QueueList.GetQueue(req.QueueName)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var task = new(model.Task)
	task.Args = make([]string, len(req.Args))
	copy(task.Args, req.Args)
	task.ExecName = req.ProgramName
	task.Priority = model.TaskPriority(req.Priority)
	task.WorkDir = req.WorkDir

	var id uint32 = 0
	id, err = tq.AddTask(task)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusInternalServerError, &errMsg)
	}

	var res = types.ListTaskRes{}
	res.ID = make([]uint32, 1)
	res.ID = res.ID[:0]
	res.ID = append(res.ID, id)
	return c.JSON(http.StatusOK, &res)
}

func Remove(c echo.Context) error {
	var req = types.TaskDetailsReq{}
	var err = c.Bind(&req)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var tq *model.TaskQueue
	tq, err = global.QueueList.GetQueue(req.QueueName)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	err = tq.RemoveTask(req.ID)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	return c.NoContent(http.StatusOK)
}

func Start(c echo.Context) error {
	var req = types.QueueReq{}
	var err = c.Bind(&req)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var tq *model.TaskQueue
	tq, err = global.QueueList.GetQueue(req.Name)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	err = tq.Start()
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	return c.NoContent(http.StatusOK)
}

func Stop(c echo.Context) error {
	var req = types.QueueReq{}
	var err = c.Bind(&req)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var tq *model.TaskQueue
	tq, err = global.QueueList.GetQueue(req.Name)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	err = tq.Stop()
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	return c.NoContent(http.StatusOK)
}
