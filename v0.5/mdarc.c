#include <dirent.h> // to use struct dirent, opendir(), readdir(), closedir()
#include <errno.h> // to use errno, EEXIST for mkdir()
#include <glob.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // to use struct stat, stat(), S_ISREG(), S_ISDIR()
#include <sys/types.h>
#include <unistd.h> // to use getopt()
#include <zlib.h> // to use compress() and uncompress()

#define VERSION "v0.5"

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
    bool r; // recurse into directories
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
int validate_file_path(const char* file_path);

void print_usage(char *errmsg); // Print program syntax, Accepts input for a custom error message

int parse_options(int argc, char *argv[], Options *opts);
int read_file_list(int argc, char *argv[], Options *opts);

int expand_wildcards_and_add(const char *pattern, Options *opts);
int add_file_to_list(Options *opts, char *file_path);
void traverse_directory(const char *dir_path, Options *opts);

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

    char file_path[1024];
    long file_size;
    uLong compressed_size;

    if (opts->l) // List contents of archive without extracting
    {
        printf("\nArchive contents:\n\n");
        while(fscanf(archive, "%1023s\n%ld\n%lu\n", file_path, &file_size, &compressed_size) == 3)
        {
            printf("%s\n", file_path);
            fseek(archive, compressed_size, SEEK_CUR);
        }
        printf("\n");
    }
    else // Extract archive contents
    {
        // While reading archive metadadta (file path, original file size, compressed size)
        while (fscanf(archive, "%1023s\n%ld\n%lu\n", file_path, &file_size, &compressed_size) == 3)
        {
            // Validate file path exists and recreate any missing subdirectorie if necessary
            if (validate_file_path(file_path) != 0)
            {
                perror("Unable to create directory structure");
                fclose(archive);
                return;
            }


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
    }
    fclose(archive);
}


void print_usage(char *errmsg)
{
    if (errmsg[0] != '\0')
    {
        printf("\n%s\n", errmsg);
    }
    printf("\n mdarc Program version: %s\n\n", VERSION);
    printf("Syntax:\n");
    printf("  mdarc (command) [-options] [password] <archive_name> <file1>, <file2>, ...\n\n");
    printf("Commands:\n");
    printf("  archive - archive specified files/folders into <archive_name>\n");
    printf("  unarchive - unarchives the specified <archive_name>\n\n");
    printf("Options for archive mode:\n");
    printf("  -a      Add files to an existing archive /TODO/\n");
    printf("  -d      Delete files from an existing archive /TODO/\n");
    printf("  -r      Recursively include files in subdirectories\n");
    printf("  -p pwd  Password protect the archive /TODO/\n\n");
    printf("Options for unarchive mode:\n");
    printf("  -l      List contents of the archive\n");
    printf("  -p pwd  Password to access the archive /TODO/\n\n");
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
    // Validate that archive name exists in arguments
    if (optind + 1 >= argc)
    {
        fprintf(stderr, "Error: Missing archive name\n");
        return 1;
    }
    opts->archive_name = strdup(argv[optind+1]); // Assign (duplicate) archive name
    if (!opts->archive_name)
    {
        perror("Error allocating memory for archive name");
        return 1;
    }

    opts->file_list = NULL; // Initialize file list
    opts->file_count = 0; // Initialize file count

    // Validate file(s) and/or patterns are provided as arguments
    if (optind + 2 >= argc && opts->archive_mode)
    {
        fprintf(stderr, "Error: No files specified to add to archive\n");
        return 1;
    }

    // Read file list
    for (int i = optind + 2; i < argc; i++)
    {
        // Treat every file input as a wildcard
        if (expand_wildcards_and_add(argv[i], opts) != 0)
        {
            return 1;
        }
    }

    if (opts->file_count == 0 && opts->archive_mode)
    {
        fprintf(stderr,"Error: no files matched input filenames or patterns. Archive not created\n");
        return 1;
    }

    return 0;
}


