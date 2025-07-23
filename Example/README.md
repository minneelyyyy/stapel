# Example Game Setup

## Getting an SDK

Once Stapel is stable there will be pre built SDKs for developers to download, but for now the only option is compiling it for yourself.

## Compiling the SDK

### Install Dependencies (Linux)

The following dependencies can be installed from your system's package manager.

- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- either X11 or Wayland development libraries
- git
- gcc or clang
- ninja
- cmake

### Install Dependencies (Windows)

- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) (only for necessary C++ tools)

You don't actually have to use Visual Studio 2022 for compilation of the SDK. Indeed, that use is untested and unsupported. It is simply the easiest way to get all necessary build tools. It should be installed with `Desktop development with C++` enabled.

## Building the SDK

`/path/to/sdk` must be a new, empty directory that the following commands will output a build to.

```
$ git clone https://github.com/minneelyyyy/stapel.git
$ cd stapel
$ cmake -G Ninja -S . -B build -DCMAKE_INSTALL_PREFIX=/path/to/sdk
$ cd build
$ ninja install
```

## Environment Setup

You need to set the `STAPEL_SDK` environment variable to your installation of the Stapel SDK.

#### Linux
```
$ export STAPEL_SDK=/path/to/sdk
```

#### Windows
You can change this in Control Panel's system properties, or by searching for "environment" and adding a new variable there.

## Compiling Example

To compile the example, `cd` into it, then run the following:

```
$ cmake -G Ninja -S . -B build
$ cd build
$ ninja
```

This will output a libgame.so.

## Run Game & Distribution

There will eventually be a script designed to do this automatically, but for now it must be done manually.
The current expected structure for any Stapel game is as follows

#### Linux
```
<game>
├── bin
│   ├── libStapelEngine.so
│   └── libgame.so
└── stapel_launcher
```

#### Windows
```
<game>
├── bin
│   ├── StapelEngine.dll
│   └── Game.dll
└── stapel_launcher.exe
```

You will need to manually copy files from your SDK to match this file structure,
then the launcher should correctly run the game.
