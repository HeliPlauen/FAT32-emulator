FAT32 emulator program that manages FAT32 filesystem backed up by a file.

It was created using Chat Gpt.

1. This application runs on Windows only!!! The Visual Studio 2019 was used to create it.

2. The emulator program works with a regular file that backs up FAT32 filesystem.

3. Path to the file is provided to the emulator as a command line parameter, but also can work without it 

4. In case if there is no command line parameter - it creates a  FAT32 image of the USB-flash  (disk F in my case!! but it may be changed)

5. The FAT32 image size in p.4 is 20Mb

6. In case if file is not a FAT32 image - it automaticaly formats into FAT32.

7. The emulator provides CLI (command line interface). The commands operate on FAT32 filesystem.

8. Emplemented commands are: "ls", and "format" (format is currently not callable from CLI and rruns only if p.7)

9. cd, mkdir and touch - coming soon.