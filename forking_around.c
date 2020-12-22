#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>

int main(){

    //Get programs that will be run
    char *arrayOfPrograms[10];
    int program_count = 2;
    char program1[5] = "ls";
    char program2[5] = "cat";
    char *final_str;
    

    // Create an array of the size of the number of commands that are 
    char** cmdv = (char**) malloc (program_count * sizeof(char*));
    cmdv[0] = program1;
    cmdv[1] = program2;
    
    //I know, i have to make this 20 dynamic, bare with me.
    char *string_list[20];

    for(int program_step = 0; program_step < program_count; program_step++){
        printf("This is the program_step at beginning : %d\n", program_step);
        // printf("This is strings_list: %s\n", string_list);
        int pipefd[2];
        int status;
        int bytes_read;
        const int BUF_LEN = 256;
        char buf[BUF_LEN];

        /* Open communication pipe */
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        int procces_id = fork();
        if(procces_id == 0){
            // printf("This is process_id: %d", procces_id);
            close(pipefd[0]); /* Close unuser read end */
             /* Redirect childs stdout to parent pipe */
            // dup2(pipefd[2], STDERR_FILENO); /* Redirect childs stderr to parent */
            /* Execute child process */

            /* Different behaviour based on different programs */
            if(program_step == 0){
                // dup2(pipefd[1], STDOUT_FILENO);
                char *arguments[] = { cmdv[0], NULL };
                execvp(cmdv[0], arguments);
            }
            else {
                // printf("%s%s%s\n", string_list[0], string_list[1], string_list[2]);
                // char *arguments[] = { cmdv[1], string_list};
                execv(cmdv[1], string_list);
            }
            

            /* only if exec failed */
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]); /* Close unused write end */

        /* wait for child process to finish */
        waitpid(procces_id, &status, 0);
        if (WIFEXITED(status))
            printf("Child %d terminated normally, with code %d\n",
                    procces_id, WEXITSTATUS(status));

        // close(pipefd[1]);

        char *str;
        str = malloc(1);
        while(bytes_read = read(pipefd[0], &buf, BUF_LEN) > 0){
            char *tempstr = malloc(strlen(buf) + strlen(str) + 1);
            strcpy(tempstr, str);
            strcat(tempstr, buf);
            str = tempstr;
        }

        

        /* Prepare the result of ls as the arguments for cat */
        if(program_step == 0) {
            //Replace EOL with space and count spaces
            int space_count = 0;
            for (int i = 0; i < strlen(str); i++)
            {
                if (str[i] == '\n')
                {
                    str[i] = ' ';
                    space_count++;
                }
            }
            
            int file_name_max_size = 400; 
            //Allocate memory to string_list
            for (int i = 0; i < space_count; i++){
                string_list[i] = (char *) malloc(sizeof(char) * file_name_max_size);
            }
            
            char modifiable_str[strlen(str)];
            strcpy(modifiable_str, str);

            // Set string delimitators
            char delim[] = " ";

            char *ptr = strtok(modifiable_str, delim);

            int i = 0;
            while(ptr != NULL)
            {
                // printf("'%s'\n", ptr);
                string_list[i] = ptr;
                ptr = strtok(NULL, delim);
                i = i + 1;
            }
            string_list[i-1]=NULL;
            
        }

        // printf("%s", string_list);

        if(program_step == program_count - 1){
            final_str = str;
        }
        printf("This is the program_step at end : %d \n", program_step);

    }
    // printf("%s", final_str);

    return 0;
}