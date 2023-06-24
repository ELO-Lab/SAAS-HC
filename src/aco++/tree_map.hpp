#ifndef _TREE_MAP_HPP_
#define _TREE_MAP_HPP_

#include "ants.h"
#include "trees.hpp"

class Tree_Map
{
public:
    Tree_Map(const std::size_t &num_city, long int **distance_matrix);
    ~Tree_Map();

    void evaporate(const double &trail_min, const double &trail_max);
    void restart_pheromone(const double &trail_restart);
    void reinforce(
        ant_struct &an_ant,
        const double &rho);
    void find_route(
        ant_struct &an_ant,
        const size_t &num_city,
        std::function<double()> rand01,
        const double &q_0,
        const double &alpha,
        const double &beta,
        const double &rho,
        std::function<void(ant_struct *a)> ant_empty_memory,
        std::function<long int(long int *t, char *visited, long int t_size, char *p)> compute_fitness,
        long int &n_tours);

protected:
    std::vector<Tree_Edge *> _tree_edge_ptrs;
    Wont_Visit_Tree *_wont_visit_tree_ptr;
    std::size_t
        _global_wont_visit_restart_times,
        _global_evap_times,
        _global_restart_times;
    double _trail_restart, _trail_min, _trail_max;

    void _set_wont_visit(const std::size_t &city_index);
    void _reset_wont_visit();
};

// hyperparameters
extern bool tree_map_flag;

extern Tree_Map *tree_map;

void tree_map_init(Tree_Map *&tree_map, const std::size_t &num_city, long int **distance_matrix);
void tree_map_fixed_parameters(long int &mmas_flag);

#endif