#include "es_aco.h"
#include "algo_config.h"
#include "tree_map.h"

#define ALPHA_IDX 0
#define BETA_IDX 1
#define PAR_A_IDX 2
#define PAR_B_IDX 3
#define PAR_C_IDX 4

#if Q0_TUNING_MACRO
#define Q0_IDX 5
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
#define TEMP_DIM (RHO_TEMP_DIM + 1)
#else
#define TEMP_DIM RHO_TEMP_DIM
#endif

#define ES_ACO_DIM TEMP_DIM
#define EPSILON_IDX ES_ACO_DIM
#define LEVY_THRESHOLD_IDX (ES_ACO_DIM + 1)
#define LEVY_RATIO_IDX (ES_ACO_DIM + 2)

unsigned long int initial_nb_dims = ES_ACO_DIM;
unsigned long int initial_lambda = 10;
const double initial_std = 0.2;

double lowerBounds[] = {
    0.01f, // alpha
    0.01f, // beta
    0.0f,  // par_a
    0.0f,  // par_b
    0.0f,  // par_c
#if Q0_TUNING_MACRO
    0.01f, // q0
#endif
#if RHO_TUNING_MACRO
    0.01f, // rho
#endif
#if TREE_MAP_MACRO
    0.01f, // neighbour_prob
#endif
    0.0f,  // epsilon
    0.0f,  // threshold
    0.0f}; // ratio

double upperBounds[] = {
    10.0f, // alpha
    10.0f, // beta
    1.0f,  // par_a
    1.0f,  // par_b
    1.0f,  // par_c
#if Q0_TUNING_MACRO
    0.99f, // q0
#endif
#if RHO_TUNING_MACRO
    0.99f, // rho
#endif
#if TREE_MAP_MACRO
    0.99f, // neighbour_prob
#endif
    1.0f,  // epsilon
    1.0f,  // threshold
    5.0f}; // ratio

std::vector<double> initialX;
std::vector<double> typicalX;
std::vector<double> initialStd;
// number of ants per individual
unsigned int indv_ants = 4;

boundary_cmaes optimizer;

// variables for ipop and bipop

unsigned int n_restarts = 0;
unsigned long int small_n_eval = 0, large_n_eval = 0;
unsigned long int popsize0 = initial_lambda;
unsigned int inc_popsize = 2;

char poptype = 0; // 0 == small , 1 == large


