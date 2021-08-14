//*********Header Files************
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<string.h>
#define MAX_LINE 80  /* 80 character per command*/
#define Delimeters " \t\n\v\f\r"
/* Declaration of Functions used 
*/
void null_args(char *arg[]);
void null_cmd(char *cm);
void clearscreen();
0
int get_input(char*);
size_t parse_input(char *arg[],char* cmd);
int check_amp(char**,size_t *);
int run_cmd(char**,size_t);
void detect_pipe(char**,size_t*,char***,size_t*);
unsigned ch_redirect(char **,size_t*,char**,char**);
int open_redirect(unsigned , char *, char *, int *, int *); 
void close_file(unsigned , int , int );
//******Main Function*********
int main(void)
{
   char *args[MAX_LINE/2+1]; /* command line (80 length) has max 40 arguments*/
   char command[MAX_LINE+1]; // command of len 80
   null_args(args);          // initalize arguments
   null_cmd(command);        // intialize commands
   int sh_run=1;             /* flag to determine when to exit program */ 
   while(sh_run)               
   {
  clearscreen();            // function call to clear screen and new prompt
  fflush(stdout);           
  fflush(stdin);
  if(get_input(command)==0)  // if getting error or no history found than function continue 
  {
  continue;
  }
0
  size_t no_of_arg= parse_input(args,command);
  if(no_of_arg==0)  
  {
  printf("Please Enter the Command or Type EXIT | exit \n");
  continue;
  }
  if((strcmp(args[0],"exit")==0)||(strcmp(args[0],"EXIT")==0)) 
  break;
 
   run_cmd(args,no_of_arg);
   
}
   
}
/* to null the content of args[] at start 
*/
void null_args(char *arg[])
{
for(int i=0;i!=MAX_LINE/2 +1;i++)
{
arg[i]=NULL;
}
}
/* to intailize the command string */
0
void null_cmd(char *cm)
{
strcpy(cm," ");
}
/* to clear the screen */
void clearscreen()
{
static int first=1;
if(first)
{
const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
  write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
  first=0;
  }
  printf("osh>");
}
/* parse input function
args    : array to store arguments
command :take command from user
to Tokanized the available string command
*/
size_t parse_input(char *arg[],char* cmd)
0
{
size_t num = 0;
    char command[MAX_LINE + 1];
    strcpy(command, cmd);  // to modify it we use strtok
    char *token = strtok(command, Delimeters); //tokenized your command
    while(token != NULL) {  
        arg[num] = malloc(strlen(token) + 1);
        strcpy(arg[num], token);
        ++num;
        token = strtok(NULL, Delimeters);
    }
    return num;
}
/*   get_input function will take input from histor */
int get_input(char *cmd)
{
char buffer[MAX_LINE +1];
if(fgets(buffer,MAX_LINE+1,stdin) == NULL)
{
fprintf(stderr,"input error\n");
return 0;
}
if(strncmp(buffer,"!!",2)==0)   //if user enter !! than display previous command
{
if(strlen(cmd)==0)     //if command length is 0 than no history yet
{
fprintf(stderr,"No history found\n");
0
return 0;
}
printf("%s",cmd);   // command uncanged and print it
}
strcpy(cmd,buffer);  //update command
return 1;           
}
/* cheak whether there is ampersand or not*/
int check_amp(char** args,size_t *size)
{
size_t len=strlen(args[*size -1]);
if(args[*size-1][len-1]!='&')
{
return 0;
}
if(len==1)   // remove if only contains &
{
free(args[*size-1]);
args[*size-1]=NULL;
--(*size);    //reduce the size
}
else
{
args[*size-1][len-1]='\0';
}
return 1;
}
/*
 * Function: detect_pipe
 *   Detect the pipe '|' and split aruguments into two parts accordingly.
 */
 void detect_pipe(char** args,size_t* args_num,char*** args2,size_t* args2_num)
 {
   for(int i=0;i!=*args_num;i++)
   {
   if(strcmp(args[i],"|")==0)
   {
   free(args[i]);
   args[i]=NULL;
   *args2_num=*args_num-i-1;
   *args_num=i;
   *args2=args+i+1;
   break;
   }
   }
   
}
/*
 * Function: Check the redirection tokens in arguments and remove such tokens
 *   returns: input/output flag (1 for output,  0 for input)
 */
 
