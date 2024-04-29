# Local-Chat

Local-Chat is a pure C "Serverless" peer-peer chat application that allows
you to chat with other people on your local network!

## Prerequisets

Local-Chat requires some external libraries to be installed on your system before usage.

### Windows

A Mingw64 enviroment with GNU make, GCC, ncurses, and pthreads installed.

### Linux

An enviroment with GNU make, GCC, ncurses, and pthreads installed.

## Compilation

Run:
``` sh
make all
```
and an executable called `app` will be created in the directory.

## Usage

``` sh
# Run the application
./app
```

When loading up the app, you will be prompted to enter your usename. This
is non binding and does not need to be unique.

In order to find clients to communicate with, you must press the `F10` key.
If your keyboard does not support this, consult your terminals handbook on how
to enter non-standard keys into the terminal. And maybe get a new keyboard,
I almost made it `F20` before remembering that most keyboards dropped `F13-F24`
many years ago.

Then begin messaging! Anytime someone else wishes to join the swarm, they will automatically
connect to you and download the history of messages. 

## Contributing

At this moment, pull requests to the main codebase NOT welcome. Feel free to fork the project,
however this is a personal project that I am using to develop my C skills. Any requests to edit
this readme, or other simple fixes that do not change how the program works, may be considered.

## Licence

[MIT](https://choosealicense.com/licenses/mit/)