void es_write_params(){
    FILE *fptr;
    // inspect cmaes_initials_default file to be more specific about each parameter
    fptr = fopen("cmaes_initials.par","w");

    fprintf(fptr,"N %d\n", initial_nb_dims);
    
    fprintf(fptr,"initialX %d:\n\t", initial_nb_dims);
    for (int i = 0; i < initial_nb_dims; i++){
        fprintf(fptr,"%f ", initialX[i]);
    }fprintf(fptr,"\n");

    fprintf(fptr,"typicalX %d:\n\t", initial_nb_dims);
    for (int i = 0; i < initial_nb_dims; i++){
        fprintf(fptr,"%f ", typicalX[i]);
    }fprintf(fptr,"\n");
    
    fprintf(fptr,"lambda %d\n", initial_lambda);
    fprintf(fptr,"seed  %d\n", seed);

    fprintf(fptr,"weights log \n");

    fprintf(fptr,"initialStandardDeviations %d:\n\t", initial_nb_dims);
    for (int i = 0; i < initial_nb_dims; i++){
        fprintf(fptr,"%f ", initialStd[i]);
    }fprintf(fptr,"\n");
    
    // fprintf(fptr,"stopTolFunHist %f\n", 0.0f);
    fprintf(fptr,"stopTolX %f\n", 1e-11);
    fprintf(fptr,"stopTolUpXFactor %f\n", 1e2);
    fprintf(fptr,"maxTimeFractionForEigendecompostion %f\n",1);

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

    long int i, k; /* counter variable */
    long int step; /* counter of the number of construction steps */

    TRACE(printf("construct solutions for all ants\n"););

    /* Mark all cities as unvisited */
    for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
    {
        ant_empty_memory(&ant[k]);
    }

    /* Place the ants at initial city 0 and set the final city as n-1 */
    for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
    {
        ant[k].tour_size = 1;
        ant[k].tour[0] = 0;
        ant[k].visited[0] = TRUE;
        ant[k].visited[instance.n - 1] = TRUE;
    }

    step = 0;
    while (step < instance.n - 2)
    {
        step++;
        for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
        {
            if (ant[k].tour[ant[k].tour_size - 1] == instance.n - 2)
            { /* previous city is the last one */
                continue;
            }

            if (iLevyFlag || iGreedyLevyFlag)
            {
                neighbour_choose_and_move_to_next_using_greedy_Levy_flight(&ant[k], step);
            }
            else
            {
                neighbour_choose_and_move_to_next(&ant[k], step);
            }

            if (acs_flag)
                local_acs_pheromone_update(&ant[k], step);
            ant[k].tour_size++;
        }
    }

    for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
    {
        ant[k].tour[ant[k].tour_size++] = instance.n - 1;
        ant[k].tour[ant[k].tour_size++] = ant[k].tour[0];
        for (i = ant[k].tour_size; i < instance.n; i++)
            ant[k].tour[i] = 0;
        ant[k].fitness = compute_fitness(ant[k].tour, ant[k].visited, ant[k].tour_size, ant[k].packing_plan);
        if (acs_flag)
            local_acs_pheromone_update(&ant[k], ant[k].tour_size - 1);
    }
    n_tours += ant.size();
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
    long int k;

    TRACE(printf("apply local search to all ants\n"););

    for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
    {
        switch (ls_flag)
        {
        case 1:
            two_opt_first(ant[k].tour, ant[k].tour_size); /* 2-opt local search */
            break;
        case 2:
            two_h_opt_first(ant[k].tour, ant[k].tour_size); /* 2.5-opt local search */
            break;
        case 3:
            three_opt_first(ant[k].tour, ant[k].tour_size); /* 3-opt local search */
            break;
        default:
            fprintf(stderr, "type of local search procedure not correctly specified\n");
            exit(1);
        }
        ant[k].fitness = compute_fitness(ant[k].tour, ant[k].visited, ant[k].tour_size, ant[k].packing_plan);
        // if (termination_condition())
        //     return;
    }
}

//   index of current offspring         genotype   number of dims
double eval_function(int index, double const *x, unsigned long N)
{
    alpha = x[ALPHA_IDX];
    beta = x[BETA_IDX];
    par_a = x[PAR_A_IDX];
    par_b = x[PAR_B_IDX];
    par_c = x[PAR_C_IDX];
    dGreedyEpsilon = x[EPSILON_IDX];
    dGreedyLevyThreshold = x[LEVY_THRESHOLD_IDX];
    dGreedyLevyRatio = x[LEVY_RATIO_IDX];
#if RHO_TUNING_MACRO
    rho = x[RHO_IDX];
#endif
#if Q0_TUNING_MACRO
    q_0 = x[Q0_IDX];
#endif
#if TREE_MAP_MACRO
    neighbour_prob = x[NEIGHBOUR_PROB_IDX];
#endif

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

    double min_fitness = INFINITY;
    double max_fitness = -INFINITY;

    double mean_fitness;
    double std_fitness;

    std::vector<double> fitnesses;
    for (int k = index * indv_ants; k < (index + 1) * indv_ants; k++)
    {
        fitnesses.push_back(double(ant[k].fitness));
        min_fitness = std::min(min_fitness, double(ant[k].fitness));
        max_fitness = std::max(max_fitness, double(ant[k].fitness));
    }

    mean_and_std(fitnesses, mean_fitness, std_fitness);

    return min_fitness;
}

