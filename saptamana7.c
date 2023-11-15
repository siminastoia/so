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

#define MAX_BUFFER_SIZE 4096
#define MAX_PATH_LENGTH 1024

void get_access_rights(struct stat fileInfo, char* user, char* group, char* other)
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

void explore_directory(char* path, char* output_file)
{
    DIR* dir = opendir(path);
    if(!dir)
    {
        perror("Unable to open directory %s\n");
        exit(-1);
    }

    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output_fd == -1) 
    {
        perror("Error opening output file\n");
        exit(-1);
    }

    struct dirent* entry = NULL;
    struct stat info;
    int input_fd = 0;
    char buffer[MAX_BUFFER_SIZE] = "";
    char entry_path[MAX_PATH_LENGTH] = "";

    while((entry = readdir(dir)))
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        strcpy(buffer, "");
        snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);
        if (lstat(entry_path, &info) == -1) 
        {
            printf("Entry named: %s\n", entry->d_name);
            perror("Error getting file information\n");
            exit(-1);
        }

        if(S_ISREG(info.st_mode))
        {
            if(strstr(entry->d_name, ".bmp") != NULL)
            {
                input_fd = open(entry_path, O_RDONLY);
                if (input_fd == -1) 
                {
                    perror("Error opening input file\n");
                    exit(-1);
                }

                if(lseek(input_fd, 18, SEEK_SET) == -1)
                {
                    perror("Error setting cursor\n");
                    close(input_fd);
                    exit(-1);
                }

                int height = 0, width = 0;
                read(input_fd, &width, 4);
                read(input_fd, &height, 4);
                char user_perms[4] = "", group_perms[4] = "", other_perms[4] = ""; 
                char date[20];
                strftime(date, sizeof(date), "%d.%m.%Y", localtime(&info.st_mtime));
                get_access_rights(info, user_perms, group_perms, other_perms);

                fflush(NULL);
                snprintf(buffer, sizeof(buffer), "nume fisier: %s\n"
                                     "dimensiune: %ld\n"
                                     "identificatorul utilizatorului: %d\n"
                                     "timpul ultimei modificari: %s\n"
                                     "contorul de legaturi: %ld\n"
                                     "drepturi de acces user: %s\n"
                                     "drepturi de acces grup: %s\n"
                                     "drepturi de acces altii: %s\n\n", 
                 entry->d_name, info.st_size, info.st_uid, date, info.st_nlink, user_perms, group_perms, other_perms);

                close(input_fd);
            }
            else
            {
                char user_perms[4] = "", group_perms[4] = "", other_perms[4] = ""; 
                char date[20];
                strftime(date, sizeof(date), "%d.%m.%Y", localtime(&info.st_mtime));
                get_access_rights(info, user_perms, group_perms, other_perms);
                snprintf(buffer, sizeof(buffer), "file name: %s\n"
                                     "size: %ld\n"
                                     "user ID: %d\n"
                                     "last modified time: %s\n"
                                     "link count: %ld\n"
                                     "user access rights: %s\n"
                                     "group access rights: %s\n"
                                     "other access rights: %s\n\n", 
                entry->d_name, info.st_size, info.st_uid, date, info.st_nlink, user_perms, group_perms, other_perms);
            }
        }
        else if(S_ISLNK(info.st_mode))
        {
            char user_perms[4] = "", group_perms[4] = "", other_perms[4] = ""; 
            get_access_rights(info, user_perms, group_perms, other_perms);
            struct stat target_info;
            if(stat(entry_path, &target_info) == -1)
            {
                perror("Error getting target file information for symbolic link\n");
                exit(-1);
            }
            snprintf(buffer, sizeof(buffer), "nume legatura: %s\n"
                                            "dimensiune legatura: %ld\n"
                                            "dimensiune fisier: %ld\n"
                                            "drepturi de acces user: %s\n"
                                            "drepturi de acces grup: %s\n"
                                            "drepturi de acces altii: %s\n\n",
             entry->d_name, info.st_size, target_info.st_size, user_perms, group_perms, other_perms);
        }
        else if(S_ISDIR(info.st_mode))
        {
            char user_perms[4] = "", group_perms[4] = "", other_perms[4] = ""; 
            get_access_rights(info, user_perms, group_perms, other_perms);
            snprintf(buffer, sizeof(buffer), "nume director: %s\n"
                                            "identificatorul utilizatorului: %d\n"
                                            "drepturi de acces user: %s\n"
                                            "drepturi de acces grup: %s\n"
                                            "drepturi de acces altii: %s\n\n",
                    entry->d_name, info.st_uid, user_perms, group_perms, other_perms);
        }

        if(strcmp(buffer, "") != 0)
        {
            if (write(output_fd, buffer, strlen(buffer))== -1)
            {
                perror("Error writing to output file");
                close(output_fd);
                exit(-1);
            }
        }
    }
    
    close(output_fd);
    closedir(dir);
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {   
        perror("Usage ./program <input_directory>\n");
        exit(-1);
    }

    explore_directory(argv[1], "statistica.txt");

    return 0;
}