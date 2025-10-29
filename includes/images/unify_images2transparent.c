#include "../printf/ft_printf.h"
#include <fcntl.h>

int main(int argc, char **argv)
{
	char *line;
	char *line2;

	if (argc == 3)
	{
		int fd = open(argv[1], O_RDONLY);
		int fd2 = open(argv[2], O_RDONLY);
		int i = 0;
		int j = 0;
		line = get_next_line(fd);
		line2 = get_next_line(fd2);
		while (line && line2)
		{
			while (line[i]!= '\0')
			{
				if (j > 21 && line[i] == ' ')
					write(1, &(line2[i]), 1);
				else
					write(1, &(line[i]), 1);
				i++;
			}
			i = 0;
			free(line);
			free(line2);
			line = get_next_line(fd);
			line2 = get_next_line(fd2);
			j++;
		}
		close(fd);
		close(fd2);
	}
	return (0);
}
