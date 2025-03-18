# mdarc - command line archiving tool
#### Video demo: <URL here>
#### Description:
mdarc is a command line tool written in C for archiving and compressing a list of files into an single archive file. It can also extract the original contents from created archive.

Additionally there are several options, in the form of flags, that can be used to tweak the program behaviour or performing additional tasks. For example recurse into subdirectories, add or remove files to/form an existing archive, password protection or just list the contents of an archive without extracting it.


### Syntax

mdarc [command] (option) (option) (-p password) (options...) [archive name] [file1] [file2] ... [dir1] [dir2] ...


### Commands/Options

archive - Archive specified files into an archive named [archive name]. Default behaviour - create [archive name] (overwrite if already exists) and archive the files specified. If a directory is specified in the input arguments, its contents are also archived, without recursing into any subdirectories that might exist in it.

- -a - Add files to existing archive. If flag -a is not specified, an existing [archive name] file will be overwritten. (default behaviour) /TODO/\
- -d - Delete files from existing archive. /TODO/
- -r - Recurse into directories. If not used, any specified directories will be archived, without recursing into subdirectories.
- -p - Encrypt using a password. Password must be entered following the **-p** option. SYNTAX: [-p PASSWORD] /TODO/

unarchive - Extract the specified archive [archive name] file.

- -l - List the files in the archive without extracting its contents.
- -p - For extracting a password protected archive. **-p** and corresponding password must be used for a password protected archive, otherwise an error will be displayed. /TODO/

Examples:

* ./mdarc archive -ap pass123 archive_name.arc file1 file2
* ./mdarc archive -a -p pass123 -r archive_name.arc file1 file2 dir1
* ./mdarc archive -r archive_name.arc dir1 dir2\
* ./mdarc archive archive_name.arc *.txt ?.bmp
* ./mdarc archive archive_name.arc filename.*
* ./mdarc unarchive -l archive_name.arc


### Supported syntax

* Program supports combining flags -a -r into -ar
* Password must be included immediately following the -p flag (if used) even if additional flags are to be used after it. See example 1 and 2.
* Wildcards support: * and ?
    1. '*' - matches zero or more characters.
    2. '?' - matches exactly one character.
* Directories are recognized with or without forwardslash. Example [dir] is the same as [dir/]


### Additional information
The program is useful for packaging a set of files for archiving files; for distribution; and for saving disk space by temporarily compressing unused files or directories.

The mdarc program puts one or more compressed files itno a single archive along with information about the files (original name and path). An entire directory structure can be packaged into an archive with a single command.

The program uses the DEFLATE compression method (utilizing the zlib library) with compression ratios of 2:1 to 3:1 being common for text files.


### Operation
Upon execution the program performs a minimum command line arguments check. If passed it executes a parse options function which reads the main commnad (archive or unarchive) and through all provided options and stores them as boolean values into a struct.

Following is a read file list fuction that reads all additional command line arguments and stores the requested archive name and a linked list of all files to be archived with their respective full path and stores them in the previusly mentioned struct.

Next, depending on the main command mode - archive or unarchive - the respective functions are executed archive_files or unarchive_files.

**archive_files**
The function iterates through the file list and for every individual file calls another function add_file_to_archive that adds the respective file to the archive.

**unarchive_files**
The function performs a check for -l (list files) option and if provided reads only the file metadata from the archive and prints the filenames of the contents without extracting. Otherwise it extracts the full contents of the archive.


### Design choices
Parsing command line arguments using the getopt function: Initially I planned to use global boolean variables to store the different options, but realized this is not a good practice (globals can make the code more error-prone and using separate varaibles will make it more difficult to read and maintain. Especially if new options are to be added in the future). I ended up using a struct containing fields for different options, parsing them using getopt in main() and passing a pointer to the struct to functions that access to it. Benefits - more organized code and reduced chance of conflicing globals, passing a single argument for options to various functions, avoid having to re-parse options if the code grows in complexity, easier to add new options.

The add_file_to_list() function was previously called add_files_to_list() and accepted a liked list of filenames to be added to the opts.file_list. It was reworked to accept a single filepath/name to be added to the otps.fil_list and any loops were moved to the caller function. This made it possible to be called from different functions working on different areas of the file input list.
For example expand_wildcards_and_add() when handling wildcard patterns creates an array of matched filenames which used to be passed directly to add_file_to_list(). Now it iterates though it and passed every single entry individually.
traverse_directory() on the other hand - handles each entry individually, so the above change made it possible to call add_file_to_list() as well, without duplicating the insertion code.


### Known limitations
- File path is limited to 1023 characters
- No options error handling - for example conflicting or duplicated options
- Unarchive function overwrites existing files with the same path
- Adding duplicate filnames and/or specifying a file name and then a wildcard which includes said filename adds it multiple times to the archive.
- Symbolic links are not handled. If present in file list or recursed folders may lead to unexpected behaviour
- No handling of large files, due to reading all file contents into memory at once


### Revision History

#### v0.5

- Created a Makefile for easier compilation with make
- Cleaned up code for read_file_list() and expand_wildcards_and_add()
- Reworked add_file_to_list() to accept a single file path to be added, rather than a linked list of files and moved the list traversion to expand_wildcards_and_add()
- Added error handling for archive and file names input
- Added suport for directory and subdirectory recursion through traverse_directory() function and optional -r flag

#### v0.4

- Added support for wildcards *, ? and [...]
- Addded support for -l option - list filenames from existing archive without extracting contents.

#### v0.3

- Use getopt() to parse input options, no actual options funcionality yet.
- Separate function to read archive name and file list
- Update functions input to use the options struct.

#### v0.2

- Implement a struct to keep all command line arguments in one place - commands, options, passwords, archive name and a file list.
- Externalize parsing command line arguments into a separate function.
- Externalize printing program syntax into a separate function

#### v0.1

Program supports archiving and unarchiving. File names must be listed individuallly. No options support yet. Basic parsing of command line arguments and file list is passed to archive_files function as &argv[3] (address of first filename)
