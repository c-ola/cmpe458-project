## Building
```
mkdir build && cd build
cmake ..
make
```

## Running
From build directory
```
./parser # runs built in tests
./parser path/to/test_file
```

If you want to disable DEBUG in stdout, comment out the line `#define DEBUG` in `src/parser.h`

