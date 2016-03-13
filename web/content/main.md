+++
title = "Obliv-C: A Language for Extensible Data-Oblivious Computation"
+++

### Obliv-C: A Language for Extensible Data-Oblivious Computation

Obliv-C is a simple GCC wrapper that makes it easy to embed secure
computation protocols inside regular C programs. The idea is simple: if
you are performing a multi-party distributed computation with sensitive
data, just write it in our Obliv-C langauge and compile/link it with
your project. The result will be a secure multi-party cryptographic
protocol that performs this operation without revealing any of the
inputs or intermediate values of the computation to any of the
parties. Only the outputs are finally shared.

### Paper

### Tutorial

### Projects Using Obliv-C






