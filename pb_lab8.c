#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>
void childProcess(char *grupa)
{
  execl("./studenti", "studenti", grupa, NULL);
}
void parentProcess(pid_t pid)
{
  char c;
  int status;
  //usleep(500000);
 
  while(1)
    {
      usleep(500000);
      if(wait(&status)> 0)
	{
	  printf("procesul copil s-a incheiat. \n ");
	  break;
	}
      if(scanf("%c", &c)==1)
	{
	  printf("Litera citita : %c", c);
	}
    }
}
int main()
{
  pid_t pid;
  char c;
  printf("introduceti grupa:");
  scanf("%c", &c);  
  pid=fork();
  if(pid<0)
    {
     perror("Eroare");
     return 1;
    }
  else if(pid==0)
    {
      childProcess(c);
    }
  else{
    parentProcess(pid);
  }
  return 0;

}
