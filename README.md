# SYCL-HashTable
A small hashtable implementation in SYCL.

### Usage:

- Initialize SYCL queue via `cl::sycl::queue q`
- Allocate hashtable using `syclhash_alloc_table<YOUR_TYPE>(q)` with `YOUR_TYPE` being `int`, `unsigned int`, `long`, `unsigned long`, `long long`, `unsigned long long`, `float` or `double`
- Insert elements into the table using `syclhash_insert(q, HASHTABLE, INSERTS)` with `HASHTABLE` being the pointer to your syclhash hashtable and `INSERTS` being a vector containing key-value-pairs of the type `sycl_hashtable::KV<YOUR_TYPE>` (keys are always of type `int`)
- Load elements from the table via `syclhash_load(q, HASHTABLE, KEYS)` with with `HASHTABLE` being the pointer to your syclhash hashtable and `KEYS` being a `int` vector containing the searched for keys
- Alternatively load a single element using `syclhash_load_one(q, HASHTABLE, KEY)`
- Delete elements from the hashtable via `syclhash_delete(q, HASHTABLE, KEYS)` using a key vector or `syclhash_delete_one(q, HASHTABLE, KEY)` for single elements