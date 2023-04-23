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


def DTW_table(solution_folder, instace_folder, tsp_bases=None, item_levels=None):
    from pathlib import Path
    import numpy as np
    import pandas as pd
    import itertools

    solution_folder = Path(solution_folder)
    instace_folder = Path(instace_folder)
    if tsp_bases == None:
        tsp_bases = ["eil51-thop", "pr107-thop", "a280-thop", "dsj1000-thop"]
    if item_levels == None:
        item_levels = ["01", "03", "05", "10"]

    Nss = []
    Dss = []
    Tss = []
    Wss = []

    for base, item_lv in itertools.product(tsp_bases, item_levels):
        print(base, item_lv)
        ls = list(
            (solution_folder / base).glob(
                f"{base.split('-')[0]}_{item_lv}_[a-z][a-z][a-z]_[0-9][0-9]*.sol"
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
            input_file = instace_folder / base / res
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

        Nss.append(base.split("-")[0] + "_" + str(item_lv))
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
    import csv
    from pathlib import Path
    import hashlib
    import base64
    from pathlib import Path

    solution_folder = Path(solution_folder)
    ls = list(solution_folder.glob("**/*.tries.log"))

    for log_path in ls:
        log_path = str(log_path)
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
