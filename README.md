Halide is a programming language designed to make it easier to write high-performance image and array processing code on modern machines.
The language is embedded in C++ with a functional language built on top to allow definition of variables based on x and y coordinates.
Writing Halide is unique in that the output values are captured in the algorithm, while the execution speed is defined by a target-specific schedule.
Halide currently targets a large number of machines including:
  * CPU architectures: X86, ARM, MIPS, Hexagon, PowerPC
  * Operating systems: Linux, Windows, Mac OS X, Android, iOS, Qualcomm QuRT
  * GPU Compute APIs: CUDA, OpenCL, OpenGL, OpenGL Compute Shaders, Apple Metal, Microsoft Direct X 12

This project extends the available targets to custom-hardware generation.
The language that is used capture hardware intent is CoreIR.
CoreIR's most basic features provide a way to create hardware modules (adders, multipliers, muxes)
and connections (how to wire them together).
Beyond this, there are analysis and transformation passes to modify the circuit.

For more detail about what Halide is, see http://halide-lang.org.
To see some example code, look in the tutorials directory.

For more specific instructions on how to create CoreIR hardware targets, visit
https://stanfordaha.github.io/CGRAFlowDoc/halide/intro.html

Build Status
============

| Linux                        |
|------------------------------|
| [![linux build status][1]][2]|

[1]: https://travis-ci.com/StanfordAHA/Halide-to-Hardware.svg?branch=master
[2]: https://travis-ci.com/StanfordAHA/Halide-to-Hardware

