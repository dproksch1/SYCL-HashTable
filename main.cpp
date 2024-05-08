#include <CL/sycl.hpp>
#include "stdio.h"
#include "hashtable.hpp"


int main()
{
    std::cout << "small test run" << std::endl;
    cl::sycl::queue q;
    sycl_hashtable::KV<double> *hashtable = cl::sycl::malloc_shared<sycl_hashtable::KV<double>>(CAPACITY, q);
    std::vector<sycl_hashtable::KV<double>> inserts = {{22, 175500.245}, {25,13.2334}, {29,-6633.153}};
    syclhash_insert(q, hashtable, inserts);

    std::cout << "expected: 17550.245  ";
    syclhash_print_one(q, hashtable, 22);
    std::cout << "expected: 13.2334  ";
    syclhash_print_one(q, hashtable, 25);
    std::cout << "expected: -6633.153  ";
    syclhash_print_one(q, hashtable, 29);
}