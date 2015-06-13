# pour builder et tester:

    make


# Pour builder et executer les tests

    make runtest


# Pour builder sans executer les tests

    make test


# pour builder l'appli sans executer les tests  :'(

    make mpi_can


# execution avec prompt :

    mpirun -np 7 ./mpi_can debug


# execution sans prompt :

    mpirun -np 7 ./mpi_can


# Execution with yield
    mpirun --mca mpi_yield_when_idle 1 -np 10 mpi_can

# execution avec prompt et en fixant une seed spécifique au générateur pseudo-aléatoire pour pouvoir reproduire les résultats (et les bugs ^^):

    mpirun -np 7 ./mpi_can seed 42 debug


# execution sans prompt et en fixant une seed spécifique au générateur pseudo-aléatoire pour pouvoir reproduire les résultats (et les bugs ^^)::

    mpirun -np 7 ./mpi_can seed 42


# Prompt:

    -- 9 nodes availables --
    > status             : show log about the state of the DHT
    > insert 2           : insert the node 2 in the overlay
    > insert all         : insert all nodes in the overlay and run etape 3
    > log                : add a textual/SVG log on logs/ directory
    > put <x> <y> <data> : put <data> in position (x, y)
    > get <x> <y>        : get a data from position (x, y)
    > shuffle <nb>       : randomly insert <nb> random data
    > rm <x> <y>         : remove a data in position (x, y)
    > etape3 <x>         : insert <x> element and try to retrieve the 5 last and first