void es_aco_init(){
    for (int i = 0; i < initial_nb_dims; i++){
        initialX.push_back(lowerBounds[i] + (new_rand01() * (upperBounds[i] - lowerBounds[i])));
        typicalX.push_back(lowerBounds[i] + (new_rand01() * (upperBounds[i] - lowerBounds[i])));
        initialStd.push_back((upperBounds[i] - lowerBounds[i]) / 5);
    }

    initialX[ALPHA_IDX] = typicalX[ALPHA_IDX] = 1.5;
    initialX[BETA_IDX] = typicalX[BETA_IDX] = 5.0;

    initialStd[ALPHA_IDX] =1.5;
    initialStd[BETA_IDX] = 2.0;

    printf("Popsize=%d\n", (long int)initial_lambda);
    if (iGreedyLevyFlag)
    {
        initial_nb_dims = ES_ACO_DIM + 3;
    }
    es_write_params();
    seed++;
    optimizer.init(eval_function, lowerBounds, upperBounds);
    ant.resize(indv_ants * optimizer.get("lambda"));
    prev_ls_ant.resize(indv_ants * optimizer.get("lambda"));
    max_packing_tries = 1;
}

void es_aco_construct_solutions()
{
    optimizer.run_a_generation();
    es_aco_set_best_params();
    
    const char *termination_reason = es_aco_termination_condition();
    if (termination_reason) {
        printf("\n%s, restart cames, ", termination_reason);
        // cmaes_flag = 0; return;
        es_aco_init();
    }
}

void ipop_cmaes_aco_construct_solutions()
{
    optimizer.run_a_generation();
    es_aco_set_best_params();

    if (es_aco_termination_condition())
    {
        printf("IPOP restart, ");
        initial_lambda *= inc_popsize;
        es_aco_init();
    }
}

void bipop_cmaes_aco_construct_solutions()
{
    optimizer.run_a_generation();
    es_aco_set_best_params();

    if (es_aco_termination_condition())
    {
        printf("BIPOP restart, ");
        long int n_eval = optimizer.get("lambda") * optimizer.get("generation");
        if (poptype == 0)
            small_n_eval += n_eval;
        else
            large_n_eval += n_eval;

        if (small_n_eval < large_n_eval)
        {
            poptype = 0;
            double popsize_multiplier = pow(inc_popsize, n_restarts);
            initial_lambda = floor(
                pow(popsize0 * popsize_multiplier, pow(new_rand01(), 2)));
        }
        else
        {
            poptype = 1;
            n_restarts += 1;
            initial_lambda = popsize0 * pow(inc_popsize, n_restarts);
        }
        es_aco_init();
    }
}

void es_aco_export_result()
{
    optimizer.end();
}

void es_aco_exit()
{
    optimizer.boundary_cmaes_exit();
}

const char* es_aco_termination_condition(){
    return optimizer.termination_condition();
}

void es_aco_set_best_params()
{
    double *xbestever = NULL;
    xbestever = optimizer.getInto("xbestever", xbestever);
    xbestever = optimizer.boundary_transformation(xbestever);

    alpha = xbestever[ALPHA_IDX];
    beta = xbestever[BETA_IDX];
    par_a = xbestever[PAR_A_IDX];
    par_b = xbestever[PAR_B_IDX];
    par_c = xbestever[PAR_C_IDX];
    // q_0 = xbestever[Q0_IDX];
    dGreedyEpsilon = xbestever[EPSILON_IDX];
    dGreedyLevyThreshold = xbestever[LEVY_THRESHOLD_IDX];
    dGreedyLevyRatio = xbestever[LEVY_RATIO_IDX];
#if RHO_TUNING_MACRO
    rho = xbestever[RHO_IDX];
#endif
#if Q0_TUNING_MACRO
    q_0 = xbestever[Q0_IDX];
#endif
#if TREE_MAP_MACRO
    neighbour_prob = xbestever[NEIGHBOUR_PROB_IDX];
#endif
    if (verbose > 0)
    {
        // printf("rho: %.4f\n", rho);
        // printf("q_0: %.4f\n", q_0);
#if TREE_MAP_MACRO
        printf("neighbour_prob: %.4f\n", neighbour_prob);
#endif
    }
}