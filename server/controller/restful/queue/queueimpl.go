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

package queue

import (
	"STQ/controller/restful/types"
	"STQ/model"
	"net/http"

	"github.com/labstack/echo/v4"
)

var global = &model.Global

func Create(c echo.Context) error {
	var req = types.QueueReq{}
	var err = c.Bind(&req)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	err = global.QueueList.CreateQueue(req.Name)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	return c.NoContent(http.StatusOK)
}

func Rename(c echo.Context) error {
	var req = types.RenameQueueReq{}
	var err = c.Bind(&req)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	err = global.QueueList.RenameQueue(req.Old, req.New)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	return c.NoContent(http.StatusOK)
}

func Delete(c echo.Context) error {
	var req = types.QueueReq{}
	var err = c.Bind(&req)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	err = global.QueueList.DeleteQueue(req.Name)
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	return c.NoContent(http.StatusOK)
}

func List(c echo.Context) error {
	var ret, err = global.QueueList.ListQueue()
	if err != nil {
		var errMsg = types.ErrMsg{}
		errMsg.Msg = err.Error()
		return c.JSON(http.StatusBadRequest, &errMsg)
	}

	var res = types.ListQueueRes{}
	res.Name = make([]string, len(ret))
	res.Name = res.Name[:0]
	res.Name = append(res.Name, ret...)

	return c.NoContent(http.StatusOK)
}
