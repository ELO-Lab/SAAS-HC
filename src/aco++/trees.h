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
    Tree_Edge(const std::size_t &num_city, const std::size_t &current_city);
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
    void _bottom_up_build_tree(std::vector<Node *> &node_ptrs);
    void _build_branch(Node *&branch_ptr, const Building_Node *&building_branch_ptr);
};

class Wont_Visit_Tree : public Tree_Base<Wont_Visit_Node, Wont_Visit_Node>
{
public:
    Wont_Visit_Tree(const std::size_t &num_city);
    ~Wont_Visit_Tree(){};

    void set_wont_visit(const std::size_t &city_index, const std::size_t &num_city, const std::size_t &global_wont_visit_restart_times);
    Wont_Visit_Node *get_root_ptr();

protected:
    void _bottom_up_build_tree(std::vector<Wont_Visit_Node *> &node_ptrs);
};

// class Building_Tree : public Tree_Base<Building_Node, Building_Leaf>
// {
// public:
//     Building_Tree(const problem &instance);
//     ~Building_Tree(){};

// protected:
//     void _make_city_features(
//         const problem &instance,
//         // returning values
//         unknown_classA &_city_features);
//     void _cluster_cities(
//         const unknown_classB &city_indexes,
//         const unknown_classA &city_features,
//         // returning values
//         unknown_classB &fisrt_cluster_indexes,
//         unknown_classA &fisrt_cluster_features,
//         unknown_classB &second_cluster_indexes,
//         unknown_classA &second_cluster_features,
//         double &centroid_x,
//         double &centroid_y);
// };

// Building_Tree::Building_Tree(const problem &instance)
// {
//     const std::size_t num_city = instance.n - 1;
//     std::size_t vec_size = 1;
//     std::vector<unknown_classA> features(vec_size);
//     std::vector<unknown_classB> indexes(vec_size);
//     std::vector<Building_Node *> parent_ptrs(vec_size);
//     std::queue<std::size_t> temp_vec_idx_queue;

//     indexes[vec_size - 1] = ? ? arange num_city ? ? ;
//     _make_city_features(instance, features[vec_size - 1]);

//     while (vec_size - temp_vec_idx_queue.size() > 0)
//     {
//         auto &city_features = features[vec_size - 1];
//         auto &city_indexes = indexes[vec_size - 1];

//         while (temp_vec_idx_queue.size() < 2)
//         {
//             temp_vec_idx_queue.push(vec_size);
//             vec_size += 1;
//         }
//         features.resize(vec_size);
//         indexes.resize(vec_size);
//         parent_ptrs.resize(vec_size);

//         temp_vec_idx_queue.add(vec_size - 1);
//         vec_size -= 1;

//         // check leaf, init leaf
//         if (??)
//         {
//             ? ? ;
//             continue;
//         }

//         // init node;
//         ? ? ;

//         // append stacks
//         ? ? ;
//         parent_ptr_stack.append(??);
//         parent_ptr_stack.append(??);
//     }
// }

#endif