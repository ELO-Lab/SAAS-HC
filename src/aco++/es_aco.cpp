#include "es_aco.h"
#include "algo_config.h"
#include "tree_map.h"
#include "acothop.h"
#include "ants.h"
#include "inout.h"
#include "adaptive_evaporation.h"

#define ALPHA_IDX 0
#define BETA_IDX 1
#define PAR_A_IDX 2
#define PAR_B_IDX 3
#define PAR_C_IDX 4

#if TREE_MAP_MACRO
#define NEIGHBOUR_PROB_IDX 5
#define ELITE_PROB_IDX 6
#define TREE_MAP_TEMP_DIM 7
#else
#define TREE_MAP_TEMP_DIM 5
#endif

#if RHO_TUNING_MACRO
#define RHO_IDX TREE_MAP_TEMP_DIM
#define RHO_TEMP_DIM (TREE_MAP_TEMP_DIM + 1)
#else
#define RHO_TEMP_DIM TREE_MAP_TEMP_DIM
#endif

#if MIN_MAX_RHO_TUNING_MACRO
#define LEFT_RHO_IDX RHO_TEMP_DIM
#define _MID_RHO_IDX (RHO_TEMP_DIM + 1)
#define RIGHT_RHO_IDX (RHO_TEMP_DIM + 2)
#define MIN_MAX_RHO_TEMP_DIM (RHO_TEMP_DIM + 3)
#else
#define MIN_MAX_RHO_TEMP_DIM RHO_TEMP_DIM
#endif

#define ES_ACO_DIM MIN_MAX_RHO_TEMP_DIM
#define EPSILON_IDX ES_ACO_DIM
#define LEVY_THRESHOLD_IDX (ES_ACO_DIM + 1)
#define LEVY_RATIO_IDX (ES_ACO_DIM + 2)

// Hyperparameters
double left_rho_mean, _mid_rho_mean, right_rho_mean,
    left_rho_std, _mid_rho_std, right_rho_std;

unsigned long int initial_nb_dims = ES_ACO_DIM;
unsigned long int initial_lambda = 10;

double lowerBounds[ES_ACO_DIM + 3], upperBounds[ES_ACO_DIM + 3];

std::vector<double> initialX;
std::vector<double> typicalX;
std::vector<double> initialStd;

int default_ls = 1;

boundary_cmaes optimizer;

double *xmean = NULL;
double *xbestgen = NULL;
double *xbestever = NULL;

double fmean;
double fbestgen;
double fbestever;

int worst_offspring_index;
double worst_offspring_fitness;

// variables for ipop and bipop

unsigned int n_restarts = 0;
unsigned long int small_n_eval = 0, large_n_eval = 0;
unsigned long int popsize0 = initial_lambda;
unsigned int inc_popsize = 2;

char poptype = 0; // 0 == small , 1 == large

char params_file_name[LINE_BUF_LEN];

long int cmaes_seed;

void es_write_params()
{
    FILE *fptr;
    // inspect cmaes_initials_default file to be more specific about each parameter
    fptr = fopen(params_file_name, "w");

    fprintf(fptr, "N %d\n", initial_nb_dims);

    fprintf(fptr, "initialX %d:\n\t", initial_nb_dims);
    for (int i = 0; i < initial_nb_dims; i++)
    {
        fprintf(fptr, "%f ", initialX[i]);
    }
    fprintf(fptr, "\n");

    fprintf(fptr, "typicalX %d:\n\t", initial_nb_dims);
    for (int i = 0; i < initial_nb_dims; i++)
    {
        fprintf(fptr, "%f ", typicalX[i]);
    }
    fprintf(fptr, "\n");

    fprintf(fptr, "lambda %d\n", initial_lambda);
    fprintf(fptr, "seed  %d\n", cmaes_seed);

    fprintf(fptr, "weights log \n");

    fprintf(fptr, "initialStandardDeviations %d:\n\t", initial_nb_dims);
    for (int i = 0; i < initial_nb_dims; i++)
    {
        fprintf(fptr, "%f ", initialStd[i]);
    }
    fprintf(fptr, "\n");

    fprintf(fptr, "stopTolFun %f\n", 0.0f);
    fprintf(fptr, "stopTolFunHist %f\n", 0.0f);
    fprintf(fptr, "stopTolX %f\n", 1e-11);
    fprintf(fptr, "stopTolUpXFactor %f\n", 1e3);
    fprintf(fptr, "maxTimeFractionForEigendecompostion %f\n", 0.2);

    // fprintf(fptr,"fac*damp %d\n", 1);

    fclose(fptr);
}

