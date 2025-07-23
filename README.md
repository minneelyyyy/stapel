# Stapel

An in development game engine framework.

# Setup (Windows)

Unfinished, however the process will look something like so:

## Install Dependencies

- [Vulkan SDK](#vulkan-sdk)
- [Visual Studio 2022](#visual-studio-2022)

### Visual Studio 2022

This project builds under MSVC. You don't actually have to use Visual Studio to work on and build this project, but you at least need it for the tools this project relies on.

### Vulkan SDK

You will need the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows) installed on your system.

To install, please go to https://vulkan.lunarg.com/sdk/home, and set up your SDK through the installer. The version currently in use is 1.3.243.0, however that is subject to change.

## Download the Code

1. Open x64 Native Tools Prompt for VS2022
2. Navigate to your desired directory
3. `$ git clone https://github.com/minneelyyyy/stapel.git`

## Prepare the Build System

Run the command `cmake -G "Visual Studio 17 2022" -A x64 -S . -B build`,
or if desired use Ninja to build the engine with `cmake -G Ninja -S . -B build`

## Create Your Project

This will require you to write a `CMakeLists.txt` file and make use of the build functions provided by Stapel to build your game for the engine (not yet ready). This will be able to handle creating a distribution build of your project ready to be shipped.

# Setup (Linux)

Unfinished, however the process will look something like so:

## Install Dependencies

- X11 or Wayland development libraries
- Vulkan developer SDK
- GCC or Clang
- make or ninja
- git
- cmake

## Download the Code

```
$ git clone https://github.com/minneelyyyy/stapel.git
```

## Generate Build Files

```
$ cd stapel
$ mkdir build/ && cd build/
$ cd build
$ cmake ..
```

## Setup Your Project

This will require you to write a `CMakeLists.txt` file and make use of the build functions provided by Stapel to build your game for the engine (not yet ready). This will be able to handle creating a distribution build of your project ready to be shipped.
