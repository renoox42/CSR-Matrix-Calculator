#include "matrix_struct.h"
#include "mtrIO.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Method to read the string from the specified file and initialize the matrix with the contained values
struct csr_matr* initial(char *path) {
    struct csr_matr *mtr = (struct csr_matr*)malloc(sizeof(struct csr_matr));

    if(mtr == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for the matrix in \"%s\".\n", path);
        return NULL;
    }

    char* string = read_file(path);

    if(string == NULL) {
        return NULL;
    }

    // Check if at least one number is contained in the file
    if(number_of_numbers(string) == 0) {
        fprintf(stderr, "Error: No values could be read from \"%s\".\n", path);
        free(string);
        free(mtr);
        return NULL;
    }

    float *arr = (float*)malloc(number_of_numbers(string) * sizeof(float));
    if(arr == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for an array during the read operation in \"%s\".\n", path);
        free(string);
        free(mtr);
        return NULL;
    }

    int *indices = (int*)malloc(4 * sizeof(int));
    if(indices == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for an index array during the read operation in \"%s\".\n", path);
        free(string);
        free(mtr);
        free(arr);
        return NULL;
    }

    char *copy = (char*)malloc(strlen(string) + 1);
    if(copy == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for a copy string during the read operation in \"%s\".\n", path);
        free(string);
        free(mtr);
        free(arr);
        free(indices);
        return NULL;
    }

    strncpy(copy, string, strlen(string));
    copy[strlen(string) + 1] = '\0';

    spliterate(copy, indices);
    filter_for_numbers(string, arr);

    // Check if any row contains no elements
    if(indices[0] == 0 || indices[1] == 0 || indices[2] == 0 || indices[3] == 0) {
        fprintf(stderr, "Error: The format in the file \"%s\" does not match the expected format.\n", path);
        free(indices);
        free(arr);
        free(string);
        free(copy);
        free(mtr);
        return NULL;
    }

    // Check if n and m are included
    if(indices[0] != 2) {
        fprintf(stderr, "Error: Unexpected number of values in line 1 in \"%s\".\n", path);
        free(string);
        free(indices);
        free(arr);
        free(copy);
        free(mtr);
        return NULL;
    }

    //Check if m and n are greater than 0
    if(arr[0] <= 0 || arr[1] <= 0) {
        fprintf(stderr, "Error: The dimensions are to small in \"%s\".\n", path);
        free(string);
        free(indices);
        free(arr);
        free(copy);
        free(mtr);
        return NULL;
    }

    // Check if there are less than (n * m) + 1 && < 0 elements in line 2
    if(indices[1] > arr[0] * arr[1] && indices[1] != 0) {
        fprintf(stderr, "Error: Unexpected number of values in line 2 in \"%s\".\n", path);
        free(string);
        free(indices);
        free(arr);
        free(copy);
        free(mtr);
        return NULL;
    }

    // Check if lines 2 and 3 contain the same number of elements
    if(indices[1] != indices[2]) {
        fprintf(stderr, "Error: The number of values in lines 2 and 3 are not equal in \"%s\".\n", path);
        free(string);
        free(indices);
        free(arr);
        free(copy);
        free(mtr);
        return NULL;
    }

    // Check if line 3 contains m + 1 elements
    if(indices[3] != (arr[0] + 1)) {
        fprintf(stderr, "Error: Unexpected number of values in line 4 in \"%s\".\n", path);
        free(string);
        free(indices);
        free(arr);
        free(copy);
        free(mtr);
        return NULL;
    }

    float *val = (float*)malloc(indices[1] * sizeof(float));
    if(val == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for the values array in \"%s\".\n", path);
        free(string);
        free(indices);
        free(arr);
        free(copy);
        free(mtr);
        return NULL;
    }

    for (int i = 0; i < indices[1]; ++i) {
        val[i] = arr[i + 2];
    }

    //Check if all values are non zero
     for (int i = 0; i < indices[1]; ++i) {
        if(val[i] == 0) {
            fprintf(stderr, "Error: Some of the values are zero in \"%s\".\n", path);
            free(string);
            free(indices);
            free(arr);
            free(copy);
            free(mtr);
            free(val);
            return NULL;
        }
    }

    uint64_t *col = (uint64_t*)malloc(indices[2] * sizeof(uint64_t));
    if(col == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for the column index array in \"%s\".\n", path);
        free(string);
        free(indices);
        free(arr);
        free(copy);
        free(mtr);
        free(val);
        return NULL;
    }

    for (int i = 0; i < indices[2]; ++i) {
        col[i] = (uint64_t)arr[i + 2 + indices[1]];
        if(arr[i + 2 + indices[1]] < 0) {
            fprintf(stderr, "Error: The values in line 3 are not within the appropriate range in \"%s\".\n", path);
            free(string);
            free(indices);
            free(arr);
            free(copy);
            free(mtr);
            free(val);
            return NULL;
        }
    }

    // Check if all values in col are >= 0 and < n
    for (int i = 0; i < indices[2]; ++i) {
        if(col[i] >= arr[1]) {
            fprintf(stderr, "Error: The values in line 3 are not within the appropriate range in \"%s\".\n", path);
            free(string);
            free(indices);
            free(arr);
            free(copy);
            free(mtr);
            free(val);
            return NULL;
        }
    }

    uint64_t *row = (uint64_t*)malloc(indices[3] * sizeof(uint64_t));
    if(row == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for the row index array in \"%s\".\n", path);
        free(string);
        free(indices);
        free(arr);
        free(copy);
        free(mtr);
        free(val);
        free(col);
        return NULL;
    }

    for (int i = 0; i < indices[3]; ++i) {
        row[i] = arr[i + 2 + indices[1] + indices[2]];
    }

    // Check if row is sorted in ascending order
    int i = 0;
    while(row[i] <= row[i+1] && i <= arr[0]) {
        i++;
    }

    if(i < arr[0]) {
        fprintf(stderr, "Error: The values in line 4 are not properly ordered in \"%s\".\n", path);
        free(string);
        free(indices);
        free(arr);
        free(copy);
        free(mtr);
        free(val);
        free(col);
        free(row);
        return NULL;
    }

    // Check if row starts at zero
    if(row[0] != 0) {
        fprintf(stderr, "Error: Line 4 does not start at zero in \"%s\".\n", path);
        free(string);
        free(indices);
        free(arr);
        free(copy);
        free(mtr);
        free(val);
        free(col);
        free(row);
        return NULL;
    }

    // Check if the last element of line 4 equals the number of elements in line 2
    if(row[(int)arr[0]] != (uint64_t)indices[1]) {
        fprintf(stderr, "Error: The last value in line 4 is not the number of elements in line 2 in \"%s\".\n", path);
        free(string);
        free(indices);
        free(arr);
        free(copy);
        free(mtr);
        free(val);
        free(col);
        free(row);
        return NULL;
    }

    // Check if each row has fewer than n elements
    int c = 0;
    uint64_t acc = 0;
    while((row[c] - acc) <= arr[1] && c <= arr[0]) {
        acc = row[c];
        c++;
    }

    if(c != arr[0] + 1) {
        fprintf(stderr, "Error: There are more than n values in one row of the matrix in \"%s\".\n", path);
        free(string);
        free(indices);
        free(arr);
        free(copy);
        free(mtr);
        free(val);
        free(col);
        free(row);
        return NULL;
    }

    //Check if col is ordered correctly
    uint64_t bound = 0;
    for(int i = 0; i < arr[0]; ++i) {
        if(row[i + 1] == 0){
            bound = 0;
        }
        else{
            bound = row[i + 1] - 1;
        }
        for(uint64_t j = row[i]; j < bound; ++j) {
            if(col[j] >= col[j + 1]) {
                fprintf(stderr, "Error: Line 3 is not ordered as expected in \"%s\".\n", path);
                free(string);
                free(indices);
                free(arr);
                free(copy);
                free(mtr);
                free(val);
                free(col);
                free(row);
                return NULL;
            }
        }
    }

    if(mtr != NULL) {
        mtr->n = arr[1];
        mtr->m = arr[0];
        mtr->numvals = indices[1];
        mtr->values = val;
        mtr->col_indices = col;
        mtr->row_ptr = row;
    } else {
        fprintf(stderr, "Error: Could not allocate memory for the matrix in \"%s\".\n", path);
    }

    free(copy);
    free(arr);
    free(string);
    free(indices);

    return mtr;
}

// Revised write method
// Writes the given matrix to the specified file
void write_matr(const char *path, const void *c) {
    FILE *file;
    const struct csr_matr *mtr = (const struct csr_matr *)c; 

    if((file = fopen(path, "w")) == NULL) {
        fprintf(stderr, "Error: Could not open file to write the output.\n");
        return;
    }

    fprintf(file, "%lu,%lu\n", mtr->m, mtr->n);

    for(uint64_t i = 0; i < mtr->numvals - 1; ++i) {
        fprintf(file, "%g,", mtr->values[i]);
    }
    fprintf(file, "%g\n", mtr->values[mtr->numvals-1]);
    
    for(uint64_t i = 0; i < mtr->numvals - 1; ++i) {
        fprintf(file, "%lu,", mtr->col_indices[i]);
    }
    fprintf(file, "%lu\n", mtr->col_indices[mtr->numvals-1]);

    for(uint64_t i = 0; i < mtr->m; ++i) {
        fprintf(file, "%lu,", mtr->row_ptr[i]);
    }
    fprintf(file, "%lu", mtr->row_ptr[mtr->m]);

    fclose(file);
}
