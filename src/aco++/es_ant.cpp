// #include <memory>

#include <Eigen/Dense>
#include <libcmaes/cmaes.h>

#include "ants.h"
#include "inout.h"
#include "utilities.h"
#include "thop.h"

#include "adaptive_evaporation.hpp"
#include "es_ant.hpp"
#include "node_clustering.h"

using namespace libcmaes;

#define SEED_IDX 0
#define PAR_A_IDX 1
#define PAR_B_IDX 2
#define PAR_C_IDX 3
#define Q_0_IDX 4
#define ALPHA_IDX 5
#define BETA_IDX 6
#define CLUSTER_ALPHA_IDX 7
#define CLUSTER_BETA_IDX 8
#define RHO_IDX 9

#define CMA_ALGO aBIPOP_CMAES
#define STRATEGY_CLASS BIPOPCMAStrategy<ACovarianceUpdate, GenoPheno<pwqBoundStrategy>>
#define PARAMETER_CLASS CMAParameters<GenoPheno<pwqBoundStrategy>>

double
	seed_stepsize_init,
	par_a, par_a_stepsize_init,
	par_b, par_b_stepsize_init,
	par_c, par_c_stepsize_init,
	cluster_alpha, cluster_alpha_stepsize_init,
	cluster_beta, cluster_beta_stepsize_init,
	alpha_stepsize_init, beta_stepsize_init,
	rho_stepsize_init, q_0_stepsize_init;
bool adapt_ant;
long int current_ant_idx = 0;
// long int n_generation_each_iteration;
// long int cmaes_algo;

ESOptimizer<STRATEGY_CLASS, PARAMETER_CLASS> optimizer;

void es_ant_set_default(void)
{
	seed_stepsize_init = (SEED_MAX - SEED_MIN) / 20;
	par_a = par_b = par_c = 0.5;
	par_a_stepsize_init = par_b_stepsize_init = par_c_stepsize_init = 0.15;
	q_0 = 0.45;
	q_0_stepsize_init = 0.25;
	// cmaes_algo = aBIPOP_CMAES;
#if NODE_CLUSTERING_VERSION == 1
	n_generation_each_iteration = 1;
	alpha = 1.550208;
	alpha_stepsize_init = 1.523180;
	beta = 4.893958;
	beta_stepsize_init = 2.067786;
	cluster_alpha = alpha;
	cluster_alpha_stepsize_init = alpha_stepsize_init;
	cluster_beta = beta;
	cluster_beta_stepsize_init = beta_stepsize_init;
	rho = 0.468542;
	rho_stepsize_init = 0.253226;
#endif
}

void es_ant_init(void)
{
	assert(node_clustering_flag == NODE_CLUSTERING_VERSION);
	assert(!adaptive_evaporation_flag);
	assert(max_packing_tries == 1);

	size_t i;
	const int LAMBDA = -1;
	std::vector<double> x0(ES_ANT_DIM), sigma(ES_ANT_DIM), lbounds(ES_ANT_DIM),
		ubounds(ES_ANT_DIM);

	lbounds[SEED_IDX] = SEED_MIN;
	ubounds[SEED_IDX] = SEED_MAX;
	x0[SEED_IDX] = seed;
	sigma[SEED_IDX] = seed_stepsize_init;

	lbounds[PAR_A_IDX] = 0;
	ubounds[PAR_A_IDX] = 1;
	x0[PAR_A_IDX] = par_a;
	sigma[PAR_A_IDX] = par_a_stepsize_init;

	lbounds[PAR_B_IDX] = 0;
	ubounds[PAR_B_IDX] = 1;
	x0[PAR_B_IDX] = par_b;
	sigma[PAR_B_IDX] = par_b_stepsize_init;

	lbounds[PAR_C_IDX] = 0;
	ubounds[PAR_C_IDX] = 1;
	x0[PAR_C_IDX] = par_c;
	sigma[PAR_C_IDX] = par_c_stepsize_init;

	lbounds[Q_0_IDX] = 0;
	ubounds[Q_0_IDX] = 1;
	x0[Q_0_IDX] = q_0;
	sigma[Q_0_IDX] = q_0_stepsize_init;

#if NODE_CLUSTERING_VERSION == 1
	lbounds[ALPHA_IDX] = 0;
	ubounds[ALPHA_IDX] = DBL_MAX;
	x0[ALPHA_IDX] = alpha;
	sigma[ALPHA_IDX] = alpha_stepsize_init;

	lbounds[BETA_IDX] = 0;
	ubounds[BETA_IDX] = DBL_MAX;
	x0[BETA_IDX] = beta;
	sigma[BETA_IDX] = beta_stepsize_init;

	lbounds[CLUSTER_ALPHA_IDX] = 0;
	ubounds[CLUSTER_ALPHA_IDX] = DBL_MAX;
	x0[CLUSTER_ALPHA_IDX] = cluster_alpha;
	sigma[CLUSTER_ALPHA_IDX] = cluster_alpha_stepsize_init;

	lbounds[CLUSTER_BETA_IDX] = 0;
	ubounds[CLUSTER_BETA_IDX] = DBL_MAX;
	x0[CLUSTER_BETA_IDX] = cluster_beta;
	sigma[CLUSTER_BETA_IDX] = cluster_beta_stepsize_init;

	lbounds[RHO_IDX] = 0;
	ubounds[RHO_IDX] = 1;
	x0[RHO_IDX] = rho;
	sigma[RHO_IDX] = rho_stepsize_init;
#endif

	for (i = 0; i < ES_ANT_DIM; i++)
	{
		assert(x0[i] >= lbounds[i]);
		assert(x0[i] <= ubounds[i]);
	}
	PARAMETER_CLASS cmaparams(x0, sigma, LAMBDA, lbounds, ubounds, seed);
	cmaparams.set_algo(CMA_ALGO);
	ESOptimizer<STRATEGY_CLASS, PARAMETER_CLASS> abipop(es_evaluate, cmaparams);
	optimizer = abipop;
}

FitFunc es_evaluate = [](const double *x, const int N)
{
	long int fitness;
	seed = (long int)x[SEED_IDX];
	assert(seed == x[SEED_IDX]);
	par_a = x[PAR_A_IDX];
	par_b = x[PAR_B_IDX];
	par_c = x[PAR_C_IDX];
	q_0 = x[Q_0_IDX];
#if NODE_CLUSTERING_VERSION == 1
	alpha = x[ALPHA_IDX];
	beta = x[BETA_IDX];
	cluster_alpha = x[CLUSTER_ALPHA_IDX];
	cluster_beta = x[CLUSTER_BETA_IDX];
	rho = x[RHO_IDX];
#endif

	an_ant_run(&fitness);

	return fitness;
};

void es_construct_solution(void)
{
	current_ant_idx = 0;
	while (current_ant_idx < n_ants)
	{
		generation_run();
	}
	? ? ant.resize(current_ant_idx);
}

void generation_run(void)
{
	? = optimzer.get_solutions();
	update solution x[SEED_IDX] = round(x[SEED_IDX]);
	optimzer.eval();
}

void an_ant_run(long int &fitness)
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
	fitness = ant[current_ant_idx].fitness;
	current_ant_idx += 1;
}