/*

       AAAA    CCCC   OOOO   TTTTTT   SSSSS  PPPPP
      AA  AA  CC     OO  OO    TT    SS      PP  PP
      AAAAAA  CC     OO  OO    TT     SSSS   PPPPP
      AA  AA  CC     OO  OO    TT        SS  PP
      AA  AA   CCCC   OOOO     TT    SSSSS   PP

######################################################
##########    ACO algorithms for the TSP    ##########
######################################################

      Version: 1.0
      File:    main.c
      Author:  Thomas Stuetzle
      Purpose: main routines and control for the ACO algorithms
      Check:   README and gpl.txt
      Copyright (C) 2002  Thomas Stuetzle
 */

/***************************************************************************

    Program's name: acotsp

    Ant Colony Optimization algorithms (AS, ACS, EAS, RAS, MMAS, BWAS) for the
    symmetric TSP

    Copyright (C) 2004  Thomas Stuetzle

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    email: stuetzle no@spam ulb.ac.be
    mail address: Universite libre de Bruxelles
                  IRIDIA, CP 194/6
                  Av. F. Roosevelt 50
                  B-1050 Brussels
                  Belgium

 ***************************************************************************/

#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

#include "ants.h"
#include "utilities.h"
#include "inout.h"
#include "thop.h"
#include "timer.h"
#include "ls.h"
#include "node_clustering.h"
#include "adaptive_evaporation.h"
#include "es_ant.h"
#include "acothop.h"
#include "custom_strategy.h"
#include "tree_map.h"
#include "algo_config.h"
#include "es_aco.h"

long int termination_condition(void)
/*
      FUNCTION:       checks whether termination condition is met
      INPUT:          none
      OUTPUT:         0 if condition is not met, number neq 0 otherwise
      (SIDE)EFFECTS:  none
 */
{
    return (((n_tours >= max_tours) && (elapsed_time(VIRTUAL) >= max_time)) ||
            (best_so_far_ant->fitness <= optimal));
}

void construct_solutions(void)
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
    for (k = 0; k < ant.size(); k++)
    {
        ant_empty_memory(&ant[k]);
    }

    /* Place the ants at initial city 0 and set the final city as n-1 */
    for (k = 0; k < ant.size(); k++)
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
        for (k = 0; k < ant.size(); k++)
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

    for (k = 0; k < ant.size(); k++)
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

void local_search(void)
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

    for (k = 0; k < ant.size(); k++)
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
        if (termination_condition())
            return;
    }
}

void update_statistics(void)
/*
      FUNCTION:       manage some statistical information about the trial, especially
                      if a new best solution (best-so-far or restart-best) is found and
                      adjust some parameters if a new best solution is found
      INPUT:          none
      OUTPUT:         none
      (SIDE)EFFECTS:  restart-best and best-so-far ant may be updated; trail_min
                      and trail_max used by MMAS may be updated
 */
{

    long int iteration_best_ant;
    double p_x; /* only used by MMAS */

    iteration_best_ant = find_best(); /* iteration_best_ant is a global variable */

    write_iterations_report(iteration_best_ant);
    if (ant[iteration_best_ant].fitness < best_so_far_ant->fitness)
    {

        time_used = elapsed_time(VIRTUAL); /* best sol found after time_used */
        copy_from_to(&ant[iteration_best_ant], best_so_far_ant);

        found_best = iteration;
        restart_found_best = iteration;
        // found_branching = node_branching(lambda);
        // branching_factor = found_branching;
        if (mmas_flag)
        {
            if (!ls_flag)
            {
                p_x = exp(log(0.05) / instance.n);
                trail_min = 1. * (1. - p_x) / (p_x * (double)((nn_ants + 1) / 2));
                trail_max = 1. / ((rho)*best_so_far_ant->fitness);
                trail_0 = trail_max;
                trail_min = trail_max * trail_min;
            }
            else
            {
                trail_max = 1. / ((rho)*best_so_far_ant->fitness);
                trail_min = trail_max / (2. * instance.n);
                trail_0 = trail_max;
            }
        }
        write_report();
    }
    if (ant[iteration_best_ant].fitness < restart_best_ant->fitness)
    {
        copy_from_to(&ant[iteration_best_ant], restart_best_ant);
        restart_found_best = iteration;
        /*printf("restart best: %ld, restart_found_best %ld, time %.2f\n",restart_best_ant->fitness, restart_found_best, elapsed_time ( VIRTUAL ));*/
    }
}

