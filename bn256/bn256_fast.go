// Copyright 2018 Péter Szilágyi. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

//go:build amd64 || arm64
// +build amd64 arm64

// Package bn256 implements the Optimal Ate pairing over a 256-bit Barreto-Naehrig curve.
package main

import "C"
import (
	bn256cf "bn/cloudflare" // "github.com/ethereum/go-ethereum/crypto/bn256/cloudflare"
	"fmt"
	"math/big"
)

// G1 is an abstract cyclic group. The zero value is suitable for use as the
// output of an operation, but cannot be used as an input.
type G1 = bn256cf.G1

// G2 is an abstract cyclic group. The zero value is suitable for use as the
// output of an operation, but cannot be used as an input.
type G2 = bn256cf.G2

// PairingCheck calculates the Optimal Ate pairing for a set of points.
//
//export PairingCheck
func PairingCheck(a []*G1, b []*G2) bool {
	return bn256cf.PairingCheck(a, b)
}

type Handler interface {
	Create()
	Close()
	// Get() interface{}
	// Set(interface{})
}

type ObjCollection struct {
	handlers []Handler
}

func (oc ObjCollection) CloseAll() {
	for i := 0; i < len(oc.handlers); i++ {
		oc.handlers[i].Close()
	}
	oc.handlers = make([]Handler, 0)
}

func (oc ObjCollection) GetHandler(id int) (handler Handler) {
	return oc.handlers[id]
}

func (oc ObjCollection) AddHandler(h Handler) (id int) {
	oc.handlers = append(oc.handlers, h)
	fmt.Println("handlers: ", oc.handlers)
	return len(oc.handlers) - 1
}

type Handler_G1 struct{ Val *G1 }
type Handler_G2 struct{ Val *G2 }
type Handler_bigInt struct{ Val *big.Int }

func (h Handler_G1) Create()     { h.Val = new(G1) }
func (h Handler_G2) Create()     { h.Val = new(G2) }
func (h Handler_bigInt) Create() { h.Val = new(big.Int) }

func (h Handler_G1) Close()     { h.Val = nil }
func (h Handler_G2) Close()     { h.Val = nil }
func (h Handler_bigInt) Close() { h.Val = nil }

// func (h Handler) Get() (interface{}) { return h.val }
func (h Handler_G1) Get() *G1             { return h.Val }
func (h Handler_G2) Get() *G2             { return h.Val }
func (h Handler_bigInt) Get() interface{} { return h.Val }

// func (h Handler) Set(x interface) () { h.val = g1 }
func (h Handler_G1) Set(g1 *G1)             { h.Val = g1 }
func (h Handler_G2) Set(g2 *G2)             { h.Val = g2 }
func (h Handler_bigInt) Set(bi interface{}) { h.Val = bi.(*big.Int) }

// //export RandomG1
// func RandomG1(r io.Reader) (Handler_bigInt, Handler_G1, error) {
// 	bi, g1, err := bn256cf.RandomG1(r)
// 	return Handler_bigInt(bi), Handler_G1(g1), err
// }

var OC ObjCollection

//export PrintOC
func PrintOC() {
	fmt.Println("OC: ", OC)
}

//export New_G1
func New_G1(k int) int {
	fmt.Println("===============")
	fmt.Println("     NEW G1")
	fmt.Println("===============")
	K := (&big.Int{}).SetInt64(int64(k))
	g1 := new(G1).ScalarBaseMult(K)
	fmt.Println("g1: ", g1)
	fmt.Println("K: ", K)
	var h Handler_G1
	h.Val = g1 // .Set(g1)
	fmt.Println("h: ", h)
	K = nil
	ind := OC.AddHandler(Handler(h))
	fmt.Println("===============")
	return ind
	// return 1
}

//export New_G2
func New_G2(k int) int {
	g2 := new(G2)
	K := new(big.Int).SetInt64(int64(k))
	var h Handler_G2
	h.Set(g2.ScalarBaseMult(K))
	K = nil
	ind := OC.AddHandler(h)
	return ind
}

//export ClearAll
func ClearAll() {
	OC.CloseAll()
}

//export Add_G1
func Add_G1(e_ind, i, j int) int {
	p1 := OC.GetHandler(i).(Handler_G1).Get()
	p2 := OC.GetHandler(j).(Handler_G1).Get()

	var e *G1
	if e_ind == -1 {
		e = &bn256cf.G1{}
		var h Handler_G1
		h.Set(e)
		e_ind = OC.AddHandler(h)
	} else {
		e = OC.GetHandler(e_ind).(Handler_G1).Get()
	}

	e.Add(p1, p2)
	return e_ind
}

// //export Add_G2
// func Add_G2(e_ind, i, j int) int {
// 	p1 := oc.GetHandler(i).Get().(*G2)
// 	p2 := oc.GetHandler(j).Get().(*G2)

// 	var e *G2
// 	if e_ind == -1 {
// 		e = &bn256cf.G2{}
// 		var h Handler_G2
// 		h.Set(e)
// 		e_ind = oc.AddHandler(h)
// 	} else {
// 		e = oc.GetHandler(e_ind).Get().(*G2)
// 	}

// 	e.Add(p1, p2)
// 	return e_ind
// }

func main() {}