void _es_construct_solutions(int index)
/*
      FUNCTION:       manage the solution construction phase
      INPUT:          none
      OUTPUT:         none
      (SIDE)EFFECTS:  when finished, all ants of the colony have constructed a solution
 */
{
    printf("\nCAUSION: Uncommenting will lead to bugs\n");
    exit(1);

    // long int i, k; /* counter variable */
    // long int step; /* counter of the number of construction steps */

    // TRACE(printf("construct solutions for all ants\n"););

    // /* Mark all cities as unvisited */
    // for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
    // {
    //     ant_empty_memory(&ant[k]);
    // }

    // /* Place the ants at initial city 0 and set the final city as n-1 */
    // for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
    // {
    //     ant[k].tour_size = 1;
    //     ant[k].tour[0] = 0;
    //     ant[k].visited[0] = TRUE;
    //     ant[k].visited[instance.n - 1] = TRUE;
    // }

    // step = 0;
    // while (step < instance.n - 2)
    // {
    //     step++;
    //     for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
    //     {
    //         if (ant[k].tour[ant[k].tour_size - 1] == instance.n - 2)
    //         { /* previous city is the last one */
    //             continue;
    //         }

    //         if (iLevyFlag || iGreedyLevyFlag)
    //         {
    //             neighbour_choose_and_move_to_next_using_greedy_Levy_flight(&ant[k], step);
    //         }
    //         else
    //         {
    //             neighbour_choose_and_move_to_next(&ant[k], step);
    //         }

    //         if (acs_flag)
    //             local_acs_pheromone_update(&ant[k], step);
    //         ant[k].tour_size++;
    //     }
    // }

    // for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
    // {
    //     ant[k].tour[ant[k].tour_size++] = instance.n - 1;
    //     ant[k].tour[ant[k].tour_size++] = ant[k].tour[0];
    //     for (i = ant[k].tour_size; i < instance.n; i++)
    //         ant[k].tour[i] = 0;
    //     ant[k].fitness = compute_fitness(ant[k].tour, ant[k].visited, ant[k].tour_size, ant[k].packing_plan);
    //     if (acs_flag)
    //         local_acs_pheromone_update(&ant[k], ant[k].tour_size - 1);
    // }
    // n_tours += ant.size();
}

void _es_local_search(int index)
/*
      FUNCTION:       manage the local search phase; apply local search to ALL ants; in
                      dependence of ls_flag one of 2-opt, 2.5-opt, and 3-opt local search
                      is chosen.
      INPUT:          none
      OUTPUT:         none
      (SIDE)EFFECTS:  all ants of the colony have locally optimal tours
      COMMENTS:       typically, best performance is obtained by applying local search
                      to all ants. It is known that some improvements (e.g. convergence
                      speed towards high quality solutions) may be obtained for some
                      ACO algorithms by applying local search to only some of the ants.
                      Overall best performance is typcially obtained by using 3-opt.
 */
{
    printf("\nCAUSION: Uncommenting will lead to bugs\n");
    exit(1);

    // long int k;

    // TRACE(printf("apply local search to all ants\n"););

    // for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
    // {
    //     switch (ls_flag)
    //     {
    //     case 1:
    //         two_opt_first(ant[k].tour, ant[k].tour_size); /* 2-opt local search */
    //         break;
    //     case 2:
    //         two_h_opt_first(ant[k].tour, ant[k].tour_size); /* 2.5-opt local search */
    //         break;
    //     case 3:
    //         three_opt_first(ant[k].tour, ant[k].tour_size); /* 3-opt local search */
    //         break;
    //     default:
    //         fprintf(stderr, "type of local search procedure not correctly specified\n");
    //         exit(1);
    //     }
    //     ant[k].fitness = compute_fitness(ant[k].tour, ant[k].visited, ant[k].tour_size, ant[k].packing_plan);
    //     if (termination_condition())
    //         return;
    // }
}

