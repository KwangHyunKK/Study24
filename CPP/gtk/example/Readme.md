### Install

```bash
$ sudo apt install libgtk-4-dev
```

### How to run example_0.c

- can compile the program above with GCC using:

```bash
gcc $( pkg-config --cflgas gtk4 ) -o example-0 example_0.c $( pkg-config --libs gtk4 ) 
```

- or you can use CMakeLists.txt

```bash
$ cd build
$ cmake ..
$ make 
$ ./example-1
```