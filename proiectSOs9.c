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
#include <libgen.h>

#define PATH_MAX_SIZE 2048 
#define BUFFER_SIZE 4096
#define MAX_HEADER 54

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

char* extract_filename(const char *path)
{
    char *filename = strdup(path); //duplicam stringul
    char *fileBasename = basename(filename);  // basename - strip directory and suffix from filenamesq
    char *result = strdup(fileBasename);
    free(filename);
    return result;
}
void convert_bmp_to_gray(const char *filename, const char *outputDir, const char *parentFolder)
 {
    int fin = open(filename, O_RDWR);
    if (fin == -1) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }
    struct stat fileInfo;
    if (stat(filename, &fileInfo) == -1) {
        perror("Error getting file information\n");
        exit(EXIT_FAILURE);
    }

    unsigned char header[54];
    //citesc headerul din bmp
    if (read(fin, header, sizeof(unsigned char) * 54) != sizeof(unsigned char) * 54) {
        perror("Error reading header");
        close(fin);
        exit(EXIT_FAILURE);
    }

    //setez pozitia curenta dupa citire
    if(lseek(fin, 18, SEEK_SET) == -1) {
        perror("Error setting cursor\n");
        close(fin);
        exit(EXIT_FAILURE);
    }
        int height = 0, width = 0;
        read(fin, &width, 4);
        read(fin, &height, 4);
        close(fin);
        char user_rights[4] = "";
        char group_rights[4] = "";
        char other_rights[4] = ""; 
        char ultima_modificare[20];
        char output[BUFFER_SIZE] ="";
        strftime(ultima_modificare, sizeof(ultima_modificare), "%d.%m.%Y", localtime(&fileInfo.st_mtime));
        drepturi_de_acces(fileInfo, user_rights, group_rights, other_rights);
        fflush(NULL);
         if(lseek(fin, 0, SEEK_SET) == -1) {
                perror("Error resetting cursor to the begining of the file\n");
                close(fin);
                exit(EXIT_FAILURE);
       }

    pid_t child = fork();

    if(child == -1) {
        perror("Error forking\n");
        exit(EXIT_FAILURE);
    }
    else if(child == 0) 
    {
        if(lseek(fin, MAX_HEADER, SEEK_SET) == -1) {
            perror("Error jumping over header\n");
            close(fin);
            exit(EXIT_FAILURE);
        }
         unsigned char colors[3];
        while (read(fin, colors, 3) == 3) {
            unsigned char grayPixel = (unsigned char)(0.299 * colors[2] + 0.587 * colors[1] + 0.114 * colors[0]);
            lseek(fin, -3, SEEK_CUR);
            write(fin, &grayPixel, 1);
            write(fin, &grayPixel, 1);
            write(fin, &grayPixel, 1);
        }

         exit(EXIT_SUCCESS);
    } else {
        int status;
        waitpid(child, &status, 0);

        if (WIFEXITED(status)) {
            printf("S-a încheiat procesul cu pid-ul %d si codul %d\n", child, WEXITSTATUS(status));
        } else {
            printf("Procesul cu pid-ul %d nu s-a încheiat normal\n", child);
        }
    }
    char *base_name=extract_filename(filename);
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
            base_name, height, width, fileInfo.st_size, fileInfo.st_uid, ultima_modificare, fileInfo.st_nlink, user_rights, group_rights, other_rights);
    
     char stat_file_name[BUFFER_SIZE] = "";
            snprintf(stat_file_name, sizeof(stat_file_name), "%s/%s_statistica.txt", outputDir, base_name);
            int file_descriptor_out = open(stat_file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (file_descriptor_out == -1) {
                perror("Error opening output file\n");
                exit(-1);
            }

            if(strcmp(output, "") != 0) {
                if (write(file_descriptor_out, output, strlen(output)) == -1) {
                    perror("Error writing to output file\n");
                   // close(fin);
                    close(file_descriptor_out);
                    exit(EXIT_FAILURE);
                }
            }
           // close(fin);
            close(file_descriptor_out);
}
void process_file(const char *filename, const char *outputDir,  const char *c)
{
    struct stat fileInfo;
    if (lstat(filename, &fileInfo) == -1) {
        perror("Error getting file information\n");
        return;
    }

    char output[PATH_MAX_SIZE]= "";

    if (S_ISREG(fileInfo.st_mode)) 
    { // Fișier obișnuit
       
        strcpy(output, "");
         char user_rights[4] = "";
         char group_rights[4] = "";
         char other_rights[4] = ""; 
         char ultima_modificare[20];
        strftime(ultima_modificare, sizeof(ultima_modificare), "%d.%m.%Y", localtime(&fileInfo.st_mtime));
        drepturi_de_acces(fileInfo, user_rights, group_rights, other_rights);
               
        char *base_name=extract_filename(filename);
        pid_t child_pid = fork();

        if (child_pid == -1) {
            perror("Error forking\n");
            exit(EXIT_FAILURE);
        } else if (child_pid == 0) {
            // Proces fiu pentru executarea scriptului și citirea rezultatului
            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("Pipe failed\n");
                exit(EXIT_FAILURE);
            }

            pid_t grandchild_pid = fork();

            if (grandchild_pid == -1) {
                perror("Error forking grandchild\n");
                exit(EXIT_FAILURE);
            } else if (grandchild_pid == 0) {
                // Proces nepot pentru executarea scriptului
                close(pipefd[0]); // Închide capătul de citire al pipe-ului
                dup2(pipefd[1], STDOUT_FILENO); // Redirecționează stdout către capătul de scriere al pipe-ului
                close(pipefd[1]); // Închide capătul de scriere al pipe-ului

                execlp("bash", "bash", "script.sh", c, NULL); // Înlocuiește cu numele și calea scriptului tău
                perror("Exec failed\n");
                exit(EXIT_FAILURE);
            } else {
                // Proces fiu pentru citirea rezultatului de la procesul nepot
                close(pipefd[1]); // Închide capătul de scriere al pipe-ului

                int count = 0;
                read(pipefd[0], &count, sizeof(count)); // Citește rezultatul de la procesul nepot

                close(pipefd[0]); // Închide capătul de citire al pipe-ului

                printf("Number of correct sentences in file %s: %d\n", base_name, count); // Poți face ce dorești cu acest rezultat
            }
            exit(EXIT_SUCCESS);
        } else {
            wait(NULL);
        }
        snprintf(output, sizeof(output), "nume fisier: %s\n"
                                     "dimensiune: %ld\n"
                                     "identificatorul utilizatorului: %d\n"
                                     "timpul ultimei modificari: %s\n"
                                     "contorul de legaturi: %ld\n"
                                     "drepturi de acces user: %s\n"
                                     "drepturi de acces grup: %s\n"
                                     "drepturi de acces altii: %s\n", 
            base_name, fileInfo.st_size, fileInfo.st_uid, ultima_modificare, fileInfo.st_nlink, user_rights, group_rights, other_rights);
            
        char stat_file_name[BUFFER_SIZE] = "";
        snprintf(stat_file_name, sizeof(stat_file_name), "%s/%s_statistica.txt", outputDir, base_name);
        int file_descriptor_out = open(stat_file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (file_descriptor_out == -1) {
                perror("Error opening output file\n");
                exit(-1);
            }
        if(strcmp(output, "") != 0) {
            if (write(file_descriptor_out, output, strlen(output)) == -1) {
                    perror("Error writing to output file\n");
                    close(file_descriptor_out);
                    exit(EXIT_FAILURE);
                }
            }

            close(file_descriptor_out);

        } else if (S_ISDIR(fileInfo.st_mode)) {
            strcpy(output, "");
            char user_rights[4] = "";
            char group_rights[4] = "";
            char other_rights[4] = ""; 
            drepturi_de_acces(fileInfo, user_rights, group_rights, other_rights);// Director
            char *base_name=extract_filename(filename);
            snprintf(output, sizeof(output), "nume director: %s\n"
                                         "identificatorul utilizatorului: %d\n"
                                         "drepturi de acces user: %s\n"
                                         "drepturi de acces grup: %s\n"
                                         "drepturi de acces altii: %s\n", 
                base_name, fileInfo.st_uid, user_rights, group_rights, other_rights);
            char stat_file_name[BUFFER_SIZE] = "";
            snprintf(stat_file_name, sizeof(stat_file_name), "%s/%s_statistica.txt", outputDir, base_name);
            int file_descriptor_out = open(stat_file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (file_descriptor_out == -1) {
                perror("Error opening output file\n");
                exit(-1);
            }

            if(strcmp(output, "") != 0) {
                if (write(file_descriptor_out, output, strlen(output)) == -1) {
                    perror("Error writing to output file\n");
                    close(file_descriptor_out);
                    exit(EXIT_FAILURE);
                }
            }

             close(file_descriptor_out);

        } else if (S_ISLNK(fileInfo.st_mode)) {
         // Legatură simbolica
        
            strcpy(output, "");
            char symbolic_name[256];
            ssize_t len = readlink(filename, symbolic_name, sizeof(symbolic_name) - 1);
            if (len != -1) 
            {
                symbolic_name[len] = '\0';
                char user_rights[4] = "", group_rights[4] = "", other_rights[4] = ""; 
                struct stat symbolic_info;
                if (stat(filename, &symbolic_info) == -1) {
                    perror("Error getting target file information\n");
                    return;
                }

                drepturi_de_acces(symbolic_info, user_rights, group_rights, other_rights);
                char *base_name=extract_filename(filename);
                snprintf(output, sizeof(output), "nume legatura: %s\n"
                                             "dimensiune: %ld\n"
                                             "dimensiune fisier: %ld\n"
                                             "drepturi de acces user: %s\n"
                                             "drepturi de acces grup: %s\n"
                                             "drepturi de acces altii: %s\n",
                    base_name, fileInfo.st_size, symbolic_info.st_size, user_rights, group_rights, other_rights);
                char stat_file_name[BUFFER_SIZE] = "";
                snprintf(stat_file_name, sizeof(stat_file_name), "%s/%s_statistica.txt", outputDir, base_name);
                int file_descriptor_out = open(stat_file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (file_descriptor_out == -1) {
                    perror("Error opening output file\n");
                    exit(-1);
                 }

                if(strcmp(output, "") != 0) {
                    if (write(file_descriptor_out, output, strlen(output)) == -1) {
                        perror("Error writing to output file\n");
                        close(file_descriptor_out);
                        exit(EXIT_FAILURE);
                }
            }

           close(file_descriptor_out);
        
        }

}
}
void processEntry(const char *entryName, const char *inputDirectory, const char *outputDirectory, const char *c){
    char inputFilePath[PATH_MAX_SIZE];
    snprintf(inputFilePath, sizeof(inputFilePath), "%s/%s", inputDirectory, entryName);

    pid_t childPid = fork();

    if (childPid == -1) 
    {
        perror("Error forking\n");
        exit(EXIT_FAILURE);
    }
     else if (childPid == 0) 
     {
        if (strstr(entryName, ".bmp") != NULL)
         {
            convert_bmp_to_gray(inputFilePath, outputDirectory, inputDirectory);
            exit(EXIT_SUCCESS);
         } else {
            process_file(inputFilePath, outputDirectory, c);
            exit(EXIT_SUCCESS);
        }
     } else {
        int status;
        waitpid(childPid, &status, 0);

        if (WIFEXITED(status)) {
            printf("S-a încheiat procesul cu pid-ul %d si codul %d\n", childPid, WEXITSTATUS(status));
        } else {
            printf("Procesul cu pid-ul %d nu s-a încheiat normal\n", childPid);
        }
    }
}



void processDirectory(const char *inputDirectory, const char *outputDirectory, const char *c) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(inputDirectory);
    if (dir == NULL) {
        perror("Error opening input directory\n");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            processEntry(entry->d_name, inputDirectory, outputDirectory, c);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Mod de utilizare: %s <director_intrare> <director_iesire> <c>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *inputDirectory = argv[1];
    const char *outputDirectory = argv[2];
    char character = argv[3][0]; // Caracterul <c>  primul caracter din al treilea argument
    processDirectory(inputDirectory, outputDirectory, &character);

    return 0;
}