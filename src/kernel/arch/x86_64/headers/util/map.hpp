#ifndef KERNEL_UTIL_MAP_HPP
#define KERNEL_UTIL_MAP_HPP

#include "util/vec.hpp"
namespace util {
template<typename K, typename V>
class Map {
    util::ManualVec<K> keys_;
    util::ManualVec<V> values_;
    bool hasInit;
public:
    void init() {
        if (hasInit) return;
        hasInit = true;
        keys_.init();
        values_.init();
    }
    V& get(K& key) {
        for (uint64_t i = 0; i < keys_.len; i++) {
            if (keys_[i] == key) return values_[i];
        }
        return values_[-1];
    }
    bool hasKey(K& key) {
        for (uint64_t i = 0; i < keys_.len; i++) {
            if (keys_[i] == key) return true;
        }
        return false;
    }
    void set(K& key, V val) {
        if (hasKey(key)) {
            for (uint64_t i = 0; i < keys_.len; i++) {
            if (keys_[i] == key) values_[i] = val;
        }
        } else {
            keys_.Append(key);
            values_.Append(val);
        }
    }
    util::ManualVec<K>& keys() {return keys_;};
    util::ManualVec<V>& values() {return values_;};
};
}

#endif // !KERNEL_UTIL_MAP_HPP
