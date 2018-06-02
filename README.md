# STEL
STEL (Sistemas de Telecomunicações) means **Telecommunication Systems** and it is a Curricular Unit of the MSc in Eletrical and Computer Engineering in FEUP.

# 1 - Simulation of a Poisson call arrival process
Consider a traffic source that generates calls with a given arrival rate lambda, according to a Poisson process.
* a) Based on the simulation method for discrete events, and given that the distribution of the interval between call arrivals is exponential, develop a simulation program for this source, enabling it to obtain: the histogram of the interval between the arrival of consecutive calls and the estimator of the average value of the interval between consecutive calls.
Compare the results obtained in the simulation with the theoretically predicted values, assuming a rate of lambda1 = 8 calls/second.
* b) Develop an alternative version of the previous program, based on the definition of a Poisson process, i.e., taking into account that the probability of an event in a basic time interval delta is delta x lambda.

**Note:** In the image `time_comparator.jpg` we can see that the first case is much more faster than the second one.

## Execution
```
git clone https://github.com/BaltasarAroso/STEL.git
cd 1_PoissonCalls
clang poisson_calls_a.c -o a
./a <file_a.csv>
clang poisson_calls_b.c -o b
./b <file_a.csv>
```

# 2 - Simulation of a waiting list
Considering a queue represented by Kendall's notation M / M / m / L / K / FCFS in which each field has the following meaning, successively:
- M = Exponential distribution of times between customer arrivals (arrival rate of each free customer l)
- M = Exponential distribution of service times (average service time dm)
- m = Number of simultaneous service / server channels
- L = Number of system buffer positions
- K = Population size of potential clients
- FCFS = Discipline of service "First come first served"

the code implemented as the porpused of calculate any case (Erlang-B, Erlang-C or Engset) considering the input values lambda, dm, m, L and K that are asked in the execution of the compiled file.

## Execution
```
git clone https://github.com/BaltasarAroso/STEL.git
cd 2_WaitingList
make
./queues <file_name.csv>
```
**Note:** After the execution of the above commands a directory is creates with the name `DotCSV` where the files _.csv_ are. In the `Graphics` and `Results` files we can see the results obtained in the different cases tested.

# 3 - Simulation of an emergency call system
Considering an emergency call system where the arrival rate is 600 calls/hour - lambda (Poisson) and taking into account that those calls follow the follwing distributions:
- 40% are serviced by the Civil Protection (PC) having a minimum duration of 1 min where is added an exponencial distribution with an average of 1.5 min, being the maximum duration of 4 min.
- 60% are urgent so are transfered to the INEM; the duration of this calls follow an gaussian distribution with an average of 45 seconds and a standard deviation of 15 seconds, being that time limited between 30 seconds and 75 seconds.

Besides that, the calls that are transfered to and answered by the INEM have a minimum duration of 1 min where is added an exponencial distribution with an average of 1.5 min. If the INEM system is full, those calls are inserted in an infinite buffer. Furthermore, the Civil Protection only hangs up a call when it is passed to the INEM. The buffer size of the Civil Protection is finite (L). Calls that came up and cannot be serviced or inserted in the buffer are lost.

Every time that a call is inserted in the Civil Protection's buffer a time prediction about the time spend in that buffer is made taking into account the system load state. That prediction is estimated, only considering the calls that was already made and not looking into the distribution behaviours.

Considering that the sizing parameters are:
- L (buffer size of the Civil Protection)
- M_PC value (number of service workstations in the Civil Protection)
- M_INEM value (number of service workstations in the INEM)
configure the system to obtain these results:
- Probability of a call being delayed at the entry of the PC system < 20%
- Probability of a call being lost at the entry to the PC system < 1 %
- Average delay time of calls that wait in the PC system entry < 30 seconds
- Average delay time of calls since they arrival at PC and are served in INEM < 60 seconds

Moreover, show the delay distribution of calls in the PC buffer in an histogram (_delay.csv_) and show the histogram that has the error prevision of the time in that buffer (_prediction.csv_). Beyond that, calculate the following variables:
- Average of the absolute error on the waiting prediction time in the input buffer
- Average of the relative error on the waiting prediction time in the input buffer
- Standard deviation of the waiting prediction time in the input buffer

To conclude, do the Sensitivity Analysis for different arrival rates (lambda) and show it in a graphic (_sensitivy.csv_), then find the confidence interval of 90% about the nominal situation of 600 calls/hour.

## Execution
```
git clone https://github.com/BaltasarAroso/STEL.git
cd 3_EmergencyCallSystem
make
./sys_calls <delay.csv> <prediction.csv>
```
**Note:** Follow the instructions shown in the terminal and verify in which names the respective _.csv_ has been saved. The _sensitivy.csv_ file is automatically created and named hardcoded and is the only _.csv_ that can not be named in the execution. The user only can decide if it is supose to do the Sensitivity Analysis (creating that file) or not.
