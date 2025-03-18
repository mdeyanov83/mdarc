# mdarc - package and compress (archive) files
#### Video demo: <URL here>
#### Description:
mdarc is a command line tool written in C for archiving and compressing a list of files into an single archive file. It can also unarchive the created file into its original contents.

Additionally there are several options that can be used like using a password to encrypt the created archive, recurse into directories, add or remove files from an existing archive as well as list the archive contents without extracting the archived files files.

### Syntax

mdarc [command] (options) _(password)_ [archive name] [file1] [file2] ... [dir1] [dir2] ...

### Commands/Options

**archive** - Archive specified files into an archive named <archive name>

- -a - Add files to existing archive. If the newlly added filename already exists in the archive, it will be overwritten. /TODO/\
Note - Unless the **-a** option is specified and the archve name already exists, the existing archive file will be overwritten.
- -d - Delete files from existing archive. /TODO/
- -r - Recurse into directories. /TODO and also figure out logic, wildcards etc./
- -p - Encrypt using a password, which must be provided immediately after the options, before the archvie name. /TODO/

**unarchive** - Unarchive the specified archived <archive name> file.

- -l - List the files in the archive without extracting its contents. /TOOD/
- -p - For extracting a password protected archive. **-p** and corresponding password must be used for a password protected archive, otherwise an error will be displayed. /TODO/

Options can be combined as follows:\
mdarc archive **-ap** pass123 archive_name.arc file1 file2

The above syntax combines options **-a** and **-p** and will create an archive named **archive_name.arc** containing **file1** and **file2** encrypted with password **pass123**.

### Additional information
The program is useful for packaging a set of files for archiving files; for distribution; and for saving disk space by temporarily compressing unused files or directories.

The mdarc program puts one or more compressed files itno a single archive along with information about the files (original name and path). An entire directory structure can be packaged into an archive with a single command.

The program uses the DEFLATE compression method (utilizing the zlib library) with compression ratios of 2:1 to 3:1 being common for text files.

### Program operation
/TODO/

### Design choices
Parsing command line arguments using the getopt function: Initially I planned to use global boolean variables to store the different options, but realized this is not a good practice (globals can make the code more error-prone and using separate varaibles will make it more difficult to read and maintain. Especially if new options are to be added in the future). I ended up using a struct containing fields for different options, parsing them using getopt in main() and passing a pointer to the struct to functions that access to it. Benefits - more organized code and reduced chance of conflicing globals, passing a single argument for options to various functions, avoid having to re-parse options if the code grows in complexity, easier to add new options.




### Examples
/TODO/

### Pattern matching (wildcards)
? - match any single character
* - match any number of characters (including none)


### Known limitations
- file path is limited to 256 characters

### Revision History

#### v0.1

Program supports archiving and unarchiving. File names must be listed individuallly. No options support yet. Basic parsing of command line arguments and file list is passed to archive_files function as &argv[3] (address of first filename)

#### v0.2

- Implement a struct to keep all command line arguments in one place - commands, options, passwords, archive name and a file list.
- Externalize parsing command line arguments into a separate function.
- Externalize printing program syntax into a separate function

#### v0.3
- Update functions input to use the options struct.
No actual options support yet, getopt() to be implemented in next version.


