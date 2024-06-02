FAT32 emulator program that manages FAT32 filesystem backed up by a file.

It was created using Chat Gpt.

1. This application runs on Windows only!!! The Visual Studio 2019 was used to create it, but it also may run using concsole.

2. The emulator program works with a regular file that backs up FAT32 filesystem.

3. Path to the file is provided to the emulator as a command line parameter, but also can work without it 

4. In case if there is no command line argument - it creates a  FAT32 image of the USB-flash  (disk F in my case!! but it may be changed)
In this case the application creates a FAT32 image which has a size 20Mb.

5. The emulator provides CLI (command line interface) with commands: cd, format, ls, mkdir, touch. 

6. CLI shows a prompt "/path>" where "path" is the current directory.

7. Command "format" creates FAT32 filesystem inside the file. All information is lost.

8. If the backed file does not contain a valid FAT32 filesystem then all commands (except for "format") output an error message.



The program has limitations:

0. NO WHITESPACES in DIRECTORY names!!!

1. CD .. returns to the root dirrectory

2. MKDIR and TOUCH create DIRS and FILES in ROOT ONLY!!! (even if start from any subdirectory!)

3. Filenames and dirnames may be not unique

4. The "ls" command  works only with a current dirrectory and does not show hidden liles and dirrectories.

5. The command "format" may work executedly!



How to compile and run:

clang FAT32SumEmul.c

./a filename.img
