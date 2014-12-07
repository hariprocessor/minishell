#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFSIZE 2097152
#define MAX_PATH 4096
#define FMASK 0644
#define ARGSIZE 10

int count(char *ptr, char c);
void reverse(char *b, char *save);
void pipe_exec(int cnt, char * save_buffer, char * buffer);
void redirect(char * buffer, char * newfile,int red);

int redirection(char *buffer){
  if(strstr(buffer, ">") != NULL && strstr(buffer, ">>") == NULL && strstr(buffer, ">!") == NULL)
    return 1;
  else if(strstr(buffer, ">>") != NULL)
    return 2;
  else if(strstr(buffer, ">!") != NULL)
    return 3;
  else if(strstr(buffer, "<") != NULL)
    return 4;
  else
    return 0;
}

int main(){
  int cnt;
  char *arg[ARGSIZE];
  char buffer[BUFSIZE];
  char save_buffer[BUFSIZE];
  int amper = 0;
  int red;
  char newfile[MAX_PATH];
  int fd = -1;
  int **fildes;
  char *ptr;
  char *command;
  char pipe_c;
  int i, j;
  int status;
  char dir[BUFSIZE];
  int semicnt;
  int semi;
  int issemi;
  char cmd[BUFSIZE];
  /*
  getcwd(dir, BUFSIZE);
  printf("%s >> ", dir);
  */
  while(read(0, buffer,BUFSIZE)){
    
    strcpy(save_buffer, buffer);

    issemi = 0;
    semicnt = 1;

    if(strstr(buffer, ";")){
      issemi = 1;
      ptr = strtok(buffer, ";");
      i = 0;
      while(ptr != NULL){
	printf("buffer : %s\n", ptr);
	ptr = strtok(NULL, ";");
	i++;
      }
      semicnt = i-1;
      strcpy(buffer, save_buffer);
      
    }

      if(issemi == 1){
	ptr = strtok(buffer, ";");
	for(j = 0; j < semi; j++){
	  printf("j : %d\n", j);
	  ptr = strtok(NULL, ";");
	}
	strcpy(save_buffer, buffer);
	printf("semi : %d\n", semi);
	printf("save_buffer %s\n", save_buffer);
	printf("semi buffer %s\n", buffer);
      }

    /* background */
    if(strstr(buffer, "&") != NULL)
      amper = 1;
    else
      amper = 0;
   
    if(fork() == 0){
      /* cd */

      ptr = strtok(buffer, " \n");
      strcpy(buffer, save_buffer);
      if(strcmp(ptr, "cd") == 0){
	ptr = strtok(buffer, " \n");
	ptr = strtok(NULL, " \n");
	getcwd(dir, BUFSIZE);
	if(ptr[0] == '/')
	  sprintf(dir, "%s", ptr);
	else
	  sprintf(dir, "%s/%s", dir, ptr);
	chdir(dir);
	strcpy(buffer, save_buffer);

      }
      /* redirection of IO */
      
      else if((red = redirection(buffer)) != 0){
	printf("********redirection\n red : %d\n", red);
	ptr = strtok(buffer, " ><!\n&");
	i = 0;
	while(ptr != NULL){
	  i++;
	  ptr = strtok(NULL, " ><!\n&");
	}
	strcpy(buffer, save_buffer);
	ptr = strtok(buffer, " ><!\n&");
	for(j = 0; j < i-1; j++)
	  ptr = strtok(NULL, " ><!\n&");
	sprintf(newfile, "%s%s", "./", ptr);
	strcpy(buffer, save_buffer);
	printf("newfile : %s\n", newfile);
	redirect(buffer, newfile,red);
      }
      /* piping */
      else if(strstr(buffer, "|")){
	printf("********piping\n");
	cnt = count(save_buffer, '|');
	pipe_exec(cnt, save_buffer, buffer);
      }

      else{
	printf("********common\n");
	if(fork() == 0){
	  strcpy(buffer, save_buffer);
	  ptr = strtok(buffer, " \n");
	  j = 0;
	  while(ptr != NULL){
	    arg[j] = ptr;
	    j++;


	    ptr = strtok(NULL, " \n");
	  }
	  arg[j] = NULL;
	  execvp(arg[0], arg);
	}
      }
      exit(0);
    }
    if(amper == 0)
      wait(NULL);
    fflush(stdin);
    for(j = 0; j < BUFSIZE; j++)
      buffer[j] = '\0';
    //
  }
}

void pipe_exec(int cnt, char * save_buffer, char *buffer){
  int fildes[cnt][2];
  char *ptr;
  char *arg[ARGSIZE];
  int i;
  int j;

  for(i = 0; i < cnt; i++)
    pipe(fildes[i]);

  for(i = 0; i < cnt+1; i++){
    ptr = strtok(buffer, "|&\n");
    for(j = 0; j < cnt-i; j++){
      ptr = strtok(NULL, "|&\n");
    }

    if(fork() == 0){
      if(i == cnt){
  	close(1);
  	dup(fildes[i-1][1]);
      }
      else if(i == 0){
  	close(0);
  	dup(fildes[0][0]);
      }
      else{
  	close(0);
  	dup(fildes[i][0]);
  	close(1);
  	dup(fildes[i-1][1]);
      }

      for(j = 0; j < cnt; j++){
	close(fildes[j][0]);
	close(fildes[j][1]);
      }


      sprintf(buffer, "%s", ptr);
      ptr = strtok(buffer, " ");
      j= 0;
      while(ptr != NULL){
  	arg[j] = ptr;
  	j++;
  	ptr = strtok(NULL, " ");
      }
      arg[j] = NULL;
      execvp(arg[0], arg);
    }

    sprintf(buffer, "%s", save_buffer);
  }
}

int count(char *ptr, char c){
  int i = 0;
  while(*ptr)
    if(*ptr++ == c)
      i++;
  return i;
}

void redirect(char *buffer, char * newfile,int red){
  char * ptr;
  int fd;
  char *arg[10];
  int j;
  int i;

  if(fork() == 0){
    /* > */
    if(red == 1){
      fd = creat(newfile, FMASK);
    }
    /* >> */
    else if(red == 2){
      if(0 == access(newfile, F_OK)){
	fd = open(newfile, O_WRONLY|O_APPEND);
      }
      else {
	fd = creat(newfile, 0644);
      }
    }
    /* >! */
    else if(red == 3){
      if(0 == access(newfile, F_OK)){	  
	/* append code nope */
      }
      else{
	fd = -1;
      }
    }
    /* < */
    else if(red == 4){
      if(0 == access(newfile, F_OK)){
	fd = open(newfile, O_RDONLY);
      }
      else{
	fd = -1;
      }
    }
    if(fd == -1){
      printf("Cannot open file !\n");
      exit(0);
    }
    if(red != 4){
      close(1);
      dup(fd);
      close(fd);
    }
    else {
      close(0);
      dup(fd);
      close(fd);
    }

    ptr = strtok(buffer, "><!\n&");
    ptr = strtok(buffer, " &");
    j= 0;
    while(ptr != NULL){
      arg[j] = ptr;
      j++;
      ptr = strtok(NULL, " &");
    }

    arg[j] = NULL;
    execvp(arg[0], arg);
  }


}
