#include "inator/inator.hpp"
#include "logs/logs.hpp"
#include "memory/heap.hpp"
#include "util/vec.hpp"
namespace inator {

Graph* graph;

void init() {
    graph = (Graph*)kmalloc(sizeof(Graph));
    graph->init();
}
void Graph::init() {
    targets.init();
    providers.init();
    rejects.init();
    loadedTargets.init();
    loadedProviders.init();
}

void Graph::finalizeGraph() {
    // setup the providers map
    // we need to map them in order plz
    for (uint64_t i = 0; i < targets.values().len; i++) {
        Target& targ = targets.values()[i];
        for (uint64_t j = 0; j < targ.provides.len; j++) {
            String& prov = targ.provides[j];
            if (!providers.hasKey(prov)) {
                util::Vec<Target*> newTargets {};
                providers.set(prov, newTargets);
            }
            providers.get(prov).Append(&targ);
        }
    }
    // okie now sort them
    for (uint64_t i = 0; i < providers.values().len; i++) {
        providers.values()[i].Sort();
    }
}

void Graph::addTarget(Target target) {
    targets.set(target.name, target);
}

int Graph::tryLoadTarget(String name, util::Vec<String>& depStack) {
    // there's going to be a ton of recursion here
    // get the relevant node
    if (!targets.hasKey(name)) return -1;
    Target& target = targets.get(name);
    
    depStack.Append(name);
    // resolve all it's dependencies
    for (uint64_t i = 0; i < target.dependencies.len; i++) {
        int r = tryLoadProvider(target.dependencies[i], depStack);
        if (r) return r;
    }
    // load it itself
    int n = target.load();
    if (n != 0) rejects.Append(name);
    else loadedTargets.Append(&target);
    return n;
}

int Graph::tryLoadProvider(String provider, util::Vec<String>& depStack) {
    // have we already loaded this provider?
    if (loadedProviders.Contains(provider)) return 0;
    // try and load a dep handler that isn't in the rejects stack,
    // isn't in depStack (else would be circular), and we haven't already tried
    if (!providers.hasKey(provider)) return -1;
    util::Vec<Target*>& targs = providers.get(provider);
    for (uint64_t i = 0; i < targs.len; i++) {
        // are we in rejects/depStack?
        String& name = targs[i]->name;
        if (depStack.Contains(name) || rejects.Contains(name)) continue;
        // try it!
        int n = tryLoadTarget(name, depStack);
        if (n == 0) {
            // we have loaded it!
            loadedProviders.Append(provider);
            return 0;
        }
        // computer says no, go to next one
    }


    return 0;
}

int Graph::tryLoadTarget(String name) {
    util::Vec<String> depStack;
    return tryLoadTarget(name, depStack);
}

// // some example targets to test
// int l0() {
//     logs::info << "l0() called\n";
//     return 0;
// }
// __attribute__ ((constructor)) void t0() {
//     Target t0;
//     t0.name = String("t0");
//     t0.provides.Append(String("p2"));
//     t0.preference = 10;
//     t0.load = l0;

//     graph->addTarget(t0);
// }
// int l1() {
//     logs::info << "l1() called\n";
//     return 0;
// }
// __attribute__ ((constructor)) void t1() {
//     Target t1;
//     t1.name = String("t1");
//     t1.provides.Append(String("p1"));
//     t1.dependencies.Append(String("p2"));
//     t1.preference = 10;
//     t1.load = l1;

//     graph->addTarget(t1);
// }



}