void search_control_and_statistics(void)

/*
      FUNCTION:       occasionally compute some statistics and check whether algorithm
                      is converged
      INPUT:          none
      OUTPUT:         none
      (SIDE)EFFECTS:  restart-best and best-so-far ant may be updated; trail_min
                      and trail_max used by MMAS may be updated
 */
{
    TRACE(printf("SEARCH CONTROL AND STATISTICS\n"););

    if (!(iteration % 100))
    {
        population_statistics();
        /*printf("\nbest so far %ld, iteration: %ld, time %.2f, b_fac %.5f\n",best_so_far_ant->fitness,iteration,elapsed_time( VIRTUAL),branching_factor);*/
        if (mmas_flag && (iteration - restart_found_best > 250))
        {
            branching_factor = node_branching(lambda);
            if (branching_factor < branch_fac)
            {
                /* MAX-MIN Ant System was the first ACO algorithm to use
                   pheromone trail re-initialisation as implemented
                   here. Other ACO algorithms may also profit from this mechanism.
                 */
                /*printf("INIT TRAILS!!!\n"); restart_best_ant->fitness = INFTY;*/
                init_pheromone_trails(trail_max);
                compute_total_information();
                restart_iteration = iteration;
                restart_time = elapsed_time(VIRTUAL);
            }
        }
        /*printf("try %li, iteration %li, b-fac %f \n\n", n_try,iteration,branching_factor);*/
    }
}

void as_update(void)
/*
      FUNCTION:       manage global pheromone deposit for Ant System
      INPUT:          none
      OUTPUT:         none
      (SIDE)EFFECTS:  all ants deposit pheromones on matrix "pheromone"
 */
{
    long int k;

    TRACE(printf("Ant System pheromone deposit\n"););

    for (k = 0; k < ant.size(); k++)
        global_update_pheromone(&ant[k]);
}

void eas_update(void)
/*
      FUNCTION:       manage global pheromone deposit for Elitist Ant System
      INPUT:          none
      OUTPUT:         none
      (SIDE)EFFECTS:  all ants plus elitist ant deposit pheromones on matrix "pheromone"
 */
{
    long int k;

    TRACE(printf("Elitist Ant System pheromone deposit\n"););

    for (k = 0; k < ant.size(); k++)
        global_update_pheromone(&ant[k]);
    global_update_pheromone_weighted(best_so_far_ant, elitist_ants);
}

void ras_update(void)
/*
      FUNCTION:       manage global pheromone deposit for Rank-based Ant System
      INPUT:          none
      OUTPUT:         none
      (SIDE)EFFECTS:  the ras_ranks-1 best ants plus the best-so-far ant deposit pheromone
                      on matrix "pheromone"
      COMMENTS:       this procedure could be implemented slightly faster, but it is
                      anyway not critical w.r.t. CPU time given that ras_ranks is
                      typically very small.
 */
{
    long int i, k, b, target;
    long int *help_b;

    TRACE(printf("Rank-based Ant System pheromone deposit\n"););

    help_b = (long int *)malloc(ant.size() * sizeof(long int));
    for (k = 0; k < ant.size(); k++)
        help_b[k] = ant[k].fitness;

    for (i = 0; i < ras_ranks - 1; i++)
    {
        b = help_b[0];
        target = 0;
        for (k = 0; k < ant.size(); k++)
        {
            if (help_b[k] < b)
            {
                b = help_b[k];
                target = k;
            }
        }
        help_b[target] = LONG_MAX;
        global_update_pheromone_weighted(&ant[target], ras_ranks - i - 1);
    }
    global_update_pheromone_weighted(best_so_far_ant, ras_ranks);
    free(help_b);
}

