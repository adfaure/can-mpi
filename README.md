# pour builder et tester:
```
make
```

# Pour builder et executer les tests
```
make runtest
```

# Pour builder sans executer les tests
```
make test
```

# pour builder l'appli ni executer les tests (snif)
```
make mpi_can
```

# execution avec prompt :
```
mpirun -np 7 ./mpi_can debug
```

# execution sans prompt :
```
mpirun -np 7 ./mpi_can
```

# execution avec prompt et en fixant une seed spécifique au générateur pseudo-aléatoire pour pouvoir reproduire les résultats (et les bugs ^^):
```
mpirun -np 7 ./mpi_can seed 42 debug
```

# execution sans prompt et en fixant une seed spécifique au générateur pseudo-aléatoire pour pouvoir reproduire les résultats (et les bugs ^^)::
```
mpirun -np 7 ./mpi_can seed 42
```

# Prompt:
```
> status     : show log about the state of the DHT
> insert 2   : insert the node 2 in the overlay
> insert all : insert all nodes in the overlay
> log        : add a textual/SVG log on logs/ directory
```
