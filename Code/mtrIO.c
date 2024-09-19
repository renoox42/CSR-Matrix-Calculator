#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

// read_file function from the tutorial exercises
char* read_file(const char* path) {
    char* string;
    FILE* file;

    if ((file = fopen(path, "rb")) == NULL) {
        fprintf(stderr, "Error: Could not open file \"%s\".\n", path);
        return NULL;
    }

    struct stat statbuf;
    if (fstat(fileno(file), &statbuf)) {
        fclose(file);
        fprintf(stderr, "Error: Could not access file \"%s\".\n", path);
        return NULL;
    }

    long filesize = statbuf.st_size;
    unsigned is_regular = S_ISREG(statbuf.st_mode);
    if (!is_regular || filesize <= 0) {
        fclose(file);
        if(filesize <= 0){
            fprintf(stderr, "Error: File %s was empty.\n", path);
        }
        else {
            fprintf(stderr, "Error: File %s is not a regular file.\n", path);
        }
        return NULL;
    }

    if ((string = malloc(filesize + 1)) == NULL) {
        fclose(file);
        fprintf(stderr, "Error: Could not allocate memory for reading string in \"%s\".\n", path);
        return NULL;
    }

    size_t bytes;

    if ((bytes = fread(string, 1, filesize, file)) != (size_t)filesize) {
        free(string);
        fclose(file);
        fprintf(stderr, "Error: Could not read string from file %s correctly.\n", path);
        return NULL;
    }

    string[filesize] = 0;

    fclose(file);
    return string;
}

// write_file function from the tutorial exercises
void write_file(const char* path, const char* string) {
    FILE * file;

    if ((file = fopen(path, "w")) == NULL) {
        fprintf(stderr, "Error: Could not open file \"%s\".\n", path);
        return;
    }

    size_t length = strlen(string);
    if ((fwrite(string, 1, length, file)) != length) {
        fprintf(stderr, "Error: Could not write string to file %s correctly.\n", path);
        return;
    }

    fclose(file);
}

// Counts the numbers in the string to initialize the array for filter_for_numbers
int number_of_numbers(const char* string) {
    int index = 0;
    int n;
    float i;
    int count = 0;
    while (1 == sscanf(string + index, " %f%n", &i, &n)) {
        index += n;
        count++;
        while (string[index] == ',') {
            index++;
        }
    }
    return count;
}

// Stores the numbers contained in the string in a float array
void filter_for_numbers(const char* string, float* arr) {
    int index = 0;
    int n;
    float i;
    int arrCount = 0;
    while (1 == sscanf(string + index, " %f%n", &i, &n)) {
        index += n;
        arr[arrCount++] = i;
        
        while (string[index] == ',' || string[index] == ' ' || string[index] == '\n') {
            index++;
        }
    } 
}

// Counts the number of elements per string line
void spliterate(char* string, int* indices) {
    char * tok = strtok(string, "\n");
    int i = 0;
    while (tok != NULL && i < 4) {
        indices[i] = number_of_numbers(tok);
        i++;
        tok = strtok(NULL, "\n");
    }
}
