#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

# define READ_BUFFSIZE 2
# define TOKEN_BUFFSIZE 2
# define DELIMS "\t\n\r\a "

int run_cd(char **args);
int run_help(char **args);
int run_exit(char **args);

void load_configs() {

};

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &run_cd,
    &run_help,
    &run_exit
};

int builtin_num = sizeof(builtin_str) / sizeof(char *);
int run_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "expected arguments for cd\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("chdir error");
        }
    }
    return 1;
}

int run_help(char **args) {
    int i;

    printf("not implemented yet.\n");
    printf("all functions: \n");
    for (i = 0; i < builtin_num; i++) {
        printf("%s\n", builtin_str[i]);
    }
    return 1;
}

int run_exit(char **args) {
    return 0;
}

char *read_command() {
    int buffsize;
    int position;
    char c;
    char *buff;

    buffsize = READ_BUFFSIZE;
    position = 0;

    buff = (char *)malloc(sizeof(char) * buffsize);
    if (!buff) {
        fprintf(stderr, "malloc buff error");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if (position == buffsize) {
            buffsize += READ_BUFFSIZE;
            buff = (char *)realloc(buff, sizeof(char) * buffsize);
            if (!buff) {
                fprintf(stderr, "realloc buff error");
                exit(EXIT_FAILURE);
            }
        }
        c = getchar();
        if (c == EOF || c == '\n') {
            c = '\0';
            buff[position] = c;
            break;
        }
        buff[position++] = c;
    }
    return buff;
}

int run_exclude_command(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("run command failed");
        }
    } else if (pid < 0) {
        perror("fork failed");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int execute_command(char **args) {
    int i;
    if (args[0] == NULL) {
        fprintf(stderr, "expected a command");
    } else {
        for (i = 0; i < builtin_num; i++) {
            if (strcmp(builtin_str[i], args[0]) == 0) {
                return (*builtin_func[i])(args);
            }
        }
        return run_exclude_command(args);
    }
    return 1;
}

char **split_command(char *line) {
    char **tokens;
    char *token;
    int tkbsize;
    int position;
    
    tkbsize = TOKEN_BUFFSIZE;
    tokens = (char**)malloc(sizeof(char **) * tkbsize);

    position = 0;
    token = strtok(line, DELIMS);
    while (token != NULL) {
        if (position == tkbsize) {
            tkbsize += TOKEN_BUFFSIZE;
            tokens = (char **)realloc(tokens, tkbsize);

            if (!tokens) {
                fprintf(stderr, "realloc tokens error");
                exit(EXIT_FAILURE);
            }
        }
        tokens[position++] = token;
        token = strtok(NULL, DELIMS);
    }
    tokens[position] = (char *)NULL;
    return tokens;
}


void loop_until_exit() {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = read_command();
        args = split_command(line);
        status = execute_command(args);

        free(line);
        free(args);
    } while(status);
}

int main(int argc, char** argv) {
    load_configs();
    loop_until_exit();
    return EXIT_SUCCESS;
}