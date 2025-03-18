#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// to use struct stat, stat(), S_ISREG(), S_ISDIR()
// #include <sys/stat.h>
// to use getopt()
// #include <unistd.h>
#include <zlib.h>

#define VERSION "0.2"

typedef struct FileNode {
    char *file_name;
    struct FileNode *next;
} FileNode;

typedef struct {
    bool archive_mode;
    bool unarchive_mode;
    bool a;
    bool d;
    bool r;
    bool p;
    bool l;
    char *password;
    char *archive_name;
    FileNode *file_list; // Linked list for all matched files
} Options;


void archive_files(const char *archive_name, int file_count, char *file_list[]);
void add_file_to_archive(FILE *archive, const char *file_path);
void unarchive_files(const char *archive_name);

void print_usage(void);
int parse_options(int argc, char *argv[], Options *opts);
// TODO void add_file_to_list();
// TODO void traverse_directory();


int main(int argc, char *argv[])
{
    Options opts = {0};

    if (parse_options(argc, argv, &opts) != 0)
    {
        return 1;
    }

    if (opts.archive_mode)
    {
        archive_files(argv[2], argc - 3, &argv[3]);
        // TODO: implement support for wildcards *, ?, also directories
        // TODO: implement support for encryption with password
        // TODO: parse options with getopt function
    }
    else if (opts.unarchive_mode)
    {
        unarchive_files(argv[2]);
        // TODO: implement support for -option to list archive contents without unarchiving
        return 0;
    }


return 0;
}


void archive_files(const char *archive_name, int file_count, char *file_list[])
{
    // OPEN archive file in write binary mode
    FILE *archive = fopen(archive_name, "wb");

    // Check if archive file opened correctly
    if (!archive)
    {
        perror("Error creating archive");
        return;
    }

    for (int i = 0; i < file_count; i++)
    {
        add_file_to_archive(archive, file_list[i]);
        // TODO check if each entry is a regular file or a directory
        // If directory add logic to handle directories recursively
    }

    fclose(archive);
}


void add_file_to_archive(FILE *archive, const char *file_path)
{
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        perror("Error opening file");
        return;
    }

    // Get size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory to read the file
    unsigned char *file_data = (unsigned char *)malloc(file_size);
    // Read file contents into memory and check if read correctly
    if (fread(file_data, 1, file_size, file) != file_size)
    {
        perror("Error rading file");
        free(file_data);
        fclose(file);
        return;
    }
    fclose(file);

    // Allocate memory and compress data
    uLongf compressed_size = compressBound(file_size);
    unsigned char *compressed_data = (unsigned char *)malloc(compressed_size);
    if (compress(compressed_data, &compressed_size, file_data, file_size) != Z_OK)
    {
        perror("Error compressing file");
        free(file_data);
        free(compressed_data);
        return;
    }
    // Free file data memory
    free(file_data);

    // Write file metadata (file path, original file size, compressed size) to archive
    fprintf(archive, "%s\n%ld\n%lu\n", file_path, file_size, compressed_size);

    // WRITE file data to archive
    fwrite(compressed_data, 1, compressed_size, archive);

    // Free compressed data memory
    free(compressed_data);
}


void unarchive_files(const char *archive_name)
{
    // Open archive file and check if opened correctly
    FILE *archive = fopen(archive_name, "rb");
    if (!archive)
    {
        perror("Error opening archive");
        return;
    }

    char file_path[256];
    long file_size;
    uLong compressed_size;

    // While reading archive metadadta (file path, original file size, compressed size)
    while(fscanf(archive, "%255s\n%ld\n%lu\n", file_path, &file_size, &compressed_size) == 3)
    {
        // Allocate memory and read compressed data
        unsigned char *compressed_data = (unsigned char *)malloc(compressed_size);
        if (fread(compressed_data, 1, compressed_size, archive) != compressed_size)
        {
            perror("Error reading compressed data");
            free(compressed_data);
            fclose(archive);
            return;
        }

        // Allocate memory to decompress data
        unsigned char *file_data = (unsigned char *)malloc(file_size);
        if (uncompress(file_data, (uLongf *)&file_size, compressed_data, compressed_size) != Z_OK)
        {
            perror("Error decompressing file");
            free(compressed_data);
            free(file_data);
            fclose(archive);
            return;
        }
        free(compressed_data);

        // Write data to a new file (with proper check) and close new file
        FILE *file = fopen(file_path, "wb");
        if (!file)
        {
            perror("Error creating file");
            free(file_data);
            fclose(archive);
            return;
        }
        fwrite(file_data, 1, file_size, file);
        fclose(file);
        free(file_data);
    }

    fclose(archive);
}

void print_usage(void)
{
    printf("Program version: v%s\n", VERSION);
    printf("Syntax:\n");
    printf("  mdarc (command) [-options] [password] <archive_name> <file1>, <file2>, ...\n");
    printf("  Commands: archive - Archive the files listed in [arguments]\n");
    printf("  TODO\n");
}


int parse_options(int argc, char *argv[], Options *opts)
{
    if (argc < 3)
    {
        print_usage();
        return 1;
    }
    if (strcmp(argv[1], "archive") == 0)
    {
        opts->archive_mode = true;

    }
    else if (strcmp(argv[1], "unarchive") == 0)
    {
        opts->unarchive_mode = true;
    }
    else
    {
        printf("Unknown command: %s\n", argv[1]);
        print_usage();
        return 1;
    }


    return 0;
}
