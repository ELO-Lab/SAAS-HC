#ifndef _TREE_MAP_H_
#define _TREE_MAP_H_

#include "ants.h"
#include "trees.h"

class Tree_Map
{
public:
    Tree_Map(const std::size_t &num_city, const struct problem &instance);
    ~Tree_Map();

    void evaporate(const double &past_trail_min);
    void restart_pheromone(const double &past_trail_restart);
    void reinforce(
        const ant_struct &an_ant,
        const double &rho,
        const double &trail_max);
    void choose_route(
        ant_struct &an_ant,
        const double &neighbour_prob,
        const double &alpha,
        const double &beta,
        const double &rho,
        long int &n_tours,
        const std::size_t &nn_ants,
        long **nn_list);
    double leaf_pheromone(
        const std::size_t &i,
        const std::size_t &j,
        const double &rho);
    double node_branching(const double &lambda);

protected:
    std::vector<Tree_Edge *> _tree_edge_ptrs;
    Wont_Visit_Tree *_wont_visit_tree_ptr;
    std::size_t
        _global_wont_visit_restart_times,
        _global_evap_times,
        _global_restart_times,
        _num_city;
    double _past_trail_restart, _past_trail_min;

    void _set_wont_visit(const std::size_t &city_index);
    void _reset_wont_visit();
};

// hyperparameters
extern bool tree_map_flag;

extern Tree_Map *tree_map;

void tree_map_init();
void tree_map_force_set_parameters();
void tree_map_deallocate();
void tree_map_construct_solutions();

#endif