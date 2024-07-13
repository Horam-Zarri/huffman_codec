# huffman_codec
A multi threaded text encoder and decoder based on the Huffman algorithm.  

## Introduction
This is my project for my university's algorithms course. Even though we were not asked to implement this project with multithreading I decided to do so to further optimize and refine it, which also enhanced my concurrency skills. 

## Implementation
The core algorithm is similar to the implementation introduced in CLRS book. The multithreading mechanism follows a design close to the Data Parallelism or Producer-Consumer pattern, where one thread is responsible to partition the text data into smaller chunks, which are processed independently by other threads, while maintaining thread safety using shared synchronization mechanisms between threads for IO access.

## Build & Usage
**Prerequisites:** 
- C++ compiler with support for C++23 standard
- CMake >= 3.28
- A build system supported by CMake (preferably Ninja)
- DirectX XII (Only required for GUI build)  

To build the CLI/GUI variant of the program pass to CMake ```-DHUFFMANCODEC_BUILD_GUI=OFF``` or
```-DHUFFMANCODEC_BUILD_GUI=ON``` respectively.

A minimal command to build the CLI varaint of the program and running it, using Ninja as build system would be:
```cmake
cmake -S . -G Ninja -B cmake-build-ninja -DHUFFMANCODEC_BUILD_GUI=OFF &&  
cd ./cmake-build-ninja && ninja && ./HuffmanCodec.exe [FILE PARAMS AND OPTIONS]
```

## Benchmarks
I haven't collected many results for now but I include one case. On my PC with Ryzen 5 5600 (12 threads) and  
32GB  ram a 1 Billion character .txt file (1GB) consisting of  5 different characters took 7.5s avg to encode, producing  
a .bin file of roughly 270MB, and took 7.8s avg to decode.  
Each of the encode and decode operations took more than one minute average when run single-threaded!
