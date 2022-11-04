//+build windows

/*
This file is modified form https://github.com/mattn/psutil

The MIT License (MIT)

Copyright (c) 2021 Yasuhiro Matsumoto

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

package model

import (
	"syscall"
	"unsafe"
)

const PROCESS_ALL_ACCESS = 0x1F0FFF

var (
	kernel32         = syscall.NewLazyDLL("kernel32")
	procGetProcessId = kernel32.NewProc("GetProcessId")
)

func flagPid(pid int) error {
	var i int = pid
	_ = i
	return nil
}

func killTree(ph syscall.Handle, code uint32) error {
	var pe syscall.ProcessEntry32
	pid, _, _ := procGetProcessId.Call(uintptr(ph))
	if pid != 0 {
		h, err := syscall.CreateToolhelp32Snapshot(syscall.TH32CS_SNAPPROCESS, 0)
		if err == nil {
			pe.Size = uint32(unsafe.Sizeof(pe))
			if syscall.Process32First(h, &pe) == nil {
				for {
					if pe.ParentProcessID == uint32(pid) {
						ph, err := syscall.OpenProcess(
							PROCESS_ALL_ACCESS, false, pe.ProcessID)
						if err == nil {
							killTree(ph, code)
							syscall.CloseHandle(ph)
						}
					}
					if syscall.Process32Next(h, &pe) != nil {
						break
					}
				}
				syscall.CloseHandle(h)
			}
		}
	}
	return syscall.TerminateProcess(syscall.Handle(ph), uint32(code))
}

func terminateTree(pid int, code int) error {
	ph, err := syscall.OpenProcess(
		PROCESS_ALL_ACCESS, false, uint32(pid))
	if err != nil {
		return err
	}
	defer syscall.CloseHandle(ph)
	return killTree(ph, uint32(code))
}

func killPid(pid int, group bool) error {
	var i bool = group
	_ = i
	return terminateTree(pid, 9)
}
