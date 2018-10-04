# Motion SDK C++ Example

Stream measurement and motion data from the Shadow. Print out the data in
CSV format.

Each sample in time is one row. Each column is one channel from one device
or joint in the Shadow skeleton.

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
