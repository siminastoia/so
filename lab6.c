#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<ctype.h>
#include<string.h>

int main(int argc, char*argv[])
{
  if(argc != 4)
    {
      fprintf(stderr, "Argumente insuficiente, use /program <fisier_intrare> <fisier_statistica> <ch> %s ", argv[0]);
      exit(1);
    }
  char *inputFile=argv[1];
  char *outputFile=argv[2];
  char character =argv[3][0];

  int inputFd = open(inputFile, O_RDONLY);
  if(inputFd == -1)
    {
      perror("Eroare la deschiderea fisierului in");
      return 1;
    }
  int outputFd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP| S_IROTH);
    if(outputFd == -1)
      {
	perror("Eroare la deschiderea fisierului out ");
	close(inputFd);
        return 1;
      }
  struct stat file_stat;
  if(fstat(inputFd, &file_stat) == -1)
    {
      perror("Eroare obtinere informatii despre fisier");
      close(inputFd);
      exit(1);
    }
  int lowercaseCount = 0;
  int uppercaseCount = 0;
  int digitCount = 0;
  int charCount = 0;
  char buffer[1];

  while(read(inputFd, buffer, 1)>0)
    {
      if(islower(buffer[0]))
	{
	  lowercaseCount++;
	}
      else if(isupper(buffer[0]))
	{
	  uppercaseCount++;
	}
      else if(isdigit(buffer[0]))
	{
	  digitCount++;
	}
      if(buffer[0] == character)
	{
	  charCount++;
	}
    }
  char output[256];
  sprintf(output, "numar litere mici : %d \n numar litere mari : %d \n numar cifre : %d \n numar caracterde din fisier: %d \n", lowercaseCount, uppercaseCount, digitCount, charCount);
  if(write(outputFd, output, strlen(output))== -1)
    {
      perror("Eroare la scrierea fisierului de output");  
    }
  close(inputFd);
  close(outputFd);
      
  
  return 0;
  
}
