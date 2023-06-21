// #include <iostream>
#include <cstddef>
#include <assert.h>
#include <limits.h>

#include "ants.h"
#include "inout.h"
#include "utilities.h"
#include "thop.h"
#include "ls.h"

#include "adaptive_evaporation.hpp"
#include "node_clustering.h"
#include "es_ant.hpp"
#include "acothop.hpp"
#include "custom_strategy.hpp"
#include "custom_strategy.cpp"

#define SEED_IDX 0
#define PAR_A_IDX 1
#define PAR_B_IDX 2
#define PAR_C_IDX 3
#define Q_0_IDX 4
#define ALPHA_IDX 5
#define BETA_IDX 6
#define ES_ANT_DIM 7

bool es_ant_flag = true;
double par_a, par_b, par_c;
size_t min_num_ants;

size_t current_ant_idx = 0;
std::array<double, ES_ANT_DIM> lbounds, ubounds;
OPTIMIZER *optim_ptr;
double best_iteration_alpha, best_iteration_beta;
long int best_iteration_fitness;

template <class TNumeric>
double normalize(const TNumeric &value, const TNumeric &lower_bound, const TNumeric &upper_bound)
{
	return (value - lower_bound) * 1.0 / (upper_bound - lower_bound);
}

template <class TNumeric>
TNumeric restore_scale(const TNumeric &normalized_value, const TNumeric &lower_bound, const TNumeric &upper_bound)
{
	return normalized_value * (upper_bound - lower_bound) + lower_bound;
}

void round_seed_candidates(dMat &candidates)
{
	size_t i;

	for (i = 0; i < candidates.cols(); i++)
	{
		auto &seed = candidates.col(i)[SEED_IDX];
		seed = restore_scale(seed, lbounds[SEED_IDX], ubounds[SEED_IDX]);
		seed = round(seed);
		seed = normalize(seed, lbounds[SEED_IDX], ubounds[SEED_IDX]);
	}
}

void clip_candidates(dMat &candidates)
{
	size_t i;

	for (i = 0; i < candidates.cols(); i++)
	{
		for (auto &value : candidates.col(i))
		{

			value = std::max(value, 0.0);
			value = std::min(value, 1.0);
		}
	}
}

void repair_candidates(dMat &candidates)
{
	clip_candidates(candidates);
	round_seed_candidates(candidates);
}

void an_ant_run()
{

	size_t i;	   /* counter variable */
	long int step; /* counter of the number of construction steps */

	ant_empty_memory(&ant[current_ant_idx]);

	/* Place the ants at initial city 0 and set the final city as n-1 */
	ant[current_ant_idx].tour_size = 1;
	ant[current_ant_idx].tour[0] = 0;
	ant[current_ant_idx].visited[0] = TRUE;
	ant[current_ant_idx].visited[instance.n - 1] = TRUE;

	step = 0;
	while (step < instance.n - 2)
	{
		step++;
		if (ant[current_ant_idx].tour[ant[current_ant_idx].tour_size - 1] == instance.n - 2)
		{ /* previous city is the last one */
			continue;
		}
		neighbour_choose_and_move_to_next(&ant[current_ant_idx], step);
		if (acs_flag)
			local_acs_pheromone_update(&ant[current_ant_idx], step);
		ant[current_ant_idx].tour_size++;
	}

	ant[current_ant_idx].tour[ant[current_ant_idx].tour_size++] = instance.n - 1;
	ant[current_ant_idx].tour[ant[current_ant_idx].tour_size++] = ant[current_ant_idx].tour[0];
	for (i = ant[current_ant_idx].tour_size; i < instance.n; i++)
		ant[current_ant_idx].tour[i] = 0;
	ant[current_ant_idx].fitness = compute_fitness(ant[current_ant_idx].tour, ant[current_ant_idx].visited, ant[current_ant_idx].tour_size, ant[current_ant_idx].packing_plan);
	if (acs_flag)
		local_acs_pheromone_update(&ant[current_ant_idx], ant[current_ant_idx].tour_size - 1);

	n_tours += 1;
}

void an_ant_local_search()
{
	switch (ls_flag)
	{
	case 1:
		two_opt_first(ant[current_ant_idx].tour, ant[current_ant_idx].tour_size); /* 2-opt local search */
		break;
	case 2:
		two_h_opt_first(ant[current_ant_idx].tour, ant[current_ant_idx].tour_size); /* 2.5-opt local search */
		break;
	case 3:
		three_opt_first(ant[current_ant_idx].tour, ant[current_ant_idx].tour_size); /* 3-opt local search */
		break;
	default:
		fprintf(stderr, "type of local search procedure not correctly specified\n");
		exit(1);
	}
	ant[current_ant_idx].fitness = compute_fitness(ant[current_ant_idx].tour, ant[current_ant_idx].visited, ant[current_ant_idx].tour_size, ant[current_ant_idx].packing_plan);
}

