#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// to use struct stat, stat(), S_ISREG(), S_ISDIR()
// #include <sys/stat.h>
#include <unistd.h> // to use getopt()
#include <zlib.h> // to use compress() and uncompress()

#define VERSION "0.3"

typedef struct FileNode
{
    char *file_name;
    struct FileNode *next;
} FileNode;

typedef struct
{
    bool archive_mode;
    bool unarchive_mode;
    bool a; // add files to existing archive TODO
    bool d; // delete files from existing archive TODO
    bool r; // recurse into directories TODO
    bool p; // input password TODO
    bool l; // list files in an archive without extracting
    char *password;
    char *archive_name;
    FileNode *file_list; // Linked list for all matched files
    unsigned int file_count;
} Options;

void archive_files(Options *opts);
void add_file_to_archive(FILE *archive, const char *file_path);
void unarchive_files(Options *opts);

void print_usage(char *errmsg); // Print program syntax, Accepts input for a custom error message
int parse_options(int argc, char *argv[], Options *opts);
int read_file_list(int argc, char *argv[], Options *opts);
    // TODO void add_file_to_list();
    // TODO void traverse_directory();
void free_opts(Options *opts);


int main(int argc, char *argv[])
{
    Options opts = {0}; //Initialize all values in opts to false/NULL/0

    // Parse command line options
    if (parse_options(argc, argv, &opts) != 0)
    {
        return 1;
    }

    // Read file list
    if (read_file_list(argc, argv, &opts) != 0)
    {
        return 1;
    }

    // Execute archive or unarchive depending on mode selection
    if (opts.archive_mode)
    {
        archive_files(&opts);
    }
    else if (opts.unarchive_mode)
    {
        unarchive_files(&opts);
    }

    // // Debug print
    // if (opts.a) {printf("-a\n");}
    // if (opts.d) {printf("-d\n");}
    // if (opts.r) {printf("-r\n");}
    // if (opts.p)
    // {
    //     printf("-p, Password: %s\n",opts.password);
    // }
    // if (opts.l) {printf("-l\n");}
    // printf("archive name: %s\n", opts.archive_name);
    // printf("file count: %u\n", opts.file_count);
    // for (FileNode *ptr = opts.file_list; ptr != NULL; ptr = ptr->next)
    // {
    //     printf("file name: %s\n", ptr->file_name);
    // }
    // // End of debug print

    free_opts(&opts);
    return 0;
}

void archive_files(Options *opts)
{
    // OPEN archive file in write binary mode
    FILE *archive = fopen(opts->archive_name, "wb");

    // Check if archive file opened correctly
    if (!archive)
    {
        perror("Error creating archive");
        return;
    }

    // Iterate through file list and add each file to archive
    FileNode *current = opts->file_list;
    while (current != NULL)
    {
        add_file_to_archive(archive, current->file_name);
        FileNode *next = current->next;
        current = next;
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
    unsigned char *file_data = (unsigned char *) malloc(file_size);
    // Read file contents into memory and check if read correctly
    if (fread(file_data, 1, file_size, file) != file_size)
    {
        perror("Error reading file");
        free(file_data);
        fclose(file);
        return;
    }
    fclose(file);

    // Allocate memory and compress data
    uLongf compressed_size = compressBound(file_size);
    unsigned char *compressed_data = (unsigned char *) malloc(compressed_size);
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

void unarchive_files(Options *opts)
{
    // Open archive file and check if opened correctly
    FILE *archive = fopen(opts->archive_name, "rb");
    if (!archive)
    {
        perror("Error opening archive");
        return;
    }

    char file_path[256];
    long file_size;
    uLong compressed_size;

    // While reading archive metadadta (file path, original file size, compressed size)
    while (fscanf(archive, "%255s\n%ld\n%lu\n", file_path, &file_size, &compressed_size) == 3)
    {
        // Allocate memory and read compressed data
        unsigned char *compressed_data = (unsigned char *) malloc(compressed_size);
        if (fread(compressed_data, 1, compressed_size, archive) != compressed_size)
        {
            perror("Error reading compressed data");
            free(compressed_data);
            fclose(archive);
            return;
        }

        // Allocate memory to decompress data
        unsigned char *file_data = (unsigned char *) malloc(file_size);
        if (uncompress(file_data, (uLongf *) &file_size, compressed_data, compressed_size) != Z_OK)
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

void print_usage(char *errmsg)
{
    if (errmsg[0] != '\0')
    {
        printf("%s\n", errmsg);
    }
    printf("\nProgram version: v%s\n\n", VERSION);
    printf("Syntax:\n");
    printf("  mdarc (command) [-options] [password] <archive_name> <file1>, <file2>, ...\n");
    printf("  Commands: archive - Archive the files listed in [arguments]\n");
    printf("  TODO\n");
}

int parse_options(int argc, char *argv[], Options *opts)
{
    // Check for minimum command line arguments
    if (argc < 3)
    {
        print_usage("Not enough command line arguments");
        return 1;
    }
    // Check for proper command input
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
        print_usage("Unknown command");
        return 1;
    }

    // Parse command line options
    int opt;
    while ((opt = getopt(argc, argv, "adrp:l")) != -1)
    {
        switch(opt)
        {
            case 'a':
                opts->a = true;
                break;
            case 'd':
                opts->d = true;
                break;
            case 'r':
                opts->r = true;
                break;
            case 'p':
                opts->p = true;
                opts->password = optarg;
                break;
            case 'l':
                opts->l = true;
                break;
            default:
                print_usage("Unknown option");
                return 1;
        }
    }

    // TODO options error handling - handle mutually exclusive options, repeating of options

    return 0;
}


int read_file_list(int argc, char *argv[], Options *opts)
{
    // Read archive name
    opts->archive_name = argv[optind+1];

    opts->file_list = NULL; // Initialize file list
    FileNode *current = NULL; // Pointer to track current node
    opts->file_count = 0; // Initialize file count

    // Read file list
    for (int i = optind + 2; i < argc; i++)
    {
        // Allocate memory for the new node
        FileNode *new_file = malloc(sizeof(FileNode));
        if (new_file == NULL)
        {
            perror("Could not allocate memmory for file node");
            return 1;
        }

        new_file->file_name = argv[i]; // Copy the file name

        new_file->next = NULL;
        if (opts->file_list == NULL) // If first element in file list
        {
            opts->file_list = new_file;
        }
        else
        {
            current->next = new_file;
        }

        current = new_file; // Update the current node
        opts->file_count++; // Increment file count
    }

    return 0;
}

void free_opts(Options *opts)
{
    // free(opts->archive_name);

    FileNode *current = opts->file_list;
    while (current != NULL)
    {
        FileNode *next = current->next;
        // free(current->file_name);
        free(current);
        current = next;
    }
    opts->file_list = NULL;
}
