#ifndef MTRIO
#define MTRIO

char* read_file(const char* path);

void write_file(const char* path, const char* string);

int number_of_numbers(const char* string);

void filter_for_numbers(const char* string, float* arr);

void spliterate(char* string, int* indices);

#endif
