package main

import "C"
import (
	"fmt"
	"math/big"
)

//export curvePoint_New
func curvePoint_New() int {
	cp := new(curvePoint)
	var h Handler_curvePoint
	h.Val = cp
	ind := AddHandler(Handler(h))
	return ind
}

//export curvePoint_String
func curvePoint_String(i int) *C.char {
	return C.CString(GetHandler(i).(Handler_curvePoint).Get().String())
}

// export curvePoint_Set
func curvePoint_Set(i, j int) {
	p1 := GetHandler(i).(Handler_curvePoint).Get()
	p2 := GetHandler(j).(Handler_curvePoint).Get()
	p1.Set(p2)
}

//export curvePoint_IsOnCurve
func curvePoint_IsOnCurve(i int) bool {
	p := GetHandler(i).(Handler_curvePoint).Get()
	return p.IsOnCurve()
}

//export curvePoint_SetInfinity
func curvePoint_SetInfinity(i int) {
	p := GetHandler(i).(Handler_curvePoint).Get()
	p.SetInfinity()
}

//export curvePoint_IsInfinity
func curvePoint_IsInfinity(i int) bool {
	p := GetHandler(i).(Handler_curvePoint).Get()
	return p.IsInfinity()
}

//export curvePoint_Add
func curvePoint_Add(c_ind, a_ind, b_ind int) {
	a := GetHandler(a_ind).(Handler_curvePoint).Get()
	b := GetHandler(b_ind).(Handler_curvePoint).Get()
	c := GetHandler(c_ind).(Handler_curvePoint).Get()

	c.Add(a, b)
}

//export curvePoint_Double
func curvePoint_Double(c_ind, a_ind int) {
	a := GetHandler(a_ind).(Handler_curvePoint).Get()
	c := GetHandler(c_ind).(Handler_curvePoint).Get()

	c.Double(a)
}

//export curvePoint_Mul
func curvePoint_Mul(c_ind, a_ind, scalar int) {
	a_h := GetHandler(a_ind).(Handler_curvePoint)
	c_h := GetHandler(c_ind).(Handler_curvePoint)
	a := a_h.Get()
	c := c_h.Get()
	scalar_bi := new(big.Int).SetInt64(int64(scalar))

	c.Mul(a, scalar_bi)
	fmt.Println(c)
	fmt.Println(a)
	c_h.Val = c
}

//export curvePoint_MakeAffine
func curvePoint_MakeAffine(c_ind int) {
	c := GetHandler(c_ind).(Handler_curvePoint).Get()
	c.MakeAffine()
}

//export curvePoint_Neg
func curvePoint_Neg(c_ind, a_ind int) {
	a := GetHandler(a_ind).(Handler_curvePoint).Get()
	c := GetHandler(c_ind).(Handler_curvePoint).Get()
	c.Neg(a)
}
