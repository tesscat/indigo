#ifndef KERNEL_UTIL_MAP_HPP
#define KERNEL_UTIL_MAP_HPP

#include "util/util.hpp"
#include "util/vec.hpp"
namespace util {
template<typename K, typename V>
class Map {
    util::Vec<K> keys_;
    util::Vec<V> values_;
public:
    Map() : keys_{}, values_{} {}
    void init() {}
    V& get(K& key) {
        for (uint64_t i = 0; i < keys_.len; i++) {
            if (keys_[i] == key) return values_[i];
        }
        panic("no value");
        return values_[-1];
    }
    bool hasKey(K& key) {
        for (uint64_t i = 0; i < keys_.len; i++) {
            if (keys_[i] == key) return true;
        }
        return false;
    }
    void set(K& key, V& val) {
        if (hasKey(key)) {
            for (uint64_t i = 0; i < keys_.len; i++) {
                if (keys_[i] == key) {
                    values_[i] = val;
                    break;
                }
            }
        } else {
            keys_.Append(key);
            values_.Append(val);
        }
    }
    util::Vec<K>& keys() {return keys_;};
    util::Vec<V>& values() {return values_;};
};
}

#endif // !KERNEL_UTIL_MAP_HPP
