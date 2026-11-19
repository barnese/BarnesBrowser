# Barnes Browser

This is a simple, experimental web browser written to explore browser concepts for educational purposes. It is based on the book [Web Browser Engineering](https://browser.engineering/) by Pavel Panchekha and Chris Harrelson.

The browser is primarily written in C++. See the code under `/src`. It uses CMake for building and package management. See `CMakeLists.txt` for build configuration settings. 

This project also includes a simple, test web server, written in Python, that helps serve various pages to test the browser. See the Test Pages section below for more information. 

⚠️ Disclaimer: This project is a work in progress. For now, only simple HTML pages will work correctly. Use at your own risk!

## Building the Project

The following instructions are for macOS. If you're building the project on Linux, see the [README for Linux](./README_LINUX.md).

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
6. [Python 3](https://www.python.org/) - (optional) used to run test web servers

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

## Debugging

To troubleshoot memory issues, consider using the Address Sanitizer by adding these settings to the `CMakeLists.txt` file:

```
target_compile_options(main PRIVATE
    -Wall
    -Wextra
    -pedantic
    -fsanitize=address
    -fno-omit-frame-pointer
    -g
)

target_link_options(main PRIVATE
    -fsanitize=address
)
```

## Test Pages

The `assets/test_pages` folder includes several sample HTML pages to use for testing.

To run a local web server, run `./run_local_server.sh` in the terminal. (Note: this script requires python3.) This will allow running those web pages at http://localhost:8000/.

## History and Notes

This project was entirely coded by hand up until the git tag `pre-claude`, after which I added Claude support to more rapidly implement features and bug fixes. Up until this point, the code has largely followed the implementation of the _Web Browser Engineering_ book, with a few exceptions:

1. The browser chrome was implemented using Qt widgets rather than implementing them from scratch, with the intention of making future chrome user interface changes easier and ultimately more flexible and standardized. My use of the `BrowserView` also is different, but fits more with recommended Qt architecture.

2. Networking has deviated heavily after implementing HTTP/1.1. See the `/src/networking/` folder for more details. Essentially, networking has expanded to having a resource loader, a HTTP connection class, HTTP cache, URL components, etc. I may consider using a networking library at some point, like https://docs.libcpr.dev/ to replace `HttpConnection`.

3. While this project uses OpenGL for graphics rendering, the implementation is very basic and needs further refinement. I would also like to use Metal for macOS at some point, while keeping OpenGL for Linux. 