// Function to expand wildcard patterns
int expand_wildcards_and_add(const char *pattern, Options *opts)
{
    struct stat path_stat;
    if (stat(pattern, &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
    {
        // The pattern is a directory, traverse it
        traverse_directory(pattern, opts);
        return 0;
    }

    // Handle file patterns (if not a directory)
    glob_t results;
    int ret = glob(pattern, 0, NULL, &results);

    if (ret != 0)
    {
        if (ret == GLOB_NOMATCH)
        {
            fprintf(stderr, "No match for the pattern: %s\n", pattern);
        }
        else
        {
            perror("Error processing glob pattern");
            return 1;
        }
    }

    // Add matched files to the list
    for (size_t i = 0; i < results.gl_pathc; i ++)
    {
        if (add_file_to_list(opts, results.gl_pathv[i]) != 0)
        {
            globfree(&results); // Clean up
            return 1;
        };
    }

    globfree(&results); // Clean up
    return 0;
}


// Function to add file name with path into the linked list
int add_file_to_list(Options *opts, char *file_path)
{
    FileNode *current = opts->file_list; // Initialize current to the head of the list

    // Traverse to the end of the current existing list (if any)
    while (current && current->next != NULL)
    {
        current = current->next;
    }

    // Allocate memory for the new node
    FileNode *new_file = malloc(sizeof(FileNode));
    if (new_file == NULL)
    {
        perror("Could not allocate memory for file node");
        return 1;
    }

    new_file->file_name = strdup(file_path); // Copy full path
    if (new_file->file_name == NULL)
    {
        perror("Could not allocate memory for file name");
        free(new_file);
        return 1;
    }

    new_file->next = NULL;

    if (opts->file_list == NULL) // If first element in file list
    {
        opts->file_list = new_file;
        current = opts->file_list;  // Initialize `current` to the new head
    }
    else
    {
        current->next = new_file; // Append to the end of the list
        current = new_file; // Update the current node
    }

    opts->file_count++; // Increment file count
    return 0;
}


void traverse_directory(const char *dir_path, Options *opts)
{
    DIR *dir = opendir(dir_path);
    if (!dir)
    {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip special cases for directories "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // Construct full path
        size_t path_len = strlen(dir_path);
        size_t entry_len = strlen(entry->d_name);
        size_t full_path_len = path_len + entry_len + 2; // +1 for '/' and +1 for '\0'
        char *full_path = malloc(full_path_len);
        if (!full_path)
        {
            perror("Error allocating memory for path");
            closedir(dir);
            return;
        }

        snprintf(full_path, full_path_len, "%s%s%s", dir_path, (dir_path[path_len - 1] == '/' ? "" : "/"), entry->d_name);

        // Use stat() to determine file type
        struct stat path_stat;
        if (stat(full_path, &path_stat) == 0)
        {
            if (S_ISDIR(path_stat.st_mode)) // If a directory
            {
                if (opts->r) // Recurse only if -r specified
                {
                    traverse_directory(full_path, opts);
                }

            }
            else if (S_ISREG(path_stat.st_mode)) // If a regular file
            {
                add_file_to_list(opts, full_path);
            }
            free (full_path);
        }
        else
        {
            perror("Error retrieving file information");
            return;
        }
    }
    closedir(dir);
}


int validate_file_path(const char* file_path)
{
    // Duplicate the file path to modify it safely
    char *dir_path = strdup(file_path);
    if (!dir_path)
    {
        perror("Failed allocatingn memory for path");
        return 1;
    }

    char initial_dir[1024];
    if (!getcwd(initial_dir, sizeof(initial_dir)))
    {
        perror("Unable to get current directory");
        free(dir_path);
        return 1;
    }

    char *dir_token = strtok(dir_path, "/");
    while (dir_token != NULL)
    {
        char *next_token = strtok(NULL, "/");

        // Try to create each directory, skipping the last token which is assumed to be a filename
        if (next_token != NULL)
        {
            // Create the directory if it does not exist
            if (mkdir(dir_token, 0777) != 0 && errno != EEXIST)
            {
                perror("Unable to create director");
                free(dir_path);
                return 1;
            }
            // Change to the newly created or existing directory
            if (chdir(dir_token) != 0)
            {
                perror("Unable to change directory");
                free(dir_path);
                return 1;
            }
        }

        // get next directory token
        dir_token = next_token;
    }

    // Return to initial directory
    if (chdir(initial_dir) != 0)
        {
            perror("Unable to return to initial directory");
            free(dir_path);
            return 1;
        }

    free(dir_path);
    return 0;
}


void free_opts(Options *opts)
{
    free(opts->archive_name);
    FileNode *current = opts->file_list;
    while (current != NULL)
    {
        FileNode *next = current->next;
        free(current->file_name);
        free(current);
        current = next;
    }
    opts->file_list = NULL;
}
