#ifndef SYCL_HASHTABLE_H
#define SYCL_HASHTABLE_H

#include <CL/sycl.hpp>
#include "stdio.h"

#define CAPACITY 400
#define EMPTY_SENTINEL -229384
#define MAX_INSERT_CAP 100

namespace sycl_hashtable {

    template <typename T>
    struct KV
    {
        int key;
        T value;
    };
}

namespace sycl_hashtable {

    inline int hash(int x) {
        return x*2654435761 % 2^32;
    }

    template <typename T>
    void insert(KV<T>* hashtable, int key, T value) {
        
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

    template <typename T>
    T load(KV<T>* hashtable, int key) {

        auto s = hash(key);
        int empty_sentinel = EMPTY_SENTINEL;
        int capacity = CAPACITY;
        int counter = 0;

        while (counter < capacity) {
            if (hashtable[s].key == key) {
                return hashtable[s].value;
            } else if (hashtable[s].key == empty_sentinel) {
                printf("-- empty\n");
                return (T) 0;
            }
            s = (s + 1) & (capacity-1);
            counter++;
        }
        return 0;
    }

    template <typename T>
    void del(KV<T>* hashtable, int key) {
        
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

template <typename T>
void syclhash_insert(cl::sycl::queue& q, sycl_hashtable::KV<T>* hashtable, std::vector<sycl_hashtable::KV<T>> inserts) {
    int n = inserts.size();
    sycl_hashtable::KV<T> inserts_arr[MAX_INSERT_CAP];
    if (n <= MAX_INSERT_CAP) {
        std::copy(inserts.begin(), inserts.end(), inserts_arr);
    }
    q.parallel_for(cl::sycl::range<1>(n), [=](cl::sycl::id<1> idx) {
        auto kv = inserts_arr[idx];
        sycl_hashtable::insert(hashtable, kv.key, kv.value);
    }).wait();
}

template <typename T>
void syclhash_print_one(cl::sycl::queue& q, sycl_hashtable::KV<T>* hashtable, int key) {
    q.parallel_for(cl::sycl::range<1>(1), [=](cl::sycl::id<1> idx) {
        T v = sycl_hashtable::load(hashtable, key);
        printf("load(d): %d\n", (int) v);
    }).wait();
}

// template <typename T>
// T syclhash_load_one(cl::sycl::queue& q, sycl_hashtable::KV<T>* hashtable, int key) {
//     T v[1];
//     // cl::sycl::buffer<T,1> v_buf(&v[0], 1);
//     printf("test??\n");
//     q.submit([&](auto &cgh) {
//         // cl::sycl::accessor v_acc(v_buf, cgh, cl::sycl::write_only, cl::sycl::no_init);
//         q.parallel_for(cl::sycl::range<1>(1), [&](cl::sycl::id<1> idx) {
//             printf("test??\n");
//             // v_acc[0] = sycl_hashtable::load(hashtable, key);
//         });
//     }).wait();
//     return v[0];
// }

template <typename T>
void syclhash_delete(cl::sycl::queue& q, sycl_hashtable::KV<T>* hashtable, std::vector<int> keys) {
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

template <typename T>
void syclhash_delete_one(cl::sycl::queue& q, sycl_hashtable::KV<T>* hashtable, int key) {
    q.parallel_for(cl::sycl::range<1>(1), [=](cl::sycl::id<1> idx) {
        sycl_hashtable::del(hashtable, key);
    }).wait();
}

#endif