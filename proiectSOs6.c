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

void drepturi_de_acces(struct stat fileInf, char* user, char* group, char* other)
{
    strcat(user, (fileInf.st_mode & S_IRUSR) ? "R" : "-");
    strcat(user, (fileInf.st_mode & S_IWUSR) ? "W" : "-");
    strcat(user, (fileInf.st_mode & S_IXUSR) ? "X" : "-");

    strcat(group, (fileInf.st_mode & S_IRGRP) ? "R" : "-");
    strcat(group, (fileInf.st_mode & S_IWGRP) ? "W" : "-");
    strcat(group, (fileInf.st_mode & S_IXGRP) ? "X" : "-");

    strcat(other, (fileInf.st_mode & S_IROTH) ? "R" : "-");
    strcat(other, (fileInf.st_mode & S_IWOTH) ? "W" : "-");
    strcat(other, (fileInf.st_mode & S_IXOTH) ? "X" : "-");
}

int openFile(const char *filename, int flags) {
    int f = open(filename, flags);
    if (f == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    return f;
}

int main(int argc, char *argv[]) {
    if (argc != 2 || strstr(argv[1], ".bmp") == NULL) {
        perror("Usage ./program <fisier_intrare>.bmp\n");
        exit(EXIT_FAILURE);
    }

    char *fisier_in = argv[1];

    int fin = openFile(fisier_in, O_RDWR); 
    int fout = openFile("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC);
    struct stat fileInf;
    if (fstat(fin, &fileInf) == -1) {
        perror("Error getting file information\n");
        close(fin);
        close(fout);
        exit(EXIT_FAILURE);
    }

    if (lseek(fin, 18, SEEK_SET) == -1) {
        perror("Error setting cursor\n");
        close(fin);
        close(fout);
        exit(EXIT_FAILURE);
    }
 int height = 0, width = 0;
    read(fin, &width, 4);
    read(fin, &height, 4);
    //printf("%d %d\n", height, width);
    char user_rights[4] = "";
    char group_rights[4] = "";
    char other_rights[4] = ""; 
    char ultima_modificare[20];
    strftime(ultima_modificare, sizeof(ultima_modificare), "%d.%m.%Y", localtime(&fileInf.st_mtime));
    drepturi_de_acces(fileInf, user_rights, group_rights, other_rights);
    fflush(NULL);
    char output[1024];
    snprintf(output, sizeof(output), "nume fisier: %s\n"
                                     "inaltime: %d\n"
                                     "lungime: %d\n"
                                     "dimensiune: %ld\n"
                                     "identificatorul utilizatorului: %d\n"
                                     "timpul ultimei modificari: %s\n"
                                     "contorul de legaturi: %ld\n"
                                     "drepturi de acces user: %s\n"
                                     "drepturi de acces grup: %s\n"
                                     "drepturi de acces altii: %s\n", 
            fisier_in, height, width, fileInf.st_size, fileInf.st_uid, ultima_modificare, fileInf.st_nlink, user_rights, group_rights, other_rights);
    

    if (write(fout, output, strlen(output)) == -1)
    {
        perror("Error writing to output file");
        close(fin);
        close(fout);
        exit(EXIT_FAILURE);
    }

    close(fin);
    close(fout);

    return 0;
}