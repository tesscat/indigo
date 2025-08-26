#include <file.hpp>
#include <uefi.h>

char* readFile(const char* name) {
    FILE* f = fopen(name, "r");
    if (f == NULL) {
        printf("Unable to open file %s\n", name);
        return nullptr;
    }
    // get size
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* data = (char*)malloc(sizeof(char) * (size + 1));
    if (data == NULL) {
        printf("Unable to allocate memory for file %s\n", name);
        fclose(f);
        return nullptr;
    }
    fread(data, size, 1, f);
    data[size] = '\0';
    fclose(f);

    return data;
}

BinaryFile readBinaryFile(const char* name) {
    BinaryFile out;
    out.data = nullptr;
    out.len  = 0;
    FILE* f  = fopen(name, "r");
    if (f == NULL) {
        printf("Unable to open file %s\n", name);
        return out;
    }
    // get size
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void* data = malloc(size);
    if (data == NULL) {
        printf("Unable to allocate memory for file %s\n", name);
        fclose(f);
        return out;
    }
    fread(data, size, 1, f);
    fclose(f);

    out.data = data;
    out.len  = size;

    return out;
}