//   index of current offspring         genotype   number of dims
double eval_function(int index, double const *x, unsigned long N)
{
    std::size_t k;
    double mean_fitness, std_fitness;
    std::vector<double> fitnesses;
    double min_fitness = INFINITY;
    double max_fitness = -INFINITY;

    alpha = x[ALPHA_IDX];
    beta = x[BETA_IDX];
    par_a = x[PAR_A_IDX];
    par_b = x[PAR_B_IDX];
    par_c = x[PAR_C_IDX];
    dGreedyEpsilon = x[EPSILON_IDX];
    dGreedyLevyThreshold = x[LEVY_THRESHOLD_IDX];
    dGreedyLevyRatio = x[LEVY_RATIO_IDX];
#if TREE_MAP_MACRO
    neighbour_prob = x[NEIGHBOUR_PROB_IDX];
    elite_prob = x[ELITE_PROB_IDX];
#endif
#if MIN_MAX_RHO_TUNING_MACRO
    left_rho = x[LEFT_RHO_IDX];
    _mid_rho = x[_MID_RHO_IDX];
    right_rho = x[RIGHT_RHO_IDX];
    update_rho();
#endif
//
#if RHO_TUNING_MACRO
    rho = x[RHO_IDX];
#endif

    /*
    _es_construct_solutions(index);
    if (ls_flag > 0)
    {
        for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
        {
            copy_from_to(&ant[k], &prev_ls_ant[k]);
        }
        _es_local_search(index);
        for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
        {
            if (ant[k].fitness > prev_ls_ant[k].fitness)
            {
                copy_from_to(&prev_ls_ant[k], &ant[k]);
            }
        }
    }
    */

    for (k = index * indv_ants; k < index * indv_ants + indv_ants; k++)
    {
#if TREE_MAP_MACRO
        if (tree_map_flag)
            tree_map->choose_route(
                ant[k],
                neighbour_prob,
                alpha,
                beta,
                rho,
                n_tours,
                nn_ants,
                instance.nn_list,
                elite_prob);
        else
#endif
            an_ant_run(k);

        if (ls_flag > 0)
        {
            copy_from_to(&ant[k], &prev_ls_ant[k]);
            an_ant_local_search(k);
            {
                if (ant[k].fitness > prev_ls_ant[k].fitness)
                {
                    copy_from_to(&prev_ls_ant[k], &ant[k]);
                }
            }
        }
    }

    for (k = index * indv_ants; k < (index + 1) * indv_ants; k++)
    {
        fitnesses.push_back(double(ant[k].fitness));
        min_fitness = std::min(min_fitness, double(ant[k].fitness));
        max_fitness = std::max(max_fitness, double(ant[k].fitness));
    }
    mean_and_std(fitnesses, mean_fitness, std_fitness);

    double offspring_fitness = mean_fitness;
    if (offspring_fitness > worst_offspring_fitness)
    {
        worst_offspring_fitness = offspring_fitness;
        worst_offspring_index = index;
    }

    ES_ACO_DEBUG(
        // printf("\tX offspring %d\t =[", index);
        // for (int i = 0; i < initial_nb_dims; i++)
        //     printf("%.3f, ", x[i]);
        // printf("], fitness=%ld\n", (long int)offspring_fitness);

        // printf("\t\tFitnesses=[");
        // for (k = 0; k < indv_ants; k++)
        //     printf("%ld, ", (long int)fitnesses[k]);
        // printf("]\n");
    )
    return offspring_fitness;
}

