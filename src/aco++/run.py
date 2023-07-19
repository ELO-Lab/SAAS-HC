import os
import shutil
import argparse
import math
from pathlib import Path
from tabulate import tabulate
import subprocess
from datetime import datetime
import multiprocessing


parameter_configurations = {
    "eil51_01_bsc": {
        "--ants": " 500",
        "--alpha": "0.66",
        "--beta": "4.42",
        "--rho": "0.90",
        "--ptries": "2",
        "--localsearch": "0",
    },
    "eil51_01_unc": {
        "--ants": "1000",
        "--alpha": "0.69",
        "--beta": "4.29",
        "--rho": "0.75",
        "--ptries": "2",
        "--localsearch": "1",
    },
    "eil51_01_usw": {
        "--ants": " 100",
        "--alpha": "0.92",
        "--beta": "2.90",
        "--rho": "0.19",
        "--ptries": "1",
        "--localsearch": "1",
    },
    "eil51_03_bsc": {
        "--ants": " 200",
        "--alpha": "0.78",
        "--beta": "3.65",
        "--rho": "0.92",
        "--ptries": "1",
        "--localsearch": "1",
    },
    "eil51_03_unc": {
        "--ants": " 200",
        "--alpha": "0.89",
        "--beta": "3.01",
        "--rho": "0.14",
        "--ptries": "1",
        "--localsearch": "0",
    },
    "eil51_03_usw": {
        "--ants": " 100",
        "--alpha": "0.80",
        "--beta": "5.26",
        "--rho": "0.84",
        "--ptries": "1",
        "--localsearch": "1",
    },
    "eil51_05_bsc": {
        "--ants": " 500",
        "--alpha": "0.79",
        "--beta": "3.12",
        "--rho": "0.35",
        "--ptries": "5",
        "--localsearch": "1",
    },
    "eil51_05_unc": {
        "--ants": " 100",
        "--alpha": "0.88",
        "--beta": "3.65",
        "--rho": "0.78",
        "--ptries": "1",
        "--localsearch": "1",
    },
    "eil51_05_usw": {
        "--ants": " 500",
        "--alpha": "0.99",
        "--beta": "1.38",
        "--rho": "0.34",
        "--ptries": "2",
        "--localsearch": "1",
    },
    "eil51_10_bsc": {
        "--ants": " 200",
        "--alpha": "0.88",
        "--beta": "5.09",
        "--rho": "0.86",
        "--ptries": "2",
        "--localsearch": "1",
    },
    "eil51_10_unc": {
        "--ants": " 100",
        "--alpha": "0.87",
        "--beta": "4.32",
        "--rho": "0.94",
        "--ptries": "1",
        "--localsearch": "1",
    },
    "eil51_10_usw": {
        "--ants": " 100",
        "--alpha": "0.86",
        "--beta": "3.89",
        "--rho": "0.59",
        "--ptries": "1",
        "--localsearch": "1",
    },
    "pr107_01_bsc": {
        "--ants": " 200",
        "--alpha": "1.64",
        "--beta": "1.67",
        "--rho": "0.18",
        "--ptries": "4",
        "--localsearch": "1",
    },
    "pr107_01_unc": {
        "--ants": "1000",
        "--alpha": "2.87",
        "--beta": "3.92",
        "--rho": "0.34",
        "--ptries": "5",
        "--localsearch": "1",
    },
    "pr107_01_usw": {
        "--ants": " 500",
        "--alpha": "0.93",
        "--beta": "2.59",
        "--rho": "0.60",
        "--ptries": "3",
        "--localsearch": "1",
    },
    "pr107_03_bsc": {
        "--ants": " 200",
        "--alpha": "1.08",
        "--beta": "2.90",
        "--rho": "0.36",
        "--ptries": "4",
        "--localsearch": "1",
    },
    "pr107_03_unc": {
        "--ants": " 500",
        "--alpha": "1.08",
        "--beta": "5.43",
        "--rho": "0.46",
        "--ptries": "3",
        "--localsearch": "1",
    },
    "pr107_03_usw": {
        "--ants": " 500",
        "--alpha": "0.92",
        "--beta": "2.35",
        "--rho": "0.68",
        "--ptries": "3",
        "--localsearch": "1",
    },
    "pr107_05_bsc": {
        "--ants": "1000",
        "--alpha": "1.38",
        "--beta": "2.60",
        "--rho": "0.29",
        "--ptries": "2",
        "--localsearch": "1",
    },
    "pr107_05_unc": {
        "--ants": "1000",
        "--alpha": "3.54",
        "--beta": "4.54",
        "--rho": "0.20",
        "--ptries": "3",
        "--localsearch": "1",
    },
    "pr107_05_usw": {
        "--ants": " 500",
        "--alpha": "1.00",
        "--beta": "1.73",
        "--rho": "0.51",
        "--ptries": "2",
        "--localsearch": "1",
    },
    "pr107_10_bsc": {
        "--ants": " 100",
        "--alpha": "0.87",
        "--beta": "2.97",
        "--rho": "0.74",
        "--ptries": "5",
        "--localsearch": "1",
    },
    "pr107_10_unc": {
        "--ants": " 200",
        "--alpha": "0.91",
        "--beta": "2.80",
        "--rho": "0.63",
        "--ptries": "4",
        "--localsearch": "1",
    },
    "pr107_10_usw": {
        "--ants": "  20",
        "--alpha": "0.90",
        "--beta": "3.20",
        "--rho": "0.12",
        "--ptries": "3",
        "--localsearch": "1",
    },
    "a280_01_bsc": {
        "--ants": " 200",
        "--alpha": "0.71",
        "--beta": "6.93",
        "--rho": "0.42",
        "--ptries": "2",
        "--localsearch": "0",
    },
    "a280_01_unc": {
        "--ants": " 200",
        "--alpha": "1.16",
        "--beta": "1.79",
        "--rho": "0.42",
        "--ptries": "3",
        "--localsearch": "1",
    },
    "a280_01_usw": {
        "--ants": " 200",
        "--alpha": "0.84",
        "--beta": "3.83",
        "--rho": "0.39",
        "--ptries": "2",
        "--localsearch": "0",
    },
    "a280_03_bsc": {
        "--ants": " 100",
        "--alpha": "0.67",
        "--beta": "7.12",
        "--rho": "0.53",
        "--ptries": "1",
        "--localsearch": "0",
    },
    "a280_03_unc": {
        "--ants": " 200",
        "--alpha": "0.75",
        "--beta": "4.10",
        "--rho": "0.74",
        "--ptries": "5",
        "--localsearch": "1",
    },
    "a280_03_usw": {
        "--ants": "  20",
        "--alpha": "0.80",
        "--beta": "5.83",
        "--rho": "0.25",
        "--ptries": "2",
        "--localsearch": "1",
    },
    "a280_05_bsc": {
        "--ants": "1000",
        "--alpha": "0.66",
        "--beta": "6.46",
        "--rho": "0.31",
        "--ptries": "1",
        "--localsearch": "0",
    },
    "a280_05_unc": {
        "--ants": " 200",
        "--alpha": "0.84",
        "--beta": "5.89",
        "--rho": "0.24",
        "--ptries": "2",
        "--localsearch": "1",
    },
    "a280_05_usw": {
        "--ants": " 200",
        "--alpha": "0.86",
        "--beta": "7.94",
        "--rho": "0.42",
        "--ptries": "2",
        "--localsearch": "1",
    },
    "a280_10_bsc": {
        "--ants": " 200",
        "--alpha": "0.77",
        "--beta": "5.71",
        "--rho": "0.27",
        "--ptries": "2",
        "--localsearch": "1",
    },
    "a280_10_unc": {
        "--ants": " 200",
        "--alpha": "0.86",
        "--beta": "6.12",
        "--rho": "0.30",
        "--ptries": "3",
        "--localsearch": "1",
    },
    "a280_10_usw": {
        "--ants": " 200",
        "--alpha": "0.77",
        "--beta": "6.30",
        "--rho": "0.35",
        "--ptries": "2",
        "--localsearch": "1",
    },
    "dsj1000_01_bsc": {
        "--ants": " 200",
        "--alpha": "4.30",
        "--beta": "9.18",
        "--rho": "0.27",
        "--ptries": "4",
        "--localsearch": "1",
    },
    "dsj1000_01_unc": {
        "--ants": "  50",
        "--alpha": "8.33",
        "--beta": "6.39",
        "--rho": "0.10",
        "--ptries": "5",
        "--localsearch": "1",
    },
    "dsj1000_01_usw": {
        "--ants": " 100",
        "--alpha": "2.49",
        "--beta": "6.19",
        "--rho": "0.37",
        "--ptries": "5",
        "--localsearch": "1",
    },
    "dsj1000_03_bsc": {
        "--ants": "  50",
        "--alpha": "0.99",
        "--beta": "5.16",
        "--rho": "0.81",
        "--ptries": "4",
        "--localsearch": "1",
    },
    "dsj1000_03_unc": {
        "--ants": " 100",
        "--alpha": "2.71",
        "--beta": "8.81",
        "--rho": "0.25",
        "--ptries": "2",
        "--localsearch": "1",
    },
    "dsj1000_03_usw": {
        "--ants": " 100",
        "--alpha": "0.90",
        "--beta": "5.03",
        "--rho": "0.84",
        "--ptries": "5",
        "--localsearch": "1",
    },
    "dsj1000_05_bsc": {
        "--ants": "  20",
        "--alpha": "0.88",
        "--beta": "6.10",
        "--rho": "0.69",
        "--ptries": "3",
        "--localsearch": "1",
    },
    "dsj1000_05_unc": {
        "--ants": " 100",
        "--alpha": "5.93",
        "--beta": "8.50",
        "--rho": "0.12",
        "--ptries": "5",
        "--localsearch": "1",
    },
    "dsj1000_05_usw": {
        "--ants": " 100",
        "--alpha": "0.88",
        "--beta": "6.18",
        "--rho": "0.83",
        "--ptries": "2",
        "--localsearch": "1",
    },
    "dsj1000_10_bsc": {
        "--ants": " 500",
        "--alpha": "3.20",
        "--beta": "7.89",
        "--rho": "0.21",
        "--ptries": "1",
        "--localsearch": "1",
    },
    "dsj1000_10_unc": {
        "--ants": " 500",
        "--alpha": "2.94",
        "--beta": "7.96",
        "--rho": "0.26",
        "--ptries": "1",
        "--localsearch": "1",
    },
    "dsj1000_10_usw": {
        "--ants": " 500",
        "--alpha": "4.04",
        "--beta": "7.82",
        "--rho": "0.39",
        "--ptries": "2",
        "--localsearch": "1",
    },
}


