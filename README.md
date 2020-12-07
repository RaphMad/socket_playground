## Introduction

This project contains some examples and experiments regarding the usage of sockets in **C** in **Windows**, using **Winsock 2**.
For building I used **Visual Studio Code** with its **C/C++** extension and the **Microsoft C++ compiler**.

## Build

* The repository contains build and debug tasks suitable for VSCode, other build environments will need to set up their build chain manually.
* You need to set up the [Microsoft C++ compiler](https://code.visualstudio.com/docs/cpp/config-msvc) and may have to adapt the path in *open_vscode.bat* to open your dev enironment.
* With VSCode, you can then just run the default build/debug task on *client.c* or *server.c* in their respective subfolders.

## Run

* When using the VSCode build tasks, executables will be placed in **out** subfolders.

## License

MIT, see **LICENSE** file.
