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
      File:    utilities.c
      Author:  Thomas Stuetzle
      Purpose: some additional useful procedures
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
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <random>

#include "inout.h"
#include "utilities.h"
#include "thop.h"
#include "ants.h"
#include "timer.h"

std::default_random_engine rand_gen;
std::uniform_real_distribution<double> distribution(0, 1);

double mean(long int *values, long int max)
/*
      FUNCTION:       compute the average value of an integer array of length max
      INPUT:          pointer to array, length of array
      OUTPUT:         average
      (SIDE)EFFECTS:  none
 */
{
    long int j;
    double m;

    m = 0.;
    for (j = 0; j < max; j++)
    {
        m += (double)values[j];
    }
    m = m / (double)max;
    return m;
}

double meanr(double *values, long int max)
/*
      FUNCTION:       compute the average value of a floating number array of length max
      INPUT:          pointer to array, length of array
      OUTPUT:         average
      (SIDE)EFFECTS:  none
 */
{
    long int j;
    double m;

    m = 0.;
    for (j = 0; j < max; j++)
    {
        m += values[j];
    }
    m = m / (double)max;
    return m;
}

double std_deviation(long int *values, long int max, double mean)
/*
      FUNCTION:       compute the standard deviation of an integer array
      INPUT:          pointer to array, length of array, mean
      OUTPUT:         standard deviation
      (SIDE)EFFECTS:  none
 */
{
    long int j;
    double dev = 0.;

    if (max <= 1)
        return 0.;
    for (j = 0; j < max; j++)
    {
        dev += ((double)values[j] - mean) * ((double)values[j] - mean);
    }
    return sqrt(dev / (double)(max - 1));
}

double std_deviationr(double *values, long int max, double mean)
/*
      FUNCTION:       compute the standard deviation of a floating number array
      INPUT:          pointer to array, length of array, mean
      OUTPUT:         standard deviation
      (SIDE)EFFECTS:  none
 */
{
    long int j;
    double dev;

    if (max <= 1)
        return 0.;
    dev = 0.;
    for (j = 0; j < max; j++)
    {
        dev += ((double)values[j] - mean) * ((double)values[j] - mean);
    }
    return sqrt(dev / (double)(max - 1));
}

long int best_of_vector(long int *values, long int l)
/*
      FUNCTION:       return the minimum value in an integer value
      INPUT:          pointer to array, length of array
      OUTPUT:         smallest number in the array
      (SIDE)EFFECTS:  none
 */
{
    long int min, k;

    k = 0;
    min = values[k];
    for (k = 1; k < l; k++)
    {
        if (values[k] < min)
        {
            min = values[k];
        }
    }
    return min;
}

long int worst_of_vector(long int *values, long int l)
/*
      FUNCTION:       return the maximum value in an integer value
      INPUT:          pointer to array, length of array
      OUTPUT:         largest number in the array
      (SIDE)EFFECTS:  none
 */
{
    long int max, k;

    k = 0;
    max = values[k];
    for (k = 1; k < l; k++)
    {
        if (values[k] > max)
        {
            max = values[k];
        }
    }
    return max;
}

double quantil(long int *v, double q, long int l)
/*
      FUNCTION:       return the q-quantil of an ordered integer array
      INPUT:          one array, desired quantil q, length of array
      OUTPUT:         q-quantil of array
      (SIDE)EFFECTS:  none
 */
{
    long int i, j;
    double tmp;

    tmp = q * (double)l;
    if ((double)((long int)tmp) == tmp)
    {
        i = (long int)tmp;
        j = (long int)(tmp + 1.);
        return ((double)v[i - 1] + (double)v[j - 1]) / 2.;
    }
    else
    {
        i = (long int)(tmp + 1.);
        return v[i - 1];
    }
}

void swap(long int *v, long int i, long int j)
/*
      FUNCTION:       auxiliary routine for sorting an integer array
      INPUT:          array, two indices
      OUTPUT:         none
      (SIDE)EFFECTS:  elements at position i and j of array are swapped
 */
{
    long int tmp;

    tmp = v[i];
    v[i] = v[j];
    v[j] = tmp;
}

void swap_double(double *v, long int i, long int j)
/*
      FUNCTION:       auxiliary routine for sorting an double array
      INPUT:          array, two indices
      OUTPUT:         none
      (SIDE)EFFECTS:  elements at position i and j of array are swapped
 */
{
    double tmp;

    tmp = v[i];
    v[i] = v[j];
    v[j] = tmp;
}