def read_arguments():
    parser = argparse.ArgumentParser()

    # run parameters
    parser.add_argument("--instance_name", type=str)
    parser.add_argument("--postfix", default="", type=str)
    parser.add_argument("--run_only", action="store_true")
    parser.add_argument("--build_only", action="store_true")
    parser.add_argument("--debug", action="store_true")
    parser.add_argument("--experiment", action="store_true")
    parser.add_argument("--silent", default=0, type=int)
    parser.add_argument("--exec", type=str)
    parser.add_argument("--sol_dir", type=str)
    parser.add_argument("--acopp_dir", default="./", type=str)
    parser.add_argument("--bypass_exist", action="store_true")
    parser.add_argument("--max_time", type=float)

    # aco++ parameters
    parser.add_argument("--ants", type=int)
    parser.add_argument("--alpha", type=float)
    parser.add_argument("--beta", type=float)
    parser.add_argument("--rho", type=float)
    parser.add_argument("--q0", type=float)
    parser.add_argument("--ptries", type=int)
    parser.add_argument("--localsearch", type=int)
    parser.add_argument("--time", type=float)
    parser.add_argument("--random_seed", default=0, type=float)
    parser.add_argument("--not_mmas", action="store_true")
    parser.add_argument("--tries", default=1, type=int)
    parser.add_argument("--no_default", action="store_true")

    # aaco_ncparameters
    parser.add_argument("--aaco_nc", action="store_true")
    parser.add_argument("--adapt_evap", action="store_true")
    parser.add_argument("--nodeclustering", action="store_true")
    parser.add_argument("--n_cluster", default=1000, type=int)
    parser.add_argument("--cluster_size", default=16, type=int)
    parser.add_argument("--sector", default=8, type=int)

    # log parameters
    parser.add_argument("--realtime_terminal_log", action="store_true")
    parser.add_argument("--log_iter", action="store_true")
    parser.add_argument("--save_ter_log", type=str)
    parser.add_argument("--no_log", action="store_true")

    # chain flags
    parser.add_argument("--chain_flags", default="", type=str)

    args = parser.parse_args()

    global instance_name
    instance_name = args.instance_name

    global postfix
    postfix = args.postfix

    global run_only
    run_only = args.run_only

    global build_only
    build_only = args.build_only

    global debug
    debug = args.debug

    global experiment
    experiment = args.experiment

    global silent
    silent = args.silent

    global executable_path
    executable_path = args.exec

    global sol_dir
    sol_dir = args.sol_dir

    global acopp_dir
    acopp_dir = args.acopp_dir

    global ants
    ants = args.ants

    global alpha
    alpha = args.alpha

    global beta
    beta = args.beta

    global rho
    rho = args.rho

    global q0
    q0 = args.q0

    global ptries
    ptries = args.ptries

    global localsearch
    localsearch = args.localsearch

    global time
    time = args.time

    global random_seed
    random_seed = args.random_seed

    global not_mmas
    not_mmas = args.not_mmas

    global tries
    tries = args.tries

    global aaco_nc
    aaco_nc = args.aaco_nc

    global adapt_evap
    adapt_evap = args.adapt_evap

    global nodeclustering
    nodeclustering = args.nodeclustering

    global n_cluster
    n_cluster = args.n_cluster

    global cluster_size
    cluster_size = args.cluster_size

    global sector
    sector = args.sector

    global log_iter
    log_iter = args.log_iter

    global save_ter_log
    save_ter_log = args.save_ter_log

    global no_log
    no_log = args.no_log

    global chain_flags
    chain_flags = args.chain_flags

    global no_default
    no_default = args.no_default

    global bypass_exist
    bypass_exist = args.bypass_exist

    global realtime_terminal_log
    realtime_terminal_log = args.realtime_terminal_log

    if realtime_terminal_log:
        silent = 2


