import os
import argparse
import math
from pathlib import Path
from tabulate import tabulate
import subprocess
from datetime import datetime

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

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--ants", type=int)
    parser.add_argument("--alpha", type=float)
    parser.add_argument("--beta", type=float)
    parser.add_argument("--rho", type=float)
    parser.add_argument("--ptries", type=int)
    parser.add_argument("--localsearch", type=int)
    parser.add_argument("--time", type=float)
    parser.add_argument("--random_seed", default=269070, type=float)
    parser.add_argument("--instance_name", type=str)
    parser.add_argument("--postfix", type=str)
    parser.add_argument("--run_only", action="store_true")
    parser.add_argument("--build_only", action="store_true")
    parser.add_argument("--not_mmas", action="store_true")
    parser.add_argument("--tries", default=1, type=int)
    parser.add_argument("--nodeclustering", action="store_true")
    parser.add_argument("--adaptevapo", action="store_true")
    parser.add_argument("--aaco_nc", action="store_true")
    parser.add_argument("--sector", default=24, type=int)
    parser.add_argument("--clustersize", default=32, type=int)
    parser.add_argument("--silent", action="store_true")
    parser.add_argument("--logiter", action="store_true")

    args = parser.parse_args()
    assert not (args.run_only and args.build_only)

    if args.instance_name:
        if args.instance_name[-5:] == ".thop":
            instance_name = args.instance_name
        else:
            instance_name = args.instance_name + ".thop"
    if args.postfix:
        if args.postfix[0] == "_":
            postfix = args.postfix
        else:
            postfix = "_" + args.postfix
    else:
        postfix = ""

    if not args.run_only:
        os.system("cmake .")
        os.system("make")
        os.rename("./acothop", f"./acothop{postfix}")
    if args.build_only:
        exit(0)

    tsp_base = instance_name.split("_")[0]
    number_of_items_per_city = int(instance_name.split("_")[1])
    knapsack_type = instance_name.split("_")[2]
    parameter_configuration_key = "%s_%02d_%s" % (
        tsp_base,
        number_of_items_per_city,
        knapsack_type,
    )

    ants = (
        args.ants
        if args.ants
        else float(parameter_configurations[parameter_configuration_key]["--ants"])
    )
    alpha = (
        args.alpha
        if args.alpha
        else float(parameter_configurations[parameter_configuration_key]["--alpha"])
    )
    beta = (
        args.beta
        if args.beta
        else float(parameter_configurations[parameter_configuration_key]["--beta"])
    )
    rho = (
        args.rho
        if args.rho
        else float(parameter_configurations[parameter_configuration_key]["--rho"])
    )
    ptries = (
        args.ptries
        if args.ptries
        else int(parameter_configurations[parameter_configuration_key]["--ptries"])
    )
    localsearch = (
        args.localsearch
        if args.localsearch
        else int(parameter_configurations[parameter_configuration_key]["--localsearch"])
    )

    random_seed = args.random_seed
    if args.aaco_nc:
        nodeclustering = True
        adaptevapo = True
    else:
        nodeclustering = args.nodeclustering
        adaptevapo = args.adaptevapo
    sector = args.sector
    clustersize = args.clustersize
    if args.time:
        time = args.time
    else:
        time = float(1) * math.ceil(
            (int("".join(filter(lambda x: x.isdigit(), tsp_base))) - 2)
            * number_of_items_per_city
            / 10.0
        )

    configurations = [
        [
            "random seed",
            "ants",
            "alpha",
            "beta",
            "rho",
            "ptries",
            "localsearch",
            "time limit",
        ],
        [random_seed, ants, alpha, beta, rho, ptries, localsearch, time],
    ]
    instance_info = [
        ["tsp base", "number of items per city", "knapsack type"],
        [tsp_base, number_of_items_per_city, knapsack_type],
    ]

    if not args.silent:
        print(tabulate(instance_info, headers="firstrow", tablefmt="fancy_grid"))
        print(tabulate(configurations, headers="firstrow", tablefmt="fancy_grid"))

        if args.nodeclustering:
            nodeclustering_config = [
                ["node clustering", "sector", "cluster size"],
                [nodeclustering, sector, clustersize],
            ]
            print(
                tabulate(
                    nodeclustering_config, headers="firstrow", tablefmt="fancy_grid"
                )
            )

        if args.adaptevapo:
            adaptevapo_config = [
                [
                    "adaptive evaporation",
                    "initial evaporation rate",
                ],
                [adaptevapo, rho],
            ]
            print(
                tabulate(adaptevapo_config, headers="firstrow", tablefmt="fancy_grid")
            )

    executable_path = f"./acothop{postfix}"
    input_path = f"../../instances/{tsp_base}-thop/{instance_name}"
    output_path = Path(
        f"../../solutions/temp/aco++/{tsp_base}-thop/{instance_name[:-5]}{postfix}.thop.sol"
    )
    command = f"{executable_path} --tries {args.tries} --seed {random_seed} --time {time} --inputfile {input_path} --outputfile {output_path} --ants {ants} --alpha {alpha} --beta {beta} --rho {rho} --ptries {ptries}{f' --nodeclustering --sector {sector} --clustersize {clustersize}' if nodeclustering else ''}{' --adaptevapo' if adaptevapo else ''}{' --logiter' if args.logiter else ''}  --localsearch {localsearch} --log"
    if not args.not_mmas:
        command += " --mmas"
    if not args.silent:
        print(command)

    os.makedirs(output_path.parent, exist_ok=True)
    # os.system(command=command)
    print("start:", datetime.now())
    returned_output = subprocess.check_output(command, shell=True)
    print("end:  ", datetime.now())
    print(str(returned_output).split(": ")[1][:-3])
