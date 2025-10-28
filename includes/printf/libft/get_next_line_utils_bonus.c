/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils_bonus.c                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eiglesia <eiglesia@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/02/16 21:49:20 by eiglesia          #+#    #+#             */
/*   Updated: 2025/07/01 17:54:11 by eiglesia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line_bonus.h"
//37.45/31.26
//06.87/5.55
//29.47T/26.31NT
size_t	ft_strlcpy(char *dst, const char *src, size_t size)
{
	size_t	a;
	size_t	i;

	i = 0;
	a = -1;
	while (src[i] != '\0')
		i++;
	if (!dst || !src)
		return (0);
	if (size == 0)
		return (i);
	while (++a < (size - 1) && src[a] != '\0')
		dst[a] = src[a];
	dst[a] = '\0';
	return (i);
}

char	*ft_realloc(char *s1, int j, int len)
{
	char	*a;
	int		i;
	int		n;

	i = len + j;
	if (i == 0)
		return (free(s1), NULL);
	a = malloc(sizeof(char) * i + 1);
	if (!a)
		return (free(s1), NULL);
	n = sizeof(char) * i + 1;
	while (n > 0)
		((char *)a)[--n] = '\0';
	if (s1)
		return (ft_strlcpy(a, s1, (i - j) + 1), free(s1), a);
	return (a);
}

int	ft_copynl(char *extra, int baits, char *string, int len)
{
	int	j;

	j = 0;
	if (string == NULL)
		return (0);
	while (extra[j] != '\n' && j < baits && extra[j])
	{
		string[len] = extra[j];
		j++;
		len++;
	}
	return (len);
}

int	leer(int fd, char *extra)
{
	int	baits;

	baits = read(fd, extra, BUFFER_SIZE);
	if (baits != -1)
		extra[baits] = 0;
	else
		extra[0] = 0;
	return (baits);
}

int	ft_is_line(char *extra, int len)
{
	int	i;

	i = 0;
	while ((len == 0 || extra[i] != '\n') && extra[i])
		i++;
	if (extra[i] == '\n' || len == 0)
		return (i);
	return (-1);
}
