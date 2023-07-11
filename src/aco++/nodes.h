#include "algo_config.h"
#ifdef TREE_MAP_MACRO

#ifndef _NODES_H_
#define _NODES_H_

#include <stdint.h>
#include <assert.h>
#include <array>

template <class NodeT>
class Node_Base
{
public:
    NodeT *parent_ptr;
    std::array<NodeT *, 2> child_ptrs;

    Node_Base(NodeT *parent_ptr, const bool &is_leaf = false)
    {
        // Top-down building
        this->parent_ptr = parent_ptr;
        if (is_leaf)
        {
            for (auto &_ptr : child_ptrs)
                _ptr = nullptr;
        }
    };
    Node_Base(NodeT *left_ptr, NodeT *right_ptr, const bool &is_root = false)
    {
        // Bottom-up building
        const bool is_leaf = (left_ptr == nullptr) && (right_ptr == nullptr);
        assert((left_ptr != nullptr && right_ptr != nullptr) || is_leaf);

        child_ptrs[0] = left_ptr;
        child_ptrs[1] = right_ptr;
        if (!is_leaf)
        {
            left_ptr->parent_ptr = (NodeT *)(this);
            right_ptr->parent_ptr = (NodeT *)(this);
        }
        if (is_root)
            parent_ptr = nullptr;
    };
    ~Node_Base()
    {
        for (auto &_ptr : child_ptrs)
            delete _ptr;
    };
};

class Leaf_Base
{
public:
    Leaf_Base(const std::size_t &city_index) { _city_index = city_index; };
    std::size_t get_city_index() { return _city_index; };
    ~Leaf_Base(){};

protected:
    std::size_t _city_index;
};

class Node : public Node_Base<Node>
{
public:
    Node(const bool &is_leaf = false);                                                                             // Root, Top-down
    Node(Node *parent_ptr, const double &heuristic, const std::size_t &n_child_leaf, const bool &is_leaf = false); // Top-down
    Node(Node *left_ptr, Node *right_ptr, const bool &is_root = false);                                            // Bottom-up
    ~Node(){};

    void reinforce(
        const double &invert_fitness,
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const double &trail_max,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);
    std::size_t choose_child_with_prob(
        const double &alpha, const double &beta, const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times,
        const double &elite_prob);
    double get_pheromone(
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);
    double prob_weight(
        const double &alpha,
        const double &beta,
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);
    void pay_evaporation_debt(
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);
    void restart_if_needed(
        const std::size_t &global_restart_times,
        const double &past_trail_restart);
    double prob_weight_without_child_leaf(
        const double &alpha, const double &beta, const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times, const std::size_t &global_evap_times);
    std::size_t get_n_child_leaf();

protected:
    double _past_pheromone; // pheromone of the edge from the parent node to itself at the time of last local evaporation
    double _heuristic;      // heuristic of the edge from the parent node to itself
    std::size_t _local_restart_times;
    std::size_t _local_evap_times;
    std::size_t _n_child_leaf;
};

class Leaf : public Node, public Leaf_Base
{
public:
    Leaf(
        Node *parent_ptr,
        const double &heuristic,
        const std::size_t &city_index); // Top-down
    Leaf(
        const std::size_t &city_index,
        const double &heuristic); // Bottom-up
    ~Leaf(){};
};

class Wont_Visit_Node : public Node_Base<Wont_Visit_Node>
{
public:
    Wont_Visit_Node(Wont_Visit_Node *parent_ptr, const bool &is_leaf = false);                           // Top-down
    Wont_Visit_Node(Wont_Visit_Node *left_ptr, Wont_Visit_Node *right_ptr, const bool &is_root = false); // Bottom-up
    ~Wont_Visit_Node(){};

    void set_wont_visit(const std::size_t &global_wont_visit_restart_times);
    bool get_wont_visit(const std::size_t &global_wont_visit_restart_times);

protected:
    bool _wont_visit;
    std::size_t _local_wont_visit_restart_times;

    void _check_wont_visit(const std::size_t &global_wont_visit_restart_times);
    void _restart_if_needed(const std::size_t &global_wont_visit_restart_times);
};

class Building_Node : public Node_Base<Building_Node>
{
public:
    Building_Node(const bool &is_leaf = false); // Root
    Building_Node(Building_Node *parent_ptr, const double &centroid_x, const double &centroid_y, const std::size_t &n_child_leaf, const bool &is_leaf = false);
    ~Building_Node(){};

    double make_heuristic(const std::size_t &city_index);
    std::size_t get_n_child_leaf();

protected:
    double _centroid_x, _centroid_y; // coordinate
    std::size_t _n_child_leaf;
};

class Building_Leaf : public Building_Node, public Leaf_Base
{
public:
    Building_Leaf(Building_Node *parent_ptr, const double &centroid_x, const double &centroid_y, const std::size_t &city_index);
    ~Building_Leaf(){};
};

#endif
#endif