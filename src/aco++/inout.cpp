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
      File:    InOut.c
      Author:  Thomas Stuetzle
      Purpose: mainly input / output / statistic routines
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#include "inout.h"
#include "thop.h"
#include "timer.h"
#include "utilities.h"
#include "ants.h"
#include "ls.h"
#include "parse.h"
#include "node_clustering.h"
#include "adaptive_evaporation.h"
#include "es_ant.h"
#include "tree_map.h"
#include "algo_config.h"
#include "es_aco.h"

extern int iLevyFlag;         // 0 or 1
extern double dLevyThreshold; // 0--1
extern double dLevyRatio;     // 0.1--5

extern double dContribution; // 0--10

extern double dGreedyEpsilon;       // 0--1
extern double dGreedyLevyThreshold; // 0--1
extern double dGreedyLevyRatio;     // 0.1--5

long int *best_in_try;
long int *best_found_at;
double *time_best_found;
double *time_total_run;

long int n_try;             /* try counter */
long int n_tours;           /* counter of number constructed tours */
long int iteration;         /* iteration counter */
long int restart_iteration; /* remember iteration when restart was done if any */
double restart_time;        /* remember time when restart was done if any */
long int max_tries;         /* maximum number of independent tries */
long int max_tours;         /* maximum number of tour constructions in one try */
long int max_packing_tries; /* number of tries to construct a good packing plan from a give tour */
long int seed;

double lambda;     /* Parameter to determine branching factor */
double branch_fac; /* If branching factor < branch_fac => update trails */

double max_time;    /* maximal allowed run time of a try  */
double time_used;   /* time used until some given event */
double time_passed; /* time passed until some moment*/
long int optimal;   /* optimal solution or bound to find */

double mean_ants;        /* average tour length */
double stddev_ants;      /* stddev of tour lengths */
double branching_factor; /* average node branching factor when searching */
double found_branching;  /* branching factor when best solution is found */

long int found_best;         /* iteration in which best solution is found */
long int restart_found_best; /* iteration in which restart-best solution is found */

/* ------------------------------------------------------------------------ */

FILE *log_file;
FILE *log_tries_file;

char input_name_buf[LINE_BUF_LEN];
char output_name_buf[LINE_BUF_LEN];
int opt;
long int log_flag;     /* --log was given in the command-line.  */
long int logiter_flag; /* --log was given in the command-line.  */
long int output_flag;
long int calibration_mode;

// ---
double par_a_mean, par_b_mean, par_c_mean,
    par_a_std, par_b_std, par_c_std,
    alpha_mean, beta_mean, rho_mean,
    alpha_std, beta_std, rho_std,
    elite_prob_mean, elite_prob_std,
    neighbour_prob_mean, neighbour_prob_std;
unsigned int indv_ants;
// ---

