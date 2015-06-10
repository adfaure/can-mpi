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
