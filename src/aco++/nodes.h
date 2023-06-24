#ifndef _NODES_H_
#define _NODES_H_

#include <stdint.h>
#include <array>

template <class NodeT>
class Node_Base
{
public:
    NodeT *parent_ptr;
    std::array<NodeT *, 2> child_ptrs;

    Node_Base(){};
    Node_Base(NodeT *&left_ptr, NodeT *&right_ptr);
    ~Node_Base();
};

template <class NodeT>
Node_Base<NodeT>::Node_Base(NodeT *&left_ptr, NodeT *&right_ptr)
{
    left_ptr->parent_ptr = (NodeT *)(this);
    right_ptr->parent_ptr = (NodeT *)(this);
    child_ptrs[0] = left_ptr;
    child_ptrs[1] = right_ptr;
}

template <class NodeT>
Node_Base<NodeT>::~Node_Base()
{
    for (auto &_ptr : child_ptrs)
        delete _ptr;
}

class Node : public Node_Base<Node>
{
public:
    Node(){};
    Node(Node *&left_ptr, Node *&right_ptr);
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
        const double &one_minus_q_0,
        const double &rand_num_01,
        const double &alpha,
        const double &beta,
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);

protected:
    double _past_pheromone; // pheromone of the edge from the parent node to itself at the time of last local evaporation
    double _heuristic;      // heuristic of the edge from the parent node to itself
    std::size_t _local_restart_times;
    std::size_t _local_evap_times;

    void _pay_evaporation_debt(
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);
    double _prob_weight(
        const double &alpha,
        const double &beta,
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);
    void _restart_if_needed(
        const std::size_t &global_restart_times,
        const double &past_trail_restart);
    double _pheromone(
        const double &one_minus_rho,
        const double &past_trail_restart,
        const double &past_trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);
};

class Root : public Node
{
public:
    Root(Node *&left_ptr, Node *&right_ptr);
    ~Root(){};
};

class Leaf : public Node
{
public:
    Leaf(
        const std::size_t &city_index,
        const std::size_t &current_city,
        long int **&distance_matrix);
    std::size_t city_index();
    ~Leaf(){};

protected:
    std::size_t _city_index;
};

class Wont_Visit_Node : public Node_Base<Wont_Visit_Node>
{
public:
    Wont_Visit_Node()
        : _wont_visit(false), _local_wont_visit_restart_times(0){};
    Wont_Visit_Node(Wont_Visit_Node *&left_ptr, Wont_Visit_Node *&right_ptr)
        : Node_Base<Wont_Visit_Node>(left_ptr, right_ptr),
          _wont_visit(false), _local_wont_visit_restart_times(0){};
    ~Wont_Visit_Node(){};

    void set_wont_visit(const std::size_t &global_wont_visit_restart_times);
    bool get_wont_visit();

protected:
    bool _wont_visit;
    std::size_t _local_wont_visit_restart_times;

    void _check_wont_visit(const std::size_t &global_wont_visit_restart_times);
    void _restart_if_needed(const std::size_t &global_wont_visit_restart_times);
};

#endif