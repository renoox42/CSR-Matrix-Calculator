#include <stdio.h>
#include <stdint.h>
#include "matrix_struct.h"

// Main algorithm
void matr_mult_csr(const void* a, const void* b, void* result) {
    struct csr_matr* a_matr = (struct csr_matr*) a;
    struct csr_matr* b_matr = (struct csr_matr*) b;
    struct csr_matr* res = (struct csr_matr*) result;

    res->numvals = 0;
    res->m = a_matr->m;
    res->n = b_matr->n;
    res->row_ptr[0] = 0;
    
    // Multiplication
    uint64_t save_index = 0;

    for(uint64_t i = 0; i < a_matr->m; i++) {
        uint64_t row_start = a_matr->row_ptr[i];
        uint64_t row_end = a_matr->row_ptr[i + 1];
        
        int zero_flag = 0;
        // Multiply one row of A with B
        for (uint64_t j = row_start; j < row_end; j++) {
            uint64_t a_col = a_matr->col_indices[j];
            float a_val = a_matr->values[j];

            // Process all values to be multiplied with a in one pass
            uint64_t b_row_start = b_matr->row_ptr[a_col];
            uint64_t b_row_end = b_matr->row_ptr[a_col + 1];
            
            for(uint64_t k = b_row_start; k < b_row_end; k++) {
                uint64_t b_col = b_matr->col_indices[k];
                float b_val = b_matr->values[k];

                // Find position in result matrix
                int found = 0;
                uint64_t save_index_finder;
                for(save_index_finder = res->row_ptr[i]; save_index_finder < save_index; save_index_finder++) {
                    if(res->col_indices[save_index_finder] == b_col) {
                        res->values[save_index_finder] += a_val * b_val;
                        res->col_indices[save_index] = b_col;
                        found = 1;
                        if(res->values[save_index_finder] == 0.0) zero_flag = 1;
                        break;
                    }
                }
                if(!found) {
                    res->values[save_index] = a_val * b_val;
                    res->col_indices[save_index] = b_col;
                    save_index++;
                }
            }
        }
        res->row_ptr[i+1] = save_index;
        res->numvals = save_index;
        
        // Sort each row in the matrix in ascending order by col_indices
        // Insertion sort
        for (uint64_t s = (res->row_ptr[i]) + 1; s < (res->row_ptr[i + 1]); s++) {
            uint64_t col = res->col_indices[s];
            float val = res->values[s];
            uint64_t t = s - 1;

            while (t >= (res->row_ptr[i]) && res->col_indices[t] > col) {
                // Overflow check
                if(t == UINT64_MAX) break;

                res->col_indices[t + 1] = res->col_indices[t];
                res->values[t + 1] = res->values[t];
                t--;
            }
            res->col_indices[t + 1] = col;
            res->values[t + 1] = val;
        }

        // Remove zero values if they could have been created
        if(zero_flag == 1) {
            for (uint64_t s = (res->row_ptr[i]); s < (res->row_ptr[i + 1]); s++) {
                if(res->values[s] == 0.0) {
                    for(uint64_t t = s; t < (res->row_ptr[i + 1])-1; t++) {
                        res->values[t] = res->values[t+1];
                        res->col_indices[t] = res->col_indices[t+1];
                    }
                    res->numvals--;
                    save_index--;
                    res->row_ptr[i+1]--;
                    s--;
                }
            }
        }
    }
}

// Inefficient trivial algorithm
void matr_mult_csr_V1(const void* a, const void* b, void* result) {
    struct csr_matr* a_matr = (struct csr_matr*) a;
    struct csr_matr* b_matr = (struct csr_matr*) b;
    struct csr_matr* res = (struct csr_matr*) result;

    res->numvals = 0;
    res->m = a_matr->m;
    res->n = b_matr->n;
    res->row_ptr[0] = 0;

    // Multiplication
    uint64_t save_index = 0;

    for(uint64_t i = 0; i < a_matr->m; i++) {
        uint64_t row_start = a_matr->row_ptr[i];
        uint64_t row_end = a_matr->row_ptr[i + 1];

        // Multiply one row of A with B
        for(uint64_t col = 0; col < b_matr->n; col++) {

            // Multiply one column with one row
            for (uint64_t j = row_start; j < row_end; j++) {
                float a_val = a_matr->values[j];
                uint64_t a_col = a_matr->col_indices[j];

                for(uint64_t search = b_matr->row_ptr[a_col]; search < b_matr->row_ptr[a_col + 1]; search++) {
                    if(b_matr->col_indices[search] == col) {
                        float b_val = b_matr->values[search];
                        uint64_t b_col = b_matr->col_indices[search];

                        // Find position in result matrix
                        int found = 0;
                        uint64_t save_index_finder;
                        for(save_index_finder = res->row_ptr[i]; save_index_finder < save_index; save_index_finder++) {
                            if(res->col_indices[save_index_finder] == b_col) {
                                res->values[save_index_finder] += a_val * b_val;
                                res->col_indices[save_index] = b_col;
                                found = 1;
                                break;
                            }
                        }
                        if(!found) {
                            res->values[save_index] = a_val * b_val;
                            res->col_indices[save_index] = b_col;
                            save_index++;
                        }
                        break;
                    }
                }
            }
        }
        res->row_ptr[i+1] = save_index;
        res->numvals = save_index;
        
        // Remove zero values
        for (uint64_t s = (res->row_ptr[i]); s < (res->row_ptr[i + 1]); s++) {
            if(res->values[s] == 0.0) {
                for(uint64_t t = s; t < (res->row_ptr[i + 1])-1; t++) {
                    res->values[t] = res->values[t+1];
                    res->col_indices[t] = res->col_indices[t+1];
                }
                res->numvals--;
                save_index--;
                res->row_ptr[i+1]--;
                s--;
            }
        }
    }
}

// Calculate an approximate value for the size of the result matrix (it will usually be a bit smaller)
int determine_size(struct csr_matr* a_matr, struct csr_matr* b_matr) {
    uint64_t size = 0;

    for(uint64_t i = 0; i < a_matr->m; i++) {
        uint64_t row_start = a_matr->row_ptr[i];
        uint64_t row_end = a_matr->row_ptr[i + 1];
        
        for (uint64_t j = row_start; j < row_end; j++) {
            uint64_t a_col = a_matr->col_indices[j];

            size += (b_matr->row_ptr[a_col + 1]) - (b_matr->row_ptr[a_col]);
        }
    }
    return size;
}
