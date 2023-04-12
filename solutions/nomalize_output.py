import csv
from pathlib import Path
import hashlib
import base64

log_path = "aco++_o/eil51-thop/eil51_01_bsc_01_01_01.thop.sol.tries.log"

run = 0
log = []
with open(log_path, "r") as f:
    line = next(f)[:-1]
    while line.startswith("begin try"):
        run = int(line[10:-1])
        line = next(f)[:-1]
        ls = []
        while not line.startswith("end try"):
            fitness = int(line.split(",")[1])
            line = next(f)[:-1]
            tour = line 
            line = next(f)[:-1]
            packing_plan = line 
            line = next(f)[:-1]
            # print("run", run, "fitness", fitness, "tour", tour, "packing_plan", packing_plan)
            solution =  tour + packing_plan
            signature = base64.b64encode(hashlib.sha256(solution.encode()).digest()).decode()
            # print("run", type(run), "fitness", type(fitness), "tour", type(tour), "packing_plan", type(packing_plan))
            ls.append([fitness, signature])
            
        log.append(ls)
        try:
            line = next(f)[:-1]
        except:
            # print("end of file")
            pass


header = ['Run','Fitness1','Solution1','Fitness2','Solution2']
table = []
for i in range(len(log)):
    for j in range(len(log[i]) - 1):
        table.append([i+1, log[i][j][0], log[i][j][1], log[i][j+1][0], log[i][j+1][1]])

input_file = Path(log_path)

csv_file = str(input_file.parents[0]) + "/" + ".".join(input_file.name.split(".")[:-1]) + ".csv"

with open(csv_file, 'w') as csvfile: 
    # creating a csv writer object 
    csvwriter = csv.writer(csvfile) 
        
    # writing the fields 
    csvwriter.writerow(header) 
        
    # writing the data rows 
    csvwriter.writerows(table)

print(csv_file)