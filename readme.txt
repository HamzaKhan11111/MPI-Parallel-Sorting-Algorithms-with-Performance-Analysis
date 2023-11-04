compile: mpic++ Q2.cpp -o file -lstdc++
execute: mpiexec -n 4 -f machinefile ./file