void init_program(long int argc, char *argv[])
/*
      FUNCTION:       initialize the program,
      INPUT:          program arguments, needed for parsing commandline
      OUTPUT:         none
      COMMENTS:
 */
{

    char temp_buffer[LINE_BUF_LEN];

    /*printf(PROG_ID_STR);*/

    set_default_parameters();
    setbuf(stdout, NULL);
    parse_commandline(argc, argv);

    rand_gen.seed(seed);

    assert(max_tries <= MAXIMUM_NO_TRIES);

    best_in_try = (long int *)calloc(max_tries, sizeof(long int));
    best_found_at = (long int *)calloc(max_tries, sizeof(long int));
    time_best_found = (double *)calloc(max_tries, sizeof(double));
    time_total_run = (double *)calloc(max_tries, sizeof(double));

    TRACE(printf("read problem data  ..\n\n");)
    read_thop_instance(input_name_buf, &instance.nodeptr, &instance.itemptr);
    TRACE(printf("\n .. done\n\n");)

    if (max_time < 0)
    {
        /* Change default parameter max_time for ceil(number of items * 0.1) */
        max_time = ceil(instance.m / 10.0);
    }

    nn_ants = MIN(nn_ants, instance.n);

    if (n_ants < 0)
        n_ants = instance.n;
    /* default setting for elitist_ants is 0; if EAS is applied and
       option elitist_ants is not used, we set the default to
       elitist_ants = instance.n */
    if (eas_flag && elitist_ants <= 0)
        elitist_ants = instance.n;

    nn_ls = MIN(instance.n - 1, nn_ls);

    assert(n_ants < MAX_ANTS - 1);
    assert(nn_ants < MAX_NEIGHBOURS);
    assert(nn_ants > 0);
    assert(nn_ls > 0);

    if (!log_flag)
    {
        sprintf(temp_buffer, "%s.log", output_name_buf);
        log_file = fopen(temp_buffer, "w");
    }
    else
    {
        log_file = NULL;
    }

    if (!logiter_flag)
    {
        sprintf(temp_buffer, "%s.tries.log", output_name_buf);
        log_tries_file = fopen(temp_buffer, "w");
    }
    else
    {
        log_tries_file = NULL;
    }

    instance.distance = compute_distances();

    write_params();

    allocate_ants();

#if ES_ANT_MACRO
    if (es_ant_flag)
        es_ant_init();
#endif
#if TREE_MAP_MACRO
    if (tree_map_flag)
        tree_map_init();
#endif

    instance.nn_list = compute_nn_lists();
    if (!tree_map_flag)
    {
        pheromone = generate_double_matrix(instance.n, instance.n);
    }

    if (!es_ant_flag && !tree_map_flag && !o1_evap_flag &&
        !cmaes_flag && !ipopcmaes_flag && !bipopcmaes_flag)
    {
        total = generate_double_matrix(instance.n, instance.n);
    }

    if (node_clustering_flag == TRUE)
        create_cluster();

    if (o1_evap_flag)
        o1_init_program();

    if (cmaes_flag || ipopcmaes_flag || bipopcmaes_flag)
        es_aco_init_program();
}

void exit_program(void)
/*
      FUNCTION:       save some final statistical information on a trial once it finishes
      INPUT:          none
      OUTPUT:         none
      COMMENTS:
 */
{
    if (log_file)
    {
        fprintf(log_file, "\n\n");
        long int ntry = 0;
        for (; ntry < max_tries; ntry++)
        {
            fprintf(log_file, "try %10ld,        best %10ld,        found at iteration %10ld,        found at time %10.2f\n", ntry, instance.UB + 1 - best_in_try[ntry], best_found_at[ntry], time_best_found[ntry]);
            fflush(log_file);
        }
    }

    long int profit = instance.UB + 1 - global_best_ant->fitness;

    if (calibration_mode)
        printf("%ld\n", -profit);
    else
        printf("Best solution: %ld\n", profit);

    if (output_flag)
        save_best_thop_solution();
}

void init_try(long int ntry)
/*
      FUNCTION: initilialize variables appropriately when starting a trial
      INPUT:    trial number
      OUTPUT:   none
      COMMENTS: none
 */
{

    TRACE(printf("INITIALIZE TRIAL\n"););

    start_timers();
    time_used = elapsed_time(VIRTUAL);
    time_passed = time_used;

    if (adaptive_evaporation_flag)
        init_adaptive_mechanism();
    if (o1_evap_flag)
        o1_init_try(); // must before init trail

    /* Initialize variables concerning statistics etc. */

    n_tours = 1;
    iteration = 1;
    restart_iteration = 1;
    lambda = 0.05;
    best_so_far_ant->fitness = INFTY;
    found_best = 0;

    /* Initialize the Pheromone trails, only if ACS is used, pheromones
       have to be initialized differently */
    if (!(acs_flag || mmas_flag || bwas_flag))
    {
        trail_0 = 1. / ((rho)*nn_tour());
        /* in the original papers on Ant System, Elitist Ant System, and
           Rank-based Ant System it is not exactly defined what the
           initial value of the pheromones is. Here we set it to some
           small constant, analogously as done in MAX-MIN Ant System.
         */
        init_pheromone_trails(trail_0);
    }
    if (bwas_flag)
    {
        trail_0 = 1. / ((double)instance.n * (double)nn_tour());
        init_pheromone_trails(trail_0);
    }
    if (mmas_flag)
    {
        trail_max = 1. / ((rho)*nn_tour());
        trail_min = trail_max / (2. * instance.n);
        init_pheromone_trails(trail_max);
    }
    if (acs_flag)
    {
        trail_0 = 1. / ((double)instance.n * (double)nn_tour());
        init_pheromone_trails(trail_0);
    }

    /* Calculate combined information pheromone times heuristic information */
    compute_total_information();

    if (log_file)
        fprintf(log_file, "\nbegin try %li \n", ntry);
    if (log_tries_file)
        fprintf(log_tries_file, "begin try %li \n", ntry);

    if (cmaes_flag || ipopcmaes_flag || bipopcmaes_flag)
        es_aco_init();

    if (verbose > 1)
    {
        printf("global_evap_times: %.4f", global_evap_times);
        printf("global_restart_times: %.4f", global_restart_times);
    }
}

