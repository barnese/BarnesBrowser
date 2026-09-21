# Building on Linux (Ubuntu)

The following steps are for installing dependencies and building the app on Ubuntu. 

## Install Dependencies

Install the following packages to install clang, gdb (GNU Debugger), pkg-config (helps build systems locate installed libraries), libgl1-mesa-dev (for OpenGL support), Qt, Catch2 (for unit tests), OpenSSL and related dependencies.

```
sudo apt install build-essential
sudo apt install clang
sudo apt install clangd
sudo apt install gdb
sudo apt install cmake
sudo apt install ninja-build
sudo apt install pkg-config
sudo apt install libgl1-mesa-dev
sudo apt install qt6-base-dev
sudo apt install qt6-tools-dev
sudo apt install openssl libssl-dev
sudo apt install catch2
```

## Visual Studio Code

If you want to use Visual Studio Code, install the following extensions:

1. CMake Tools - for CMake build tooling
2. clangd - for C++ compilation
3. CodeLLDB - for debugging using LLDB

## Clone the Git repo

Clone the git repo. Open that project folder in VS Code.

## Build and Run

The existing CMake tasks and settings should enable you to now build and run the application. Press F5.

Note: you may want to disable the "C++: on throw" breakpoint.

