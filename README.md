# Self-Adaptive Ant System with Hierarchical Clustering for the Thief Orienteering Problem
- This repository implements the SAAS-HC algorithm (Self-Adaptive Ant System with Hierarchical Clustering) for the Thief Orienteering problem. The algorithm explanation was presented in the paper "Self-Adaptive Ant System with Hierarchical Clustering for the Thief Orienteering Problem" by Vu Hoang Huynh, The Viet Le, and Ngoc Hoang Luong
- Furthermore, this repository provides the SAAS-HC solution for the ThOP benchmark and the solutions of previous algorithms: ILS, BRKGA, ACO, and ACO++. The implementations of these algorithms can be found in the [repository](https://github.com/jonatasbcchagas/acoplusplus_thop). All the solution files of each algorithm were stored in the /experiments folder.
## Contributor
[Vu Hoang Huynh](https://github.com/vuhh2002), [The Viet Le](https://github.com/LETHEVIET/), and Ngoc Hoang Luong
## Installation
### Install environment
- Linux OS
#### Create Conda environment and install necessary dependencies
```shell
conda create -n saas_hc python tqdm tabulate pyaml gxx cmake make mlpack cereal 
``` 
### Extract ThOP benchmark instances
```shell
tar -xzf instances/thop_instances.tar.gz -C instances/
```
### 
## Usage
### Activate Conda environment
```shell
conda activate saas_hc
```
### Run command
```shell
cd src
python run_experiments.py --sol_dir ../experiments/saas_hc
```
## Acknowledgement
- Our code is heavily inspired by this [repository](https://github.com/jonatasbcchagas/acoplusplus_thop) which is the source code of the paper ["Efficiently solving the thief orienteering problem with a max-min ant colony optimization algorithm"](https://link.springer.com/article/10.1007/s11590-021-01824-y) by Jonatas B. C. Chagas and Markus Wagner.
- Besides that, we used the K-Means library from [mlpack](https://github.com/mlpack/mlpack) and the C language implementation of the CMA-ES algorithm from this [repository](https://github.com/CMA-ES/c-cmaes) by Nikolaus Hansen
