.RECIPEPREFIX +=
CC  =   gcc
CFLAGS = -Wall -lm -lrt -lwiringPi -lpthread

PROG = bin/*
OBJ = obj/*

default:
    $(CC) $(CFLAGS) -c src/Monitor.c -o obj/Monitor
    $(CC) $(CFLAGS) -c src/Current_Time.c -o obj/Current_Time
    $(CC) $(CFLAGS) obj/Monitor obj/Current_Time -o bin/Clock

run:
    sudo ./bin/Clock

clean:
    rm $(PROG) $(OBJS)
