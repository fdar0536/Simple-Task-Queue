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

package console

import (
	"STQ/model"

	"github.com/gorilla/websocket"
	"github.com/labstack/echo/v4"
)

var global = &model.Global

func Output(c echo.Context) error {
	var flag bool = false
	var err error
	var tq *model.TaskQueue
	var out string
	var outBuf []byte

	var upgrader = websocket.Upgrader{}
	var ws *websocket.Conn
	ws, err = upgrader.Upgrade(c.Response(), c.Request(), nil)
	if err != nil {
		return err
	}
	defer ws.Close()

	for { // for loop 1
		_, outBuf, err = ws.ReadMessage()
		if err != nil {
			_ = ws.WriteMessage(websocket.TextMessage, []byte(err.Error()))
			return err
		}

		out = string(outBuf[:])
		if flag {
			if out != "c" {
				break
			}
		} else {
			flag = true
			tq, err = global.QueueList.GetQueue(out)
			if err != nil {
				_ = ws.WriteMessage(websocket.TextMessage, []byte(err.Error()))
				return err
			}
		}

		out, err = tq.ReadCurrentOutput()
		if err != nil {
			_ = ws.WriteMessage(websocket.TextMessage, []byte(err.Error()))
			return err
		}

		err = ws.WriteMessage(websocket.TextMessage, []byte(out))
		if err != nil {
			_ = ws.WriteMessage(websocket.TextMessage, []byte(err.Error()))
			return err
		}
	} // end for loop 1

	return nil
}
