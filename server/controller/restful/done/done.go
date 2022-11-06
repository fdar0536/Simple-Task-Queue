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

package done

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
	ret, err = tq.ListFinished()
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
	err = tq.FinishedDetails(req.ID, &task)
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

func Clear(c echo.Context) error {
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

	tq.ClearFinished()
	return c.NoContent(http.StatusOK)
}
