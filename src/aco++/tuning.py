import numpy as np
import itertools
import os
epsilon = np.array([0.0, 0.5, 0.6, 0.7, 0.8, 0.9])
levy_flight_threshold = np.arange(0, 1, 0.05, dtype=float)
Levy_flight_altering_ratio = np.arange(0, 2, 0.2, dtype=float)


while True:
    a = np.random.choice(epsilon, size=1)[0]
    b = np.random.choice(levy_flight_threshold, size=1)[0]
    c = np.random.choice(Levy_flight_altering_ratio, size=1)[0]
    command = f'./build/acothop --tries 1 --seed 269070 --time 32.0 --inputfile ./../../instances/pr107-thop/pr107_03_unc_05_02.thop --ants 500.0 --alpha 1.08 --beta 5.43 --rho 0.46 --ptries 3 --localsearch 1 --outputfile ../../solutions/temp/aco++/pr107-thop/pr107_03_unc_05_02_greedylevyacothop.thop.sol --log --mmas -G {a}:{b}:{c}'
    result = os.system(command)
    # print(result.stdout.decode())