void sort(long int *v, long int left, long int right)
/*
      FUNCTION:       recursive routine (quicksort) for sorting an array
      INPUT:          one array, two indices
      OUTPUT:         none
      (SIDE)EFFECTS:  elements at position i and j of the two arrays are swapped
 */
{
    long int k, last;

    if (left >= right)
        return;
    swap(v, left, (left + right) / 2);
    last = left;
    for (k = left + 1; k <= right; k++)
        if (v[k] < v[left])
            swap(v, ++last, k);
    swap(v, left, last);
    sort(v, left, last);
    sort(v, last + 1, right);
}

void sort_double(double *v, long int left, long int right)
/*
      FUNCTION:       recursive routine (quicksort) for sorting an double array
      INPUT:          one array, two indices
      OUTPUT:         none
      (SIDE)EFFECTS:  elements at position i and j of the two arrays are swapped
 */
{
    long int k, last;

    if (left >= right)
        return;
    swap_double(v, left, (left + right) / 2);
    last = left;
    for (k = left + 1; k <= right; k++)
        if (v[k] < v[left])
            swap_double(v, ++last, k);
    swap_double(v, left, last);
    sort_double(v, left, last);
    sort_double(v, last + 1, right);
}

void swap2(long int *v, long int *v2, long int i, long int j)
/*
      FUNCTION:       auxiliary routine for sorting an integer array
      INPUT:          two arraya, two indices
      OUTPUT:         none
      (SIDE)EFFECTS:  elements at position i and j of the two arrays are swapped
 */
{
    long int tmp;

    tmp = v[i];
    v[i] = v[j];
    v[j] = tmp;
    tmp = v2[i];
    v2[i] = v2[j];
    v2[j] = tmp;
}

void swap2_double(double *v, double *v2, long int i, long int j)
/*
      FUNCTION:       auxiliary routine for sorting an double array
      INPUT:          two arraya, two indices
      OUTPUT:         none
      (SIDE)EFFECTS:  elements at position i and j of the two arrays are swapped
 */
{
    double tmp;

    tmp = v[i];
    v[i] = v[j];
    v[j] = tmp;
    tmp = v2[i];
    v2[i] = v2[j];
    v2[j] = tmp;
}

void sort2(long int *v, long int *v2, long int left, long int right)
/*
      FUNCTION:       recursive routine (quicksort) for sorting one array; second
                      arrays does the same sequence of swaps
      INPUT:          two arrays, two indices
      OUTPUT:         none
      (SIDE)EFFECTS:  elements at position i and j of the two arrays are swapped
 */
{
    long int k, last;

    if (left >= right)
        return;
    swap2(v, v2, left, (left + right) / 2);
    last = left;
    for (k = left + 1; k <= right; k++)
        if (v[k] < v[left])
            swap2(v, v2, ++last, k);
    swap2(v, v2, left, last);
    sort2(v, v2, left, last);
    sort2(v, v2, last + 1, right);
}

void sort2_double(double *v, double *v2, long int left, long int right)
/*
      FUNCTION:       recursive routine (quicksort) for sorting one double array; second
                      arrays does the same sequence of swaps
      INPUT:          two arrays, two indices
      OUTPUT:         none
      (SIDE)EFFECTS:  elements at position i and j of the two arrays are swapped
 */
{
    long int k, last;

    if (left >= right)
        return;
    swap2_double(v, v2, left, (left + right) / 2);
    last = left;
    for (k = left + 1; k <= right; k++)
        if (v[k] < v[left])
            swap2_double(v, v2, ++last, k);
    swap2_double(v, v2, left, last);
    sort2_double(v, v2, left, last);
    sort2_double(v, v2, last + 1, right);
}

long int **generate_int_matrix(long int n, long int m)
/*
      FUNCTION:       malloc a matrix and return pointer to it
      INPUT:          size of matrix as n x m
      OUTPUT:         pointer to matrix
      (SIDE)EFFECTS:
 */
{
    long int i;
    long int **matrix;

    if ((matrix = (long int **)malloc(sizeof(long int) * n * m + sizeof(long int *) * n)) == NULL)
    {
        printf("Out of memory, exit.");
        exit(1);
    }
    for (i = 0; i < n; i++)
    {
        matrix[i] = (long int *)(matrix + n) + i * m;
    }

    return matrix;
}

double **generate_double_matrix(long int n, long int m)
/*
      FUNCTION:       malloc a matrix and return pointer to it
      INPUT:          size of matrix as n x m
      OUTPUT:         pointer to matrix
      (SIDE)EFFECTS:
 */
{

    long int i;
    double **matrix;

    if ((matrix = (double **)malloc(sizeof(double) * n * m + sizeof(double *) * n)) == NULL)
    {
        printf("Out of memory, exit.");
        exit(1);
    }
    for (i = 0; i < n; i++)
    {
        matrix[i] = (double *)(matrix + n) + i * m;
    }
    return matrix;
}

double new_rand01(void)
{
    return distribution(rand_gen);
}
