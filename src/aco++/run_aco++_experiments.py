#!/usr/bin/python
# -*- coding: utf-8 -*-

import itertools
import os
import subprocess
import multiprocessing
from tqdm import tqdm
import argparse
import parser
from datetime import datetime

random_seeds = [
    269070,
    99470,
    126489,
    644764,
    547617,
    642580,
    73456,
    462018,
    858990,
    756112,
    701531,
    342080,
    613485,
    131654,
    886148,
    909040,
    146518,
    782904,
    3075,
    974703,
    170425,
    531298,
    253045,
    488197,
    394197,
    519912,
    606939,
    480271,
    117561,
    900952,
    968235,
    345118,
    750253,
    420440,
    761205,
    130467,
    928803,
    768798,
    640300,
    871462,
    639622,
    90614,
    187822,
    594363,
    193911,
    846042,
    680779,
    344008,
    759862,
    661168,
    223420,
    959508,
    62985,
    349296,
    910428,
    964420,
    422964,
    384194,
    985214,
    57575,
    639619,
    90505,
    435236,
    465842,
    102567,
    189997,
    741017,
    611828,
    699223,
    335142,
    52119,
    49256,
    324523,
    348215,
    651525,
    517999,
    830566,
    958538,
    880422,
    390645,
    148265,
    807740,
    934464,
    524847,
    408760,
    668587,
    257030,
    751580,
    90477,
    594476,
    571216,
    306614,
    308010,
    661191,
    890429,
    425031,
    69108,
    435783,
    17725,
    335928,
]


def launcher(arg):
    
    instance_name, repetition = arg
    global aaco_nc_flag, number_of_runs

    _random_seed = str(random_seeds[repetition])
    postfix = str(repetition + 1) if repetition + 1 >= 10 else f"0{repetition+1}"
    command = [
        "python3",
        "run.py",
        "--experiment",
        "--instance_name",
        instance_name,
        "--run_only",
        "--exec",
        "./acothop_experiment",
        "--random_seed",
        _random_seed,
        "--postfix",
        postfix,
    ]
    if aaco_nc_flag:
        command += ["--aaco_nc"]

    output = ''
    if repetition != number_of_runs - 1:
        command += ["--silent", "2"]
        subprocess.run(command)
    else:
        result = subprocess.run(command, stdout=subprocess.PIPE)
        output = result.stdout.decode()

    return (instance_name, repetition, output)


def clean_and_build():
    result = subprocess.run(
        "python3 ../../utils/cmake_clean.py .".split(), stdout=subprocess.PIPE
    )
    print(result.stdout.decode())
    result = subprocess.run(
        "python3 run.py --build_only --experiment".split(),
        stdout=subprocess.PIPE,
    )
    print(result.stdout.decode())
    result = subprocess.run(
        "rm -r ../../solutions/experiment/aco++".split(), stdout=subprocess.PIPE
    )
    print(result.stdout.decode())

def imap_unordered_bar(func, args, total, n_processes = 2):
    p = multiprocessing.Pool(n_processes)
    
    with tqdm(total=total, desc='outer') as pbar:
        for i, result in tqdm(enumerate(p.imap_unordered(func, args)), desc='iner', disable=True):
            instance_name, repetition, output = result
            if (repetition == number_of_runs - 1):
                pbar.write(f"{instance_name} is completed at {datetime.now()}")
                pbar.write(output)
                pbar.update(number_of_runs)
    pbar.close()
    p.close()
    p.join()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--aaco_nc", action="store_true")
    args = parser.parse_args()
    global aaco_nc_flag
    aaco_nc_flag = args.aaco_nc

    tsp_base = [
        "eil51",
        "pr107",
        "a280",
        "dsj1000",
    ]
    number_of_items_per_city = [
        "01",
        "03",
        "05",
        "10",
    ]
    knapsack_type = [
        "bsc",
        "unc",
        "usw",
    ]
    knapsack_size = [
        "01",
        "05",
        "10",
    ]
    maximum_travel_time = [
        "01",
        "02",
        "03",
    ]
    global number_of_runs
    number_of_runs = 30
    # number_of_runs = 3

    clean_and_build()

    n_processes = max(1, multiprocessing.cpu_count() // 2)

    total=len(tsp_base) \
        * len(number_of_items_per_city) \
        * len(knapsack_type) \
        * len(knapsack_size) \
        * len(maximum_travel_time) \
        * number_of_runs
    
    args = []
    for _product in itertools.product(
        tsp_base,
        number_of_items_per_city,
        knapsack_type,
        knapsack_size,
        maximum_travel_time,
    ):
        instance_name = "_".join(_product)
        for repetition in range(number_of_runs):
            args.append((instance_name, repetition))
            
    imap_unordered_bar(launcher, args, total, n_processes)
