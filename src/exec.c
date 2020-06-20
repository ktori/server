#include "exec.h"
#include "vhttpsl/bits/str.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define P_READ 0
#define P_WRITE 1

char **
kv_to_env(struct kv_list_s *kv)
{
	char **env;
	int size, i;
	struct kv_node_s *node;

	size = 1;
	node = kv->head;
	while (node != NULL)
	{
		++size;
		node = node->next;
	}

	env = calloc(size, sizeof(const char *));

	i = 0;
	node = kv->head;
	while (node != NULL)
	{
		env[i] = calloc(fmtlen("%s=%s", node->key, node->value) + 1, 1);
		sprintf(env[i], "%s=%s", node->key, node->value);
		++i;
		node = node->next;
	}

	return env;
}

char **
kv_to_args(struct kv_list_s *kv)
{
	char **args_;
	int size, i;
	struct kv_node_s *node;

	size = 1;
	node = kv->head;
	while (node != NULL)
	{
		++size;
		node = node->next;
	}

	args_ = calloc(size, sizeof(const char *));

	i = 0;
	node = kv->head;
	while (node != NULL)
	{
		args_[i] = calloc(strlen(node->key) + 1, 1);
		strcpy(args_[i], node->key);
		++i;
		node = node->next;
	}

	return args_;
}

int
pexec(const char *path,
	  const char **args,
	  const char **env,
	  const char *input,
	  int input_length,
	  char **output,
	  int *output_length)
{
	int pipes_cp[2];
	int pipes_pc[2];
	int pid;
	int out;
	int out_bufsize;
	char *buf;
	int written;
	int tmp;
	int retval;
	int status;

	out = 0;
	out_bufsize = 200;
	written = 0;

	if (pipe(pipes_cp) < 0 || pipe(pipes_pc) < 0)
	{
		return -1;
	}

	signal(SIGCHLD, SIG_DFL);

	pid = fork();
	if (pid < 0)
	{
		return -2;
	}
	else if (pid == 0)
	{
		dup2(pipes_cp[P_WRITE], 1);
		dup2(pipes_pc[P_READ], 0);

		close(pipes_cp[P_READ]);
		close(pipes_pc[P_WRITE]);

		retval = execve(path, (char *const *) args, (char *const *) env);
		exit(retval);
	}
	else
	{
		close(pipes_cp[P_WRITE]);
		close(pipes_pc[P_READ]);

		buf = calloc(out_bufsize, 1);

		while (written < input_length)
		{
			tmp = write(pipes_pc[P_WRITE], input + written, input_length - written);
			if (tmp < 0)
			{
				break;
			}
			written += tmp;
		}
		close(pipes_pc[P_WRITE]);

		tmp = read(pipes_cp[P_READ], buf, out_bufsize);
		while (tmp > 0)
		{
			out += tmp;

			if (out_bufsize <= out)
			{
				out_bufsize *= 2;
				buf = realloc(buf, out_bufsize);
			}

			tmp = read(pipes_cp[P_READ], buf + out, out_bufsize - out);
		}
		/* waitpid(pid, NULL, 0); */
		*output = buf;
		*output_length = out;

		do
		{
			int w = waitpid(pid, &status, 0);
			if (w < 0)
			{
				perror("waitpid");
				return EXIT_FAILURE;
			}
		}
		while (!WIFEXITED(status) && !WIFSIGNALED(status));

		fprintf(stderr, "cgi process exited with code %d\n", WEXITSTATUS(status));

		if (WEXITSTATUS(status) != 0)
		{
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}

	return -3;
}