A shell utilizing V8 thats entirely controlled via Javascript and love.

# Installation
## Requirements

You can download the prebuild executable from the release section of this repo
- Windows 10 or higher
- Windows 10 SDK 10.0.20348 or higher

Additionally if you want to build the source yourself
- Build v8 as a `static monolithic library`, [the v8 docs will guide you through the process](https://v8.dev/docs)
(this project contains an [args.gn](./args.gn) file that you can use as a gn config
for bulding the library)
- support for `C++ 17`
- `Ninja` and `CMake` (recommended)
- Set the environment variables `V8_INCLUDE` and `V8_LIB` respective to the paths of the v8
include directory and the directory containing the v8 static lib
- with Ninja and CMake installed just execute inside the root dir:
```bash
cmake --preset x64-release
ninja -C ./build/x64-release/
```

to run unit tests:
```bash
ctest --test-dir ./build/x64-release/tests 
```

# Usage

V8Shell is a shell that aims to be able to be fully controllable via JavaScript.
For that it embedds a v8 runtime to evaluate input as js and exposes functions which
invoke native system code.

## As of now the following functions are implemented:

### exit()

Alias: quit

Terminates the shell process

---

### ls (printToStd = true)

Alias: ll

Prints contents of the current working directory, coloring files/directories differently. Returns undefined.
If printToStd is set to false, it will instead return an array of objects with the signature
```js
{
    filename: "test.js",    // string
    isDirectory: false,     // boolean
}
```
Note: printToStd parameter enforces strict equality with the boolean type.

---

### cd(dir)

Changes the current working directory. To enter a sub-directory, pass a string with the name
of the desired directory:
```js
cd('src')
cd('src/Commands') // can also enter sub-sub-directories
```
To go back to a parent directory, pass the number of directories you want to go up:
```js
cd(1)
```

---

### print(expression)

Evaluates a given expression and prints the result. Usually the shell will automatically
print the result of the evaluation of it's input, so this is useful to print the result
of a specific expression.

---

### version()

Returns a string containing the current v8 version used by the embedded engine.

---

### read(filename)

Reads a given file and returns it's contents as a string.

---

### execute(filename)

Reads a given file, parses it's content as JavaScript, compiles and executes it.