void exit_try(long int ntry)
/*
      FUNCTION:       save some statistical information on a trial once it finishes
      INPUT:          trial number
      OUTPUT:         none
      COMMENTS:
 */
{
    best_in_try[ntry] = best_so_far_ant->fitness;
    best_found_at[ntry] = found_best;
    time_best_found[ntry] = time_used;
    time_total_run[ntry] = elapsed_time(VIRTUAL);

    if (best_so_far_ant->fitness < global_best_ant->fitness)
    {
        copy_from_to(best_so_far_ant, global_best_ant);
    }

    if (log_file)
        fprintf(log_file, "end try %li \n", ntry);
    if (log_tries_file)
        fprintf(log_tries_file, "end try %li \n", ntry);
}

void read_thop_instance(const char *input_file_name, struct point **nodeptr, struct item **itemptr)
/*
      FUNCTION: parse and read instance file
      INPUT:    instance name
      OUTPUT:   list of coordinates for all nodes
      COMMENTS: Instance files have to be in TSPLIB format, otherwise procedure fails
 */
{
    FILE *input_file;
    char buf[LINE_BUF_LEN];
    long int i, j, k;

    input_file = fopen(input_file_name, "r");
    if (input_file == NULL)
    {
        fprintf(stderr, "No instance file specified, abort\n");
        exit(1);
    }
    assert(input_file != NULL);
    /*printf("\nreading thop-file %s ... \n\n", input_file_name);*/

    fscanf(input_file, "PROBLEM NAME: %s\n", buf);
    fscanf(input_file, "KNAPSACK DATA TYPE: %[^\n]\n", instance.knapsack_data_type);
    fscanf(input_file, "DIMENSION: %ld\n", &instance.n);
    ++instance.n;
    assert(instance.n > 3 && instance.n < 6000);
    fscanf(input_file, "NUMBER OF ITEMS: %ld\n", &instance.m);
    fscanf(input_file, "CAPACITY OF KNAPSACK: %ld\n", &instance.capacity_of_knapsack);
    fscanf(input_file, "MAX TIME: %lf\n", &instance.max_time);
    fscanf(input_file, "MIN SPEED: %lf\n", &instance.min_speed);
    fscanf(input_file, "MAX SPEED: %lf\n", &instance.max_speed);
    fscanf(input_file, "EDGE_WEIGHT_TYPE: %s\n", buf);
    if (strcmp("EUC_2D", buf) == 0)
    {
        distance = round_distance;
        distance_with_coordinate = euclid_distance;
    }
    else if (strcmp("CEIL_2D", buf) == 0)
    {
        distance = ceil_distance;
        distance_with_coordinate = ceil_distance;
    }
    else if (strcmp("GEO", buf) == 0)
    {
        distance = geo_distance;
        distance_with_coordinate = geo_distance;
    }
    else if (strcmp("ATT", buf) == 0)
    {
        distance = att_distance;
        distance_with_coordinate = att_distance;
    }
    fgets(buf, LINE_BUF_LEN, input_file); /* NODE_COORD_SECTION  (INDEX, X, Y): */

    if ((*nodeptr = (point *)malloc(sizeof(struct point) * (instance.n))) == NULL)
        exit(EXIT_FAILURE);
    else
    {
        for (i = 0; i < instance.n - 1; i++)
        {
            fscanf(input_file, "%ld %lf %lf\n", &j, &(*nodeptr)[i].x, &(*nodeptr)[i].y);
        }
    }
    TRACE(printf("number of cities is %ld\n", n);)

    fgets(buf, LINE_BUF_LEN, input_file); /* ITEMS SECTION    (INDEX, PROFIT, WEIGHT, ASSIGNED NODE NUMBER): */

    if ((*itemptr = (item *)malloc(sizeof(struct item) * instance.m)) == NULL)
        exit(EXIT_FAILURE);
    else
    {
        for (i = 0; i < instance.m; i++)
        {
            fscanf(input_file, "%ld %ld %ld %ld\n", &j, &(*itemptr)[i].profit, &(*itemptr)[i].weight, &(*itemptr)[i].id_city);
            (*itemptr)[i].id_city -= 1;
        }
    }

    double *item_vector = (double *)malloc(instance.m * sizeof(double));
    double *help_vector = (double *)malloc(instance.m * sizeof(double));

    for (j = 0; j < instance.m; j++)
    {
        item_vector[j] = (-1.0 * (*itemptr)[j].profit) / (*itemptr)[j].weight;
        help_vector[j] = j;
    }

    sort2_double(item_vector, help_vector, 0, instance.m - 1);

    instance.UB = 0;
    long int _w = 0;
    for (k = 0; k < instance.m; k++)
    {
        j = help_vector[k];
        if (_w + (*itemptr)[j].weight <= instance.capacity_of_knapsack)
        {
            _w += (*itemptr)[j].weight;
            instance.UB += (*itemptr)[j].profit;
        }
        else
        {
            instance.UB += ceil((instance.capacity_of_knapsack - _w) / (double)(*itemptr)[j].weight * (*itemptr)[j].profit);
            break;
        }
    }

    free(item_vector);
    free(help_vector);

    TRACE(printf("number of items is %ld\n", instance.m);)
    TRACE(printf("\n... done\n");)

    fclose(input_file);
}