void mmas_update(void)
/*
      FUNCTION:       manage global pheromone deposit for MAX-MIN Ant System
      INPUT:          none
      OUTPUT:         none
      (SIDE)EFFECTS:  either the iteration-best or the best-so-far ant deposit pheromone on matrix "pheromone"
 */
{
    /* we use default upper pheromone trail limit for MMAS and hence we
       do not have to worry regarding keeping the upper limit */

    long int iteration_best_ant;

    TRACE(printf("MAX-MIN Ant System pheromone deposit\n"););

    if (iteration % u_gb)
    {
        iteration_best_ant = find_best();
        global_update_pheromone(&ant[iteration_best_ant]);
    }
    else
    {
        if (u_gb == 1 && (iteration - restart_found_best > 50))
            global_update_pheromone(best_so_far_ant);
        else
            global_update_pheromone(restart_best_ant);
    }

    if (ls_flag)
    {
        /* implement the schedule for u_gb as defined in the
           Future Generation Computer Systems article or in Stuetzle's PhD thesis.
           This schedule is only applied if local search is used.
         */
        if ((iteration - restart_iteration) < 25)
            u_gb = 25;
        else if ((iteration - restart_iteration) < 75)
            u_gb = 5;
        else if ((iteration - restart_iteration) < 125)
            u_gb = 3;
        else if ((iteration - restart_iteration) < 250)
            u_gb = 2;
        else
            u_gb = 1;
    }
    else
        u_gb = 25;
}

void bwas_update(void)
/*
      FUNCTION:       manage global pheromone deposit for Best-Worst Ant System
      INPUT:          none
      OUTPUT:         none
      (SIDE)EFFECTS:  either the iteration-best or the best-so-far ant deposit pheromone
                      on matrix "pheromone"
 */
{
    long int iteration_worst_ant, distance_best_worst;

    TRACE(printf("Best-worst Ant System pheromone deposit\n"););

    global_update_pheromone(best_so_far_ant);
    iteration_worst_ant = find_worst();
    bwas_worst_ant_update(&ant[iteration_worst_ant], best_so_far_ant);
    distance_best_worst = distance_between_ants(best_so_far_ant, &ant[iteration_worst_ant]);
    /*    printf("distance_best_worst %ld, tour length worst %ld\n",distance_best_worst,ant[iteration_worst_ant].fitness); */
    if (distance_best_worst < (long int)(0.05 * (double)instance.n))
    {
        // restart_best_ant->fitness = INFTY;
        init_pheromone_trails(trail_0);
        restart_iteration = iteration;
        restart_time = elapsed_time(VIRTUAL);
        /*printf("init pheromone trails with %.15f, iteration %ld\n",trail_0,iteration);*/
    }
    else
        bwas_pheromone_mutation();
}

void acs_global_update(void)
/*
      FUNCTION:       manage global pheromone deposit for Ant Colony System
      INPUT:          none
      OUTPUT:         none
      (SIDE)EFFECTS:  the best-so-far ant deposits pheromone on matrix "pheromone"
      COMMENTS:       global pheromone deposit in ACS is done per default using
                      the best-so-far ant; Gambardella & Dorigo examined also iteration-best
                      update (see their IEEE Trans. on Evolutionary Computation article),
                      but did not use it for the published computational results.
 */
{
    TRACE(printf("Ant colony System global pheromone deposit\n"););

    global_acs_pheromone_update(best_so_far_ant);
}

void pheromone_trail_update(void)
/*
      FUNCTION:       manage global pheromone trail update for the ACO algorithms
      INPUT:          none
      OUTPUT:         none
      (SIDE)EFFECTS:  pheromone trails are evaporated and pheromones are deposited
                      according to the rules defined by the various ACO algorithms.
 */
{
    if (fitness_entropy_flag)
        adaptive_mechanism();
    else if (adaptive_evaporation_flag && !(tree_map_flag && es_ant_flag))
        update_rho();


/* Simulate the pheromone evaporation of all pheromones; this is not necessary
   for ACS (see also ACO Book) */
#if TREE_MAP_MACRO
    if (tree_map_flag)
        tree_map->global_evaporate(trail_min);
    else
#endif
    if (!acs_flag)
    {
        if (o1_evap_flag)
            o1_global_evaporate();
        else if (node_clustering_flag)
            evaporation_nc_list();
        else if (as_flag || eas_flag || ras_flag || bwas_flag || mmas_flag)
        {
            if (ls_flag)
            {
                if (mmas_flag)
                    mmas_evaporation_nn_list();
                else
                    evaporation_nn_list();
                /* evaporate only pheromones on arcs of candidate list to make the
                pheromone evaporation faster for being able to tackle large TSP
                instances. For MMAS additionally check lower pheromone trail limits.
                */
            }
            else
            {
                /* if no local search is used, evaporate all pheromone trails */
                evaporation();
            }
        }
    }

    /* Next, apply the pheromone deposit for the various ACO algorithms */
    if (tree_map_flag || mmas_flag)
        mmas_update();
    else if (as_flag)
        as_update();
    else if (eas_flag)
        eas_update();
    else if (ras_flag)
        ras_update();
    else if (bwas_flag)
        bwas_update();
    else if (acs_flag)
        acs_global_update();

    /* check pheromone trail limits for MMAS; not necessary if local
     search is used, because in the local search case lower pheromone trail
     limits are checked in procedure mmas_evaporation_nn_list */
    if (
        mmas_flag && !ls_flag &&
        !adaptive_evaporation_flag &&
        !tree_map_flag && !o1_evap_flag)
        check_pheromone_trail_limits();

    /* Compute combined information pheromone times heuristic info after
     the pheromone update for all ACO algorithms except ACS; in the ACS case
     this is already done in the pheromone update procedures of ACS */
    if (!acs_flag && !es_ant_flag && !tree_map_flag)
    {
        if (node_clustering_flag)
            compute_total_information();
        else if (as_flag || eas_flag || ras_flag || mmas_flag || bwas_flag)
        {
            if (ls_flag)
            {
                compute_nn_list_total_information();
            }
            else
            {
                compute_total_information();
            }
        }
    }
}

