#include "utils.h"
#include "obfuscator.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        usage_exit();
    }
    if (!isValidFile(argv[1])) {
        printf("invalid file extension\n");
        usage_exit();
    }

    FILE* f = open_file(argv[1]);
    char** data;
    int size = file_to_array(f, &data);
    char** obfs_data = malloc(sizeof(char*) * size);

    init();
    srand(time(NULL));  

    //TODO: dynamic file naming
    int fd = open("obfuscated.c", O_RDWR | O_CREAT | O_TRUNC, 0777);
    if (fd < 0) {
        perror("Failed to open obfuscated.c");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < size; i++) {
        remove_comments(data[i]);
        char* res = obfuscate_line(data[i]);

        obfs_data[i] = malloc(strlen(res) + 1);
        strcpy(obfs_data[i], res);
        write(fd, obfs_data[i], strlen(obfs_data[i]));
        free(res);
    }

    close(fd);

    for (int i = 0; i < size; i++) {
        free(data[i]);
        free(obfs_data[i]);
    }
    free(data);
    free(obfs_data);

    return 0;
}
