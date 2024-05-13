package main

import "C"
import (
	bn256 "bn/cloudflare"
	"fmt"
	"math/big"
)

type Handler interface {
	Create()
	Close()
}

//export OC
var OC []Handler

func CloseAll() {
	for i := 0; i < len(OC); i++ {
		OC[i].Close()
	}
	OC = make([]Handler, 0)
}

func GetHandler(id int) (handler Handler) {
	return OC[id]
}

func AddHandler(h Handler) (id int) {
	OC = append(OC, h)
	// fmt.Println("handlers: ", OC)
	// fmt.Println(OC)
	return len(OC) - 1
}

type Handler_G1 struct{ Val *G1 }
type Handler_G2 struct{ Val *G2 }
type Handler_bigInt struct{ Val *big.Int }
type Handler_curvePoint struct{ Val *curvePoint }
type Handler_gfP struct{ Val *gfP }

func (h Handler_G1) Create()         { h.Val = new(G1) }
func (h Handler_G2) Create()         { h.Val = new(G2) }
func (h Handler_bigInt) Create()     { h.Val = new(big.Int) }
func (h Handler_curvePoint) Create() { h.Val = new(curvePoint) }
func (h Handler_gfP) Create()        { h.Val = bn256.Fuzz_newGFp(0) }

func (h Handler_G1) Close()         { h.Val = nil }
func (h Handler_G2) Close()         { h.Val = nil }
func (h Handler_bigInt) Close()     { h.Val = nil }
func (h Handler_curvePoint) Close() { h.Val = nil }
func (h Handler_gfP) Close()        { h.Val = nil }

func (h Handler_G1) Get() *G1                 { return h.Val }
func (h Handler_G2) Get() *G2                 { return h.Val }
func (h Handler_bigInt) Get() *big.Int        { return h.Val }
func (h Handler_curvePoint) Get() *curvePoint { return h.Val }
func (h Handler_gfP) Get() *gfP               { return h.Val }

func (h Handler_G1) Set(g1 *G1)                 { h.Val = g1 }
func (h Handler_G2) Set(g2 *G2)                 { h.Val = g2 }
func (h Handler_bigInt) Set(bi *big.Int)        { h.Val = bi }
func (h Handler_curvePoint) Set(cp *curvePoint) { h.Val = cp }
func (h Handler_gfP) Set(x *gfP)                { h.Val = x }

//export PrintOC
func PrintOC() {
	fmt.Println("OC: ", OC)
}

//export ClearAll
func ClearAll() {
	CloseAll()
}
