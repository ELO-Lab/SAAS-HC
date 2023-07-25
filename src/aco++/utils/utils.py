def check_solution(input_file, solution_file):
    import math

    EPS = 10e-4

    file = open(input_file, "r")
    lines = file.readlines()

    number_of_cities = int(lines[2].split()[-1])
    number_of_items = int(lines[3].split()[-1])
    capacity_of_knapsack = int(lines[4].split()[-1])
    max_time = float(lines[5].split()[-1])
    min_speed = float(lines[6].split()[-1])
    max_speed = float(lines[7].split()[-1])

    vertices = {}
    for line in lines[10 : 10 + number_of_cities]:
        index, pos_x, pos_y = line.split()
        vertices[int(index)] = (
            float(pos_x),
            float(pos_y),
        )

    items = {}
    for line in lines[11 + number_of_cities : 11 + number_of_cities + number_of_items]:
        index, profit, weight, id_city = line.split()
        items[int(index)] = (
            float(profit),
            float(weight),
            int(id_city),
        )

    file.close()

    is_feasible = True
    error_info = ""

    file = open(solution_file, "r")
    lines = file.readlines()
    lines[0] = lines[0].replace(",", " ").replace("[", "").replace("]", "").split()
    performed_tour = [1] + [int(city) for city in lines[0]] + [number_of_cities]

    for city in performed_tour:
        if city < 1 or city > number_of_cities:
            is_feasible = False
            error_info += "[some visited city does not exist] "
            break

    lines[1] = lines[1].replace(",", " ").replace("[", "").replace("]", "").split()
    collected_items = [int(item) for item in lines[1]]

    for item in collected_items:
        if item < 1 or item > number_of_items:
            is_feasible = False
            error_info += "[some collected item does not exist] "
            break

    set_items = {item: 1 for item in collected_items}
    if len(set_items) != len(collected_items):
        is_feasible = False
        error_info += "[there are repeated items] "

    for item in collected_items:
        if 1 <= item <= number_of_items:
            was_collected = False
            for city in performed_tour:
                if 1 <= city <= number_of_cities:
                    if items[item][2] == city:
                        was_collected = True
                        break
            if not was_collected:
                is_feasible = False
                error_info += "[the city of some collected item is not visited] "
                break

    if not is_feasible:
        print(
            "%-30s %-30s %s"
            % (input_file, solution_file, "infeasible solution " + str(error_info))
        )
        return

    accumulate_profit = [0 for _ in range(0, len(performed_tour) + 1)]
    accumulate_weight = [0 for _ in range(0, len(performed_tour) + 1)]
    for i in range(len(performed_tour)):
        for j in range(len(collected_items)):
            if items[collected_items[j]][2] == performed_tour[i]:
                accumulate_profit[i] += items[collected_items[j]][0]
                accumulate_weight[i] += items[collected_items[j]][1]

    v = (max_speed - min_speed) / capacity_of_knapsack

    prev = 1
    current_capacity_of_knapsack = 0
    current_time = 0.0
    current_profit = 0.0

    distance = 0
    for i in range(1, len(performed_tour)):
        distance += math.ceil(
            math.sqrt(
                sum(
                    (vertices[prev][k] - vertices[performed_tour[i]][k]) ** 2
                    for k in range(2)
                )
            )
        )
        current_time += math.ceil(
            math.sqrt(
                sum(
                    (vertices[prev][k] - vertices[performed_tour[i]][k]) ** 2
                    for k in range(2)
                )
            )
        ) / (max_speed - v * current_capacity_of_knapsack)
        current_capacity_of_knapsack += accumulate_weight[i]
        current_profit += accumulate_profit[i]
        prev = performed_tour[i]

    if current_capacity_of_knapsack - EPS > capacity_of_knapsack:
        is_feasible = False
        error_info += "[capacity exceeded] "

    if current_time - EPS > max_time:
        is_feasible = False
        error_info += "[time exceeded] %f %f " % (current_time, max_time)

    if is_feasible == False:
        print(error_info)

    D = distance / len(performed_tour)
    T = (current_time / max_time) * 100.0
    W = (current_capacity_of_knapsack / capacity_of_knapsack) * 100.0

    return D, T, W, current_profit


