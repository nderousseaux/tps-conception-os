#!/bin/sh

#
# Script shell pour ajouter l'argument 0 à tous les appels de exit
# et wait dans les programmes utilisateurs de xv6.
#
# Note : ce script pourrait être plus court si tous les systèmes
# disposaient de GNU-sed (option --inplace).
#

LISTE="
	forktest.c \
	cat.c \
	echo.c \
	forktest.c \
	grep.c \
	init.c \
	kill.c \
	ln.c \
	ls.c \
	mkdir.c \
	rm.c \
	sh.c \
	stressfs.c \
	usertests.c \
	wc.c \
	zombie.c \
"

for f in $LISTE
do
    sed -e 's/exit();/exit(0);/' \
	-e 's/\([^a-z]\)wait()/\1wait(0)/' \
	$f > $f.bak && mv $f.bak $f
done

exit 0
