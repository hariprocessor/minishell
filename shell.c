#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFSIZE 2097152
#define MAX_PATH 4096
#define FMASK 0644
#define ARGSIZE 10
void pipe_exec(char *buffer, char *ptr);
int count(char *ptr, char c);
void reverse(char *b, char *save);
int redirection(char *buffer){
  if(strstr(buffer, ">") != NULL)
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
  char arg[ARGSIZE][100];
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
  char test[BUFSIZE];
  while(read(0, buffer,BUFSIZE)){
    strcpy(save_buffer, buffer);

    /* background */
    if(strstr(buffer, "&") != NULL)
      amper = 1;
    else
      amper = 0;
    
    if(fork() == 0){
      /* newfile string */
      
      ptr = strtok(buffer, " ><!\n");
      ptr = strtok(NULL, " ><!\n");
      sprintf(newfile, "%s%s", "./", ptr);
      strcpy(buffer, save_buffer);
      /* redirection of IO */
      if((red = redirection(buffer)) != 0){
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
	    fd = creat(newfile, 0644);
	  }
	}
	/* < */
	if(fd == -1){
	  printf("Cannot open file !");
	}
	//	close(1);
	//	dup(fd);
	//	close(fd);
      }
      
      /* piping */
      /*
	printf("buffer : %s\n", buffer);
	printf("save_buffer : %s\n", save_buffer);
	printf("newfile : %s\n", newfile);
      */
      if(strstr(buffer, "|")){
	i = count(save_buffer, '|');
	fildes = (int**)malloc(sizeof(int*)*i);
	for(j = 0; j < i; j++)
	  *(fildes+j) = (int *)malloc(sizeof(int)*2);

	for(i = 0; i < count(save_buffer, '|')+1; i++){
	  printf("i : %d\n", i);
	  ptr = strtok(buffer, "|\n");

	  for(j = 0; j < count(save_buffer, '|')-i; j++){
	    ptr = strtok(NULL, "|\n");
	  }
	  pipe(fildes[i]);
	  if(fork() == 0){

	    /*
	    if(i == count(save_buffer, '|')){
	      close(1);
	      dup(fildes[i-1][1]);
	      close(fildes[i-1][0]);
	      close(fildes[i-1][1]);
	    }
	    else if(i == 0){
	      close(0);
	      dup(fildes[i][0]);
	      close(fildes[i][0]);
	      close(fildes[i][1]);
	    }
	    else{
	      close(0);
	      dup(fildes[i-1][0]);
	      close(fildes[i-1][0]);
	      close(fildes[i-1][1]);
	      close(1);
	      dup(fildes[i][1]);
	      close(fildes[i][0]);
	      close(fildes[i][1]);
	    }

	    */
	    sprintf(buffer, "%s", ptr);
	    printf("%d buffer : %s\n", i, buffer);
	    ptr = strtok(buffer, " ");
	    printf("%d str : %s\n", i, ptr);
	    j= 0;
	    ptr = strtok(NULL, " ");
	    while(ptr != NULL){
	      sprintf(arg[j], "%s", ptr);
	      printf("arg[%d] : %s\n", j, arg[j]);
	      j++;
	      ptr = strtok(NULL, " ");
	    }

	    //	    execve();
	    exit(0);
	  }
	  sprintf(buffer, "%s", save_buffer);
	  //	  pipe(fildes);
	  
	  /*
	  if(fork() == 0){
	    close(1);
	    dup(fildes[1]);
	    close(fildes[1]);
	    close(fildes[0])
	    }*/
	  //	  wait(NULL);
	}
	for(j = 0; j < ARGSIZE; j++)
	  free(fildes[j]);
	free(fildes);

      }

    }
    fflush(stdin);
    free(fildes);
  }
}


/*
void pipe_exec(char *buffer, char *ptr){
  int fd[2];
  char command[BUFSIZE];

  if(!ptr){
    exit(0);
  }
  else{
    pipe(fd);

    if(fork() == 0){
      sprintf(command, "/bin/%s", ptr);
      close(1);
      dup(fd[1]);
      execve("/bin/")
    }
    printf("ptr : %s\n", ptr);
    ptr = strtok(NULL, "| \n");
    pipe_exec(buffer, ptr);
    exit(0);
  }
}
*/

int count(char *ptr, char c){
  int i = 0;
  while(*ptr)
    if(*ptr++ == c)
      i++;
  return i;
}
