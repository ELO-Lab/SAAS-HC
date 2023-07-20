#ifndef ES_ACO_H
#define ES_ACO_H

#include <iostream>
#include <cstdio>
#include <math.h>

#include "boundary_cmaes.h"
#include "ants.h"
#include "utilities.h"
#include "inout.h"
#include "thop.h"
#include "timer.h"
#include "ls.h"
#include "algo_config.h"

#define ES_ACO_DEBUG(x)

extern unsigned long int initial_nb_dims;
extern unsigned long int initial_lambda;

extern double lowerBounds[];
extern double upperBounds[];

extern boundary_cmaes optimizer;

extern int default_ls;
extern unsigned int n_restarts;

void _es_construct_solutions(int index);

void _es_local_search(int index);

//   index of current offspring         genotype   number of dims
double eval_function(int index, double const *x, unsigned long N);

void es_aco_init();

void es_aco_construct_and_local_search();

void ipop_cmaes_aco_construct_solutions();

void bipop_cmaes_aco_construct_solutions();

void es_aco_export_result();

void es_aco_exit();

const char *es_aco_termination_condition();

void es_aco_update_statistics();

void resize_ant_colonies();

void es_aco_init_program();

#endif