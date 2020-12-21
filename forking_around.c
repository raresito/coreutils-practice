#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>

char *get_next_start(char *str, char delim, int terminate) {
    char *p;
    p = strchr(str, delim);
    if (p != NULL) {
        if (terminate) {
            /* only nul-terminate the string on the second pass */
            *p = '\0';
        }
        p += 1;
    }
    return p;
}

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
    
    char *string_list;

    for(int program_step = 0; program_step < program_count; program_step++){
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
            printf("This is process_id: %d", procces_id);
            close(pipefd[0]); /* Close unuser read end */
            dup2(pipefd[1], STDOUT_FILENO); /* Redirect childs stdout to parent pipe */
            /* Execute child process */

            /* Different behaviour based on different programs */
            if(program_step == 0){
                char *arguments[] = { cmdv[0], NULL };
                execvp(cmdv[0], arguments);
            }
            else {
                char *arguments[] = { cmdv[1], string_list};
                execv(cmdv[1], arguments);
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
            for (int i = 0; i < strlen(str); i++)
            {
                if (str[i] == '\n') str[i] = ' ';
            }

            char *p;
            size_t i;

            /* count the files that will be concatenated... */
            i = 0;
            for (p = str; p != NULL; p = get_next_start(p, ' ', 0)) {
                i += 1;
            }
            printf("items: %zd\n", i);

            string_list = malloc(sizeof(char*) * i);

            /* populate the table */
            i = 0;
            for (p = str; p != NULL; p = get_next_start(p, ' ', 1)) {
                string_list[i] = p; /* store the next item... */
                i += 1;
            }
            string_list[i] = NULL; /* terminate the list with NULL */
        }

        if(program_step = program_count - 1){
            final_str = str;
        }
    }
    printf("%s", final_str);

    return 0;
}