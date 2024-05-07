#include <CL/sycl.hpp>
#include "stdio.h"
#include "hashtable.hpp"


int main()
{
    std::cout << "small test run" << std::endl;
    cl::sycl::queue q;
    sycl_hashtable::KV *hashtable = cl::sycl::malloc_shared<sycl_hashtable::KV>(CAPACITY, q);
    std::vector<sycl_hashtable::KV> inserts = {{22, 175500}, {25,13}, {29,-6633}};
    syclhash_insert(q, hashtable, inserts);

    std::cout << "expected: 17550  ";
    syclhash_print(q, hashtable, 22);
    std::cout << "expected: 13  ";
    syclhash_print(q, hashtable, 25);
    std::cout << "expected: -6633  ";
    syclhash_print(q, hashtable, 29);
}