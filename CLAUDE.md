# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

An educational web browser in C++23, following the book [Web Browser Engineering](https://browser.engineering/). Uses Qt6 (UI + rendering), OpenSSL (HTTPS), Duktape + Dukglue (JavaScript), and Catch2 v3 (tests). Catch2 and Dukglue are pulled via CMake `FetchContent`; Duktape is vendored in `third_party/duktape`.

## Commands

Run all commands from the repo root (the app loads `assets/...` via paths relative to the working directory).

```sh
# Configure (debug options: SHOW_COMPOSITED_LAYER_BORDERS, VERBOSE_DEBUG_OUTPUT, NETWORK_DEBUG_OUTPUT)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build and run the browser
cmake --build build --target main && ./build/main

# Build and run all tests
cmake --build build --target tests && ctest --test-dir build --output-on-failure

# Run tests for a single file (each test file defines a TAG matching its basename)
./build/tests "[test_url]"
# Run a single test case by name
./build/tests "resolve full url"

# Serve assets/test_pages at http://localhost:8000 (the browser opens this URL on startup)
./run_local_server.sh
```

- New source files must be added explicitly to `add_executable(main ...)` in `CMakeLists.txt`, and also to `add_executable(tests ...)` if code under test depends on them (the tests target lists its own subset of sources; it has no Qt rendering/layout code).
- New test files go in `src/tests/` and should `#define TAG "[<file basename>]"` and pass it to every `TEST_CASE`, so the VS Code "Run Tests in Current File" task works.
- Address sanitizer flags are present but commented out in `CMakeLists.txt`.
- `MeasureTime` writes a Chrome-trace-format `browser.trace` file to the working directory for performance profiling.

## Architecture

The design mirrors the book's later chapters (multi-threaded, composited rendering):

- **`Browser`** (`src/browser.*`, a `QWidget`) runs on the main/browser thread. It owns the `Chrome` (tab bar/address bar UI), the `BrowserView`, and all `BrowserTab`s. It handles input, then performs **raster → composite → draw** of the active tab's display list into `CompositedLayer`s, driven by dirty flags (`m_needsRaster`, `m_needsComposite`, `m_needsDraw`, ...). It also handles accessibility (screen-reader speech), zoom, and dark mode.
- **`BrowserTab`** (`src/browser_tab.*`) runs work on its own thread via its `TaskRunner` (`src/tasks/`, a FIFO queue on a `std::jthread`). It owns the root `Frame`, history, per-origin `JSContext`s, and the accessibility tree. It runs animation frames, paints all frames into a display list, and hands results to the browser thread with `Browser::commit(tab, CommitData&)` (`src/commit_data.h`). Browser → tab communication happens by scheduling tasks on the tab's `TaskRunner`; shared browser state is guarded by `Browser::m_mutex`.
- **`Frame`** (`src/frame.*`) owns one document: the HTML tree, the `DocumentLayout` tree, scroll, and CSP. It loads resources (scripts, stylesheets, images, iframes — iframes create child `Frame`s), applies styles, runs layout, and handles events/form submission. Frames are addressed by window ID (`BrowserTab::windowIdToFrame`) for `postMessage` and per-frame JS.

Pipeline subsystems:

- `src/networking/` — `URL`, raw HTTP/1.1 over sockets/OpenSSL (`HttpConnection`, keep-alive), `HttpCache`, `ResourceLoader`.
- `src/html_tree/` — `Node`/`Element`/`Text` and the HTML parser.
- `src/styles/` — CSS parser, selectors (tag, descendant, pseudo-class), style cascade helpers, and CSS transitions/animations (`NumericAnimation`).
- `src/layout/` — layout tree (`DocumentLayout` → `BlockLayout` → `LineLayout` → `TextLayout`/`EmbedLayout` subclasses for input, image, iframe). Layout and node fields use **`ProtectedField<T>`** (`protected_field.h/.tpp`) for incremental layout: each field tracks a dependency graph and dirty flag, `get()` asserts the field isn't dirty, and `mark()` invalidates downstream fields. Dependencies passed at `initialize()` are frozen; otherwise they are discovered on read. Protected fields are non-copyable/non-movable to avoid dangling invalidation pointers.
- `src/commands/` — display list: paint commands (`DrawText`, `DrawImage`, ...) and visual effects (`Blend`, `Transform`) that form a tree; `CompositedLayer` groups commands for compositing.
- `src/scripting/` — `JSContext` wraps a Duktape heap, exposing C++ methods via `dukglue_register_method`. The DOM-like JS API (`document`, `Node`, `addEventListener`, `XMLHttpRequest`, `setTimeout`, `requestAnimationFrame`, ...) is implemented in `assets/scripts/runtime.js` on top of those native hooks.
- `src/accessibility/` — accessibility tree built from the DOM.

Other assets: `assets/styles/browser.css` (default user-agent stylesheet), `assets/styles/chrome.qss` (Qt UI styles), `assets/test_pages/` (manual test pages served by `local_server.py`, which sends `Cache-Control: max-age=60` to exercise caching).

`Logger` (`src/logger.h`) prefixes output with a tag and thread ID; `threadRegistry()` maps thread IDs to readable names.
