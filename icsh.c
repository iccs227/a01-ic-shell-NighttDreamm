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
    int last_exit_status = 0;
    FILE *input = stdin;

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
        if (fgets(buffer, MAX_CMD_BUFFER, input) == NULL) 
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
            last_exit_status = status;
            exit(status);
        
        } else if (strcmp(token, "echo") == 0) {
            token = strtok(NULL, " ");
            while (token != NULL) {
                if (strcmp(token, "$?") == 0) {
                    printf("%d", last_exit_status);
                } else {
                    printf("%s", token);
                }
                char *next = strtok(NULL, " ");
                if (next != NULL) {
                    printf(" ");
                }
                token = next;
            }
            printf("\n");
            last_exit_status = 0;
        } else {
            printf("bad command\n");
            last_exit_status = 1;
        }
    }
    if (input != stdin) fclose(input);
    return 0;
}