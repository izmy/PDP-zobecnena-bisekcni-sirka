# all after symbol '#' is comment

# === which communication library to use ===
CC	=	mpiCC      # for ethernet and infiniband networks

CFLAGS	=
LIBS	=	-lm

default:	priklad1

priklad1:main.cpp
	$(CC) $(CFLAGS) -o mpi main.cpp $(LIBS)

clear:
	rm priklad?

