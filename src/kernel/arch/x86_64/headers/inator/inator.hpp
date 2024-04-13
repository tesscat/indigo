#ifndef KERNEL_INATOR_INATOR_HPP
#define KERNEL_INATOR_INATOR_HPP

#include "libstd/string.hpp"
#include "util/map.hpp"
#include "util/vec.hpp"
namespace inator {
void init();


struct Target {
    String name;
    util::ManualVec<String> dependencies;
    util::ManualVec<String> provides;
    uint64_t preference;
    int (*load)();
    bool operator<(Target& other) {
        // reversed since imsort sorts ascending
        return preference > other.preference;
    }
    Target() = default;
    Target& operator=(Target& other) = default;
};
struct Graph {
    friend void inator::init();
    util::Map<String, Target> targets;
    util::Map<String, util::Vec<Target*>> providers;
    util::ManualVec<Target*> loadedTargets;
    util::ManualVec<String> loadedProviders;
    util::ManualVec<String> rejects;
    void init();
    int tryLoadTarget(String name, util::Vec<String>& depStack);
    int tryLoadProvider(String name, util::Vec<String>& depStack);
public:
    void addTarget(Target target);
    int tryLoadTarget(String name);
    void finalizeGraph();
};

extern Graph* graph;
}

#endif // !KERNEL_INATOR_INATOR_HPP
