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
      File:    utilities.h
      Author:  Thomas Stuetzle
      Purpose: some additional useful procedures
      Check:   README.txt and legal.txt
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
#ifndef UTILITIES_H /* only include ones */
#define UTILITIES_H

#include <random>
#include <climits>

#include <random>

#define INFTY LONG_MAX
#define MAXIMUM_NO_TRIES 100

#define TRUE 1
#define FALSE 0

/* general macros */

#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#define MIN(x, y) ((x) <= (y) ? (x) : (y))

#define DEBUG(x) x

#define TRACE(x)

/* constants for a random number generator, for details see numerical recipes in C */

#define IA 16807
#define IM 2147483647
#define AM (1.0 / IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876

extern long int seed;
extern std::default_random_engine rand_gen;

double mean(long int *values, long int max);

double meanr(double *values, long int max);

double std_deviation(long int *values, long int i, double mean);

double std_deviationr(double *values, long int i, double mean);

long int best_of_vector(long int *values, long int i);

long int worst_of_vector(long int *values, long int i);

void swap(long int *v, long int i, long int j);

void swap_double(double *v, long int i, long int j);

void sort(long int *v, long int left, long int right);

void sort_double(double *v, long int left, long int right);

double quantil(long int *v, double q, long int numbers);

void swap2(long int *v, long int *v2, long int i, long int j);

void swap2_double(double *v, double *v2, long int i, long int j);

void sort2(long int *v, long int *v2, long int left, long int right);

void sort2_double(double *v, double *v2, long int left, long int right);

long int **generate_int_matrix(long int n, long int m);

double **generate_double_matrix(long int n, long int m);

double new_rand01(void);

template <class numericT>
void mean_and_std(const std::vector<numericT> &vec, double &mean_value, double &std_value)
{
      double dtemp;

      mean_value = 0;
      for (const auto &value : vec)
      {
            mean_value += double(value) / vec.size();
      }

      std_value = 0;
      for (const auto &value : vec)
      {
            dtemp = value - mean_value;
            std_value += dtemp / vec.size() * dtemp;
      }
      std_value = sqrt(std_value);
}

#endif
