package main

import "C"
import (
	bn256cf "bn/cloudflare"
	"math/big"
)

//export New_G1
func New_G1(k int) int {
	// fmt.Println("===============")
	// fmt.Println("     NEW G1")
	// fmt.Println("===============")
	K := (&big.Int{}).SetInt64(int64(k))
	g1 := new(G1).ScalarBaseMult(K)
	// fmt.Println("g1: ", g1)
	// fmt.Println("K: ", K)
	var h Handler_G1
	h.Val = g1 // .Set(g1)
	// fmt.Println("h: ", h)
	K = nil
	// PrintOC()
	ind := AddHandler(Handler(h))
	// PrintOC()
	// fmt.Println("===============")
	return int(ind)
}

//export New_G2
func New_G2(k int) int {
	g2 := new(G2)
	K := new(big.Int).SetInt64(int64(k))
	var h Handler_G2
	h.Set(g2.ScalarBaseMult(K))
	K = nil
	ind := AddHandler(h)
	return ind
}

//export Add_G1
func Add_G1(e_ind, i, j int) int {
	p1 := GetHandler(i).(Handler_G1).Get()
	p2 := GetHandler(j).(Handler_G1).Get()

	var e *G1
	if e_ind < 0 {
		e = &bn256cf.G1{}
		var h Handler_G1
		h.Val = e
		e_ind = AddHandler(h)
	} else {
		e = GetHandler(e_ind).(Handler_G1).Get()
	}

	e.Add(p1, p2)
	return e_ind
}

//export Add_G2
func Add_G2(e_ind, i, j int) int {
	p1 := GetHandler(i).(Handler_G2).Get()
	p2 := GetHandler(j).(Handler_G2).Get()

	var e *G2
	if e_ind == -1 {
		e = &bn256cf.G2{}
		var h Handler_G2
		h.Val = e
		e_ind = AddHandler(h)
	} else {
		e = GetHandler(e_ind).(Handler_G2).Get()
	}

	e.Add(p1, p2)
	return e_ind
}

//export Neg_G1
func Neg_G1(e_ind, i int) int {
	p := GetHandler(i).(Handler_G1).Get()

	var e *G1
	if e_ind == -1 {
		e = &bn256cf.G1{}
		var h Handler_G1
		h.Val = e
		e_ind = AddHandler(h)
	} else {
		e = GetHandler(e_ind).(Handler_G1).Get()
	}

	e.Neg(p)
	return e_ind
}

//export Neg_G2
func Neg_G2(e_ind, i int) int {
	p := GetHandler(i).(Handler_G2).Get()

	var e *G2
	if e_ind == -1 {
		e = &bn256cf.G2{}
		var h Handler_G2
		h.Val = e
		e_ind = AddHandler(h)
	} else {
		e = GetHandler(e_ind).(Handler_G2).Get()
	}

	e.Neg(p)
	return e_ind
}

//export ScalarBaseMult_G1
func ScalarBaseMult_G1(e_ind, k int) int {
	K := (&big.Int{}).SetInt64(int64(k))
	var e *G1
	if e_ind == -1 {
		e = &bn256cf.G1{}
		var h Handler_G1
		h.Val = e
		e_ind = AddHandler(h)
	} else {
		e = GetHandler(e_ind).(Handler_G1).Get()
	}

	e.ScalarBaseMult(K)
	return e_ind
}

//export ScalarBaseMult_G2
func ScalarBaseMult_G2(e_ind, k int) int {
	K := (&big.Int{}).SetInt64(int64(k))
	var e *G2
	if e_ind == -1 {
		e = &bn256cf.G2{}
		var h Handler_G2
		h.Val = e
		e_ind = AddHandler(h)
	} else {
		e = GetHandler(e_ind).(Handler_G2).Get()
	}

	e.ScalarBaseMult(K)
	return e_ind
}

//export ScalarMult_G1
func ScalarMult_G1(e_ind, i, k int) int {
	p := GetHandler(i).(Handler_G1).Get()

	K := (&big.Int{}).SetInt64(int64(k))
	var e *G1
	if e_ind == -1 {
		e = &bn256cf.G1{}
		var h Handler_G1
		h.Val = e
		e_ind = AddHandler(h)
	} else {
		e = GetHandler(e_ind).(Handler_G1).Get()
	}

	e.ScalarMult(p, K)
	return e_ind
}

//export ScalarMult_G2
func ScalarMult_G2(e_ind, i, k int) int {
	p := GetHandler(i).(Handler_G2).Get()

	K := (&big.Int{}).SetInt64(int64(k))
	var e *G2
	if e_ind == -1 {
		e = &bn256cf.G2{}
		var h Handler_G2
		h.Val = e
		e_ind = AddHandler(h)
	} else {
		e = GetHandler(e_ind).(Handler_G2).Get()
	}

	e.ScalarMult(p, K)
	return e_ind
}
