# Barnes Browser

This is a simple, experimental browser written to explore browser concepts for educational purposes. Use at your own risk!

It is based on the excellent [Web Browser Engineering](https://browser.engineering/) by Pavel Panchekha & Chris Harrelson.

## C++ Version

Work in progress. See the code under `/cpp`.

### How to Build the Project

For building, this project uses CMake. See `CMakeLists.txt` for the setup. To do a debug build, run the following commands:

```
cmake -S cpp -B cpp/build -DCMAKE_BUILD_TYPE=Debug
cmake --build cpp/build
```

This will put the executable in the `/build` folder. 

Note: if you use Visual Studio Code, the `tasks.json` file allows building by task for convenience.
Also note the `CMakeLists.txt` uses explicit list of `.cpp` files, so if you add a new one, be sure to add it to `add_executable`.

### Dependencies

These are the dependencies used in this project.

1. [OpenSLL](https://www.openssl.org/) - used for SLL network connections
2. [Qt](https://doc.qt.io/) - used for the graphical user interface


## Python Version

Partially completed version written in Python. This largely follows the example code in the book. See the code under `/python`.

## Tests

### Test Pages

The `test_pages` folder includes several sample HTML pages to use for testing.
