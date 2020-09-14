# QVM (Quantum Virtual Machine)

A simple ragister-based virtual machine for fun.

## Features
- memory manager (using a special register called AX)
- stack manager (using the special registers SSX, SPX and SLX)
- code segment manager (using the special registers CSX, IPX and CLX)
- flags is stored in FX register
- system interruptions are stored in SX register
- set of 16 general purpose registers (X1...X16)
- set of 36 instructions
- all registers are 32-bit length
