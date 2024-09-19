#ifndef MATRIX_STRUCT
#define MATRIX_STRUCT

#include <stdint.h>

// Internal representation of the matrix
struct csr_matr {
    uint64_t n;
    uint64_t m;
    uint64_t numvals;
    float* values;
    uint64_t* col_indices;
    uint64_t* row_ptr;
};

#endif