#ifdef	__APPLE__
#  include <sys/wait.h>
#else
#  include "wait.h"
#endif
#include "execute.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <ctype.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "parser.h"

/* Executes the command given to the shell, using a version of the grandchild method. */
void execute_command_line(Command_line *cmd) {
  pid_t pid1, pid2, pid3;
  int fd[2];
  int fdprev[2];
  int i;
  int file;
  if ((pid1 = fork()) < 0){
    err(EX_OSERR, "fork error");
  }
  if (pid1 > 0){ /* parent code */
    /* I'm the shell, only process that returns from this function */
    wait(NULL);
    return;
  }
  /*do input and output redirection*/
  if (cmd->outfile != NULL){
    if (cmd->append != 0){
      if ((file = open(cmd->outfile, O_RDWR | O_APPEND | O_CREAT, DEF_MODE)) < 0){
	printf("Open error. 2\n");
	exit(EX_OSERR);
      }
      dup2(file, STDOUT_FILENO);
    }
    else {
      if ((file = open(cmd->outfile, O_RDWR | O_CREAT, DEF_MODE)) < 0){
	printf("Open error. 2(3)\n");
	exit(EX_OSERR);
      }
      dup2(file, STDOUT_FILENO);
    }
  }
  /* start running through programs backwards */
  for (i = cmd->num_progs; i >= 0; i--){
    if (cmd->num_progs > 1){
      if (pipe(fd) < 0){
	printf("Pipe error.\n");
	exit(EX_OSERR);
      }
    }
    if (i > 1){
      if ((pid2 = fork()) < 0){
	printf("Fork error.\n");
	exit(EX_OSERR);
      }
      if (pid2 > 0){ /* parent code 2 / child code 1*/ 
	if (cmd->bg == 0){
	  /* redirect stdin from pipe */
	  if (i != 1) {
	    close(fd[1]);
	    dup2(fd[0], STDIN_FILENO);
	    close(fd[0]);
	  }
	  if (i < (cmd->num_progs)){
	    close(fdprev[0]);
	    dup2(fdprev[1], STDOUT_FILENO);
	    close(fdprev[1]);
	  }
	  /* execute program */
	  execvp(cmd->argvs[i-1][0], cmd->argvs[i-1]);
	  printf("Error executing command\n");
	  exit(EX_OSERR);
	}
	else { /* backgrounding */
	  return;
	}
      }
      else { /* child code 2 / grandchild 1*/
	/* redirect stdout from pipe */
	if (i > 1){
	  fdprev[0] = fd[0];
	  fdprev[1] = fd[1];
	}
      }
    }
    /* is only program to execute, or last one to execute */
    else {
      /* redirection */
      if (i != cmd->num_progs) {
	close(fdprev[0]);
	dup2(fdprev[1], 1);
	close(fdprev[1]);
	
	dup2(fd[0], 0);
	close(fd[0]);
	close(fd[1]);
      }
      if (cmd->infile != NULL){
	if ((file = open(cmd->infile, O_RDWR, DEF_MODE)) < 0){
	  printf("Couldn't open file.\n");
	  exit(EX_OSERR);
	}
	dup2(file, STDIN_FILENO);
      }
      if (cmd->bg == 0){
	execvp(cmd->argvs[i-1][0], cmd->argvs[i-1]);
	printf("Error executing command\n");
	exit(EX_OSERR);
      }
      /*backgrounding check */
      else {
	if ((pid3 = fork()) < 0){
	  printf("Fork error last.\n");
	  exit(EX_OSERR);
	}
	if (pid3 > 0) {  /* parent */
	  return;
	}
	else { /* child */
	  execvp(cmd->argvs[i-1][0], cmd->argvs[i-1]);
	  printf("Error exeuting command\n");
	  exit(EX_OSERR);
	}
      }
    }
  }
  if (cmd->num_progs > 1){
    close(fdprev[0]);
    close(fdprev[1]);
  }
}