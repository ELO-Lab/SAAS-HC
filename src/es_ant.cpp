#include "algo_config.h"
#if ES_ANT_MACRO

#include <cstddef>
#include <assert.h>
#include <limits.h>
#include <iostream>

#include "ants.h"
#include "inout.h"
#include "utilities.h"
#include "thop.h"
#include "ls.h"
#include "es_ant.h"
#include "acothop.h"
#include "custom_strategy.h"
#include "tree_map.h"
#include "algo_config.h"

#define ALPHA_IDX 0
#define BETA_IDX 1
#define PAR_A_IDX 2
#define PAR_B_IDX 3
#define PAR_C_IDX 4

#if Q_0_TUNING_MACRO
#define Q_0_IDX 5
#define Q0_TEMP_DIM 6
#else
#define Q0_TEMP_DIM 5
#endif

#if RHO_TUNING_MACRO
#define RHO_IDX Q0_TEMP_DIM
#define RHO_TEMP_DIM (Q0_TEMP_DIM + 1)
#else
#define RHO_TEMP_DIM Q0_TEMP_DIM
#endif

#if TREE_MAP_MACRO
#define NEIGHBOUR_PROB_IDX RHO_TEMP_DIM
#define TREE_TEMP_DIM (RHO_TEMP_DIM + 1)
#else
#define TREE_TEMP_DIM RHO_TEMP_DIM
#endif

#if SEED_TUNING_MACRO
#define SEED_IDX TREE_TEMP_DIM
#define SEED_TEMP_DIM (TREE_TEMP_DIM + 1)
#else
#define SEED_TEMP_DIM TREE_TEMP_DIM
#endif

#define ES_ANT_DIM SEED_TEMP_DIM

// hyperparameters
double rand_seed_std;
std::size_t min_n_ants;

std::size_t current_ant_idx = 0;
std::array<double, ES_ANT_DIM> lbounds, ubounds;
OPTIMIZER *optim_ptr;
double best_iteration_alpha, best_iteration_beta, best_iteration_rho;
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
#if SEED_TUNING_MACRO
    size_t i;

    for (i = 0; i < candidates.cols(); i++)
    {
        auto &seed = candidates.col(i)[SEED_IDX];
        seed = restore_scale(seed, lbounds[SEED_IDX], ubounds[SEED_IDX]);
        seed = round(seed);
        seed = normalize(seed, lbounds[SEED_IDX], ubounds[SEED_IDX]);
    }
#endif
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

long int an_ant_evaluate(const double *x, const int &N)
{
    std::vector<double> parameters(N);
    size_t i;

    for (i = 0; i < N; i++)
        parameters[i] = restore_scale(x[i], lbounds[i], ubounds[i]);

    alpha = parameters[ALPHA_IDX];
    beta = parameters[BETA_IDX];
    par_a = parameters[PAR_A_IDX];
    par_b = parameters[PAR_B_IDX];
    par_c = parameters[PAR_C_IDX];
#if SEED_TUNING_MACRO
    rand_gen.seed(round(parameters[SEED_IDX]));
#endif
#if RHO_TUNING_MACRO
    rho = x[RHO_IDX];
#endif
#if Q_0_TUNING_MACRO
    q_0 = x[Q_0_IDX];
#endif
#if TREE_MAP_MACRO
    neighbour_prob = parameters[NEIGHBOUR_PROB_IDX];
#endif

#if TREE_MAP_MACRO
    if (tree_map_flag)
        tree_map->choose_route(
            ant[current_ant_idx],
            neighbour_prob,
            alpha,
            beta,
            rho,
            n_tours,
            nn_ants,
            instance.nn_list,
            q_0);
    else
#endif
        an_ant_run(current_ant_idx);

    if (ls_flag > 0)
    {
        copy_from_to(&ant[current_ant_idx], &prev_ls_ant[current_ant_idx]);
        an_ant_local_search(current_ant_idx);
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
        best_iteration_rho = rho;
    }

    current_ant_idx += 1;
    return ant[current_ant_idx - 1].fitness;
}

libcmaes::FitFunc es_evaluate = [](const double *x, const int &N)
{
    if (termination_condition())
        return double(-1);

    std::size_t i;
    double mean_value, fitness, worst_fitness, best_fitness;

    worst_fitness = -1;
    best_fitness = instance.UB + 1;
    for (i = 0; i < indv_ants; i++)
    {
        fitness = an_ant_evaluate(x, N);
        mean_value = fitness / indv_ants;
        if (worst_fitness < fitness)
            worst_fitness = fitness;
        if (best_fitness > fitness)
            best_fitness = fitness;
    }

    assert(worst_fitness > -1);
    // return worst_fitness;
    return mean_value;
    // return best_fitness;
};

