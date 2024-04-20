##
# CNIT315 Final Project
#
# @file
# @version 0.1

CFLAGS = -g -Wall -Werror -Wextra -Wpedantic -Wvla -Wnull-dereference -Wswitch-enum
CLIBS  = -lpthread -lpanelw -lncursesw -DNCURSES_STATIC


all: app

ifeq ($(OS),Windows_NT)
	CLIBS += -lws2_32
endif

app: server.o ui.o main.c global.o
	gcc $(CFLAGS) main.c -o app message.o user.o server.o ui.o uuid.o messagelist.o userlist.o log.o global.o $(CLIBS)

server.o: server.c message.o messagelist.o log.o server.h global.h
	gcc $(CFLAGS) -c -o server.o server.c

ui.o: ui.c log.o ui.h user.o userlist.o message.o messagelist.o global.h
	gcc $(CFLAGS) -c ui.c -o ui.o

message.o: message.c uuid.o message.h
	gcc $(CFLAGS) -c message.c -o message.o

user.o: user.c uuid.o user.h
	gcc $(CFLAGS) -c user.c -o user.o

uuid.o: uuid.c uuid.h
	gcc $(CFLAGS) -c uuid.c -o uuid.o

messagelist.o: messagelist.c message.o messagelist.h
	gcc $(CFLAGS) -c messagelist.c -o messagelist.o

userlist.o: userlist.c user.o userlist.h
	gcc $(CFLAGS) -c userlist.c -o userlist.o

log.o: log.c log.h
	gcc $(CFLAGS) -c log.c -o log.o

global.o: global.c global.h
	gcc $(CFLAGS) -c global.c -o global.o


# end
