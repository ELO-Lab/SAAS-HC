#ifndef _TREES_H_
#define _TREES_H_

#include <vector>

#include "nodes.h"

template <class RootT, class LeafT>
class Tree_Base
{
public:
    Tree_Base() {}
    ~Tree_Base() { delete _root_ptr; };

protected:
    RootT *_root_ptr;
    std::vector<LeafT *> _leaf_ptrs;
};

class Tree_Edge : public Tree_Base<Node, Leaf>
{
public:
    Tree_Edge(const std::size_t &num_city, const std::size_t &current_city, long int **&distance_matrix);
    ~Tree_Edge(){};

    std::size_t choose_next_city(
        Wont_Visit_Node *wont_visit_root_ptr,
        const double &one_minus_q_0,
        const double &alpha,
        const double &beta,
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times,
        const std::size_t &global_wont_visit_restart_times);
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
    void _build_tree(std::vector<Node *> &node_ptrs);
};

class Wont_Visit_Tree : public Tree_Base<Wont_Visit_Node, Wont_Visit_Node>
{
public:
    Wont_Visit_Tree(const std::size_t &num_city);
    ~Wont_Visit_Tree(){};

    void set_wont_visit(const std::size_t &city_index, const std::size_t &num_city, const std::size_t &global_wont_visit_restart_times);
    Wont_Visit_Node *get_root_ptr();

protected:
    void _build_tree(std::vector<Wont_Visit_Node *> &node_ptrs);
};

#endif