/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_fd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/18 13:11:22 by marvin            #+#    #+#             */
/*   Updated: 2021/09/18 13:11:22 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	blake(int n, int fd)
{
	int	i;

	i = 0;
	if (n < 0)
	{
		n = n * -1;
		write(fd, "-", 1);
	}
	if (n >= 10)
		i = blake(n / 10, fd);
	return (i + ft_putchar_fd_r(n % 10 + 48, fd));
}

int	ft_putnbr_fd_r(int n, int fd)
{
	if (n > 0)
		return (blake(n, fd));
	else if (n != 0 && n != -2147483648)
		return (blake(n, fd) + 1);
	else if (n == -2147483648)
		return (write(fd, "-2147483648", 11));
	else
		return (write(fd, "0", 1));
}
