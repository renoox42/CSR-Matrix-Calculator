#ifndef CALCULATIONS
#define CALCULATIONS

#include "matrix_struct.h"

void matr_mult_csr(const void* a, const void* b, void* result);

void matr_mult_csr_V1(const void* a, const void* b, void* result);

int determine_size(struct csr_matr* a_matr, struct csr_matr* b_matr);

#endif
