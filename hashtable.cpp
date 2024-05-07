#include <functional>
#include "hashtable.hpp"

namespace sycl_hashtable {

    inline int hash(int x) {
        return x*2654435761 % 2^32;
    }

    void insert(KV* hashtable, int key, int value) {
        
        auto s = hash(key);
        int empty_sentinel = EMPTY_SENTINEL;
        int capacity = CAPACITY;
        int counter = 0;


        while (counter < capacity) {
            auto ref = cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed,
                                    cl::sycl::memory_scope::device,
                                    cl::sycl::access::address_space::global_space>(hashtable[s].key);
            auto success = ref.compare_exchange_strong(empty_sentinel, key);
            if (success) {
                hashtable[s].value = value;
                break;
            }
            s = (s + 1) & (capacity-1);
            counter++;                   
        }

    }

    int load(KV* hashtable, int key) {

        auto s = hash(key);
        int empty_sentinel = EMPTY_SENTINEL;
        int capacity = CAPACITY;
        int counter = 0;

        while (counter < capacity) {
            if (hashtable[s].key == key) {
                return hashtable[s].value;
            } else if (hashtable[s].key == empty_sentinel) {
                printf("-- empty\n");
                return 0;
            }
            s = (s + 1) & (capacity-1);
            counter++;
        }
        return 0;
    }

    void del(KV* hashtable, int key) {
        
        auto s = hash(key);
        int empty_sentinel = EMPTY_SENTINEL;
        int capacity = CAPACITY;
        int counter = 0;

        while (counter < capacity) {
            if (hashtable[s].key == key) {
                hashtable[s].value = empty_sentinel;
                break;
            } else if (hashtable[s].key == empty_sentinel) {
                break;
            }
            s = (s + 1) & (capacity-1);
            counter++;
        }
    }
}

void syclhash_insert(cl::sycl::queue& q, sycl_hashtable::KV* hashtable, std::vector<sycl_hashtable::KV> inserts) {
    int n = inserts.size();
    sycl_hashtable::KV inserts_arr[MAX_INSERT_CAP];
    if (n <= MAX_INSERT_CAP) {
        std::copy(inserts.begin(), inserts.end(), inserts_arr);
    }
    q.parallel_for(cl::sycl::range<1>(n), [=](cl::sycl::id<1> idx) {
        auto kv = inserts_arr[idx];
        sycl_hashtable::insert(hashtable, kv.key, kv.value);
    }).wait();
}

void syclhash_print(cl::sycl::queue& q, sycl_hashtable::KV* hashtable, int key) {
    q.parallel_for(cl::sycl::range<1>(1), [=](cl::sycl::id<1> idx) {
        int v = sycl_hashtable::load(hashtable, key);
        printf("load: %d\n", v);
    }).wait();
}

void syclhash_delete(cl::sycl::queue& q, sycl_hashtable::KV* hashtable, std::vector<int> keys) {
    int n = keys.size();
    int keys_arr[MAX_INSERT_CAP];
    if (n <= MAX_INSERT_CAP) {
        std::copy(keys.begin(), keys.end(), keys_arr);
    }
    q.parallel_for(cl::sycl::range<1>(n), [=](cl::sycl::id<1> idx) {
        auto key = keys_arr[idx];
        sycl_hashtable::del(hashtable, key);
    }).wait();
}

void syclhash_delete_one(cl::sycl::queue& q, sycl_hashtable::KV* hashtable, int key) {
    q.parallel_for(cl::sycl::range<1>(1), [=](cl::sycl::id<1> idx) {
        sycl_hashtable::del(hashtable, key);
    }).wait();
}