# Buddhabrot
A Buddhabrot generator written in C++

## Cloning & dependencies

To clone this repository, use `git clone --recurse-submodules https://github.com/Clematrics/Buddhabrot.git`

You will need to have glfw installed :
* With apt, install the package `libglfw3-dev` or `libglfw3`.
* With vcpkg, install the package `glfw3`.

## Compiling

To compile under Linux, do the following :
```
mkdir build
cd build
cmake ..
make
```

To compile under Windows with vcpkg :
```
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake ..
```
then build the generated solution with Visual Studio or with the target you selected.