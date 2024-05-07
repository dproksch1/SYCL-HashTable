#ifndef SYCL_HASHTABLE_H
#define SYCL_HASHTABLE_H

#include <CL/sycl.hpp>
#include "stdio.h"

#define CAPACITY 400
#define EMPTY_SENTINEL -229384
#define MAX_INSERT_CAP 100

namespace sycl_hashtable {
    struct KV
    {
        int key;
        int value;
    };
}

void syclhash_insert(cl::sycl::queue&, sycl_hashtable::KV*, std::vector<sycl_hashtable::KV>);
void syclhash_print(cl::sycl::queue&, sycl_hashtable::KV*, int key);
void syclhash_delete(cl::sycl::queue&, sycl_hashtable::KV*, std::vector<int>);
void syclhash_delete_one(cl::sycl::queue&, sycl_hashtable::KV*, int key);

#endif