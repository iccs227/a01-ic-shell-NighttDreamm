/* ICCS227: Project 1: icsh
 * Name: Kemanut Kanthasaksiri
 * StudentID: 6580088
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <signal.h>
 #include <sys/types.h>
 #include <sys/wait.h>
 
 #define MAX_CMD_BUFFER 255
 #define MAX_ARGS 64
 #define MAX_JOBS 64
 
 typedef struct {
     int id;
     pid_t pid;
     char command[MAX_CMD_BUFFER];
     int running; // 1 = running, 0 = stopped
 } Job;
 
 Job jobs[MAX_JOBS];
 int job_count = 0;
 int next_job_id = 1;
 pid_t foreground_pid = -1;
 
 void print_prompt() {
     write(STDOUT_FILENO, "icsh $ ", 7);
 }
 
 void add_job(pid_t pid, char *command, int running) {
     if (job_count < MAX_JOBS) {
         jobs[job_count].id = next_job_id++;
         jobs[job_count].pid = pid;
         strncpy(jobs[job_count].command, command, MAX_CMD_BUFFER);
         jobs[job_count].running = running;
         job_count++;
         printf("[%d] %d\n", jobs[job_count - 1].id, pid);
     }
 }
 
 int find_job_index_by_pid(pid_t pid) {
     for (int i = 0; i < job_count; i++) {
         if (jobs[i].pid == pid) return i;
     }
     return -1;
 }
 
 int find_job_index_by_id(int id) {
     for (int i = 0; i < job_count; i++) {
         if (jobs[i].id == id) return i;
     }
     return -1;
 }
 
 void remove_job(int index) {
     if (index >= 0 && index < job_count) {
         for (int i = index; i < job_count - 1; i++) {
             jobs[i] = jobs[i + 1];
         }
         job_count--;
     }
 }
 
 void check_background_jobs() {
     int status;
     for (int i = 0; i < job_count;) {
         pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
         if (result > 0) {
             printf("[%d]+  Done\t\t%s\n", jobs[i].id, jobs[i].command);
             remove_job(i);
         } else {
             i++;
         }
     }
 }
 
 void handle_sigint(int sig) {
     if (foreground_pid > 0) {
         kill(foreground_pid, SIGINT);
     } else {
         write(STDOUT_FILENO, "\n", 1);
         print_prompt();
     }
 }
 
 void handle_sigtstp(int sig) {
     if (foreground_pid > 0) {
         kill(foreground_pid, SIGTSTP);
     } else {
         write(STDOUT_FILENO, "\n", 1);
         print_prompt();
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
         check_background_jobs();
 
         if (input == stdin) {
             print_prompt();
             fflush(stdout);
         }
 
         if (fgets(buffer, MAX_CMD_BUFFER, input) == NULL) break;
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
 
         // Parsing
         char *args[MAX_ARGS];
         int i = 0;
         char *token = strtok(buffer, " ");
         if (!token) continue;
 
         char *input_file = NULL;
         char *output_file = NULL;
         int background = 0;
 
         while (token != NULL && i < MAX_ARGS - 1) {
             if (strcmp(token, "<") == 0) {
                 token = strtok(NULL, " ");
                 if (token) input_file = token;
             } else if (strcmp(token, ">") == 0) {
                 token = strtok(NULL, " ");
                 if (token) output_file = token;
             } else if (strcmp(token, "&") == 0) {
                 background = 1;
             } else {
                 args[i++] = token;
             }
             token = strtok(NULL, " ");
         }
         args[i] = NULL;
         if (args[0] == NULL) continue;
 
         // Built-in commands
         if (strcmp(args[0], "exit") == 0) {
             int status = (args[1] != NULL) ? (atoi(args[1]) & 0xFF) : 0;
             printf("User has left the shell\n");
             if (input != stdin) fclose(input);
             exit(status);
         } else if (strcmp(args[0], "echo") == 0) {
             for (int j = 1; args[j] != NULL; j++) {
                 if (strcmp(args[j], "$?") == 0) {
                     printf("%d", last_exit_status);
                 } else {
                     printf("%s", args[j]);
                 }
                 if (args[j + 1] != NULL) printf(" ");
             }
             printf("\n");
             last_exit_status = 0;
         } else if (strcmp(args[0], "jobs") == 0) {
             for (int j = 0; j < job_count; j++) {
                 printf("[%d]%c  %s\t\t%s\n", jobs[j].id,
                        (j == job_count - 1) ? '+' : '-',
                        jobs[j].running ? "Running" : "Stopped",
                        jobs[j].command);
             }
         } else if (strcmp(args[0], "fg") == 0 && args[1]) {
             int job_id = atoi(args[1] + 1);
             int idx = find_job_index_by_id(job_id);
             if (idx >= 0) {
                 foreground_pid = jobs[idx].pid;
                 jobs[idx].running = 1;
                 kill(foreground_pid, SIGCONT);
                 printf("%s\n", jobs[idx].command);
                 int status;
                 waitpid(foreground_pid, &status, WUNTRACED);
                 if (WIFSTOPPED(status)) {
                     jobs[idx].running = 0;
                     printf("\n[%d]+  Stopped\t\t%s\n", jobs[idx].id, jobs[idx].command);
                 } else {
                     remove_job(idx);
                     last_exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
                 }
                 foreground_pid = -1;
             } else {
                 printf("No such job\n");
             }
         } else if (strcmp(args[0], "bg") == 0 && args[1]) {
             int job_id = atoi(args[1] + 1);
             int idx = find_job_index_by_id(job_id);
             if (idx >= 0) {
                 jobs[idx].running = 1;
                 kill(jobs[idx].pid, SIGCONT);
                 printf("[%d]+ %s &\n", jobs[idx].id, jobs[idx].command);
             } else {
                 printf("No such job\n");
             }
         } else {
             pid_t pid = fork();
             if (pid == 0) {
                 signal(SIGINT, SIG_DFL);
                 signal(SIGTSTP, SIG_DFL);
 
                 if (input_file) {
                     int fd_in = open(input_file, O_RDONLY);
                     if (fd_in < 0) {
                         perror("Input redirection failed");
                         exit(1);
                     }
                     dup2(fd_in, STDIN_FILENO);
                     close(fd_in);
                 }
 
                 if (output_file) {
                     int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                     if (fd_out < 0) {
                         perror("Output redirection failed");
                         exit(1);
                     }
                     dup2(fd_out, STDOUT_FILENO);
                     close(fd_out);
                 }
 
                 execvp(args[0], args);
                 fprintf(stderr, "bad command\n");
                 exit(1);
             } else if (pid > 0) {
                 if (background) {
                     add_job(pid, last_command, 1);
                 } else {
                     foreground_pid = pid;
                     int status;
                     waitpid(pid, &status, WUNTRACED);
                     if (WIFSTOPPED(status)) {
                         add_job(pid, last_command, 0);
                         printf("\n[%d]+  Stopped\t\t%s\n", jobs[job_count - 1].id, last_command);
                     } else {
                         last_exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
                     }
                     foreground_pid = -1;
                 }
             } else {
                 perror("fork failed");
                 last_exit_status = 1;
             }
         }
     }
 
     if (input != stdin) fclose(input);
     return 0;
 }
 