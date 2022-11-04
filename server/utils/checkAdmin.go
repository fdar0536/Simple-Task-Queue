// +build windows

package utils

import (
	"fmt"
	"os"

	"golang.org/x/sys/windows"
)

// UserIsAdmin is check user is admin or not
func UserIsAdmin() bool {
	var sid *windows.SID
	err := windows.AllocateAndInitializeSid(
		&windows.SECURITY_NT_AUTHORITY,
		2,
		windows.SECURITY_BUILTIN_DOMAIN_RID,
		windows.DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&sid)

	if err != nil {
		fmt.Fprintf(os.Stderr, "SID Error: %s\n", err)
		return true
	}

	token := windows.Token(0)

	res, err := token.IsMember(sid)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Token Membership Error: %s\n", err)
		return true
	}

	return res
}
