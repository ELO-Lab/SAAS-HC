import itertools
import os
import subprocess
import multiprocessing
from tqdm import tqdm
import argparse
from datetime import datetime
import yaml

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

def profit_table(solution_folder, save_folder, postfix=""):
    from pathlib import Path
    import pandas as pd
    import itertools
    import os
    from tqdm import tqdm
    import pickle

    solution_folder = Path(solution_folder)
    save_folder = Path(save_folder)

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
    number_of_runs = 30

    pbar = tqdm(
        total=len(tsp_base)
        * len(number_of_items_per_city)
        * len(knapsack_type)
        * len(knapsack_size)
        * len(maximum_travel_time)
        * number_of_runs
    )
    res_table = {}

    for _product in itertools.product(
        tsp_base,
        number_of_items_per_city,
        knapsack_type,
        knapsack_size,
        maximum_travel_time,
    ):
        col_name = "_".join(map(str, _product))
        col_values = []
        _tsp_base = _product[0]

        for repeat_time in range(1, number_of_runs + 1):
            pbar.update(1)
            repeat_time = str(repeat_time) if repeat_time >= 10 else f"0{repeat_time}"

            file_path = (
                solution_folder
                / f"{_tsp_base}-thop"
                / f"{col_name}_{repeat_time}_{postfix}.thop.sol.log"
            )
            if not os.path.isfile(file_path):
                continue

            with open(file_path, "r") as f:
                best_profit = f.readlines()[-1].split(",")[1]
                best_profit = best_profit.split(" ")[-1]
                col_values.append(int(best_profit))

        if len(col_values) == 0:
            continue
        res_table[col_name] = col_values

    pbar.close()
    res_table = pd.DataFrame(res_table)
    with open(save_folder / f"profit_table_{postfix}.pkl", "wb") as f:
        pickle.dump(res_table, f)
    with open(save_folder / f"profit_table_{postfix}.md", "w") as f:
        f.write(res_table.describe().T.to_markdown())

    return res_table


def run_command(command):
    result = subprocess.run(command, capture_output=True)
    assert (
        result.returncode == 0
    ), f"""
command:
{"$ "+' '.join(command)}
returncode: {result.returncode}
stderr:
{result.stderr.decode()}
stdout:
{result.stdout.decode()}
"""
    return result


def launcher(arg):
    global aaco_nc_flag, number_of_runs, sol_dir, debug_log, postfix
    instance_name, repetition = arg

    _random_seed = str(random_seeds[repetition])
    temp_postfix = str(repetition + 1) if repetition + 1 >= 10 else f"0{repetition+1}"
    temp_postfix += f"_{postfix}"
    command = [
        "python3",
        "run.py",
        "--experiment",
        "--sol_dir",
        sol_dir,
        "--postfix",
        temp_postfix,
        "--run_only",
        "--instance_name",
        instance_name,
        "--random_seed",
        _random_seed,
        "--chain_flag",
        f'{chain_flags}',
    ]
    if aaco_nc_flag:
        command += ["--aaco_nc"]

    stdout_log = ""
    if repetition != number_of_runs - 1:
        command += ["--silent", "2" if not debug_log else "-1"]
        if debug_log:
            print("$ " + " ".join(command))
        run_command(command)
    else:
        result = run_command(command)
        stdout_log = result.stdout.decode()

    return (instance_name, repetition, stdout_log)


def build():
    command = ["python3", "run.py", "--build_only", "--experiment"]
    print("$ " + " ".join(command))
    result = run_command(command)
    print(result.stdout.decode())


def imap_unordered_bar(func, args, total, n_processes=2):
    p = multiprocessing.Pool(n_processes)

    with tqdm(total=total) as pbar:
        for i, result in tqdm(
            enumerate(p.imap_unordered(func, args)), desc="iner", disable=True
        ):
            instance_name, repetition, stdout_log = result
            pbar.update(1)
            if repetition == number_of_runs - 1:
                pbar.write(f"{instance_name} is completed at {datetime.now()}")
                pbar.write(stdout_log)
    pbar.close()
    p.close()
    p.join()


def get_argument():
    parser = argparse.ArgumentParser()
    parser.add_argument("--aaco_nc", action="store_true")
    parser.add_argument("--sol_dir", required=True, type=str)
    parser.add_argument("--debug_log", action="store_true")
    parser.add_argument("--exist_ok", action="store_true")
    parser.add_argument("--postfix", default="", type=str)
    parser.add_argument("--chain_flags", default="", type=str)
    args = parser.parse_args()

    global aaco_nc_flag, sol_dir, debug_log, exist_ok, postfix, chain_flags
    aaco_nc_flag = args.aaco_nc
    sol_dir = args.sol_dir
    debug_log = args.debug_log
    exist_ok = args.exist_ok
    postfix = args.postfix
    chain_flags = args.chain_flags


if __name__ == "__main__":
    global number_of_runs, debug_log, sol_dir, exist_ok

    get_argument()
    assert not os.path.isfile(sol_dir)
    assert exist_ok or not (os.path.isdir(sol_dir) and len(os.listdir(sol_dir)) > 0)

    tsp_base = [
        # "eil51",
        # "pr107",
        "a280",
        # "dsj1000",
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
    number_of_runs = 5
    if debug_log:
        number_of_runs = 3

    instances = {
        "tsp_base": tsp_base,
        "number_of_items_per_city": number_of_items_per_city,
        "knapsack_type": knapsack_type,
        "knapsack_size": knapsack_size,
        "maximum_travel_time": maximum_travel_time,
        "number_of_runs": number_of_runs,
    }
    print(yaml.dump(instances))

    build()

    n_processes = max(1, multiprocessing.cpu_count() // 2)
    # if debug_log:
    #     n_processes = 1

    total = (
        len(tsp_base)
        * len(number_of_items_per_city)
        * len(knapsack_type)
        * len(knapsack_size)
        * len(maximum_travel_time)
        * number_of_runs
    )

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

    profit_table(sol_dir, sol_dir, postfix)
