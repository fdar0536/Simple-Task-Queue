//go:build !windows
// +build !windows

package model

import "syscall"

func flagPid(pid int) error {
	return syscall.Setpgid(pid, 0)
}

func killPid(pid int, group bool) error {
	var toKill = pid
	if group {
		toKill *= -1
	}

	return syscall.Kill(toKill, 9)
}