def preprocess_arguments():
    global acopp_dir
    acopp_dir = Path(acopp_dir)

    global instance_name
    if instance_name:
        if instance_name[-5:] == ".thop":
            instance_name = instance_name
        else:
            instance_name = instance_name + ".thop"

    global postfix
    if postfix:
        if postfix[0] == "_":
            postfix = postfix
        else:
            postfix = "_" + postfix

    global executable_path
    if experiment:
        executable_path = f"{acopp_dir}/acothop_experiment"
    if executable_path is None:
        executable_path = f"{acopp_dir}/acothop{postfix}"

    global nodeclustering, adapt_evap, not_mmas
    if aaco_nc:
        nodeclustering = True
        adapt_evap = True
    if adapt_evap:
        not_mmas = False

    global sol_dir
    sol_dir = sol_dir if sol_dir else Path(f"{acopp_dir}/../../solutions/temp/aco++")


def load_default_hyperparams():
    global tsp_base, number_of_items_per_city, knapsack_type, parameter_configuration_key
    global input_path, output_path

    tsp_base = instance_name.split("_")[0]
    number_of_items_per_city = int(instance_name.split("_")[1])
    knapsack_type = instance_name.split("_")[2]
    parameter_configuration_key = "%s_%02d_%s" % (
        tsp_base,
        number_of_items_per_city,
        knapsack_type,
    )
    input_path = f"{acopp_dir}/../../instances/{tsp_base}-thop/{instance_name}"
    output_path = Path(
        f"{sol_dir}/{tsp_base}-thop/{instance_name[:-5]}{postfix}.thop.sol"
    )

    global time
    if time:
        time = time
    else:
        time = float(1) * math.ceil(
            (int("".join(filter(lambda x: x.isdigit(), tsp_base))) - 2)
            * number_of_items_per_city
            / 10.0
        )

    if not no_default:
        global ants, alpha, beta, rho, ptries, localsearch
        ants = (
            ants
            if ants
            else float(parameter_configurations[parameter_configuration_key]["--ants"])
        )
        alpha = (
            alpha
            if alpha
            else float(parameter_configurations[parameter_configuration_key]["--alpha"])
        )
        beta = (
            beta
            if beta
            else float(parameter_configurations[parameter_configuration_key]["--beta"])
        )
        rho = (
            rho
            if rho
            else float(parameter_configurations[parameter_configuration_key]["--rho"])
        )
        ptries = (
            ptries
            if ptries
            else int(parameter_configurations[parameter_configuration_key]["--ptries"])
        )
        localsearch = (
            localsearch
            if localsearch
            else int(
                parameter_configurations[parameter_configuration_key]["--localsearch"]
            )
        )