libcmaes::FitFunc es_evaluate = [](const double *x, const int N)
{
	if (termination_condition())
		return -1;

	std::vector<double> parameters(N);
	size_t i;

	for (i = 0; i < N; i++)
		parameters[i] = restore_scale(x[i], lbounds[i], ubounds[i]);

	rand_gen.seed(round(parameters[SEED_IDX]));
	par_a = parameters[PAR_A_IDX];
	par_b = parameters[PAR_B_IDX];
	par_c = parameters[PAR_C_IDX];
	q_0 = parameters[Q_0_IDX];
	alpha = parameters[ALPHA_IDX];
	beta = parameters[BETA_IDX];

	an_ant_run();

	if (ls_flag > 0)
	{
		copy_from_to(&ant[current_ant_idx], &prev_ls_ant[current_ant_idx]);
		an_ant_local_search();
		{
			if (ant[current_ant_idx].fitness > prev_ls_ant[current_ant_idx].fitness)
			{
				copy_from_to(&prev_ls_ant[current_ant_idx], &ant[current_ant_idx]);
			}
		}
	}

	if (ant[current_ant_idx].fitness < best_iteration_fitness)
	{
		best_iteration_fitness = ant[current_ant_idx].fitness;
		best_iteration_alpha = alpha;
		best_iteration_beta = beta;
	}

	current_ant_idx += 1;
	return ant[current_ant_idx - 1].fitness;
};

void es_ant_set_default(void)
{
	max_packing_tries = 1;
	acs_flag = FALSE;
	ls_flag = 1;
	par_a = par_b = par_c = 0.5;
	q_0 = 0;

	// temp
	alpha = 1.550208;
	beta = 4.893958;
	min_num_ants = n_ants * 8 / 10;
	adaptive_evaporation_flag = false;
	rho = 0.468542;
	node_clustering_flag = FALSE;
	////
}

void init_optimizer(void)
{
	const long int ALGO_CODE = aBIPOP_CMAES, NRESTARTS = INT_MAX;
	size_t i;
	std::vector<double> x0(ES_ANT_DIM), sigma(ES_ANT_DIM);

	lbounds[SEED_IDX] = rand_gen.min();
	ubounds[SEED_IDX] = rand_gen.max();
	x0[SEED_IDX] = (ubounds[SEED_IDX] - lbounds[SEED_IDX]) / 2.0;
	sigma[SEED_IDX] = 0.05;

	lbounds[PAR_A_IDX] = 0.01;
	ubounds[PAR_A_IDX] = 1;
	x0[PAR_A_IDX] = par_a;
	sigma[PAR_A_IDX] = 0.05;

	lbounds[PAR_B_IDX] = 0.01;
	ubounds[PAR_B_IDX] = 1;
	x0[PAR_B_IDX] = par_b;
	sigma[PAR_B_IDX] = 0.05;

	lbounds[PAR_C_IDX] = 0.01;
	ubounds[PAR_C_IDX] = 1;
	x0[PAR_C_IDX] = par_c;
	sigma[PAR_C_IDX] = 0.05;

	lbounds[Q_0_IDX] = 0;
	ubounds[Q_0_IDX] = 0.99;
	x0[Q_0_IDX] = q_0;
	sigma[Q_0_IDX] = 0.05;

	lbounds[ALPHA_IDX] = 0.01;
	ubounds[ALPHA_IDX] = 10;
	x0[ALPHA_IDX] = alpha;
	sigma[ALPHA_IDX] = 1.523180 / (ubounds[ALPHA_IDX] - lbounds[ALPHA_IDX]);

	lbounds[BETA_IDX] = 0.01;
	ubounds[BETA_IDX] = 10;
	x0[BETA_IDX] = beta;
	sigma[BETA_IDX] = 2.067786 / (ubounds[BETA_IDX] - lbounds[BETA_IDX]);

	for (i = 0; i < ES_ANT_DIM; i++)
	{
		assert(x0[i] >= lbounds[i]);
		assert(x0[i] <= ubounds[i]);
		x0[i] = normalize(x0[i], lbounds[i], ubounds[i]);
	}
	PARAMETER<GENO_PHENO> cmaparams(x0, sigma, -1, std::vector<double>(), std::vector<double>(), rand_gen());
	cmaparams.set_algo(ALGO_CODE);
	cmaparams.set_mt_feval(false);
	cmaparams.set_restarts(NRESTARTS);
	optim_ptr = new OPTIMIZER(es_evaluate, cmaparams);
}

void es_ant_init(void)
{
	es_ant_set_default();
	init_optimizer();
}

void es_ant_construct_and_local_search(void)
{
	size_t capacity_need;
	current_ant_idx = 0;
	best_iteration_fitness = instance.UB + 1;

	while (current_ant_idx < min_num_ants or current_ant_idx == 0)
	{
		capacity_need = current_ant_idx + optim_ptr->get_lambda();
		ant.resize(capacity_need);
		prev_ls_ant.resize(capacity_need);

		optim_ptr->generation_run();
		if (termination_condition())
			return;
	}

	alpha = best_iteration_alpha;
	beta = best_iteration_beta;
	n_ants = capacity_need;
}

double make_ant_weight(size_t i, size_t j)
{
	if (!es_ant_flag)
		return total[i][j];

	return pow(pheromone[i][j], alpha) * pow(HEURISTIC(i, j), beta);
}