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

std::vector<std::vector<std::vector<int>>> cluster_chunk;
std::vector<std::vector<int>> clusters;
std::vector<int> cluster;

std::vector<std::vector<double>> total_cluster;
std::vector<double> t_cluster;

int node_clustering_flag;
int n_sector;
int cluster_size;

void evaporation_nc_list(void){

    TRACE(printf("pheromone evaporation nn_list\n"););

    for (int i = 0; i < instance.n; i++)
    {
        for (int cur_cluster = 0; cur_cluster < cluster_chunk[i].size(); cur_cluster++)
        {
            for (int j = 0; j < cluster_chunk[i][cur_cluster].size(); j++){
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
                if (cluster_chunk[i][cluster_index].size() >= cluster_size) {
                    cluster_index++;   
                    cluster_chunk[i].push_back(cluster);
                }
                cluster_chunk[i][cluster_index].push_back(node);
                nb_visited++;
                visited[node] = true;
            }
        }

        while (nb_visited < instance.n)
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

    if ((q_0 > 0.0) && (ran01(&seed) < q_0))
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
    double rnd = ran01(&seed);
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
        else if (selected_cluster == cluster_chunk[current_city].size() - 1){
            choose_best_next(a, phase);
            return;
        }else{
            selected_cluster++;
        }
    }

    // select next city
    std::vector<double> total_candidates;
    lp = 0;
    for (int i = 0; i < candidates.size(); i++)
    {
        lp = lp + total[current_city][candidates[i]];
        total_candidates.push_back(lp);
    }

    int selected_city = 0;
    rnd = ran01(&seed);
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
