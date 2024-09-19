#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include "convert.h"
#include "calculations.h"


void print_help(void);
struct csr_matr* allocate_matrix(uint64_t m, uint64_t n, uint64_t nnz);
void free_matrix(struct csr_matr* matrix);

int main(int argc, char* argv[]) {
    int opt;  // Variable for command line options
    int impl_version = 0;  // Default implementation version
    int repetitions = 1;  // Default number of repetitions
    char *file_a = NULL, *file_b = NULL, *output_file = NULL;  // File variables
    int time = 0; // Boolean for timing

    // Definition of long options for getopt_long
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // Loop to process command line arguments
    while ((opt = getopt_long(argc, argv, "V:B::a:b:o:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'V':  // Option for implementation version
                impl_version = atoi(optarg);
                break;
            case 'B':  // Option for runtime measurement
                if (optarg) {
                    repetitions = atoi(optarg);
                } else {
                    repetitions = 1;
                }
                time = 1;
                break;
            case 'a':  // Option for input file A
                file_a = optarg;
                break;
            case 'b':  // Option for input file B
                file_b = optarg;
                break;
            case 'o':  // Option for output file
                output_file = optarg;
                break;
            case 'h':  // Help option
            case '?':  // Invalid option
            default:
                print_help();
                exit(EXIT_SUCCESS);
        }
    }

    // Check if necessary options are set
    if (!file_a || !file_b || !output_file) {
        fprintf(stderr, "Error: Missing required options.\n");
        print_help();
        exit(EXIT_FAILURE);
    }

    // Error handling: repetitions
    if(repetitions <= 0) {
        fprintf(stderr, "Error: No or negative repetitions are not possible.\n");
        return EXIT_FAILURE;
    }

    // Read matrices from input files
    struct csr_matr *matrix_a = initial(file_a);
    if(matrix_a == NULL) {
        return EXIT_FAILURE;
    }

    struct csr_matr *matrix_b = initial(file_b);
    if(matrix_b == NULL) {
        free_matrix(matrix_a);
        return EXIT_FAILURE;
    }

    // Error handling: Incorrect dimensions of matrices A and B
    if(matrix_a -> n != matrix_b -> m) {
        fprintf(stderr, "Error: Matrix with dimensions (%lu x %lu) cannot be multiplied with matrix with dimensions (%lu x %lu).\n", 
        matrix_a -> m, matrix_a -> n, matrix_b -> m, matrix_b -> n);
        free_matrix(matrix_a);
        free_matrix(matrix_b);
        return EXIT_FAILURE;
    }

    // Allocate memory for the result matrix
    struct csr_matr *result_matrix = allocate_matrix(matrix_a->m, matrix_b->n, (determine_size(matrix_a, matrix_b)));
    if(result_matrix == NULL) {
        free_matrix(matrix_a);
        free_matrix(matrix_b);
        return EXIT_FAILURE;
    }

    // Runtime measurement
    struct timespec start_time, end_time;
    uint64_t average_time_ns = 0;

    if(impl_version == 0) {
        if(time) {
            for (int i = 0; i < repetitions; i++) {
                clock_gettime(CLOCK_MONOTONIC, &start_time);
                // Perform matrix multiplication
                matr_mult_csr(matrix_a, matrix_b, result_matrix);
                clock_gettime(CLOCK_MONOTONIC, &end_time);
                average_time_ns += ((end_time.tv_sec*1000000000-start_time.tv_sec*1000000000) + (end_time.tv_nsec-start_time.tv_nsec))/repetitions;
                // At least one second gap between measurements
                sleep(1);
            }
        } else {
            matr_mult_csr(matrix_a, matrix_b, result_matrix);
        }
    } else if(impl_version == 1) {
        if(time) {
            for (int i = 0; i < repetitions; i++) {
                clock_gettime(CLOCK_MONOTONIC, &start_time);
                // Perform matrix multiplication
                matr_mult_csr_V1(matrix_a, matrix_b, result_matrix);
                clock_gettime(CLOCK_MONOTONIC, &end_time);
                average_time_ns += ((end_time.tv_sec*1000000000-start_time.tv_sec*1000000000) + (end_time.tv_nsec-start_time.tv_nsec))/repetitions;
                // At least one second gap between measurements
                sleep(1);
            }
        } else {
            matr_mult_csr_V1(matrix_a, matrix_b, result_matrix);
        }
    } else {
        fprintf(stderr, "Error: This implementation version does not exist.\n");
        free_matrix(matrix_a);
        free_matrix(matrix_b);
        free_matrix(result_matrix);
        return EXIT_FAILURE;
    }

    if(time) {
        printf("Average runtime: %lu nanoseconds\n", average_time_ns);
    }

    if(result_matrix -> numvals == 0) {
        fprintf(stderr, "Error: Multiplication resulted in empty matrix. No matrix was written.\n");
        free_matrix(matrix_a);
        free_matrix(matrix_b);
        free_matrix(result_matrix);
        return EXIT_FAILURE;
    }

    // Write result matrix to output file
    write_matr(output_file, result_matrix);
    
    // Free allocated memory
    free_matrix(matrix_a);
    free_matrix(matrix_b);
    free_matrix(result_matrix);
    
    return EXIT_SUCCESS;
}

// Function to free a CSR matrix
void free_matrix(struct csr_matr* matrix) {
    free(matrix -> col_indices);
    free(matrix -> values);
    free(matrix -> row_ptr);
    free(matrix);
}

// Function to allocate a CSR matrix
struct csr_matr* allocate_matrix(uint64_t m, uint64_t n, uint64_t nnz) {
    struct csr_matr* matrix = (struct csr_matr*)malloc(sizeof(struct csr_matr));
    if (!matrix) {
        fprintf(stderr, "Error: Memory could not be allocated.\n");
        return NULL;
    }

    matrix -> m = m;
    matrix -> n = n;
    matrix -> numvals = 0;

    matrix->values = (float*)malloc(nnz * sizeof(float));
    if (!matrix->values) {
        fprintf(stderr, "Error: Memory could not be allocated for values.\n");
        free(matrix);
        return NULL;
    }

    matrix->col_indices = (uint64_t*)malloc(nnz * sizeof(uint64_t));
    if (!matrix->col_indices) {
        fprintf(stderr, "Error: Memory could not be allocated for col_indices.\n");
        free(matrix->values);
        free(matrix);
        return NULL;
    }

    matrix->row_ptr = (uint64_t*)malloc((m + 1) * sizeof(uint64_t));
    if (!matrix->row_ptr) {
        fprintf(stderr, "Error: Memory could not be allocated for row_ptr.\n");
        free(matrix->values);
        free(matrix->col_indices);
        free(matrix);
        return NULL;
    }

    return matrix;
}

// Function to print the help message
void print_help() {
    printf("Help message:\n");
    printf("Usage: prog [OPTIONS]\n");
    printf("Options:\n");
    printf("  -V <number>    Specify implementation version. Default is 0.\n");
    printf("  -B[<number>]   Runtime measurement. Optionally, the number of repetitions can be specified.\n");
    printf("  -a <filename>  Input file containing matrix A.\n");
    printf("  -b <filename>  Input file containing matrix B.\n");
    printf("  -o <filename>  Output file.\n");
    printf("  --help, -h     Display this help and exit.\n");
    printf("\nExamples:\n");
    printf("  ./program -a matrix_a.txt -b matrix_b.txt -o result.txt\n");
    printf("  ./program -V 1 -B5 -a matrix_a.txt -b matrix_b.txt -o result.txt\n");
}
