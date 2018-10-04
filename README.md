# Motion SDK C++ Example

## Build the example

### Windows

Use the Visual Studio project in the main folder. Supports Visual Studio 2017.

### macOS and Linux

Use make. Uses Clang by default but GCC works as well.

```
make
```


## Run the example

By default, the example application will read as many samples as possible and
print them to the standard output. The samples are printed as they arrive,
every ~10 milliseconds.

```
Usage: example [options...]

Allowed options:
  --help         show help message
  --file arg     output file
  --frames N     read N frames
  --header       show channel names in the first row
```
