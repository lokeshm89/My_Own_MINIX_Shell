#include "msh.h"
#include <stdlib.h>

jmp_buf jump_buffer;

int main(void)
{
	char cmdline[BUFSIZE];
	static const char profile_path[] = "./profile.src";
	char * profile[256];
	char *argv[BUFSIZE];
	FILE * file = fopen(profile_path, "r");
	e_variables=(struct env_var *)malloc(sizeof(struct env_var)*256);

	read_profile(profile);

	init_sh(profile);

while(1)
  {
	print_prompt_sign(profile);
	read_cmdline(cmdline);

	if (strcmp("exit", cmdline) == 0)
	      exit(0);

	execute(cmdline, argv);
  }
  puts("Bye");
  return 0;
}

char read_var(char ** profile, char * var_name)
{
  char * result = NULL;
  char ** line = profile;
  while (*line)
  {
    struct entry par;
    char *c = *line;
    int counter = 0;
    while (c++)
    {
      counter++;
      if (*c == '=')
      {
        par.var_name = (char *) malloc((counter + 1) * sizeof(char));
        strncpy(par.var_name, *line, counter);
        par.var_name[counter] = '\0';

        par.var_value = (char *) malloc(strlen(c) - 1);
        strncpy(par.var_value, c + 1, strlen(c) - 1);
        par.var_value[strlen(c) - 2] = '\0';

        break;
      }
    }
    if (par.var_name != NULL)
    {
      if (strcmp(par.var_name, var_name) == 0)
      {
        result = par.var_value;
        return result;
      }
      else
      {
        line++;
        continue;
      }
    }
    else
      break;
  }
  return NULL;
}

void init_sh(char **profile)
{
  char *path = read_var(profile, "HOME");
  if(chdir(path)==0)
  {
    setenv("HOME", path, 1);
  }
  else
    perror("init error");
}

void print_prompt_sign(char ** profile)
{
  char * sign = read_var(profile, "SIGN");
  printf("[Bhargav] %s ", sign);
  fflush(stdout);
}

int read_profile(char ** profile)
{
  static const char profile_path[] = "./profile.src";
  FILE * file = fopen(profile_path, "r");
  if (file != NULL)
  {
    char line[128];

    int i = 0;
    while (fgets(line, sizeof(line), file) != NULL)
    {
      profile[i] = (char *) (malloc(sizeof(line)));
      strcpy(profile[i], line);
      i++;
    }
    fclose(file);
  }
  else
  {
    perror(profile_path);
    return -1;
  }

  return 0;
}

int read_env(char ** profile)
{
  static const char profile_path[] = "test.txt";
  FILE * file = fopen(profile_path, "r");
  if (file != NULL)
  {
    char line[128];

    int i = 0;
    while (fgets(line, sizeof(line), file) != NULL)
    {
      profile[i] = (char *) (malloc(sizeof(line)));
      strcpy(profile[i], line);
      i++;
    }
    fclose(file);
  }
  else
  {
    perror(profile_path);
    return -1;
  }

  return 0;
}

void read_cmdline(char * cmdline)
{

int len = 0;
  char c;
signal(SIGINT, ctrl_CHandler);
  setjmp(jump_buffer);
	while ((c = getchar()) != '\n')
    cmdline[len++] = c;
  cmdline[len] = '\0';
}


void ctrl_CHandler(int param)
{

 /*  char alter[150];
  printf("\n Exit from the shell: Are you sure? [yes/no]: ");
  scanf("%31s", alter);

  if((strcmp(alter, "y") == 0) || (strcmp(alter, "Y") == 0) || (strcmp(alter, "yes") == 0)
      || (strcmp(alter, "YES") == 0))
    exit(0);
  else
    longjmp(jump_buffer,1); */
    // Commented to stop the program from responding to signals
    return();
}

