//go:build (amd64 && !generic) || (arm64 && !generic)
// +build amd64,!generic arm64,!generic

package bn256

// This file contains forward declarations for the architecture-specific
// assembly implementations of these functions, provided that they exist.

import (
	"golang.org/x/sys/cpu"
)

//nolint:varcheck,unused,deadcode
var hasBMI2 = cpu.X86.HasBMI2

//go:noescape
func gfpNeg(c, a *gfP)
func Fuzz_gfpNeg(c, a *gfP) { gfpNeg(c, a) }

//go:noescape
func gfpAdd(c, a, b *gfP)
func Fuzz_gfpAdd(c, a, b *gfP) { gfpAdd(c, a, b) }

//go:noescape
func gfpSub(c, a, b *gfP)
func Fuzz_gfpSub(c, a, b *gfP) { gfpSub(c, a, b) }

//go:noescape
func gfpMul(c, a, b *gfP)
func Fuzz_gfpMul(c, a, b *gfP) { gfpMul(c, a, b) }
