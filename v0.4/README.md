# mdarc - package and compress (archive) files
#### Video demo: <URL here>
#### Description:
mdarc is a command line tool written in C for archiving and compressing a list of files into an single archive file. It can also unarchive the created file into its original contents.

Additionally there are several options that can be used like using a password to encrypt the created archive, recurse into directories, add or remove files from an existing archive as well as list the archive contents without extracting the archived files files.

### Syntax

mdarc [command] (option) (option) (-p password) (options...) [archive name] [file1] [file2] ... [dir1] [dir2] ...

### Commands/Options

**archive** - Archive specified files into an archive named <archive name>

- -a - Add files to existing archive. If the newlly added filename already exists in the archive, it will be overwritten. /TODO/\
Note - Unless the **-a** option is specified and the archve name already exists, the existing archive file will be overwritten.
- -d - Delete files from existing archive. /TODO/
- -r - Recurse into directories. /TODO and also figure out logic, wildcards etc./
- -p - Encrypt using a password. Password must be entered following the **-p** option. SYNTAX: [-p PASSWORD] /TODO/

**unarchive** - Unarchive the specified archived <archive name> file.

- -l - List the files in the archive without extracting its contents.
- -p - For extracting a password protected archive. **-p** and corresponding password must be used for a password protected archive, otherwise an error will be displayed. /TODO/

Options can be combined as follows:\
mdarc archive **-ap** pass123 archive_name.arc file1 file2
mdarc archive **-ap** pass123 **-r** archive_name.arc file1 file2 dir1

The above syntax combines options **-a** and **-p** and will create an archive named **archive_name.arc** containing **file1** and **file2** encrypted with password **pass123**. Adding an additional option **-r** after the password handles recursively any directories in the input.

### Examples
/TODO/

### Additional information
The program is useful for packaging a set of files for archiving files; for distribution; and for saving disk space by temporarily compressing unused files or directories.

The mdarc program puts one or more compressed files itno a single archive along with information about the files (original name and path). An entire directory structure can be packaged into an archive with a single command.

The program uses the DEFLATE compression method (utilizing the zlib library) with compression ratios of 2:1 to 3:1 being common for text files.

### Program operation
Upon execution the program performs a minimum command line arguments check. If passed it executes a parse options function which reads the main commnad (archive or unarchive) and through all provided options and stores them as boolean values into a struct.

Following is a read file list fuction that reads all additional command line arguments and stores the requested archive name and a linked list of all files to be archived with their respective full path and stores them in the previusly mentioned struct.

Next, depending on the main command mode - archive or unarchive - the respective functions are executed archive_files or unarchive_files.

**archive_files**
The function iterates through the file list and for every individual file calls another function add_file_to_archive that adds the respective file to the archive.

**unarchive_files**
The function performs a check for -l (list files) option and if provided reads only the file metadata from the archive and prints the filenames of the contents without extracting. Otherwise it extracts the full contents of the archive.


### Design choices
Parsing command line arguments using the getopt function: Initially I planned to use global boolean variables to store the different options, but realized this is not a good practice (globals can make the code more error-prone and using separate varaibles will make it more difficult to read and maintain. Especially if new options are to be added in the future). I ended up using a struct containing fields for different options, parsing them using getopt in main() and passing a pointer to the struct to functions that access to it. Benefits - more organized code and reduced chance of conflicing globals, passing a single argument for options to various functions, avoid having to re-parse options if the code grows in complexity, easier to add new options.



### Pattern matching (wildcards)
* - matches zero or more characters.
? - matches exactly one character.
[...] - matches any character inside the brackets.


### Known limitations
- file path is limited to 256 characters
- no filenames (archive and input files) error handling - segmentation fault if missing
- no options error handling - for example conflicting or duplicated options
- unarchive function overwrites existing files with the same path


### Revision History

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