void parse_cmdline(char *cmdline, char **argv)
{
  while(*cmdline !='\0')
  {
    while(*cmdline == ' ' || *cmdline == '\t' ||
        *cmdline == '\n')
      *cmdline++ = '\0';
    *argv++ = cmdline;
    while(*cmdline != '\0' && *cmdline != ' ' &&
        *cmdline != '\t' && *cmdline != '\n')
      cmdline++;
  }
  *argv = '\0';
}

void execute(char *cmdline, char **argv){
char *childcmd[2];
pid_t childpid;
int out_type = 0;
int status, options, ret = 0,n=0;
char *cmd = cmdline;
char * profile[256];
size_t len;

	 if(strlen(cmd) > 2)
	 	   {
	 	     if(strncmp(cmd,"put=$",5)==0){
				updateVariable(cmd);
				return;
			 }
	 	     if(strncmp(cmd,"get=$",5)==0){

				read_env(profile);
				printVariable(cmd,profile);
				return;
			 }
	 	     if(strncmp(cmd, "x=",2)==0)
	 	     {
	 	       point5(cmdline);
	 	       return;
	 	     }

	   }


	if(strlen(cmdline)<=0)
	    return;

	  childcmd[0] = cmdline;

	  while(*cmdline != '>' && *cmdline != '|' &&
	      *cmdline != '$' &&
	      *cmdline != '\0' && *cmdline != '\n')
	  {
	    cmdline++;
	  }
	  if(*cmdline=='>')
	  {
	    out_type = 1;
	    *cmdline++ = '\0';
	    childcmd[1] = cmdline;
	  }
	  else if(*cmdline=='|')
	  {
	    out_type = 2;
	    *cmdline++ = '\0';
	    childcmd[1] = cmdline;
	  }
	  else if(*cmdline=='$')
	  {
	    if(strncmp(cmdline, "$(", 2)==0)
	    {
	      out_type = 3;
	      *cmdline++ = '\0';
	      childcmd[1] = cmdline;
	    }
	  }

	  n = strlen(childcmd[0]) - 1;
	  while(n>0)
	  {
	   if(childcmd[0][n]==' ')
	    {
		     childcmd[0][n]='\0';
	    }
	    else
	      break;
	    n--;
	  }

	  while(*childcmd[1] != '\0')
	  {
	    if(*childcmd[1]==' ')
	      childcmd[1]++;
	    else
	      break;
	  }

	  parse_cmdline(childcmd[0], argv);

	  ret = handle_builtin(argv);
	  if(ret==0)
	    return;

	  childpid = fork();
	  if (childpid == -1)
	  {
	    perror("Cannot proceed. fork() error");
	    exit(1);
	  }
	  else if (childpid == 0)
	  {

	    if(out_type==0)
	    {
	      childexec(argv);
	    }
	    else if(out_type==1)
	    {
	      int fd;
	      close(STDOUT);
	      fd = open(childcmd[1], O_APPEND);
	      if(fd<0)
	      {
	        fd = creat(childcmd[1], 0666);
	        if(fd<0)
	        {
	          perror("create file failed");
	          exit(-1);
	        }
	      }
	      dup(fd);
	      childexec(argv);
	      close(fd);
	    }
	    else if(out_type==2)
	    {
		int fd[2];
	      pipe(&fd[0]);
	      if(fork()!=0)
	      {
	        close(fd[0]);
	        close(STDOUT);
	        dup(fd[1]);
	        childexec(argv);
	        close(fd[1]);
	      }
	      else
	      {
	        close(fd[1]);
	        close(STDIN);
	        dup(fd[0]);
	        parse_cmdline(childcmd[1], argv);
	        childexec(argv);
	        close(fd[0]);
	      }
	      puts("\n");
	    }
	    else if(out_type==3)
	    {
	      int fd[2];
	      childcmd[1]++;
	      len = strlen(childcmd[1]);
	      childcmd[1][len-1] = '\0';
		  printf("childcmd[0]:%s\n", childcmd[0]);
		  printf("childcmd[1]:%s\n", childcmd[1]);

		pipe(&fd[0]);
		if(fork()!=0)
	      {
			  printf("Inside Parent process");
	        close(fd[0]);
	        close(STDOUT);
	        dup(fd[1]);
			close(fd[1]);
	        childexecline(childcmd[0]);
		}
	      else{
			  printf("Inside child process");
	      close(fd[1]);
			    close(STDIN);
			    dup(fd[0]);
			    close(fd[0]);
			    childexecline(childcmd[1]);
	      }
	  }
	      puts("\n");
	    }

	  else
	  {
	    waitpid(childpid, &status, options);
	    if (!WIFEXITED(status))
	      printf("Parent: child has not terminated normally.\n");
  }

    }


