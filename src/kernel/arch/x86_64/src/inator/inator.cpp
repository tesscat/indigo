#include "inator/inator.hpp"
#include "memory/heap.hpp"
#include "util/util.hpp"
namespace inator {

Graph* graph;

void init() {
    graph = (Graph*)kmalloc(sizeof(Graph));
}
void Graph::init() {
    nodes.init();
}
void Graph::addNode(Node node) {
    nodes.Append(node);
}

int Graph::tryLoadTarget(String name) {
    // there's going to be a ton of recursion here
    // get the relevant node
    Node* node = nullptr;
    for (uint64_t i = 0; i < nodes.len; i++) {
        if (nodes[i].name == name) {
            node = &nodes[i];
        }
    }
    if (!node) return -1;
    // resolve all it's dependencies
    for (uint64_t i = 0; i < node->dependencies->len; i++) {
        int r = tryLoadProvider(node->dependencies[i]);
        if (r) return r;
    }
    // load it itself
    return node->load();
}

int Graph::tryLoadProvider(String provider) {
    return 1;
}

// have to dummy it else someone complains
// extern "C" int __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle) {
//     panic("__cxa_atexit called");
// }
}
