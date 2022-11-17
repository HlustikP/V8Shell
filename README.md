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
- compiler support for `C++ 17`
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

### help()

Prints an overview of all available shell functions.

---

### exit()

Alias: `quit`

Terminates the shell process

---

### print(expression)

Evaluates a given expression and prints the result. Usually the shell will automatically
print the result of the evaluation of it's input, so this is useful to print the result
of a specific expression.

---

### version()

Returns a string containing the current v8 version used by the embedded engine.

---

## File System Functions:

### ls (printToStd = true)

Alias: `ll`

Prints contents of the current working directory, coloring files/directories differently. Returns undefined.
If `printToStd` is set to false, it will instead return an array of objects with the signature
```js
{
    filename: "test.js",    // string
    isDirectory: false,     // boolean
}
```
Note: `printToStd` parameter enforces strict equality with the boolean type.

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

### createFile(filename)

Alias: `touch`

Creates a new file with the name `filename`.

---

### createDir(dirname)

Alias: `mkdir`

Creates a new directory with the name `dirname`.

---

### removeFile(filename)

Alias: `rf`

<p style="color:red">DANGER - USE WITH CARE</p>

Removes the file `filename` from the filesystem. This function does NOT remove the
entity if its a directory so consider using it to delete files and have a possible
safeguard against accidentally deleting a directory.

---

### removeDir(dirname)

Alias: `rd`

<p style="color:red">DANGER - USE WITH CARE</p>

Recursively removes the directory `dirname` and all it's contents from the file system.
This function does not remove the entity if `dirname` is a file.

---

### rm(entityName)

<p style="color:red">DANGER - USE WITH CARE</p>

Removes the filesystem entity from the filesystem, regardless of type. If it's a directory
then the directory's contents will be removed recursively aswell.

---

### rename(oldName, newName)

Changes the name of the path entity `oldName` to `newName`. Note that this function does not
rename anything if the target would be moved to a different location. This is to prevent
accidental moving, if thats intended, use the `move` function.

---

### move(from, to)

Alias: `mv`

Moves the path entity `from` to a new location defined in `to`. Note that `to` is the new
path assigned to it, including the file/directory name.

---

### copy(from, to)

Alias: `cp`

Constructs a copy of `from` at the path of `to`.

---

### read(filename)

Reads a given file and returns it's contents as a string.

---

### execute(filename)

Reads a given file, parses it's content as JavaScript, compiles and executes it.

---

### runSync(filename, parameters, verbose = true)

Creates a child process and halts execution of the shell until the child process terminates.
First argument `filename` is a string with the name of the application to be executed which 
is first looked for in the current executable's directory, otherwise the PATH is searched.
This function will also capture the standard out, eror and in streams and redirect them
to the shell. Note however that this behaviour is not guarenteed, especially in cases where
a new window is created by the child process.
```js
runSync('calc')
runSync('C:/cool-app/app.exe') // can also pass an absolute path
```

Second argument is an optional additional object with parameters to be passed to the executable.
```js
// cmd equivalent: cmake --preset x64-release
runSync('cmake', { preset: 'x64-debug' })

// cmd equivalent: git -v
runSync('git', { v: '' })
```

Third argument is controls verbosity of this functions. If set to `false` then some
messages wont be printed, however the standard out/error streams of the child process will
still be captured and printed.
```js
runSync('git', { v: '' }, false)
```
