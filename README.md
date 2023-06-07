
# Internet Relay Chat

## What is IRC?
Internet Relay Chat is a forum made for group discussions made and popular in pre-socical media era. IRC servers usually follow TCP protocols and a tree topology. In today's world this is a very obsolete technology but it helps to learn the basics of how messages are commuted between computers connected on a local area network.

## Intended functionalities
+ build a decentralised chatting platform using socket programming in C language
+ implement commands, different chatrooms and channels in the IRC
+ handle multiple clients using a tree topology

## Environment Setup

### For windows users
+ setup Windows Subsystem for Linux (WSL) to be able to use linux command line
  + [wsl installation guide](https://learn.microsoft.com/en-us/windows/wsl/install "guide")
+ install gcc compiler since the prgram is written in c language
  + [install gcc on windows](https://www.scaler.com/topics/c/c-compiler-for-windows/)

### For MacOS and Linux
Environment setup is simple, you just need to setup gcc to compile c programs.
+ for mac, install gcc using homebrew or any other package manager (preferably) 
  - [gcc installation guide for mac](https://www.scaler.com/topics/c/c-compiler-for-windows/)
+ install linux bild essentials
  - [installation guide](https://www.makeuseof.com/install-use-build-essential-ubuntu/)


## How to run the application
+ open the directory where you've cloned the repo in your terminal or cli
+ run the following commonds for both serverside and client side code
  + to compile and run on windows
    - `gcc path/to/c/file.c -o path/to/output.exe` 
    - `.path/to/output.exe portNumber` (for severside files)
    - `./path/to/output.exe ipAddress portNumber` (for clientside files)
  + to compile and run on MacOS or Linux
    - `gcc path/to/c/file.c -o path/to/output` 
    - `.path/to/output portNumber` (for severside files)
    - `./path/to/output ipAddress portNumber` (for clientside files)

## Contributing

Contributions are always welcome!

See [contributing.md](CONTRIBUTING.md) for ways to get started.

Please adhere to this project's [code of conduct](CODE_OF_CONDUCT.md).
