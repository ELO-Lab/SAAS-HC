#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>
#include <float.h>

#include <iostream>
#include <vector>
#include <cmath>

#include "inout.h"
#include "thop.h"
#include "ants.h"
#include "ls.h"
#include "utilities.h"
#include "timer.h"

#include "es_ant.h"

std::vector<std::vector<std::vector<int>>> cluster_chunk;
std::vector<std::vector<int>> clusters;
std::vector<int> cluster;

std::vector<std::vector<double>> total_cluster;
std::vector<double> t_cluster;

int node_clustering_flag;
int n_cluster;
int cluster_size;
int n_sector;

void evaporation_nc_list(void)
{

    TRACE(printf("pheromone evaporation nn_list\n"););

    for (int i = 0; i < instance.n; i++)
    {
        for (int cur_cluster = 0; cur_cluster < cluster_chunk[i].size(); cur_cluster++)
        {
            for (int j = 0; j < cluster_chunk[i][cur_cluster].size(); j++)
            {
                int help_city = cluster_chunk[i][cur_cluster][j];
                pheromone[i][help_city] = (1 - rho) * pheromone[i][help_city];
            }
        }
    }
}

int find_closest_node(int pivot_node, const std::vector<bool> &visited)
{
    int closest_node = -1;
    double min_distance = FLT_MAX;
    for (int node = 0; node < instance.n - 1; node++)
        if (!visited[node])
        {
            if (instance.distance[pivot_node][node] < min_distance)
            {
                closest_node = node;
                min_distance = instance.distance[pivot_node][node];
            }
        }

    return closest_node;
}

bool check_in_sector(point a, point b, int n_sector, int sector_index)
{
    double angle_1 = (2 * M_PI / n_sector) * (sector_index);
    double angle_2 = (2 * M_PI / n_sector) * (sector_index + 1);

    double x1 = 1;
    double y1 = 0;
    double x2 = b.x - a.x;
    double y2 = b.y - a.y;

    double angle = atan2(x1 * x2 + y1 * y2, x1 * y2 - y1 * x2);

    if (angle < 0)
        angle = 2 * M_PI + angle;

    return angle_1 <= angle && angle < angle_2;
}

int find_closest_node_in_sector(int pivot_node, int n_sector, int sector_index)
{
    int closest_node = -1;
    double min_distance = FLT_MAX;
    for (int node = 0; node < instance.n - 1; node++)
        if (node != pivot_node)
        {
            bool in_sector = check_in_sector(instance.nodeptr[pivot_node], instance.nodeptr[node], n_sector, sector_index);
            if (in_sector && instance.distance[pivot_node][node] < min_distance)
            {
                closest_node = node;
                min_distance = instance.distance[pivot_node][node];
            }
        }

    return closest_node;
}

void update_cluter_total(void)
{
    total_cluster.clear();
    for (int i = 0; i < instance.n; i++)
    {
        total_cluster.push_back(t_cluster);
        for (int c = 0; c < cluster_chunk[i].size(); c++)
        {
            double p = 0;
            double h = 0;
            for (int v = 0; v < cluster_chunk[i][c].size(); v++)
            {
                p += pheromone[i][v];
                h += pheromone[i][v];
            }

            p /= cluster_chunk[i][c].size();
            h *= cluster_chunk[i][c].size();

            total_cluster[i].push_back(pow(p, alpha) * pow(h, beta));
        }
    }
}

void create_cluster(void)
{

    cluster_chunk.clear();

    for (int i = 0; i < instance.n; i++)
    {
        cluster_chunk.push_back(clusters);
        // Node curr_node = nodes[i];
        std::vector<bool> visited(instance.n, false);
        int nb_visited = 0;
        visited[i] = true;
        nb_visited++;

        int cluster_index = 0;
        cluster_chunk[i].push_back(cluster);

        for (int j = 0; j < n_sector; j++)
        {
            int node = find_closest_node_in_sector(i, n_sector, j);
            if (node != -1)
            {
                if (cluster_chunk[i][cluster_index].size() >= cluster_size)
                {
                    cluster_index++;
                    cluster_chunk[i].push_back(cluster);
                }
                cluster_chunk[i][cluster_index].push_back(node);
                nb_visited++;
                visited[node] = true;
            }
        }

        while (nb_visited < instance.n && cluster_chunk[i].size() < n_cluster)
        {
            while (cluster_chunk[i][cluster_index].size() < cluster_size)
            {
                int node = find_closest_node(i, visited);
                if (node != -1)
                {
                    cluster_chunk[i][cluster_index].push_back(node);
                    nb_visited++;
                    visited[node] = true;
                }
                if (nb_visited == instance.n - 1)
                    break;
            }
            if (nb_visited < instance.n - 1)
            {
                cluster_index++;
                cluster_chunk[i].push_back(cluster);
            }
            else
            {
                break;
            }
        }
    }

    // update_cluter_total();
}