void set_default_parameters(void)
/*
      FUNCTION: set default parameter settings
      INPUT:    none
      OUTPUT:   none
      COMMENTS: none
 */
{
    ls_flag = 1;
    dlb_flag = TRUE; /* apply don't look bits in local search */
    nn_ls = 20;      /* use fixed radius search in the 20 nearest neighbours */
    n_ants = 300;    /* number of ants */
    nn_ants = 20;    /* number of nearest neighbours in tour construction */
    q_0 = 0.0;
    max_tries = 1;
    max_tours = 0;
    max_packing_tries = 1;
    seed = (long int)time(NULL);
    max_time = -1;
    optimal = 1;
    branch_fac = 1.00001;
    u_gb = INFTY;
    as_flag = FALSE;
    eas_flag = FALSE;
    ras_flag = FALSE;
    mmas_flag = TRUE;
    bwas_flag = FALSE;
    acs_flag = FALSE;
    ras_ranks = 0;
    elitist_ants = 0;
    // --adapt_evap --cmaes --lambda 18.0 --mean_ary 0.7681441:5.637383:0.804593:0.78718877:0.05282157:0.14134586:0.4584047 --std_ary 0.01:1.4254404:0.20251788:0.11338539:0.01:0.30373466:0.23759665 --adpt_rho 0.4587274:0.26949337:0.7514611 --indv_ants 9.0:8.064333:57.13875
    initial_lambda = 18;

    alpha = alpha_mean = 0.7681441;
    alpha_std = 0.01;

    beta = beta_mean = 5.637383;
    beta_std = 1.4254404;

    par_a_mean = 0.804593;
    par_a_std = 0.20251788;

    par_b_mean = 0.78718877;
    par_b_std = 0.11338539;

    par_c_mean = 0.05282157;
    par_c_std = 0.01;

    elite_prob = elite_prob_mean = 0.14134586;
    elite_prob_std = 0.30373466;

    neighbour_prob = neighbour_prob_mean = 0.4584047;
    neighbour_prob_std = 0.23759665;

    rho = init_rho = 0.4587274;
    min_rho = 0.26949337;
    max_rho = 0.7514611;

    indv_ants = init_indv_ants = 9.0;
    min_indv_ants = 8.064333;
    max_indv_ants = 57.13875;

    // no need for tuning
    min_min_rho = 0.01;
    max_max_rho = 0.99;
    const double temp_min_rho = (min_rho - min_min_rho) / (max_max_rho - min_min_rho);
    const double temp_max_rho = (max_rho - min_min_rho) / (max_max_rho - min_min_rho);

    left_rho = left_rho_mean = temp_min_rho;
    _mid_rho = _mid_rho_mean = temp_max_rho - temp_min_rho;
    right_rho = right_rho_mean = 1 - temp_max_rho;
    left_rho_std = _mid_rho_std = right_rho_std = 1.0 / 5;
    //

    rho_mean = 0.5;
    rho_std = 0.2;

    n_cluster = 4;
    cluster_size = 16;
    n_sector = 8;
}

