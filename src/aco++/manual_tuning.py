import itertools
import multiprocessing
import subprocess
from tqdm import tqdm
import argparse

# clustersize_ls = [4,8,16,24,32]
# sector_ls = [1,8,12,16]


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--instance_name", type=str)
    args = parser.parse_args()

    clustersize_ls = [4,8,16,24,32,64]
    sector_ls = [24,32,64]

    best = {
        "sector": -1,
        "clustersize": -1,
        "score": -1,
    }

    instance_name = ''
    if args.instance_name:
        if args.instance_name[-5:] == ".thop":
            instance_name = args.instance_name
        else:
            instance_name = args.instance_name + ".thop"

    print(instance_name)
    def run(sector, cluster_size):
        command = f"python3 run.py --tries 30 --instance_name {instance_name} --nodeclustering --sector {sector} --clustersize {cluster_size} --postfix node_clustering --run_only --silent"
        returned_output = subprocess.check_output(command, shell=True)

        score = int(str(returned_output)[2:-3])
        if score > best["score"]:
            best["score"] = score
            best["sector"] = sector
            best["clustersize"] = cluster_size
        print(sector, cluster_size, score)

    pool = multiprocessing.Pool(processes=max(1, multiprocessing.cpu_count() // 2))

    for sector, cluster_size in tqdm(itertools.product(sector_ls, clustersize_ls)):
        # score = pool.apply_async(run, args=(sector, cluster_size))
        run(sector, cluster_size)

    print("best:", best)
