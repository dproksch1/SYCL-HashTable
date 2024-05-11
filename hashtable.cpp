#include <functional>
#include "hashtable.hpp"

template<>
void syclhash_print_one(cl::sycl::queue& q, sycl_hashtable::KV<double>* hashtable, int key) {
    q.parallel_for(cl::sycl::range<1>(1), [=](cl::sycl::id<1> idx) {
        double v = sycl_hashtable::load(hashtable, key);
        printf("%f", v);
    }).wait();
}