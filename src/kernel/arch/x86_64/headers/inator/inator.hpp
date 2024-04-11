#ifndef KERNEL_INATOR_INATOR_HPP
#define KERNEL_INATOR_INATOR_HPP

#include "libstd/string.hpp"
#include "util/vec.hpp"
namespace inator {
void init();

struct Node {
    String name;
    String* dependencies;
    String* provides;
    int (*load)();
};
struct Graph {
    friend void inator::init();
    util::Vec<Node> nodes;
    bool hasInited;
    void init();
public:
    void addNode(Node node);
    int tryLoadTarget(String name);
    int tryLoadProvider(String name);
};
}

#endif // !KERNEL_INATOR_INATOR_HPP
