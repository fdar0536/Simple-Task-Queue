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

package restful

import (
	"STQ/controller/restful/access"
	"STQ/controller/restful/console"
	"STQ/controller/restful/done"
	"STQ/controller/restful/pending"
	"STQ/controller/restful/queue"
	"STQ/model"
	"STQ/utils"
	"fmt"
	"os"
	"runtime"

	"github.com/labstack/echo/v4"
	"github.com/ziflex/lecho/v3"
)

var global = &model.Global

type RestController struct {
	Server *echo.Echo
}

func Init(s *RestController, config *utils.ConfigParam) int {
	if s == nil || config == nil {
		var _, file, line, _ = runtime.Caller(0)
		fmt.Fprintf(os.Stderr, "%s:%d invalid input\n", file, line)
		return 1
	}

	s.Server.Logger = lecho.From(global.Logger)

	// access
	s.Server.GET("/access/echo", access.Echo)
	s.Server.POST("/access/stop", access.Stop)

	// queue list
	s.Server.POST("/queue/create", queue.Create)
	s.Server.POST("/queue/rename", queue.Rename)
	s.Server.DELETE("/queue/delete", queue.Delete)
	s.Server.GET("/queue/list", queue.List)

	// pending list
	s.Server.GET("/pending/list", pending.List)
	s.Server.GET("/pending/details", pending.Details)
	s.Server.GET("/pending/current", pending.Current)
	s.Server.POST("/pending/add", pending.Add)
	s.Server.POST("/pending/remove", pending.Remove)
	s.Server.POST("/pending/start", pending.Start)
	s.Server.POST("/pending/stop", pending.Stop)

	//console
	s.Server.GET("/console", console.Output)

	// done list
	s.Server.GET("/done/list", done.List)
	s.Server.GET("/done/details", done.Details)
	s.Server.POST("/done/clear", done.Clear)

	return 0
}
