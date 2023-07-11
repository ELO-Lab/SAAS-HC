#include "algo_config.h"
#if TREE_MAP_MACRO

#ifndef _TREES_H_
#define _TREES_H_

#include <vector>
#include <mlpack.hpp>

#include "nodes.h"

template <class RootT, class LeafT>
class Tree_Base
{
public:
    Tree_Base() {}
    ~Tree_Base() { delete _root_ptr; }

    RootT *get_root_ptr() { return _root_ptr; }

protected:
    RootT *_root_ptr;
    std::vector<LeafT *> _leaf_ptrs;
};

class Wont_Visit_Tree : public Tree_Base<Wont_Visit_Node, Wont_Visit_Node>
{
public:
    Wont_Visit_Tree(const std::size_t &num_city);                                   // Bottom-up
    Wont_Visit_Tree(const std::size_t &num_city, Building_Node *building_root_ptr); // Top-down
    ~Wont_Visit_Tree(){};

    void set_wont_visit(const std::size_t &city_index, const std::size_t &num_city, const std::size_t &global_wont_visit_restart_times);
    bool check_city_visited(const std::size_t &city_index, const std::size_t &global_wont_visit_restart_times);

protected:
    void _build_childs(Wont_Visit_Node *&parent_ptr, Building_Node *building_parent_ptr);
    void _bottom_up_build_tree(std::vector<Wont_Visit_Node *> &node_ptrs);
};

class Tree_Edge : public Tree_Base<Node, Leaf>
{
public:
    Tree_Edge(const std::size_t &num_city, const std::size_t &current_city);                                   // Bottom-up
    Tree_Edge(const std::size_t &num_city, const std::size_t &current_city, Building_Node *building_root_ptr); // Top-down
    ~Tree_Edge(){};

    std::size_t choose_next_city(
        Wont_Visit_Tree *wont_visit_tree_ptr,
        const double &neighbour_prob,
        const double &alpha,
        const double &beta,
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times,
        const std::size_t &global_wont_visit_restart_times,
        const std::size_t &nn_ants,
        long *nn_list,
        const std::size_t &num_city,
        const double &elite_prob);
    void reinforce(
        const std::size_t &city_index,
        const double &invert_fitness,
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const double &trail_max,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);
    double leaf_pheromone(
        const std::size_t &city_index,
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);

protected:
    // std::size_t _current_city;

    void _bottom_up_build_tree(std::vector<Node *> &node_ptrs);
    void _build_childs(Node *&parent_ptr, Building_Node *building_parent_ptr, const std::size_t &current_city);
    std::size_t _walk_from_root(
        Wont_Visit_Node *wont_visit_root_ptr,
        const double &alpha,
        const double &beta,
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times,
        const std::size_t &global_wont_visit_restart_times,
        const double &elite_prob);
    std::size_t _choose_neighbour_leaf(
        Wont_Visit_Tree *wont_visit_tree_ptr,
        const double &alpha,
        const double &beta,
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times,
        const std::size_t &global_wont_visit_restart_times,
        const std::size_t &nn_ants,
        long *nn_list,
        const std::size_t &num_city,
        const double &elite_prob);
    std::size_t _choose_best_leaf(
        Wont_Visit_Tree *wont_visit_tree_ptr,
        const double &alpha, const double &beta, const double &one_minus_rho,
        const double &past_trail_restart, const double &past_trail_min,
        const std::size_t &global_restart_times, const std::size_t &global_evap_times,
        const std::size_t &global_wont_visit_restart_times,
        const std::size_t &num_city);
};

typedef struct
{
    std::vector<std::size_t> indexes;
    arma::mat features;
    double centroid_x, centroid_y;
} cluster_struct;

class Building_Tree : public Tree_Base<Building_Node, Building_Leaf>
{
public:
    Building_Tree(const struct problem &instance);
    ~Building_Tree(){};

protected:
    void _make_first_cluster(
        const struct problem &instance,
        // returning values
        cluster_struct &cluster);
    void _cluster_cities(
        const std::vector<std::size_t> &indexes,
        const arma::mat &features,
        // returning values
        std::vector<cluster_struct> &clusters);
    void _build_childs(Building_Node *parent_ptr, const std::vector<std::size_t> &city_indexes, const arma::mat &city_features);
};

#endif
#endif