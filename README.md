# performance_model
Neural network branch predictor integrated into sniper simulator


# comparison of testing data
```
./run-sniper -c gainestown.cfg test/fft/fft
```
&nbsp;|[perf_model/branch_predictor]<br>type = pentium_m|[perf_model/branch_predictor]<br>type = nn
-|:-|:-
num correct|63514|65680
num incorrect|4393|2197
misprediction rate|6.47%|3.24%
mpki|2.78|1.39
Elapsed time|49.60s|52.73s
