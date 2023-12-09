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
#include <dirent.h>

#define PATH_MAX_SIZE 2048 

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
    int fd = open(filename, flags);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    return fd;
}


void process_file(const char *filename, int fout, int fin) {
    struct stat fileInfo;
    if (lstat(filename, &fileInfo) == -1) {
        perror("Error getting file information\n");
        return;
    }

    char output[PATH_MAX_SIZE]= "";

    //int file_descriptor = 0;
    if (S_ISREG(fileInfo.st_mode)) { // Fișier obișnuit
       if (strstr(filename, ".bmp"))
             {
                 int file_descriptor = open(filename, O_RDONLY);
                 if (file_descriptor == -1) {
                    perror("Error opening input file\n");
                    exit(EXIT_FAILURE);
                }

                if (lseek(file_descriptor, 18, SEEK_SET) == -1) {
                      perror("Error setting cursor\n");
                     close(file_descriptor);
                    exit(EXIT_FAILURE);
                }

            int height = 0, width = 0;
            read(file_descriptor, &width, 4);
            read(file_descriptor, &height, 4);
            close(file_descriptor);
            char user_rights[4] = "";
            char group_rights[4] = "";
            char other_rights[4] = ""; 
            char ultima_modificare[20];
            strftime(ultima_modificare, sizeof(ultima_modificare), "%d.%m.%Y", localtime(&fileInfo.st_mtime));
            drepturi_de_acces(fileInfo, user_rights, group_rights, other_rights);
            fflush(NULL);
          
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
            filename, height, width, fileInfo.st_size, fileInfo.st_uid, ultima_modificare, fileInfo.st_nlink, user_rights, group_rights, other_rights);
          
        } else {
                char user_rights[4] = "";
                char group_rights[4] = "";
                char other_rights[4] = ""; 
                char ultima_modificare[20];
                strftime(ultima_modificare, sizeof(ultima_modificare), "%d.%m.%Y", localtime(&fileInfo.st_mtime));
                drepturi_de_acces(fileInfo, user_rights, group_rights, other_rights);
            //snprintf(output, sizeof(output), "nume fisier: %s\n", filename);
            snprintf(output, sizeof(output), "nume fisier: %s\n"
                                     "dimensiune: %ld\n"
                                     "identificatorul utilizatorului: %d\n"
                                     "timpul ultimei modificari: %s\n"
                                     "contorul de legaturi: %ld\n"
                                     "drepturi de acces user: %s\n"
                                     "drepturi de acces grup: %s\n"
                                     "drepturi de acces altii: %s\n", 
            filename, fileInfo.st_size, fileInfo.st_uid, ultima_modificare, fileInfo.st_nlink, user_rights, group_rights, other_rights);
        }
    } else if (S_ISDIR(fileInfo.st_mode)) {
        char user_rights[4] = "";
        char group_rights[4] = "";
        char other_rights[4] = ""; 
        drepturi_de_acces(fileInfo, user_rights, group_rights, other_rights);// Director
        snprintf(output, sizeof(output), "nume director: %s\n"
                                         "identificatorul utilizatorului: %d\n"
                                         "drepturi de acces user: %s\n"
                                         "drepturi de acces grup: %s\n"
                                         "drepturi de acces altii: %s\n", 
                filename, fileInfo.st_uid, user_rights, group_rights, other_rights);
    } else if (S_ISLNK(fileInfo.st_mode)) { // Legatură simbolica
        char symbolic_name[256];
        ssize_t len = readlink(filename, symbolic_name, sizeof(symbolic_name) - 1);
        if (len != -1) {
            symbolic_name[len] = '\0';
            char user_rights[4] = "", group_rights[4] = "", other_rights[4] = ""; 
            struct stat symbolic_info;
            if (lstat(symbolic_name, &symbolic_info) == -1) {
                perror("Error getting target file information\n");
                return;
            }
            drepturi_de_acces(symbolic_info, user_rights, group_rights, other_rights);
            snprintf(output, sizeof(output), "nume legatura: %s\n"
                                             "dimensiune: %ld\n"
                                             "dimensiune fisier: %ld\n"
                                             "drepturi de acces user: %s\n"
                                             "drepturi de acces grup: %s\n"
                                             "drepturi de acces altii: %s\n",
                    filename, fileInfo.st_size, symbolic_info.st_size, user_rights, group_rights, other_rights);
        }
    }

    if (strlen(output) > 0) {
        if (write(fout, output, strlen(output)) == -1) {
            perror("Error writing to output file");
            return;
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Usage ./program <director_intrare>\n");
        exit(EXIT_FAILURE);
    }

    char *dir_path = argv[1];

    int fin = openFile(dir_path, O_RDONLY);
    int fout = openFile("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC);
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(dir_path)) != NULL) 
    {
        while ((entry = readdir(dir)) != NULL) 
        {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
            {
                char filepath[PATH_MAX_SIZE];
                size_t dir_len = strlen(dir_path);
                size_t entry_len = strlen(entry->d_name);

                if (dir_len + entry_len + 2 <= sizeof(filepath)) { // Adăugăm 2 pentru separatorul '/' și terminatorul nul
                    snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, entry->d_name);
                } else {
                    fprintf(stderr, "Path too long: %s/%s\n", dir_path, entry->d_name);
                    continue;
                }

                process_file(filepath, fout, fin);
            }
        }
        closedir(dir);
    } else {
        perror("Error opening directory\n");
        exit(EXIT_FAILURE);
    }
    
    close(fin);
    close(fout);
    
    return 0;
}