/* --- main program ------------------------------------------------------ */

int main(int argc, char *argv[])
{
    /*
      FUNCTION:       main control for running the ACO algorithms
      INPUT:          none
      OUTPUT:         none
      (SIDE)EFFECTS:  none
      COMMENTS:       this function controls the run of "max_tries" independent trials
     */

    long int i, k;

    start_timers();

    init_program(argc, argv);

    if (verbose > 0)
    {
        printf("Seed=%ld, Upper Bound=%ld\n", seed, instance.UB);
    }

    if (adaptive_evaporation_flag) init_adaptive_mechanism();

    time_used = elapsed_time(VIRTUAL);
    /*printf("Initialization took %.10f seconds\n",time_used);*/
    for (n_try = 0; n_try < max_tries; n_try++)
    {
        init_try(n_try);
        // printf("%dth try \n", n_try + 1);

        while (!termination_condition())
        {
            if (cmaes_flag)
                es_aco_construct_solutions();
            else if (ipopcmaes_flag)
                ipop_cmaes_aco_construct_solutions();
            else if (bipopcmaes_flag)
                bipop_cmaes_aco_construct_solutions();
#if ES_ANT_MACRO
            else if (es_ant_flag)
                es_ant_construct_and_local_search();
#endif
            else
            {
#if TREE_MAP_MACRO
                if (tree_map_flag)
                    tree_map_construct_solutions();
                else
#endif
                    if (node_clustering_flag)
                    construct_node_clustering_solution();
                else
                    construct_solutions();

                if (ls_flag > 0)
                {
                    for (k = 0; k < ant.size(); k++)
                    {
                        copy_from_to(&ant[k], &prev_ls_ant[k]);
                    }
                    local_search();
                    for (k = 0; k < ant.size(); k++)
                    {
                        if (ant[k].fitness > prev_ls_ant[k].fitness)
                        {
                            copy_from_to(&prev_ls_ant[k], &ant[k]);
                        }
                    }
                }
            }

            update_statistics();
            pheromone_trail_update();
            search_control_and_statistics();

            iteration++;
        }

        if (verbose > 0)
        {
            printf("Number of iterations: %ld\n", iteration);
            printf("Number of times that CAMES restarts: %ld\n", n_restarts);
            // printf("seed: %ld\n", seed);
        }

        exit_try(n_try);
        // if (cmaes_flag) es_aco_export_result();
    }

    exit_program();
    if (cmaes_flag || ipopcmaes_flag || bipopcmaes_flag)
        es_aco_exit();
#if ES_ANT_MACRO
    if (es_ant_flag)
        es_ant_deallocate();
#endif
#if TREE_MAP_MACRO
    if (tree_map_flag)
        tree_map_deallocate();
#endif

    free(instance.distance);
    free(instance.nn_list);
    free(pheromone);
    free(total);
    free(best_in_try);
    free(best_found_at);
    free(time_best_found);
    free(time_total_run);

    free(best_so_far_ant->tour);
    free(best_so_far_ant->visited);
    free(best_so_far_ant->packing_plan);

    free(restart_best_ant->tour);
    free(restart_best_ant->visited);
    free(restart_best_ant->packing_plan);

    free(global_best_ant->tour);
    free(global_best_ant->visited);
    free(global_best_ant->packing_plan);

    free(prob_of_selection);
    return 0;
}