void init_optimizer(void)
{
    constexpr long int ALGO_CODE = aBIPOP_CMAES, NRESTARTS = INT_MAX;
    size_t i;
    std::vector<double> x0(ES_ANT_DIM), sigma(ES_ANT_DIM);

#if SEED_TUNING_MACRO
    lbounds[SEED_IDX] = rand_gen.min();
    ubounds[SEED_IDX] = rand_gen.max();
    x0[SEED_IDX] = (ubounds[SEED_IDX] - lbounds[SEED_IDX]) / 2.0;
    sigma[SEED_IDX] = rand_seed_std / (ubounds[SEED_IDX] - lbounds[SEED_IDX]);
#endif

    lbounds[PAR_A_IDX] = 0.01;
    ubounds[PAR_A_IDX] = 1;
    x0[PAR_A_IDX] = par_a_mean;
    sigma[PAR_A_IDX] = par_a_std / (ubounds[PAR_A_IDX] - lbounds[PAR_A_IDX]);

    lbounds[PAR_B_IDX] = 0.01;
    ubounds[PAR_B_IDX] = 1;
    x0[PAR_B_IDX] = par_b_mean;
    sigma[PAR_B_IDX] = par_b_std / (ubounds[PAR_B_IDX] - lbounds[PAR_B_IDX]);

    lbounds[PAR_C_IDX] = 0.01;
    ubounds[PAR_C_IDX] = 1;
    x0[PAR_C_IDX] = par_c_mean;
    sigma[PAR_C_IDX] = par_c_std / (ubounds[PAR_C_IDX] - lbounds[PAR_C_IDX]);

#if Q_0_TUNING_MACRO
    lbounds[Q_0_IDX] = 0;
    ubounds[Q_0_IDX] = 0.99;
    x0[Q_0_IDX] = q_0_mean;
    sigma[Q_0_IDX] = q_0_std / (ubounds[Q_0_IDX] - lbounds[Q_0_IDX]);
#endif

    lbounds[ALPHA_IDX] = 0.01;
    ubounds[ALPHA_IDX] = 10;
    x0[ALPHA_IDX] = alpha_mean;
    sigma[ALPHA_IDX] = alpha_std / (ubounds[ALPHA_IDX] - lbounds[ALPHA_IDX]);

    lbounds[BETA_IDX] = 0.01;
    ubounds[BETA_IDX] = 10;
    x0[BETA_IDX] = beta_mean;
    sigma[BETA_IDX] = beta_std / (ubounds[BETA_IDX] - lbounds[BETA_IDX]);

#if RHO_TUNING_MACRO
    lbounds[RHO_IDX] = 0.01;
    ubounds[RHO_IDX] = 0.99;
    x0[RHO_IDX] = rho_mean;
    sigma[RHO_IDX] = rho_std / (ubounds[RHO_IDX] - lbounds[RHO_IDX]);
#endif

#if TREE_MAP_MACRO
    lbounds[NEIGHBOUR_PROB_IDX] = 0.01;
    ubounds[NEIGHBOUR_PROB_IDX] = 0.99;
    x0[NEIGHBOUR_PROB_IDX] = neighbour_prob_mean;
    sigma[NEIGHBOUR_PROB_IDX] = neighbour_prob_std / (ubounds[NEIGHBOUR_PROB_IDX] - lbounds[NEIGHBOUR_PROB_IDX]);
#endif

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

void es_ant_construct_and_local_search(void)
{
    size_t capacity_need;
    current_ant_idx = 0;
    best_iteration_fitness = instance.UB + 1;

    while (current_ant_idx < min_n_ants or current_ant_idx == 0)
    {
        capacity_need = current_ant_idx + optim_ptr->get_lambda() * indv_ants;
        ant.resize(capacity_need);
        prev_ls_ant.resize(capacity_need);

        optim_ptr->generation_run();
        if (termination_condition())
            return;
    }

    alpha = best_iteration_alpha;
    beta = best_iteration_beta;
#if RHO_TUNING_MACRO
    rho = best_iteration_rho;
#endif
    n_ants = capacity_need;

    if (verbose > 0)
    {
        // printf("n_ants: %ld\n", n_ants);
#if TREE_MAP_MACRO
        // printf("neighbour_prob: %.2f\n", neighbour_prob);
#endif
    }
}

void es_ant_set_default_hyperparameters(void)
{
    // min_n_ants = n_ants * 0.8;
    // min_n_ants = n_ants;
    min_n_ants = 0;

    rand_seed_std = (rand_gen.max() - rand_gen.min()) / 20.0;
}

void es_ant_init(void)
{
    es_ant_set_default_hyperparameters();
    init_optimizer();
}

void es_ant_deallocate()
{
    delete optim_ptr;
}

#endif