void updateVariable(char *input){
	char variable[5],name[1],value[1];
	static const char env_path[] = "test.txt";
	FILE * fp ;
	substring(5,strlen(input),input,variable,sizeof variable );
	fp = fopen(env_path, "aw");
	      if (fp == NULL) {
	         printf("I couldn't open results.dat for writing.\n");
	         exit(0);
	      }
      fprintf(fp, variable);
      fprintf(fp, "\n");
      fclose(fp);
}

void printVariable(char *input,char ** profile){
	char variable[6];
	static const char env_path[] = "test.txt";
	FILE * fp ;
	fopen(env_path, "a");
	fclose(fp);
	substring(5,strlen(input),input,variable,sizeof variable );
	printf("Variable value of %s is %s\n",variable,read_var_env(profile, variable));
}


int childexec(char ** argv)
{
  if(execvp(*argv, argv) < 0)
  {
    perror("ERROR: exec failed ");
    exit(1);
  }
  return 0;
}

int handle_builtin(char **argv)
{
  if(strcmp(argv[0], "cd") == 0)
  {
    if(argv[1])
      change_dir(argv[1]);
    return 0;
  }
  return 1;
}

void change_dir(char *path)
{
  if(path==NULL)
  {
    path = getenv("HOME");
  }
  if(chdir(path)==0)
  {
  }
  else
    perror("init error");
}


void point5(char *input)
{
  char variables1[150][150];
  char op1[150];
  char destination1[20];
  int destinationvalue,index_i=0,found=0,sourcefound=0,destinationfound=0,s_index=0,d_index=0;
  int i=0,j=0,k=0,op=0;
  int valuesofvariables[150];
  int multi1;
  int divi;
  int addition1;
  int subtraction1;
  int oper;
  char command[20];
  memset(valuesofvariables,0,sizeof(int)*150);

  while(input[i] != '\0')
  {
    if(input[i]!=' ')
    {
      command[j]=input[i];
      j++;
      i++;
    }
    else
      i++;
  }
  command[j]='\0';


  j=i=0;
  while(command[i] != '\0')
  {
    if(command[i]=='=')
    {
      destination1[i]='\0';
      i++;
      break;
    }

    else
    {
      destination1[i]=command[i];
      i++;
    }
  }


  index_i=0;
  k=0; op=0;
  while(command[i] != '\0')
  {
    j=command[i];
    if(j=='+' || j=='/' || j=='-' || j=='*')
    {
      variables1[index_i][k]='\0';
      op1[op]=j;
      op++;
      index_i++;
      k=0;
    }
    else
    {
      if(j!='$' && j!='(' && j!=')')
      {
        variables1[index_i][k]=j;
        k++;
      }
    }
    i++;
  }


  variables1[index_i][k]='\0';
  index_i++;
  op1[op]='\0';


  i=j=0;
  while(i < index_i)
  {
    while(e_variables[j].name != NULL)
    {
      if(!strcmp(e_variables[j].name,variables1[i]) )
      {
        valuesofvariables[i]=e_variables[j].value;
        break;
      }
      j++;
    }


    if(atoi(variables1[i]))
      valuesofvariables[i]=atoi(variables1[i]);
    j=0;
    i++;
  }


  multi1=index(command,'*');
  divi=index(command,'/');
  addition1=index(command,'+');
  subtraction1=index(command,'-');


    if(index(command,'$'))
    {


      printf("In the right block\n");


      i=0;
      while(e_variables[i].name != NULL)
      {
        if(!strcmp(e_variables[i].name,variables1[0]))
        {
          printf("Found source at %d\n",i);
          sourcefound=1;
          s_index=i;
        }

        else if(!strcmp(e_variables[i].name,destination1) )
        {
          printf("Found dest at %d, %d, %s,%s\n",i,e_variables[i].value,destination1,e_variables[i].name);
          destinationfound=1;
          d_index=i;
        }
        i++;
      }


      if(!sourcefound)
      {       printf("Source Not Found\n");
        return;
      }


      if(sourcefound && destinationfound)
      {
        printf("both found\n");
        e_variables[d_index].value=e_variables[s_index].value;
        return;
      }


      if(sourcefound && !destinationfound)
      {

        e_variables[i].name = (char *)malloc(sizeof(char)*strlen(destination1)+1);
        memcpy(e_variables[i].name,(char *)destination1,strlen(destination1)+1);
        strncat(e_variables[i].name,"\0",1);
        e_variables[i].value=e_variables[sourcefound].value;
        return;
      }
    }


    else
    {

      destinationvalue=atoi(variables1[0]);
	  printf("Destination value is  %d\n",destinationvalue);

      i=0;
      while(e_variables[i].name != NULL)
      {
		  printf("Inside While Loop %s ,%s,%s\n",e_variables[i].name,e_variables[i].value,destination1);
        if(!strcmp(e_variables[i].name,destination1))
        {

          found=1;
          e_variables[i].value=destinationvalue;
          break;
        }
        i++;
      }


      if(!found)
      {
        e_variables[i].name = (char *)malloc(sizeof(char)*strlen(destination1)+1);
        memcpy(e_variables[i].name,(char *)destination1,strlen(destination1)+1);
        strncat(e_variables[i].name,"\0",1);
        e_variables[i].value=destinationvalue;
        printf("added variable %s",e_variables[i].name);
      }
    }

  return;
}

