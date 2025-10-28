/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_bonus.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eiglesia <eiglesia@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/02/16 21:46:34 by eiglesia          #+#    #+#             */
/*   Updated: 2025/07/01 17:54:20 by eiglesia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line_bonus.h"

static char	*ft_clean(char *string, char *extra, int len, int u)
{
	int	i;
	int	j;

	i = 0;
	j = 0;
	if (string == NULL)
		return (NULL);
	if (u == 1)
		return (free(string), NULL);
	if (extra)
	{
		while (extra[i] != '\n' && extra[i])
			i++;
		while ((&extra[i + 1])[j] != '\0')
			j++;
		if (extra[i] == '\n')
		{
			ft_strlcpy(extra, &extra[i + 1], j + 1);
			string[len] = '\n';
		}
		else if (!extra[i])
			extra[0] = 0;
	}
	return (ft_realloc(string, 0, ft_is_line(string, 0)));
}

static char	*get_line(int fd, char *extra)
{
	char			*string;
	int				baits;
	int				len;
	int				cap;

	cap = BUFFER_SIZE;
	if (extra && ft_is_line(extra, 1) != -1)
		string = ft_realloc(NULL, 0, ft_is_line(extra, 1) + 2);
	else
		string = ft_realloc(NULL, 0, BUFFER_SIZE + 1);
	len = ft_copynl(extra, BUFFER_SIZE, string, 0);
	if (string == NULL || extra[len])
		return (ft_clean(string, extra, len, 0));
	while (ft_is_line(extra, 1) == -1)
	{
		baits = leer(fd, extra);
		if (baits == 0 || baits == -1)
			return (ft_clean(string, extra, len, (!string[0] || baits == -1)));
		if (len + baits > cap)
			string = ft_realloc(string, baits, (cap = cap * 2));
		if (string == NULL || extra[0] == 0)
			return (ft_clean(string, extra, len, 1));
		len = ft_copynl(extra, baits, string, len);
	}
	return (ft_clean(string, extra, len, 0));
}

char	*get_next_line(int fd)
{
	static char	extra[OPEN_MAX][BUFFER_SIZE + 1];

	if (fd > OPEN_MAX || fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	return (get_line(fd, extra[fd]));
}
