#include "es_aco.h"

#define ALPHA_IDX 1
#define BETA_IDX  2
#define PAR_A_IDX 3
#define PAR_B_IDX 4
#define PAR_C_IDX 5

//                      alpha   beta  par_a  par_b  par_c
double lowerBounds[] = { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f}; 
double upperBounds[] = {10.0f, 10.0f,  1.0f,  1.0f,  1.0f};

// number of ants per individual
unsigned int indv_ants = 10;

int cmaes_flag = 0;

boundary_cmaes optimizer;

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
    for (int k = index * indv_ants; k < (index+1) * indv_ants; k++)
    {
        ant_empty_memory(&ant[k]);
    }

    /* Place the ants at initial city 0 and set the final city as n-1 */
    for (int k = index * indv_ants; k < (index+1) * indv_ants; k++)
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
        for (int k = index * indv_ants; k < (index+1) * indv_ants; k++)
        {
            if (ant[k].tour[ant[k].tour_size - 1] == instance.n - 2)
            { /* previous city is the last one */
                continue;
            }
            neighbour_choose_and_move_to_next(&ant[k], step);
            if (acs_flag)
                local_acs_pheromone_update(&ant[k], step);
            ant[k].tour_size++;
        }
    }

    for (int k = index * indv_ants; k < (index+1) * indv_ants; k++)
    {
        ant[k].tour[ant[k].tour_size++] = instance.n - 1;
        ant[k].tour[ant[k].tour_size++] = ant[k].tour[0];
        for (i = ant[k].tour_size; i < instance.n; i++)
            ant[k].tour[i] = 0;
        ant[k].fitness = compute_fitness_es(ant[k].tour, ant[k].visited, ant[k].tour_size, ant[k].packing_plan);
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

    for (int k = index * indv_ants; k < (index+1) * indv_ants; k++)
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
        ant[k].fitness = compute_fitness_es(ant[k].tour, ant[k].visited, ant[k].tour_size, ant[k].packing_plan);
        // if (termination_condition())
        //     return;
    }
}

//   index of current offspring         genotype   number of dims
double eval_function (int index, double const *x, unsigned long N){
    alpha = x[ALPHA_IDX];
    beta  = x[BETA_IDX];
    par_a = x[PAR_A_IDX];
    par_b = x[PAR_B_IDX];
    par_c = x[PAR_C_IDX];

    _es_construct_solutions(index);
    if (ls_flag > 0)
    {
        for (int k = index * indv_ants; k < (index+1) * indv_ants; k++)
        {
            copy_from_to(&ant[k], &prev_ls_ant[k]);
        }
        _es_local_search(index);
        for (int k = index * indv_ants; k < (index+1) * indv_ants; k++)
        {
            if (ant[k].fitness > prev_ls_ant[k].fitness)
            {
                copy_from_to(&prev_ls_ant[k], &ant[k]);
            }
        }
    }

    double min_fitness =  INFINITY;
    double max_fitness = -INFINITY;
    
    double mean_fitness;
    double std_fitness;

    std::vector<double> fitnesses;
    for (int k = index * indv_ants; k < (index+1) * indv_ants; k++)
    {
        fitnesses.push_back(double(ant[k].fitness));
        min_fitness = std::min(min_fitness, double(ant[k].fitness));
        max_fitness = std::max(max_fitness, double(ant[k].fitness));
    }

    // mean_and_std(fitnesses, mean_fitness, std_fitness);

    return min_fitness;
}

void es_aco_init(){
    optimizer.init(eval_function, lowerBounds, upperBounds);
    ant.resize(indv_ants * optimizer.get("lambda"));
    prev_ls_ant.resize(indv_ants * optimizer.get("lambda"));
}

void es_aco_construct_solutions(){
    optimizer.run_a_generation();
}

void es_aco_export_result(){
    optimizer.end();
}

void es_aco_exit(){
    optimizer.boundary_cmaes_exit();
}