void generating_random_vector()
{
    initialX.clear();
    typicalX.clear();
    initialStd.clear();

    double rand_std_left = 2;
    double rand_std_right = 20;
    for (int i = 0; i < initial_nb_dims; i++)
    {
        initialX.push_back(lowerBounds[i] + (new_rand01() * (upperBounds[i] - lowerBounds[i])));
        typicalX.push_back(lowerBounds[i] + (new_rand01() * (upperBounds[i] - lowerBounds[i])));

        double frac;
        if (random_initial_std)
            frac = rand_std_left + new_rand01() * (rand_std_right - rand_std_left);
        else
            frac = 5;

        initialStd.push_back((upperBounds[i] - lowerBounds[i]) / frac);
    }
}

void setup_cmaes()
{
    printf("Popsize=%d\n", (long int)initial_lambda);
    printf("Initial X  =[");
    for (int i = 0; i < initial_nb_dims; i++)
        printf("%.3f, ", initialX[i]);
    printf("]\nInitial STD=[");
    for (int i = 0; i < initial_nb_dims; i++)
        printf("%.3f, ", initialStd[i]);
    printf("]\n");

    if (iGreedyLevyFlag)
    {
        initial_nb_dims = ES_ACO_DIM + 3;
    }
    es_write_params();
    optimizer.init(eval_function, lowerBounds, upperBounds, params_file_name);

    resize_ant_colonies();

    // default_ls = ls_flag;

    es_aco_update_statistics();
}

void es_aco_init()
{
    n_restarts = 0;
    sprintf(params_file_name, "%s.%ld.cmaes_initials.par", output_name_buf, seed);
    printf("CMA-ES's Initial parameters will be to %s.\n", params_file_name);

    cmaes_seed = seed;
    generating_random_vector();
    // printf("\t\tmean\tstd\n");
    // printf("neighbour_prob\t\t%lf\t%lf\n", initialX[NEIGHBOUR_PROB_IDX], initialStd[NEIGHBOUR_PROB_IDX]);
    // printf("q_0\t\t%lf\t%lf\n", initialX[ELITE_PROB_IDX], initialStd[ELITE_PROB_IDX]);

    initialX[ALPHA_IDX] = typicalX[ALPHA_IDX] = alpha_mean;
    initialStd[ALPHA_IDX] = alpha_std;

    initialX[BETA_IDX] = typicalX[BETA_IDX] = beta_mean;
    initialStd[BETA_IDX] = beta_std;

    initialX[PAR_A_IDX] = typicalX[PAR_A_IDX] = par_a_mean;
    initialStd[PAR_A_IDX] = par_a_std;

    initialX[PAR_B_IDX] = typicalX[PAR_B_IDX] = par_b_mean;
    initialStd[PAR_B_IDX] = par_b_std;

    initialX[PAR_C_IDX] = typicalX[PAR_C_IDX] = par_c_mean;
    initialStd[PAR_C_IDX] = par_c_std;

#if TREE_MAP_MACRO
    initialX[NEIGHBOUR_PROB_IDX] = typicalX[NEIGHBOUR_PROB_IDX] = neighbour_prob_mean;
    initialStd[NEIGHBOUR_PROB_IDX] = neighbour_prob_std;

    initialX[ELITE_PROB_IDX] = typicalX[ELITE_PROB_IDX] = elite_prob_mean;
    initialStd[ELITE_PROB_IDX] = elite_prob_std;
#endif
#if MIN_MAX_RHO_TUNING_MACRO
    initialX[LEFT_RHO_IDX] = typicalX[LEFT_RHO_IDX] = left_rho_mean;
    initialStd[LEFT_RHO_IDX] = left_rho_std;
    initialX[_MID_RHO_IDX] = typicalX[_MID_RHO_IDX] = _mid_rho_mean;
    initialStd[_MID_RHO_IDX] = _mid_rho_std;
    initialX[RIGHT_RHO_IDX] = typicalX[RIGHT_RHO_IDX] = right_rho_mean;
    initialStd[RIGHT_RHO_IDX] = right_rho_std;
#endif

    setup_cmaes();
}

