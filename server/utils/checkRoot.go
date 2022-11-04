// +build !windows

package utils

//#include "unistd.h"
//#cgo CFLAGS: -Wall -Wextra -Werror -Wno-unused-parameter
import "C"

// UserIsAdmin is check user is super user or not
func UserIsAdmin() bool {
	return (int(C.geteuid()) == 0)
}
