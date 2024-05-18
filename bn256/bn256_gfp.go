package main

import (
	"C"
)
import (
	bn256cb "bn/cloudflare"
	"fmt"
)

type gfP = bn256cb.FuzzGFP

//export gfP_newGFP
func gfP_newGFP(x int64) int {
	gfp := bn256cb.Fuzz_newGFp(x)
	var h Handler_gfP
	h.Val = gfp
	i := AddHandler(Handler(h))
	return i
}

//export gfP_String
func gfP_String(i int) *C.char {
	e := GetHandler(i).(Handler_gfP).Get()
	return C.CString(e.String())
}

//export gfP_Set
func gfP_Set(dst_ind, src_ind int) {
	e := GetHandler(dst_ind).(Handler_gfP).Get()
	f := GetHandler(src_ind).(Handler_gfP).Get()

	e.Set(f)
}

//export gfP_Invert
func gfP_Invert(dst_ind, src_ind int) {
	e := GetHandler(dst_ind).(Handler_gfP).Get()
	f := GetHandler(src_ind).(Handler_gfP).Get()

	e.Invert(f)
}

//export gfP_Add
func gfP_Add(c_ind, a_ind, b_ind int) {
	a := GetHandler(a_ind).(Handler_gfP).Get()
	b := GetHandler(b_ind).(Handler_gfP).Get()
	c := GetHandler(c_ind).(Handler_gfP).Get()

	bn256cb.Fuzz_gfpAdd(c, a, b)
}

//export gfP_Neg
func gfP_Neg(c_ind, a_ind int) {
	a := GetHandler(a_ind).(Handler_gfP).Get()
	c := GetHandler(c_ind).(Handler_gfP).Get()

	bn256cb.Fuzz_gfpNeg(c, a)
}

//export gfP_Mul
func gfP_Mul(c_ind, a_ind, b_ind int) {
	a := GetHandler(a_ind).(Handler_gfP).Get()
	b := GetHandler(b_ind).(Handler_gfP).Get()
	c := GetHandler(c_ind).(Handler_gfP).Get()

	bn256cb.Fuzz_gfpMul(c, a, b)
}

//export gfP_Sub
func gfP_Sub(c_ind, a_ind, b_ind int) {
	a := GetHandler(a_ind).(Handler_gfP).Get()
	b := GetHandler(b_ind).(Handler_gfP).Get()
	c := GetHandler(c_ind).(Handler_gfP).Get()

	bn256cb.Fuzz_gfpSub(c, a, b)
}

//export gfP_Equal
func gfP_Equal(i, j int) bool {
	p1 := GetHandler(i).(Handler_gfP).Get()
	p2 := GetHandler(j).(Handler_gfP).Get()

	return *p1 == *p2
}

//export gfP_Marshal
func gfP_Marshal(e_ind int, out []byte) int {
	if e_ind < 0 {
		e_ind = gfP_newGFP(0)
	}
	e := GetHandler(e_ind).(Handler_gfP).Get()
	e.Marshal(out)

	return e_ind
}

//export gfP_Unmarshal
func gfP_Unmarshal(e_ind int, in []byte) int {
	if e_ind < 0 {
		e_ind = gfP_newGFP(0)
	}
	e := GetHandler(e_ind).(Handler_gfP).Get()
	err := e.Unmarshal(in)
	if err != nil {
		fmt.Println(err)
		// panic(err)
		return -1
	}
	return e_ind
}

// //export gfP_montEncode
// func gfP_montEncode(c_ind, a_ind int) {
// 	a := GetHandler(a_ind).(Handler_gfP).Get()
// 	c := GetHandler(c_ind).(Handler_gfP).Get()

// }
