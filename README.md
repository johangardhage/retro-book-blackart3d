# retro-book-blackart3d

The demos from Black Art of 3D Game Programming, brought back from the 90's.

![Screenshot](/screenshots/krk.png "krk")

## Prerequisites

To build the demo programs, you must first install the following tools:

- [GCC](https://gcc.gnu.org/)
- [Ninja](https://ninja-build.org/)
- [SDL2](https://www.libsdl.org/)

### Install dependencies

#### openSUSE

`$ sudo zypper install ninja gcc-c++ libSDL2-devel`

#### Ubuntu

`$ sudo apt install ninja-build g++ libsdl2-dev`

#### Windows

* Manually download and install [MSYS2](https://www.msys2.org/).
* Run `View advanced system settings` from the Start menu and add a PATH to `C:\msys64\mingw64\bin`.
* Run `MSYS2 MSYS` from the Start menu. Update the package database and base packages with `pacman -Syu`.
* Run `MSYS2 MSYS` from the Start menu again. Update the rest of the base packages with `pacman -Syu`.
* Install the development tools with `pacman -S git mingw-w64-x86_64-gcc mingw-w64-x86_64-ninja mingw-w64-x86_64-SDL2`.
* Close the `MSYS2 MSYS` window and run `MSYS MinGW 64-bit` from the Start menu.
* Clone the git repository with `git clone https://github.com/johangardhage/retro-book-gardens.git`.
* Finally, to be able to build the demos on Windows, edit the file `build.ninja` and uncomment the line [#  command = $cc $in $windows -o $out.](build.ninja#L10).

## Build instructions

To build the demo programs, run:

`$ ninja`

A `build` directory will be created, containing the demo programs.

## Usage

```
Usage: demo [OPTION]...

Options:
 -h, --help           Display this text and exit
 -w, --window         Render in a window
     --fullwindow     Render in a fullscreen window
 -f, --fullscreen     Render in fullscreen
 -v, --vsync          Enable sync to vertical refresh
     --novsync        Disable sync to vertical refresh
 -l, --linear         Render using linear filtering
     --nolinear       Render using nearest pixel sampling
 -c, --showcursor     Show mouse cursor
     --nocursor       Hide mouse cursor
     --showfps        Show frame rate in window title
     --nofps          Hide frame rate
     --capfps=VALUE   Limit frame rate to the specified VALUE
```

## License

Licensed under MIT license. See [LICENSE](LICENSE) for more information.

## Authors

Original code written by Andre Lamothe for [Black Art of 3D Game Programming](https://www.amazon.com/Black-Art-Game-Programming-High-Speed/dp/1571690042/) (Waite Group Press)

## Screenshots

![Screenshot](/screenshots/sortdemo.png "sortdemo")
![Screenshot](/screenshots/zdemo.png "zdemo")