unsigned ch_redirect(char **args,size_t *size,char **input,char **out)
{
0
  unsigned flag = 0; //declaration of flag to return output
    size_t to_rmv[4], rmv_cnt = 0;  //remove variables 
    for(size_t i = 0; i != *size; ++i) {
        if(rmv_cnt >= 4) {
            break;
        }
        if(strcmp("<", args[i]) == 0) {     // input 
            to_rmv[rmv_cnt++] = i;
            if(i == (*size) - 1) {
                fprintf(stderr, "No input file provided!\n");
                break;
            }
            flag |= 1;
            *input = args[i + 1];
            to_rmv[rmv_cnt++] = ++i;
        } else if(strcmp(">", args[i]) == 0) {   // output
            to_rmv[rmv_cnt++] = i;
            if(i == (*size) - 1) {
                fprintf(stderr, "No output file provided!\n");
                break;
            }
            flag |= 2;
            *out = args[i + 1];  //output file
            to_rmv[rmv_cnt++] = ++i;
        }
    }
    /* Remove Input/output indicators and filenames from arguments */
    for(int i = rmv_cnt - 1; i >= 0; --i) {
0
        size_t pos = to_rmv[i];  // the index of arg to remove
        // printf("%lu %s\n", pos, args[pos]);
        while(pos != *size) {
            args[pos] = args[pos + 1];
            ++pos;
        }
        --(*size);
    }
    return flag;
}
/*
Funtion to Open redirect input/output and files*/  
int open_redirect(unsigned flag, char *input, char *out, int *input_fd, int *out_fd) {
 
    if(flag & 2) {  // redirecting output
        *out_fd = open(out, O_WRONLY | O_CREAT | O_TRUNC, 644);
        if(*out_fd < 0) {
            fprintf(stderr, "Failed to open the output file: %s\n", out);
            return 0;
        }
       
        dup2(*out_fd, 1);
    }
    if(flag & 1) { // redirecting input
        *input_fd = open(input, O_RDONLY, 0644);
        if(*input_fd < 0) {
            fprintf(stderr, "Failed to open the input file: %s\n", input);
            return 0;
0
        }
        // printf("Input from: %s %d\n", input_file, *input_desc);
        dup2(*input_fd, STDIN_FILENO);
    }
    return 1;
}
/*Function to close file*/
void close_file(unsigned flag, int input_fd, int out_fd)
{
 if(flag & 2) {
        close(out_fd);
    }
    if(flag & 1) {
        close(input_fd);
    }
}
/*
 * Function: run_comman
 *   returns: success or not
 */
 int run_cmd(char **args,size_t args_num)
 {
 
    /* Detect '&' to determine whether to run concurrently */
    int amp = check_amp(args, &args_num);
    /* Detect pipe */
    char **args2;
    size_t args_num2 = 0;
0
    detect_pipe(args, &args_num, &args2, &args_num2);
    /* Create a child process and execute the command */
    pid_t pid = fork();
    if(pid < 0) {   // fork failed
        fprintf(stderr, "Failed to fork!\n");
        return 0;
    } else if (pid == 0) { // child process
        if(args_num2 != 0) {    // pipe
            /* Create pipe */
            int fd[2];
            pipe(fd);
            /* Fork into another two processes */
            pid_t pid2 = fork();
            if(pid2 > 0) {  // child process for the second command
                /* Redirect I/O */
                char *input, *out;
                int input_fd, out_fd;
                unsigned flag = ch_redirect(args2, &args_num2, &input, &out);    // bit 1 for output, bit 0 
for input
                flag &= 2;   // disable input redirection
                if(open_redirect(flag, input, out, &input_fd, &out_fd) == 0) {
                    return 0;
                }
                close(fd[1]);
                dup2(fd[0], STDIN_FILENO);
                wait(NULL);     // wait for the first command to finish
                execvp(args2[0], args2);
                close_file(flag, input_fd, out_fd);
0
                close(fd[0]);
                fflush(stdin);
            } else if(pid2 == 0) {  // grandchild process for the first command
                /* Redirect I/O */
                char *input, *out;
                int input_fd, out_fd;
                unsigned flag = ch_redirect(args, &args_num, &input, &out);    // bit 1 for output, bit 0 for 
input
                flag &= 1;   // disable output redirection
                if(open_redirect(flag, input, out, &input_fd, &out_fd) == 0) {
                    return 0;
                }
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                execvp(args[0], args);
                close_file(flag, input_fd, out_fd);
                close(fd[1]);
                fflush(stdin);
            }
        } else {    // no pipe
            /* Redirect I/O */
            char *input, *out;
            int input_fd, out_fd;
            unsigned flag = ch_redirect(args, &args_num, &input, &out);    // bit 1 for output, bit 0 for 
input
            if(open_redirect(flag, input, out, &input_fd, &out_fd) == 0) {
                return 0;
            }
            execvp(args[0], args);
0
            close_file(flag, input_fd, out_fd);
            fflush(stdin);
        }
    } else { // parent process
        if(!amp) { // parent and child run concurrently
            wait(NULL);
        }
    }
    return 1;
}
}
