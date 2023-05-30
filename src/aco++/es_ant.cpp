// #include <memory>

#include <libcmaes/cmaes.h>
#include <Eigen/Dense>

#include "ants.h"
#include "inout.h"
#include "utilities.h"

#include "adaptive_evaporation.hpp"
#include "node_clustering.h"
#include "es_ant.hpp"

using namespace libcmaes;

#define PAR_A_IDX 0
#define PAR_B_IDX 1
#define PAR_C_IDX 2
#define SEED_IDX 3
#define RHO_IDX 4
#define ALPHA_IDX 5
#define BETA_IDX 6
#define CLUSTER_ALPHA_IDX 7
#define CLUSTER_BETA_IDX 8
#define ES_ALGO aCMAES

bool adapt_ant;
double
    cluster_alpha,
    cluster_alpha_sigma_init,
    cluster_beta, cluster_beta_sigma_init,
    par_a, par_a_sigma_init,
    par_b, par_b_sigma_init,
    par_c, par_c_sigma_init,
    rho_sigma_init, alpha_sigma_init, beta_sigma_init;

double seed_sigma_init;
CMASolutions cmasols;

void es_ant_set_default(void)
{
    rho = 0.468542;
    rho_sigma_init = 0.253226;
    beta = 4.893958;
    beta_sigma_init = 2.067786;
    alpha = 1.550208;
    alpha_sigma_init = 1.523180;
    par_a = par_b = par_c = 0.5;
    par_a_sigma_init = par_b_sigma_init = par_c_sigma_init = 0.05;
    cluster_beta = beta;
    cluster_beta_sigma_init = beta_sigma_init;
    cluster_alpha = alpha;
    cluster_alpha_sigma_init = alpha_sigma_init;
    seed_sigma_init = seed / 20;
}

void es_ant_init(void)
{
    assert(node_clustering_flag == TRUE);
    assert(!adaptive_evaporation_flag);
    assert(max_packing_tries == 1);

    long int i;
    const int LAMBDA = -1;
    std::vector<double> x0(ES_ANT_DIM), sigma(ES_ANT_DIM), lbounds(ES_ANT_DIM), ubounds(ES_ANT_DIM);

    lbounds[PAR_A_IDX] = 0;
    ubounds[PAR_A_IDX] = 1;
    x0[PAR_A_IDX] = par_a;
    sigma[PAR_A_IDX] = par_a_sigma_init;

    lbounds[PAR_B_IDX] = 0;
    ubounds[PAR_B_IDX] = 1;
    x0[PAR_B_IDX] = par_b;
    sigma[PAR_B_IDX] = par_b_sigma_init;

    lbounds[PAR_C_IDX] = 0;
    ubounds[PAR_C_IDX] = 1;
    x0[PAR_C_IDX] = par_c;
    sigma[PAR_C_IDX] = par_c_sigma_init;

    lbounds[RHO_IDX] = 0;
    ubounds[RHO_IDX] = 1;
    x0[RHO_IDX] = rho;
    sigma[RHO_IDX] = rho_sigma_init;

    lbounds[ALPHA_IDX] = 0;
    ubounds[ALPHA_IDX] = DBL_MAX;
    x0[ALPHA_IDX] = alpha;
    sigma[ALPHA_IDX] = alpha_sigma_init;

    lbounds[BETA_IDX] = 0;
    ubounds[BETA_IDX] = DBL_MAX;
    x0[BETA_IDX] = beta;
    sigma[BETA_IDX] = beta_sigma_init;

    lbounds[SEED_IDX] = SEED_MIN;
    ubounds[SEED_IDX] = SEED_MAX;
    x0[SEED_IDX] = seed;
    sigma[SEED_IDX] = seed_sigma_init;

    lbounds[CLUSTER_ALPHA_IDX] = 0;
    ubounds[CLUSTER_ALPHA_IDX] = DBL_MAX;
    x0[CLUSTER_ALPHA_IDX] = cluster_alpha;
    sigma[CLUSTER_ALPHA_IDX] = cluster_alpha_sigma_init;

    lbounds[CLUSTER_BETA_IDX] = 0;
    ubounds[CLUSTER_BETA_IDX] = DBL_MAX;
    x0[CLUSTER_BETA_IDX] = cluster_beta;
    sigma[CLUSTER_BETA_IDX] = cluster_beta_sigma_init;

    for (i = 0; i < ES_ANT_DIM; i++)
    {
        assert(x0[i] >= lbounds[i]);
        assert(x0[i] <= ubounds[i]);
    }

    CMAParameters<GenoPheno<pwqBoundStrategy>> cmaparams(x0, sigma, LAMBDA, lbounds, ubounds, seed);
    cmaparams.set_algo(ES_ALGO);
    cmasols = cmaes<GenoPheno<pwqBoundStrategy>>(es_evaluate, cmaparams);
}

FitFunc es_evaluate = [](const double *x, const int N) {
};

void es_construct_solution(void)
{
}
