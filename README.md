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

This will put the executable in the `cpp/build` folder. To run it: `./cpp/build/main`.

### Visual Studio Code Setup

If you use Visual Studio Code to run this project, consider adding the following extensions:

1. CMake Tools - for extended CMake support
2. clangd - for code completion

The `.vscode/tasks.json` file includes various tasks for building the project. The default is a task to build and run, so Shift+Command+B should execute this task.

Also note the `CMakeLists.txt` uses explicit list of `.cpp` files, so if you add a new one, be sure to add it to `add_executable`.

### Project Dependencies

These are the dependencies used in this project.

1. [OpenSLL](https://www.openssl.org/) - used for SLL network connections
2. [Qt](https://doc.qt.io/) - used for the graphical user interface
3. [Catch2](https://github.com/catchorg/Catch2) - used for unit tests

## Unit Tests

This project uses the library [Catch2](https://github.com/catchorg/Catch2) for unit testing. The tests are located until `/cpp/tests/`. The tests are run using CTest. For VS Code, see `tasks.json`'s "Build Tests" and "Run Tests" tasks. For convenience, consider adding a keybinding in `keybindings.json` for quickly running tests, such as:
```
{
    "key": "cmd+shift+t",
    "command": "workbench.action.tasks.runTask",
    "args": "Run Tests"
}
```

If you want to run tests in the current file, you can use the "Run Tests in Current File" task. In order for this to work, the tests have to use a tag that matches the current file's name. For example, the file `css_parser_tests.cpp` has the tag `[css_parser_tests]`. That tag is passed to Catch2 in each test case, so when "Run Tests in Current File" is run, it filters the tests by `[${fileBasenameNoExtension}]`. You may want to set up a keybinding for this as well:
```
{
    "key": "shift+cmd+y",
    "command": "workbench.action.tasks.runTask",
    "args": "Run Tests in Current File"
}
```

## Python Version

Partially completed version written in Python. This largely follows the example code in the book. See the code under `/python`.

## Tests

### Test Pages

The `test_pages` folder includes several sample HTML pages to use for testing.
