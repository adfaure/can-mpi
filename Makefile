all: mpi_can runtest

mpi_can: obj/utils.o obj/can_communication.o obj/cartesian_space.o
	mpicc obj/*.o src/mpi_can.c -o mpi_can -lm

obj/utils.o: src/utils.c
	mpicc -c src/utils.c -o obj/utils.o -lm

obj/can_communication.o: src/can_communication.c
	mpicc -c src/can_communication.c -o obj/can_communication.o -lm

obj/cartesian_space.o: src/cartesian_space.c
	mpicc -c src/cartesian_space.c -o obj/cartesian_space.o -lm

test: obj/utils.o obj/can_communication.o obj/cartesian_space.o
	mpicc obj/*.o test/cartesian_space_test.c -o test/exe/cartesian_space_test -lm -lcunit

runtest: test obj/utils.o obj/can_communication.o obj/cartesian_space.o
	test/exe/cartesian_space_test
clean:
		rm -f obj/*.o test/exe/* mpi_can