void node_clustering_move(ant_struct *a, long int phase)
{

    if ((q_0 > 0.0) && (new_rand01() < q_0))
    {
        choose_best_next(a, phase);
        return;
    }

    int first = 1;
    long int current_city = a->tour[phase - 1];

    // select cluster
    double lp = 0;
    std::vector<double> pC;
    int cluster_size = total_cluster[current_city].size();
    for (int i = 0; i < cluster_size; i++)
    {
        lp = lp + total_cluster[current_city][i];
        pC.push_back(lp);
    }

    int selected_cluster = 0;
    double rnd = new_rand01();
    rnd *= lp;
    while (rnd >= pC[selected_cluster] && selected_cluster < cluster_size - 1)
        selected_cluster++;

    // if (selected_cluster == cluster_size){
    //     choose_best_next(a, phase);
    //     return;
    // }

    std::vector<long int> candidates;
    while (true)
    {
        for (int i = 0; i < cluster_chunk[current_city][selected_cluster].size(); i++)
        {
            long int city = cluster_chunk[current_city][selected_cluster][i];
            if (a->visited[city])
                continue;

            candidates.push_back(city);
        }

        if (candidates.size() != 0)
            break;

        if (first == 1 && selected_cluster != 0)
        {
            selected_cluster = 0;
            first = 0;
        }
        else if (selected_cluster == cluster_chunk[current_city].size() - 1)
        {
            choose_best_next(a, phase);
            return;
        }
        else
        {
            selected_cluster++;
        }
    }

    // select next city
    std::vector<double> total_candidates;
    lp = 0;
    for (int i = 0; i < candidates.size(); i++)
    {
        lp = lp + edge_weight(current_city, candidates[i]);
        total_candidates.push_back(lp);
    }

    int selected_city = 0;
    rnd = new_rand01();
    rnd *= lp;
    while (total_candidates[selected_city] <= rnd && selected_city < candidates.size() - 1)
        selected_city++;

    // if (selected_city == candidates.size()){
    //     neighbour_choose_best_next(a, phase);
    //     return;
    // }

    a->tour[phase] = candidates[selected_city];
    a->visited[candidates[selected_city]] = TRUE;
}

void construct_node_clustering_solution(void)
{
    long int i, k; /* counter variable */
    long int step; /* counter of the number of construction steps */

    TRACE(printf("construct solutions for all ants\n"););

    /* Mark all cities as unvisited */
    for (k = 0; k < n_ants; k++)
    {
        ant_empty_memory(&ant[k]);
    }

    /* Place the ants at initial city 0 and set the final city as n-1 */
    for (k = 0; k < n_ants; k++)
    {
        ant[k].tour_size = 1;
        ant[k].tour[0] = 0;
        ant[k].visited[0] = TRUE;
        ant[k].visited[instance.n - 1] = TRUE;
    }

    update_cluter_total();

    step = 0;

    while (step < instance.n - 2)
    {
        step++;
        for (k = 0; k < n_ants; k++)
        {
            if (ant[k].tour[ant[k].tour_size - 1] == instance.n - 2)
            { /* previous city is the last one */
                continue;
            }
            node_clustering_move(&ant[k], step);
            if (acs_flag)
                local_acs_pheromone_update(&ant[k], step);
            ant[k].tour_size++;
        }
    }

    for (k = 0; k < n_ants; k++)
    {
        ant[k].tour[ant[k].tour_size++] = instance.n - 1;
        ant[k].tour[ant[k].tour_size++] = ant[k].tour[0];
        for (i = ant[k].tour_size; i < instance.n; i++)
            ant[k].tour[i] = 0;
        ant[k].fitness = compute_fitness(ant[k].tour, ant[k].visited, ant[k].tour_size, ant[k].packing_plan);
        if (acs_flag)
            local_acs_pheromone_update(&ant[k], ant[k].tour_size - 1);
    }
    n_tours += n_ants;
}
