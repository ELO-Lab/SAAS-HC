// #ifndef _NODE_TREE_TEMPLATE_HPP_
// #define _NODE_TREE_TEMPLATE_HPP_

// #include <vector>
// #include <array>

// template <class NodeT>
// class Node_Base
// {
// public:
//     NodeT *parent_ptr;
//     std::array<NodeT *, 2> child_ptrs;

//     Node_Base(){};
//     Node_Base(NodeT *left_ptr, NodeT *right_ptr)
//     {
//         left_ptr->parent_ptr = (NodeT *)(this);
//         right_ptr->parent_ptr = (NodeT *)(this);
//         child_ptrs[0] = left_ptr;
//         child_ptrs[1] = right_ptr;
//     };
//     ~Node_Base()
//     {
//         for (auto &_ptr : child_ptrs)
//             delete _ptr;
//     };
// };

// template <class RootT, class LeafT>
// class Tree_Base
// {
// public:
//     Tree_Base() {}
//     ~Tree_Base() { delete _root_ptr; };

// protected:
//     RootT *_root_ptr;
//     std::vector<LeafT *> _leaf_ptrs;
// };

// #endif