#ifndef SYCL_HASHTABLE_H
#define SYCL_HASHTABLE_H

#include <CL/sycl.hpp>
#include "stdio.h"

#define CAPACITY 400
#define EMPTY_SENTINEL -123456789
#define MAX_INSERT_CAP 100
#define MAX_LOAD_CAP 100

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
sycl_hashtable::KV<T>* syclhash_alloc_table(cl::sycl::queue& q) {
    return cl::sycl::malloc_shared<sycl_hashtable::KV<T>>(CAPACITY, q);
}

template <typename T>
void syclhash_free_table(cl::sycl::queue& q, sycl_hashtable::KV<T>* hashtable) {
    cl::sycl::free(hashtable, q);
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
        T v = sycl_hashtable::load<T>(hashtable, key);
        printf("%d", (int) v);
    }).wait();
}

template <typename T>
T syclhash_load_one(cl::sycl::queue& q, sycl_hashtable::KV<T>* hashtable, int key) {
    T v[1];
    {
        cl::sycl::buffer<T,1> v_buf(&v[0], 1);
        q.submit([&](auto &cgh) {
            cl::sycl::accessor v_acc(v_buf, cgh, cl::sycl::write_only, cl::sycl::no_init);
            cgh.parallel_for(cl::sycl::range<1>(1), [=](cl::sycl::id<1> idx) {
                v_acc[0] = sycl_hashtable::load<T>(hashtable, key);
            });
        }).wait();
    }
    return v[0];
}

template <typename T>
std::vector<T> syclhash_load(cl::sycl::queue& q, sycl_hashtable::KV<T>* hashtable, std::vector<int> keys) {
    T v_arr[MAX_LOAD_CAP];
    int n = keys.size();
    int keys_arr[MAX_LOAD_CAP];
    if (n <= MAX_LOAD_CAP) {
        std::copy(keys.begin(), keys.end(), keys_arr);
    }
    {
        cl::sycl::buffer<T,1> v_buf(&v_arr[0], n);
        cl::sycl::buffer<int,1> keys_buf(&keys_arr[0], n);
        q.submit([&](auto &cgh) {
            cl::sycl::accessor v_acc(v_buf, cgh, cl::sycl::write_only, cl::sycl::no_init);
            cl::sycl::accessor keys_acc(keys_buf, cgh, cl::sycl::read_only);
            cgh.parallel_for(cl::sycl::range<1>(n), [=](cl::sycl::id<1> idx) {
                v_acc[idx] = sycl_hashtable::load<T>(hashtable, keys_acc[idx]);
            });
        }).wait();
    }
    return std::vector<T>(std::begin(v_arr), std::end(v_arr));
}

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