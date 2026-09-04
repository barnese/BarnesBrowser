# Barnes Browser

This is a simple, experimental web browser written to explore browser concepts for educational purposes. It is a work in progress. Use at your own risk!

It is based on the book [Web Browser Engineering](https://browser.engineering/) by Pavel Panchekha & Chris Harrelson.

The browser is primarily written in C++. See the code under `/src`. It uses CMake for building and package management. See `CMakeLists.txt` for build configuration settings. 

This project also includes a simple, test web server, written in Python, that helps serve various pages to test the browser.

## Building the Project

If you use Visual Studio Code to build and run this project, add the following extensions:

1. CMake Tools - for extended CMake support
2. clangd - for code completion

The `.vscode/tasks.json` file includes various tasks for building the project. The default is a task to build and run, so Shift+Command+B should execute this task.

## Project Dependencies

These are the dependencies used in this project.

1. [OpenSLL](https://www.openssl.org/) - used for SLL network connections
2. [Qt](https://doc.qt.io/) - used for the graphical user interface and content rendering
3. [Catch2](https://github.com/catchorg/Catch2) - used for unit tests
4. [Duktape](https://duktape.org/) - used for the Javascript engine
5. [Dukglue](https://github.com/Aloshi/dukglue) - a Duktape wrapper for convenience functions

## Unit Tests

This project uses the library [Catch2](https://github.com/catchorg/Catch2) for unit testing. The tests are located until `/src/tests/`. The tests are run using CTest. For VS Code, see `tasks.json`'s "Build Tests" and "Run Tests" tasks. For convenience, consider adding a keybinding in `keybindings.json` for quickly running tests, such as:
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

## Test Pages

The `assets/test_pages` folder includes several sample HTML pages to use for testing.

To run a local web server, run `./run_local_server.sh` in the terminal. (Note: this script requires python3.) This will allow running those web pages at http://localhost:8000/.