#

CFLAGS = -g 

FILES = tpgmain.o input.o level.o sim.o podem.o test.o init_flist.o pfedfs.o display.o

atpg : $(FILES) 
	gcc $(CFLAGS) $(FILES) -lm -o atpg -I ../cudd-2.4.2/include/ -L ../cudd-2.4.2/cudd/ -L ../cudd-2.4.2/util/ -L ../cudd-2.4.2/mtr/ -L ../cudd-2.4.2/st/ -L ../cudd-2.4.2/epd/ -lcudd -lutil -lepd -lmtr -lst

tpgmain.o : atpg.h miscell.h
	gcc -c tpgmain.c
input.o : atpg.h miscell.h
	gcc -c input.c
level.o : atpg.h miscell.h
	gcc -c level.c
sim.o : atpg.h miscell.h logic_tbl.h
	gcc -c sim.c
podem.o : atpg.h miscell.h
	gcc -c podem.c
init_flist.o : atpg.h miscell.h
	gcc -c init_flist.c
pfedfs.o : atpg.h miscell.h
	gcc -c pfedfs.c
test.o : atpg.h miscell.h
	gcc -c test.c
display.o : atpg.h miscell.h
	gcc -c display.c
clean:
	rm -f atpg *.o