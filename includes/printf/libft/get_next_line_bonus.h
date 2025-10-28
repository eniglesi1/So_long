/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_bonus.h                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eiglesia <eiglesia@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/02/16 21:47:56 by eiglesia          #+#    #+#             */
/*   Updated: 2025/07/01 17:54:24 by eiglesia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_NEXT_LINE_BONUS_H
# define GET_NEXT_LINE_BONUS_H
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <stddef.h>
# include <stdio.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 42
# endif

# ifndef OPEN_MAX
#  define OPEN_MAX 128
# endif

char	*get_next_line(int fd);
char	*ft_realloc(char *s1, int i, int len);
size_t	ft_strlcpy(char *dst, const char *src, size_t size);
int		ft_copynl(char *extra, int baits, char *string, int aux);
int		leer(int fd, char *extra);
int		ft_is_line(char *extra, int len);

#endif
