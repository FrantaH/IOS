#
# Projekt: prace s processy
# Autor: Frantisek Horazny 
# 
#
AUTHOR=xhoraz02
NAME=proj2

CC=gcc # prekladac jazyka C
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic -g -Werror # parametry prekladace

ALLFILES=main.c Makefile # obsah projektu

$(NAME): main.c
	$(CC) $(CFLAGS) main.c -lrt -o $(NAME)

.PHONY: build debug pack clean clean-exe clean-all

build: $(NAME)


debug: $(NAME) #lazeni
	export XEDITOR=gvim;ddd $(NAME)

pack:	#zabaleni
	tar cvzf $(AUTHOR).tar.gz $(ALLFILES)

clean:	#vycisteni
	rm -f *~ *.bak

clean-exe:
	rm -f $(NAME)

clean-all:  #vycisteni vseho
	clean-exe clean
