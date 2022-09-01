#pragma once

#include <map>
#include <mutex>
#include <vector>

template <typename Key, typename Value>
class ConcurrentMap {
public:
    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count) : buckets_(bucket_count) {};

    Access operator[](const Key& key) {
        const uint64_t index = static_cast<uint64_t>(key) % buckets_.size();
        return {std::lock_guard (buckets_[index].mutex), buckets_[index].bucket[key]};
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> buckets_all_;
        for (auto& bucket : buckets_) {
        std::lock_guard guard(bucket.mutex);
        buckets_all_.merge(bucket.bucket);
        }
        return buckets_all_;
    }

    void Erase(Key value) {
        for (auto& bucket: buckets_) {
            if (bucket.bucket.count(value)) {
                std::lock_guard guard (bucket.mutex);
                bucket.bucket.erase(value);
                break;
            }
        }
    }

private:
    struct Bucket {
        std::map<Key, Value> bucket;
        std::mutex mutex;
    };

    std::vector<Bucket> buckets_;
};