int childexecline(char * cmdline)
{
  char *argv[BUFSIZE];
  parse_cmdline(cmdline, argv);
  return childexec(argv);
}


void exec_pipelines(char * cmdline, int fd[2])
{
  if(output_type(cmdline)==0)
  {
	close(fd[0]);
    close(STDOUT);
    dup(fd[1]);
    childexecline(cmdline);
    close(fd[1]);
  }

}

int output_type(char * cmdline)
{
	  int output = 0;
  char * c = cmdline;
  while(*c!='\0' && *c!='$' &&
      *c!='>' && *c!='|' &&
      *c!='\n')
  {
    c++;
  }
  if(*c=='>')
    output = 1;
  else if(*c=='|')
    output = 2;
  else if(*c=='$')
  {
    if(strncmp(c, "$(", 2)==0)
      output = 3;
  }
  return output;
}

char * substring(size_t start, size_t stop, const char *src, char *dst, size_t size)
{
   int count = stop - start;
   if ( count >= --size )
   {
      count = size;
   }
   sprintf(dst, "%.*s", count, src + start);
   return dst;
}

char * read_var_env(char ** profile, char * var_name)
{
  char * result = NULL;
  char ** line = profile;
  while (*line)
  {
    struct entry par;
    char *c = *line;
    int counter = 0;
    while (c++)
    {
      counter++;
      if (*c == '=')
      {
        par.var_name = (char *) malloc((counter + 1) * sizeof(char));
        strncpy(par.var_name, *line, counter);
        par.var_name[counter] = '\0';

        par.var_value = (char *) malloc(strlen(c) - 1);
        strncpy(par.var_value, c + 1, strlen(c) - 1);
        par.var_value[strlen(c) - 2] = '\0';
        break;

      }
    }
    if (par.var_name != NULL)
    {
      if (strcmp(par.var_name, var_name) == 0)
      {
        result = par.var_value;
        line++;
        continue;
      }
      else
      {
        line++;
        continue;
      }
    }
    else
      break;
  }
  return result;
}
