#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

int b_process_fcounter;
int b_process_counter;
bool f_running = false;
bool loop_entered = false;
char pwd[100];

/* Splits the string by space and returns the array of tokens
 *
 */
char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for (i = 0; i < strlen(line); i++)
	{

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t')
		{
			token[tokenIndex] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
		}
		else
		{
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL;
	return tokens;
}

void child_termination_handler()
{
	int child_t_status;
	int pid = waitpid(-1, &child_t_status, WNOHANG);
	if (pid > 0)
	{
		b_process_fcounter++;
		// printf("Shell : Backgroud Process execution finished\n");
	}
	while (pid > 0)
	{
		pid = waitpid(-1, &child_t_status, WNOHANG);
		if (pid > 0)
		{
			b_process_fcounter++;
			// printf("Shell : Backgroud Process execution finished\n");
		}
	}
}

int free_tokens(char **tokens)
{
	for (int i = 0; tokens[i] != NULL; i++)
	{
		free(tokens[i]);
	}
	free(tokens);
}

void ctrl_c_handler()
{

}

int main(int argc, char *argv[])
{
	char line[MAX_INPUT_SIZE];
	char **tokens;
	int i;
	int ppid = getppid();
	signal(SIGCHLD, child_termination_handler);
	signal(SIGINT, ctrl_c_handler);
	while (1)
	{
		loop_entered = true;
		/* BEGIN: TAKING INPUT */
		int no_of_args = 0;
		bool is_b_proc = false;

		bzero(line, sizeof(line));
		printf("%s ~$ ", getcwd(pwd, 100));
		scanf("%[^\n]", line);
		getchar();

		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; // terminate with new line
		tokens = tokenize(line);
		if (tokens[0] == NULL)
			continue;

		while (tokens[no_of_args++] != NULL)
			;
		no_of_args -= 2;
		if (!strcmp(tokens[0], "cd"))
		{
			if (tokens[1] == NULL)
			{
				int status = chdir(getenv("HOME"));
				if (status != 0)
					printf("File or Directory not found \n");
			}
			else
			{
				int status = chdir(tokens[1]);
				if (status != 0)
					printf("File or Directory not found \n");
			}
			free_tokens(tokens);
			continue;
		}
		if (!strcmp(tokens[0], "exit"))
		{
			killpg(getppid(), SIGINT);
			int kp_pid = waitpid(-1, NULL, WNOHANG);
			while (kp_pid != -1)
			{
				kp_pid = waitpid(-1, NULL, WNOHANG);
			}
			free_tokens(tokens);
			break;
		}
		while (b_process_fcounter > 0)
		{
			b_process_fcounter--;
			printf("Shell : Backgroud Process execution finished \n");
		}

		if (!strcmp(tokens[no_of_args], "&"))
		{
			is_b_proc = true;
			tokens[no_of_args] = NULL;
		}

		int f_process = fork();

		if (f_process < 0)
		{
			perror("fork");
		}
		else if (f_process == 0)
		{
			if (is_b_proc)
				setpgid(getpid(), ppid);
			execvp(tokens[0], tokens);
			printf("%s: command not found\n", tokens[0]);
			_exit(1);
		}
		if (!is_b_proc)
		{
			int t_process = waitpid(f_process, NULL, 0);
			// while (t_process != f_process)
			// {
			// 	printf("Shell : Backgroud Process execution finished\n");
			// 	// t_process = wait(NULL);
			// }
		}
		// do whatever you want with the commands, here we just print them

		// for (i = 0; tokens[i] != NULL; i++)
		// {
		// 	printf("found token %s (remove this debug output later)\n", tokens[i]);
		// }

		// Freeing the allocated memory
		free_tokens(tokens);
		loop_entered = false;
	}
	return 0;
}