void set_default_as_parameters(void)
{
    assert(as_flag);
}

void set_default_eas_parameters(void)
{
    assert(eas_flag);
    elitist_ants = n_ants;
}

void set_default_ras_parameters(void)
{
    assert(ras_flag);
    ras_ranks = 6;
}

void set_default_bwas_parameters(void)
{
    assert(bwas_flag);
}

void set_default_mmas_parameters(void)
{
    assert(mmas_flag);
}

void set_default_acs_parameters(void)
{
    assert(acs_flag);
    q_0 = 0.9;
}

void set_default_ls_parameters(void)
{
    assert(ls_flag);
}

void set_default_node_clustering_parameters(void)
{
    q_0 = 0.98;
}

void save_best_thop_solution(void)
{

    int i, first_print;
    char *visited = (char *)calloc(instance.n, sizeof(char));
    visited[0] = visited[instance.n - 2] = 1;

    long int profit = 0.0;

    for (i = 0; i < instance.m; i++)
    {
        if (global_best_ant->packing_plan[i])
        {
            visited[instance.itemptr[i].id_city] = 1;
            profit += instance.itemptr[i].profit;
        }
    }

    FILE *sol_file = fopen(output_name_buf, "w");

    first_print = TRUE;
    fprintf(sol_file, "[");
    for (i = 1; i < global_best_ant->tour_size - 3; i++)
    {
        if (visited[global_best_ant->tour[i]])
        {
            if (first_print == TRUE)
            {
                first_print = FALSE;
                fprintf(sol_file, "%ld", global_best_ant->tour[i] + 1);
            }
            else
                fprintf(sol_file, ",%ld", global_best_ant->tour[i] + 1);
        }
    }
    fprintf(sol_file, "]\n[");

    first_print = TRUE;
    for (i = 0; i < instance.m; i++)
    {
        if (global_best_ant->packing_plan[i])
        {
            if (first_print == TRUE)
            {
                first_print = FALSE;
                fprintf(sol_file, "%d", i + 1);
            }
            else
                fprintf(sol_file, ",%d", i + 1);
        }
    }
    fprintf(sol_file, "]\n");
    free(visited);

    fclose(sol_file);
}

void write_report(void)
/*
      FUNCTION: output some info about trial (best-so-far solution quality, time)
      INPUT:    none
      OUTPUT:   none
      COMMENTS: none
 */
{
    if (log_file)
    {
        fprintf(log_file, "best %10ld,        iteration: %10ld,        time %10.2f\n", instance.UB + 1 - best_so_far_ant->fitness, iteration, elapsed_time(VIRTUAL));
        fflush(log_file);
    }
}

