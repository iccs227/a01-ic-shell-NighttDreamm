/* ICCS227: Project 1: icsh
 * Name: Kemanut Kanthasaksiri
 * StudentID: 6580088
 */

#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#define MAX_CMD_BUFFER 255

int main(int argc, char *argv[]) {
    char buffer[MAX_CMD_BUFFER];
    char last_command[MAX_CMD_BUFFER] = "";
    printf("Initializing IC Shell\n");
    while (1) {
        printf("icsh $ ");
        fflush(stdout);
        if (fgets(buffer, MAX_CMD_BUFFER, stdin) == NULL) 
        {
            break;
        }
        buffer[strcspn(buffer, "\n")] = '\0';
        if(strcmp(buffer, "!!") == 0)
        {
            if (strlen(last_command) == 0) 
            {
                printf("No commands in history.\n");
                continue;
            }
            strcpy(buffer, last_command);
            printf("%s\n", buffer);
        }
            else
            {
            strcpy(last_command,buffer);
            }

        char *token = strtok(buffer, " ");

        if(token ==  NULL)
        {
            continue;
        }

        if (strcmp(token, "exit") == 0) {
            char *exit_arg = strtok(NULL, " ");
            int status = 0;
        
            if (exit_arg != NULL) {
                status = atoi(exit_arg) & 0xFF;
            }
        
            printf("User has left the shell\n");
            exit(status);
        
        } else if (strcmp(token, "echo") == 0) {
            token = strtok(NULL, " ");
            while (token != NULL) {
                printf("%s", token);
                char *next = strtok(NULL, " ");
                if (next != NULL) {
                    printf(" ");
                }
                token = next;
            }
            printf("\n");
        
        } else {
            printf("bad command\n");
        }
        
        // printf("you said that : %s\n", buffer);
    }

}