def check_validation():
    assert not (run_only and build_only)
    assert not (no_log and (log_iter or save_ter_log))
    assert not (experiment and debug)
    assert not (experiment and (not run_only and not build_only))
    assert not (experiment and (run_only and not no_log and not sol_dir))
    assert os.path.isdir(acopp_dir)


def table_log():
    configurations = [
        [
            "ants",
            "alpha",
            "beta",
            "rho",
            "q_0",
            "ptries",
            "localsearch",
            "random seed",
            "time limit",
        ],
        [ants, alpha, beta, rho, q0, ptries, localsearch, random_seed, time],
    ]
    instance_info = [
        ["tsp base", "number of items per city", "knapsack type"],
        [tsp_base, number_of_items_per_city, knapsack_type],
    ]
    aaco_nc_config = [
        ["adapt_evap", "nodeclustering", "sector", "cluster_size", "n_cluster"],
        [adapt_evap, nodeclustering, sector, cluster_size, n_cluster],
    ]
    print(tabulate(instance_info, headers="firstrow", tablefmt="fancy_grid"))
    print(tabulate(configurations, headers="firstrow", tablefmt="fancy_grid"))
    print(tabulate(aaco_nc_config, headers="firstrow", tablefmt="fancy_grid"))


def build():
    if not experiment:
        build_dir = f"{acopp_dir}/build"
    else:
        build_dir = f"{acopp_dir}/temp_build_experiment"

    generator = 'Unix Makefiles'
    
    if realtime_terminal_log:
        generator = '"' + generator + '"'

    command = [
        "cmake",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE",
        "-G",
        generator,
        # f"{'Ninja' if not debug else 'Unix Makefiles'}",
        f"-S{acopp_dir}",
        f"-B{build_dir}",
        f"-DCMAKE_BUILD_TYPE:STRING={'Release' if not debug else 'Debug'}",
    ]
    if silent <= 0:
        print("$ " + " ".join(command))
    result = run_command(command)
    if silent <= 0:
        print(result.stdout.decode())

    n_processes = max(1, multiprocessing.cpu_count() // 2)
    command = ["cmake", "--build", build_dir, "-j", str(n_processes)]
    if silent <= 0:
        print("$ " + " ".join(command))
    result = run_command(command)
    if silent <= 0:
        print(result.stdout.decode())

    shutil.copy(f"{build_dir}/acothop", executable_path)


def format_aco_command():
    command = [
        executable_path,
        "--tries",
        tries,
        "--time",
        time,
        "--inputfile",
        input_path,
    ]

    if random_seed > 0:
        command += [
            "--seed",
            random_seed,
        ]

    if ants:
        command += [
            "--ants",
            ants,
        ]
    if alpha:
        command += [
            "--alpha",
            alpha,
        ]
    if beta:
        command += [
            "--beta",
            beta,
        ]
    if rho:
        command += [
            "--rho",
            rho,
        ]
    if ptries:
        command += [
            "--ptries",
            ptries,
        ]
    if localsearch:
        command += [
            "--localsearch",
            localsearch,
        ]
    if not no_log:
        command += [
            "--outputfile",
            output_path,
            "--log",
        ]
    if q0 != None:
        command += ["--q0", q0]
    if not not_mmas and not no_default:
        command += ["--mmas"]
    if adapt_evap:
        command += ["--adapt_evap"]
    if nodeclustering:
        command += [
            "--nodeclustering",
            "--sector",
            sector,
            "--clustersize",
            cluster_size,
            "--n_cluster",
            n_cluster,
        ]
    if log_iter:
        command += ["--logiter"]
    if chain_flags:
        command += chain_flags.split()

    command = list(map(str, command))

    return command


def run_command(command):
    if (realtime_terminal_log):
        print('>', ' '.join(command))
        result = os.system(' '.join(command))
        result = 'hi'
        return result
    else:
        result = subprocess.run(command, capture_output=True)
    assert (
        result.returncode == 0
    ), f"""
command:
{"$ " + ' '.join(command)}
returncode: {result.returncode}
stderr:
{result.stderr.decode()}
stdout:
{result.stdout.decode()}
"""
    return result


if __name__ == "__main__":
    read_arguments()

    check_validation()

    preprocess_arguments()

    if not run_only:
        build()
    if build_only:
        exit(0)

    load_default_hyperparams()

    if (os.path.exists(output_path) and bypass_exist) or (max_time and time > max_time):
        print(f"bypass {output_path.stem}")
        exit(0)

    if silent <= 0 and not no_default:
        table_log()

    command = format_aco_command()
    if silent <= 0:
        print("$ " + " ".join(command))

    os.makedirs(output_path.parent, exist_ok=True)
    start = datetime.now()
    result = run_command(command)
    end = datetime.now()

    if silent > 1:
        exit()

    stdout_log = result.stdout.decode()
    best_profit = str(stdout_log).split(": ")[-1]

    if silent <= -1:
        print("stdout:")
        print(stdout_log)
    if silent <= 0:
        print(f"Start at {start}")
        print(f"End at {end}")
        print(f"Run in {end - start}")
        print("Best profit:", best_profit)
    if silent == 1:
        print(best_profit)

    if save_ter_log:
        with open(save_ter_log, "w") as f:
            f.write(str(stdout_log))
