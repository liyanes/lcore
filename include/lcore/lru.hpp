#pragma once
#include "base.hpp"
#include <functional>
#include <list>
#include <map>

LCORE_NAMESPACE_BEGIN

template <
    typename K,
    typename V,
    typename Evictor = std::function<void(const K&, const V&)>,
    typename KView = K
>
class LRUCache {
public:
    using KeyType = K;
    using ValueType = V;
    using EvictorType = Evictor;

    LRUCache(std::size_t max_size, EvictorType evictor = nullptr)
        : max_size(max_size), evictor(evictor) {}

    ValueType* Get(KView key) {
        auto it = cache_map.find(key);
        if (it == cache_map.end()) {
            return nullptr; // Not found
        }
        // Move the accessed item to the front of the LRU list
        lru_list.splice(lru_list.begin(), lru_list, it->second.lru_it);
        return &it->second.value;
    }

    void Put(K key, V value) {
        auto it = cache_map.find(key);
        if (it != cache_map.end()) {
            // Update existing item
            it->second.value = std::move(value);
            lru_list.splice(lru_list.begin(), lru_list, it->second.lru_it);
            return;
        }
        // Insert new item
        if (cache_map.size() >= max_size) {
            // Evict the least recently used item
            const K& lru_key = lru_list.back();
            auto lru_it = cache_map.find(lru_key);
            if (lru_it != cache_map.end()) {
                if (evictor) {
                    evictor(lru_it->second.key, lru_it->second.value);
                }
                cache_map.erase(lru_it);
            }
            lru_list.pop_back();
        }
        lru_list.push_front(key);
        cache_map[key] = {std::move(key), std::move(value), lru_list.begin()};
    }

    bool Exists(KView key) const {
        return cache_map.find(key) != cache_map.end();
    }

    void Clear() {
        if (evictor) {
            for (const auto& pair : cache_map) {
                evictor(pair.second.key, pair.second.value);
            }
        }
        cache_map.clear();
        lru_list.clear();
    }
private:
    struct CacheItem {
        KeyType key;
        ValueType value;
        typename std::list<KeyType>::iterator lru_it;
    };

    std::size_t max_size;
    EvictorType evictor;
    std::list<KeyType> lru_list; // Most recently used at front
    std::map<KeyType, CacheItem> cache_map;

};

LCORE_NAMESPACE_END
