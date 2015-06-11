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

# pour builder l'appli sans builder ni executer les tests (snif)
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