void write_iterations_report(long int iteration_best_ant)
{

    if (log_tries_file)
    {

        int i, first_print;
        char *visited = (char *)calloc(instance.n, sizeof(char));
        visited[0] = visited[instance.n - 2] = 1;

        long int profit = 0.0;

        for (i = 0; i < instance.m; i++)
        {
            if (ant[iteration_best_ant].packing_plan[i])
            {
                visited[instance.itemptr[i].id_city] = 1;
                profit += instance.itemptr[i].profit;
            }
        }

        fprintf(log_tries_file, "%ld,%ld,%ld,%.2f\n", iteration, profit, iteration, elapsed_time(VIRTUAL));

        first_print = TRUE;
        fprintf(log_tries_file, "[");
        for (i = 1; i < ant[iteration_best_ant].tour_size - 3; i++)
        {
            if (visited[ant[iteration_best_ant].tour[i]])
            {
                if (first_print == TRUE)
                {
                    first_print = FALSE;
                    fprintf(log_tries_file, "%ld", ant[iteration_best_ant].tour[i] + 1);
                }
                else
                    fprintf(log_tries_file, ",%ld", ant[iteration_best_ant].tour[i] + 1);
            }
        }
        fprintf(log_tries_file, "]\n[");

        first_print = TRUE;
        for (i = 0; i < instance.m; i++)
        {
            if (ant[iteration_best_ant].packing_plan[i])
            {
                if (first_print == TRUE)
                {
                    first_print = FALSE;
                    fprintf(log_tries_file, "%d", i + 1);
                }
                else
                    fprintf(log_tries_file, ",%d", i + 1);
            }
        }
        fprintf(log_tries_file, "]\n");
        free(visited);

        fflush(log_tries_file);
    }
}

void write_params(void)
/*
      FUNCTION:       writes chosen parameter settings in log file
      INPUT:          none
      OUTPUT:         none
 */
{
    if (log_file)
    {
        fprintf(log_file, "Parameter-settings: \n\n");
        fprintf(log_file, "--inputfile               %s\n", input_name_buf);
        fprintf(log_file, "--outputfile              %s\n", output_name_buf);
        fprintf(log_file, "--tries                   %ld\n", max_tries);
        fprintf(log_file, "--tours                   %ld\n", max_tours);
        fprintf(log_file, "--ptries                  %ld\n", max_packing_tries);
        fprintf(log_file, "--time                    %.2f\n", max_time);
        fprintf(log_file, "--seed                    %ld\n", seed);
        fprintf(log_file, "--optimum                 %ld\n", optimal);
        fprintf(log_file, "--ants                    %ld\n", n_ants);
        fprintf(log_file, "--nnants                  %ld\n", nn_ants);
        fprintf(log_file, "--alpha                   %.2f\n", alpha);
        fprintf(log_file, "--beta                    %.2f\n", beta);
        fprintf(log_file, "--rho                     %.2f\n", rho);
        fprintf(log_file, "--q0                      %.2f\n", q_0);
        fprintf(log_file, "--elitistants             %ld\n", elitist_ants);
        fprintf(log_file, "--rasranks                %ld\n", ras_ranks);
        fprintf(log_file, "--localsearch             %ld\n", ls_flag);
        fprintf(log_file, "--nnls                    %ld\n", nn_ls);
        fprintf(log_file, "--dlb                     %ld\n", dlb_flag);
        fprintf(log_file, "--as                      %ld\n", as_flag);
        fprintf(log_file, "--eas                     %ld\n", eas_flag);
        fprintf(log_file, "--ras                     %ld\n", ras_flag);
        fprintf(log_file, "--mmas                    %ld\n", mmas_flag);
        fprintf(log_file, "--bwas                    %ld\n", bwas_flag);
        fprintf(log_file, "--acs                     %ld\n", acs_flag);
        fprintf(log_file, "--contribution_ratio      %ld\n", dContribution);
        fprintf(log_file, "--greedy_levy_flag        %ld\n", iGreedyLevyFlag);
        fprintf(log_file, "--greedy_epsilon          %lf\n", dGreedyEpsilon);
        fprintf(log_file, "--greedy_levy_threadhold  %lf\n", dGreedyLevyThreshold);
        fprintf(log_file, "--greedy_levy_ratio       %lf\n\n", dGreedyLevyRatio);
    }
}
