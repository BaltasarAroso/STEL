## STEL
STEL (Sistemas de Telecomunicações) means **Telecommunication Systems** and it is a Curricular Unit of the MSc in Eletrical and Computer Engineering in FEUP.

## 1 - Simulation of a Poisson call arrival process
Consider a traffic source that generates calls with a given arrival rate lambda, according to a Poisson process.
* a) Based on the simulation method for discrete events, and given that the distribution of the interval between call arrivals is exponential, develop a simulation program for this source, enabling it to obtain: the histogram of the interval between the arrival of consecutive calls and the estimator of the average value of the interval between consecutive calls.
Compare the results obtained in the simulation with the theoretically predicted values, assuming a rate of lambda1 = 8 calls/second.
* b) Develop an alternative version of the previous program, based on the definition of a Poisson process, i.e., taking into account that the probability of an event in a basic time interval delta is delta x lambda.

**Note:** In the image `time_comparator.jpg` we can see that the first case is much more faster than the second one.

**Execution**
```
git clone <url>
cd 1_PoissonCalls
clang poisson_calls_a.c -o a
./a <file_a.csv>
clang poisson_calls_b.c -o b
./b <file_a.csv>
```

## 2 - Simulation of a waiting list
Considering a queue represented by Kendall's notation M / M / m / L / K / FCFS in which each field has the following meaning, successively:
- M = Exponential distribution of times between customer arrivals (arrival rate of each free customer l)
- M = Exponential distribution of service times (average service time dm)
- m = Number of simultaneous service / server channels
- L = Number of system buffer positions
- K = Population size of potential clients
- FCFS = Discipline of service "First come first served"
the code implemented as the porpused of calculate any case (Erlang-B, Erlang-C or Engset) considering the input values lambda, dm, m, L and K that are asked in the execution of the compiled file.

**Execution**
```
git clone <url>
cd 2_WaitingList
make
./queues <file_name.csv>
```
