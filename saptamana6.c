#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

void  get_access_rights(struct stat fileInfo, char* user, char* group, char* other)
{
    strcat(user, (fileInfo.st_mode & S_IRUSR) ? "R" : "-");
    strcat(user, (fileInfo.st_mode & S_IWUSR) ? "W" : "-");
    strcat(user, (fileInfo.st_mode & S_IXUSR) ? "X" : "-");

    strcat(group, (fileInfo.st_mode & S_IRGRP) ? "R" : "-");
    strcat(group, (fileInfo.st_mode & S_IWGRP) ? "W" : "-");
    strcat(group, (fileInfo.st_mode & S_IXGRP) ? "X" : "-");

    strcat(other, (fileInfo.st_mode & S_IROTH) ? "R" : "-");
    strcat(other, (fileInfo.st_mode & S_IWOTH) ? "W" : "-");
    strcat(other, (fileInfo.st_mode & S_IXOTH) ? "X" : "-");
}

int main(int argc, char *argv[]) 
{
    if (argc != 2 || strstr(argv[1], ".bmp") == NULL) 
    {
        perror("Usage ./program <input_file>\n");
        exit(-1);
    }

    char *inputFile = argv[1];

    int inputFd = open(inputFile, O_RDONLY);
    if (inputFd == -1) 
    {
        perror("Error opening input file\n");
        exit(-1);
    }

    int outputFd = open("statistics.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (outputFd == -1) 
    {
        perror("Error opening output file\n");
        close(inputFd);
        exit(-1);
    }

    struct stat fileData;
    if (fstat(inputFd, &fileData) == -1) 
    {
        perror("Error getting file information\n");
        close(inputFd);
        close(outputFd);
        exit(-1);
    }

    if(lseek(inputFd, 18, SEEK_SET) == -1)
    {
        perror("Error setting cursor\n");
        close(inputFd);
        close(outputFd);
        exit(-1);
    }

    int imgHeight = 0, imgWidth = 0;
    read(inputFd, &imgWidth, 4);
    read(inputFd, &imgHeight, 4);
    char userPerms[4] = "", groupPerms[4] = "", otherPerms[4] = ""; 
    char modifiedDate[20];
    strftime(modifiedDate, sizeof(modifiedDate), "%d.%m.%Y", localtime(&fileData.st_mtime));
     get_access_rights(fileData, userPerms, groupPerms, otherPerms);

    fflush(NULL);

    char outputData[1024];
    snprintf(outputData, sizeof(outputData),  "nume fisier: %s\n"
                                     "inaltime: %d\n"
                                     "lungime: %d\n"
                                     "dimensiune: %ld\n"
                                     "identificatorul utilizatorului: %d\n"
                                     "timpul ultimei modificari: %s\n"
                                     "contorul de legaturi: %ld\n"
                                     "drepturi de acces user: %s\n"
                                     "drepturi de acces grup: %s\n"
                                     "drepturi de acces altii: %s\n", 
            inputFile, imgHeight, imgWidth, fileData.st_size, fileData.st_uid, modifiedDate, fileData.st_nlink, userPerms, groupPerms, otherPerms);

    if (write(outputFd, outputData, strlen(outputData)) == -1)
    {
        perror("Error writing");
        close(inputFd);
        close(outputFd);
        exit(-1);
    }

    close(inputFd);
    close(outputFd);

    return 0;
}
