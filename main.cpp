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
    auto t = syclhash_load(q, hashtable, {22,25,29});
    printf("%f\n", t[0]);
    std::cout << "expected: 13.2334  ";
    printf("%f\n", t[1]);
    std::cout << "expected: -6633.153  ";
    printf("%f\n", t[2]);
    syclhash_delete(q, hashtable, {22,29});
    printf("post_del: ");
    syclhash_print_one(q, hashtable, 22); std::cout << " ";
    syclhash_print_one(q, hashtable, 25); std::cout << " ";
    syclhash_print_one(q, hashtable, 29); std::cout << "\n";
    cl::sycl::free(hashtable, q);
}