def DTW_table(
    solution_folder, instace_folder, tsp_bases=None, num_item_each_city_list=None
):
    from pathlib import Path
    import numpy as np
    import pandas as pd
    import itertools

    solution_folder = Path(solution_folder)
    instace_folder = Path(instace_folder)
    if tsp_bases == None:
        tsp_bases = ["eil51-thop", "pr107-thop", "a280-thop", "dsj1000-thop"]
    if num_item_each_city_list == None:
        num_item_each_city_list = ["01", "03", "05", "10"]

    Nss = []
    Dss = []
    Tss = []
    Wss = []

    for _tsp_base, _num_item_each_city in itertools.product(
        tsp_bases, num_item_each_city_list
    ):
        print(_tsp_base, _num_item_each_city)
        ls = list(
            (solution_folder / _tsp_base).glob(
                f"{_tsp_base.split('-')[0]}_{_num_item_each_city}_[a-z][a-z][a-z]_[0-9][0-9]*.sol"
            )
        )
        if len(ls) == 0:
            continue
        ls.sort()

        Ds = []
        Ts = []
        Ws = []
        Ps = []
        for solution_file in ls:
            name = solution_file.stem.split("_")
            res = "_".join(name[:-1]) + ".thop"
            input_file = instace_folder / _tsp_base / res
            assert solution_file.name.startswith(
                input_file.stem
            ), f"{solution_file.stem} {input_file.name}"

            D, T, W, P = check_solution(input_file, solution_file)
            Ds.append(D)
            Ts.append(T)
            Ws.append(W)
            Ps.append(P)

        npD = np.array(Ds).reshape((-1, 30))
        npT = np.array(Ts).reshape((-1, 30))
        npW = np.array(Ws).reshape((-1, 30))
        npP = np.array(Ps).reshape((-1, 30))
        index = np.argmax(npP, axis=1)

        D = np.mean([npD[i][index[i]] for i in range(index.shape[0])], axis=0)
        T = np.mean([npT[i][index[i]] for i in range(index.shape[0])], axis=0)
        W = np.mean([npW[i][index[i]] for i in range(index.shape[0])], axis=0)

        Nss.append(_tsp_base.split("-")[0] + "_" + str(_num_item_each_city))
        Dss.append(D)
        Tss.append(T)
        Wss.append(W)

    dic = {"Instance Name": Nss, "D": Dss, "T": Tss, "W": Wss}
    table = pd.DataFrame(dic)
    return table


def merge_csv(solution_folder, output_folder):
    import pandas as pd
    from pathlib import Path
    import os

    solution_folder = Path(solution_folder)
    ls = list((solution_folder).glob("**/*.csv"))
    ls.sort()

    output_folder = Path(
        "C:\\Data\\MEGA\\Projects\\Work\\Public\\acoplusplus_thop_modified\\solutions\\STN_csv"
    )
    os.makedirs(output_folder, exist_ok=True)

    for i in range(0, len(ls), 30):
        name = paths[0].name.split(".")[0][:-3] + ".csv"
        print(name)

        paths = ls[i : i + 30]
        df = pd.read_csv(paths[0])
        for j in range(1, 30):
            df2 = pd.read_csv(paths[j])
            df2.Run += j
            df = pd.concat([df, df2])

        df.to_csv(output_folder / name)


def normalize_output(solution_folder):
    from pathlib import Path

    solution_folder = Path(solution_folder)
    ls = list(solution_folder.glob("**/*.tries.log"))

    for log_path in ls:
        log_path = str(log_path)
        normalize_single_file(log_path)


def normalize_single_file(log_path):
    import csv
    from pathlib import Path
    import hashlib
    import base64

    log = []
    with open(log_path, "r") as f:
        line = next(f)[:-1]

        while line.startswith("begin try"):
            line = next(f)[:-1]
            ls = []

            while not line.startswith("end try"):
                fitness = int(line.split(",")[1])
                line = next(f)[:-1]
                tour = line
                line = next(f)[:-1]
                packing_plan = line
                line = next(f)[:-1]
                solution = tour + packing_plan

                signature = base64.b64encode(
                    hashlib.sha256(solution.encode()).digest()
                ).decode()
                ls.append([fitness, signature])

            log.append(ls)
            try:
                line = next(f)[:-1]
            except:
                pass

    header = ["Run", "Fitness1", "Solution1", "Fitness2", "Solution2"]
    table = []
    for i in range(len(log)):
        for j in range(len(log[i]) - 1):
            table.append(
                [
                    i + 1,
                    log[i][j][0],
                    log[i][j][1],
                    log[i][j + 1][0],
                    log[i][j + 1][1],
                ]
            )

    input_file = Path(log_path)
    csv_file = (
        str(input_file.parents[0])
        + "/"
        + ".".join(input_file.name.split(".")[:-1])
        + ".csv"
    )

    with open(csv_file, "w") as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(header)
        csvwriter.writerows(table)
    print(csv_file)


def profit_table(solution_folder, save_folder, postfix=""):
    from pathlib import Path
    import pandas as pd
    import itertools
    import os
    from tqdm import tqdm
    import pickle

    solution_folder = Path(solution_folder)
    save_folder = Path(save_folder)
    if len(postfix) > 0 and postfix[0] != "_":
        postfix = f"_{postfix}"
    os.makedirs(save_folder, exist_ok=True)

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
                / f"{col_name}_{repeat_time}{postfix}.thop.sol.log"
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
    with open(save_folder / f"profit_table{postfix}.pkl", "wb") as f:
        pickle.dump(res_table, f)
    # with open(save_folder / f"profit_table{postfix}.md", "w") as f:
    #     f.write(res_table.describe().T.to_markdown())
    res_table.describe().T.to_csv(save_folder / f"profit_table{postfix}.csv")

    return res_table
