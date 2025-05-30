/* ICCS227: Project 1: icsh
 * Name: Kemanut Kanthasaksiri
 * StudentID: 6580088
 */

 #include "stdio.h"
 #include "string.h"
 #include "stdlib.h"
 #include "unistd.h"
 #include "sys/types.h"
 #include "sys/wait.h"
 #include "signal.h"
 
 #define MAX_CMD_BUFFER 255
 #define MAX_ARGS 64
 
 pid_t foreground_pid = -1;
 
 void handle_sigint(int sig) {
     if (foreground_pid > 0) {
         kill(foreground_pid, SIGINT);
     } else {
         write(STDOUT_FILENO, "\nicsh $ ", 8);
     }
 }
 
 void handle_sigtstp(int sig) {
     if (foreground_pid > 0) {
         kill(foreground_pid, SIGTSTP);
     } else {
         write(STDOUT_FILENO, "\nicsh $ ", 8);
     }
 }
 
 int main(int argc, char *argv[]) {
     char buffer[MAX_CMD_BUFFER];
     char last_command[MAX_CMD_BUFFER] = "";
     int last_exit_status = 0;
     FILE *input = stdin;
 
     signal(SIGINT, handle_sigint);
     signal(SIGTSTP, handle_sigtstp);
 
     if (argc > 1) {
         input = fopen(argv[1], "r");
         if (!input) {
             perror("Error opening file");
             return 1;
         }
     }
 
     printf("Initializing IC Shell\n");
     while (1) {
         if (input == stdin) {
             printf("icsh $ ");
             fflush(stdout);
         }
 
         if (fgets(buffer, MAX_CMD_BUFFER, input) == NULL) {
             break;
         }
 
         buffer[strcspn(buffer, "\n")] = '\0';
 
         if (strcmp(buffer, "!!") == 0) {
             if (strlen(last_command) == 0) {
                 printf("No commands in history.\n");
                 continue;
             }
             strcpy(buffer, last_command);
             printf("%s\n", buffer);
         } else {
             strcpy(last_command, buffer);
         }
 
         char *args[MAX_ARGS];
         int i = 0;
         char *token = strtok(buffer, " ");
 
         if (token == NULL) {
             continue;
         }
 
         while (token != NULL && i < MAX_ARGS - 1) {
             args[i++] = token;
             token = strtok(NULL, " ");
         }
         args[i] = NULL;
 
         if (strcmp(args[0], "exit") == 0) {
             int status = 0;
             if (args[1] != NULL) {
                 status = atoi(args[1]) & 0xFF;
             }
             printf("User has left the shell\n");
             last_exit_status = status;
             if (input != stdin) fclose(input);
             exit(status);
         } else if (strcmp(args[0], "echo") == 0) {
             for (int j = 1; args[j] != NULL; j++) {
                 if (strcmp(args[j], "$?") == 0) {
                     printf("%d", last_exit_status);
                 } else {
                     printf("%s", args[j]);
                 }
                 if (args[j + 1] != NULL) {
                     printf(" ");
                 }
             }
             printf("\n");
             last_exit_status = 0;
         } else {
             pid_t pid = fork();
             if (pid == 0) {
                 // Child process
                 signal(SIGINT, SIG_DFL);
                 signal(SIGTSTP, SIG_DFL);
                 execvp(args[0], args);
                 perror("execvp failed");
                 exit(1);
             } else if (pid > 0) {
                 foreground_pid = pid;
                 int status;
                 waitpid(pid, &status, WUNTRACED);  // Handle stopped processes too
 
                 if (WIFSTOPPED(status)) {
                     printf("\n[%d] suspended\n", pid);
                 } else {
                     last_exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
                 }
                 foreground_pid = -1;
             } else {
                 perror("fork failed");
                 last_exit_status = 1;
             }
         }
     }
 
     if (input != stdin) fclose(input);
     return 0;
 }
 