void es_aco_restart(const char *termination_reason)
{
    n_restarts++;
    // exit(1);

    cmaes_seed = optimizer.get("randomseed");
    printf("\nRestart CMA-ES, %s\nNumber of iteration: %ld, Number of ants: %ld, Value of rho: %f, ", termination_reason, (long int)(optimizer.get("iter")), indv_ants * (long int)(optimizer.get("lambda")), rho);

    /*
    double *xbestever = NULL;
    xbestever = optimizer.getInto("xbestever", xbestever);
    xbestever = optimizer.boundary_transformation(xbestever);

    for (int i = 0; i < initial_nb_dims; i++){
        typicalX[i] = xbestever[i];
        initialX[i] = xbestever[i];
    }
    */

    generating_random_vector();

    setup_cmaes();
}

void resize_ant_colonies()
{
    const std::size_t nb_indvs = optimizer.get("lambda");
    ant.resize(indv_ants * nb_indvs);
    prev_ls_ant.resize(indv_ants * nb_indvs);
    n_ants = ant.size();
}

void es_aco_construct_and_local_search()
{
    worst_offspring_index = 0;
    worst_offspring_fitness = -INFINITY;

    // ls_flag = default_ls;

    ES_ACO_DEBUG(
        printf("\nGeneration %ld:\tX mean=[", (long int)(optimizer.get("iter")));
        for (int i = 0; i < initial_nb_dims; i++)
            printf("%.3f, ", xmean[i]);
        printf("]\n");)

    resize_ant_colonies();
    optimizer.run_a_generation();
    if (termination_condition())
        return;

    es_aco_update_statistics();

    ES_ACO_DEBUG(
        printf("\t\tGeneration fitness=%.4f\n", fbestgen);
        // printf("\t\tentropy=%f, fitness_entropy=%f, rho=%f, indv_ants=%d \n", entropy, fitness_entropy, rho, indv_ants);
    )

    if (replace_worst_by_bestever)
        eval_function(worst_offspring_index, xbestever, optimizer.get("lambda"));

    const char *termination_reason = es_aco_termination_condition();
    if (termination_reason)
    {
        // cmaes_flag = 0; return;
        es_aco_restart(termination_reason);
    }
}

void ipop_cmaes_aco_construct_solutions()
{
    printf("\nCAUSION: Uncommenting will lead to bugs\n");
    exit(1);
    //     optimizer.run_a_generation();
    //     if (termination_condition())
    //         return;
    //     es_aco_set_best_params();

    //     const char *termination_reason = es_aco_termination_condition();
    //     if (termination_reason)
    //     {
    //         initial_lambda *= inc_popsize;
    //         es_aco_restart(termination_reason);
    //     }
}

void bipop_cmaes_aco_construct_solutions()
{
    printf("\nCAUSION: Uncommenting will lead to bugs\n");
    exit(1);
    //     optimizer.run_a_generation();
    //     if (termination_condition())
    //         return;
    //     es_aco_set_best_params();

    //     const char *termination_reason = es_aco_termination_condition();
    //     if (termination_reason)
    //     {
    //         printf("BIPOP restart, ");
    //         long int n_eval = (int)(optimizer.get("lambda")) * optimizer.get("generation");
    //         if (poptype == 0)
    //             small_n_eval += n_eval;
    //         else
    //             large_n_eval += n_eval;

    //         if (small_n_eval < large_n_eval)
    //         {
    //             poptype = 0;
    //             double popsize_multiplier = pow(inc_popsize, n_restarts);
    //             initial_lambda = floor(
    //                 pow(popsize0 * popsize_multiplier, pow(new_rand01(), 2)));
    //         }
    //         else
    //         {
    //             poptype = 1;
    //             n_restarts += 1;
    //             initial_lambda = popsize0 * pow(inc_popsize, n_restarts);
    //         }

    //         es_aco_restart(termination_reason);
    //     }
}

void es_aco_export_result()
{
    optimizer.end();
}

void es_aco_exit()
{
    optimizer.boundary_cmaes_exit();
    free(xbestever);
}

const char *es_aco_termination_condition()
{
    return optimizer.termination_condition();
}

