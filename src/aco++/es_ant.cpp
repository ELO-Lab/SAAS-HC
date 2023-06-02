// #include <memory>
#include <Eigen/Dense>
#include <libcmaes/cmaes.h>

#include "ants.h"
#include "inout.h"
#include "utilities.h"

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
unsigned char n_generation_each_iteration, cmaes_algo;

CMASolutions cmasols;

void es_ant_set_default(void)
{
	seed_stepsize_init = (SEED_MAX - SEED_MIN) / 20;
	par_a = par_b = par_c = 0.5;
	par_a_stepsize_init = par_b_stepsize_init = par_c_stepsize_init = 0.15;
	q_0 = 0.45;
	q_0_stepsize_init = 0.25;
	n_generation_each_iteration = 1;
	cmaes_algo = aBIPOP_CMAES;
#if NODE_CLUSTERING_VERSION
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
	assert(node_clustering_flag == TRUE);
	assert(!adaptive_evaporation_flag);
	assert(max_packing_tries == 1);
#if 1 - NODE_CLUSTERING_VERSION
	assert(node_clustering_flag == FALSE);
#endif

	long int i;
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

#if NODE_CLUSTERING_VERSION
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

	CMAParameters<GenoPheno<pwqBoundStrategy>> cmaparams(x0, sigma, LAMBDA,
														 lbounds, ubounds, seed);
	cmaparams.set_algo(cmaes_algo);
	cmasols = cmaes<GenoPheno<pwqBoundStrategy>>(es_evaluate, cmaparams);
}

FitFunc es_evaluate = [](const double *x, const int N)
{
	long int fitness;
	seed = (long int)x[SEED_IDX];
	par_a = x[PAR_A_IDX];
	par_b = x[PAR_B_IDX];
	par_c = x[PAR_C_IDX];
	q_0 = x[Q_0_IDX];
#if NODE_CLUSTERING_VERSION
	alpha = x[ALPHA_IDX];
	beta = x[BETA_IDX];
	cluster_alpha = x[CLUSTER_ALPHA_IDX];
	cluster_beta = x[CLUSTER_BETA_IDX];
	rho = x[RHO_IDX];
#endif

	ant run ? ;

	return fitness;
};

void es_construct_solution(void)
{
	update solution x[SEED_IDX] = round(x[SEED_IDX]);
}
