#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

typedef struct s_micro
{
	int		nb_args;
	char	**args;
	int		fd_in;
	int		fd_out;
	struct s_micro	*next;
} t_micro;

void	ft_putchar_fd(char c, int fd)
{
	write(fd, &c, 1);
}

void	ft_putstr_fd(char *str, int fd)
{
	int	i;

	i = 0;
	while (str[i])
		ft_putchar_fd(str[i++], fd);
}

void	ft_putendl_fd(char *str, int fd)
{
	ft_putstr_fd(str, fd);
	ft_putchar_fd('\n', fd);
}

t_micro	*ft_init(void)
{
	t_micro	*micro;

	micro = (t_micro *)malloc(sizeof(t_micro));
	micro->args = (char **)malloc(sizeof(char *) * 1024);
	micro->fd_in = 0;
	micro->fd_out = 1;
	micro->next = NULL;
	return (micro);
}

void	ft_read(int *idx, int ac, char **av, t_micro *micro)
{
	int	i;

	i = 0;
	while (*idx < ac && strcmp("|", av[*idx]) && strcmp(";", av[*idx]))
	{
		micro->args[i] = av[*idx];
		i++;
		(*idx)++;
	}
	micro->args[i] = NULL;
	micro->nb_args = i;
}

void	ft_pipe(int *idx, t_micro *micro)
{
	int	fd[2];

	(*idx)++;
	if (pipe(fd))
	{
		ft_putendl_fd("error: fatal", 2);
		exit (1);
	}
	micro->fd_out = fd[1];
	micro->next = ft_init();
	micro->next->fd_in = fd[0];
}

void	ft_exec(t_micro *micro, char **env)
{
	pid_t	child;

	if (!strcmp("cd", micro->args[0]))
	{
		if (micro->nb_args != 2 || !strcmp("-", micro->args[1]))
			ft_putendl_fd("error: cd: bad arguments", 2);
		else if (chdir(micro->args[1]))
		{
			ft_putstr_fd("error: cd: cannot change directory to", 2);
			ft_putendl_fd(micro->args[1], 2);
		}
		return ;
	}
	child = fork();
	if (child == -1)
	{
		ft_putendl_fd("error: fatal", 2);
		exit (1);
	}
	if (!child)
	{
		if (micro->fd_in != 0)
		{
			dup2(micro->fd_in, 0);
			close(micro->fd_in);
		}
		if (micro->fd_out != 1)
		{
			close(micro->next->fd_in);
			dup2(micro->fd_out, 1);
			close(micro->fd_out);
		}
		if (execve(micro->args[0], micro->args, env) == -1)
		{
			ft_putstr_fd("error: cannot execute ", 2);
			ft_putendl_fd(micro->args[0], 2);
			return ;
		}
	}
	if (micro->fd_out == 1)
	{
		waitpid(child, NULL, 0);
		while (waitpid(-1, NULL, 0) != -1);
	}
}

void	ft_free(t_micro *micro)
{
	if (micro->fd_in != 0)
		close(micro->fd_in);
	if (micro->fd_out != 1)
		close(micro->fd_out);
	free(micro->args);
	micro->args = NULL;
	free(micro);
	micro = NULL;
}

int	main(int ac, char **av, char **env)
{
	int		i;
	t_micro *tmp;
	t_micro *micro;

	micro = NULL;
	i = 1;
	while (i < ac)
	{
		if (!strcmp(";", av[i]) && i++)
			continue ;
		if (!micro)
			micro = ft_init();
		ft_read(&i, ac, av, micro);
		if (i < ac && !strcmp("|", av[i]))
			ft_pipe(&i, micro);
		ft_exec(micro, env);
		tmp = micro;
		micro = micro->next;
		ft_free(tmp);
	}
	return (0);
}