void es_aco_update_statistics()
{
    xmean = optimizer.getInto("xmean", xmean);
    xbestgen = optimizer.getInto("xbest", xbestgen);
    xbestever = optimizer.getInto("xbestever", xbestever);

    xmean = optimizer.boundary_transformation(xmean);
    xbestgen = optimizer.boundary_transformation(xbestgen);
    xbestever = optimizer.boundary_transformation(xbestever);

    fbestgen = optimizer.get("fitness");
    fbestever = optimizer.get("fbestever");

    alpha = xbestever[ALPHA_IDX];
    beta = xbestever[BETA_IDX];
    par_a = xbestever[PAR_A_IDX];
    par_b = xbestever[PAR_B_IDX];
    par_c = xbestever[PAR_C_IDX];
    dGreedyEpsilon = xbestever[EPSILON_IDX];
    dGreedyLevyThreshold = xbestever[LEVY_THRESHOLD_IDX];
    dGreedyLevyRatio = xbestever[LEVY_RATIO_IDX];
#if TREE_MAP_MACRO
    neighbour_prob = xbestever[NEIGHBOUR_PROB_IDX];
    elite_prob = xbestever[ELITE_PROB_IDX];
#endif
#if MIN_MAX_RHO_TUNING_MACRO
    left_rho = xbestever[LEFT_RHO_IDX];
    _mid_rho = xbestever[_MID_RHO_IDX];
    right_rho = xbestever[RIGHT_RHO_IDX];
    update_rho();
#endif
//
#if RHO_TUNING_MACRO
    rho = xbestever[RHO_IDX];
#endif
    if (verbose > 0)
    {
        // printf("min_rho: %.4f\n", min_rho);
        // printf("max_rho: %.4f\n", max_rho);
        // printf("alpha: %.4f\n", alpha);
        // printf("beta: %.4f\n", beta);
        // printf("rho: %.4f\n", rho);
        // printf("q_0: %.4f\n", q_0);
#if TREE_MAP_MACRO
        // printf("neighbour_prob: %.4f\n", neighbour_prob);
#endif
    }
}

void es_aco_init_program()
{
    lowerBounds[ALPHA_IDX] = 0.01;
    upperBounds[ALPHA_IDX] = 10;

    lowerBounds[BETA_IDX] = 0.01;
    upperBounds[BETA_IDX] = 10;

    lowerBounds[PAR_A_IDX] = 0.01;
    upperBounds[PAR_A_IDX] = 1;

    lowerBounds[PAR_B_IDX] = 0.01;
    upperBounds[PAR_B_IDX] = 1;

    lowerBounds[PAR_C_IDX] = 0.01;
    upperBounds[PAR_C_IDX] = 1;

#if TREE_MAP_MACRO
    lowerBounds[NEIGHBOUR_PROB_IDX] = 0;
    upperBounds[NEIGHBOUR_PROB_IDX] = 1;
    lowerBounds[ELITE_PROB_IDX] = 0;
    upperBounds[ELITE_PROB_IDX] = 0.99;
#endif
#if MIN_MAX_RHO_TUNING_MACRO
    lowerBounds[LEFT_RHO_IDX] = 0;
    lowerBounds[_MID_RHO_IDX] = 0;
    lowerBounds[RIGHT_RHO_IDX] = 0;

    upperBounds[LEFT_RHO_IDX] = 1;
    upperBounds[_MID_RHO_IDX] = 1;
    upperBounds[RIGHT_RHO_IDX] = 1;
#endif
    //
#if RHO_TUNING_MACRO
    lowerBounds[RHO_IDX] = 0.01;
    upperBounds[RHO_IDX] = 0.99;
#endif

    lowerBounds[EPSILON_IDX] = 0.0;
    upperBounds[EPSILON_IDX] = 1.0;

    lowerBounds[LEVY_THRESHOLD_IDX] = 0.0;
    upperBounds[LEVY_THRESHOLD_IDX] = 1.0;

    lowerBounds[LEVY_RATIO_IDX] = 0.0;
    upperBounds[LEVY_RATIO_IDX] = 5.0;
}