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

static void	blake(int n, int fd)
{
	if (n < 0)
	{
		n = n * -1;
		write(fd, "-", 1);
	}
	if (n >= 10)
		blake(n / 10, fd);
	ft_putchar_fd(n % 10 + 48, fd);
}

void	ft_putnbr_fd(int n, int fd)
{
	if (n != 0 && n != -2147483648)
		blake(n, fd);
	else if (n == -2147483648)
		write(fd, "-2147483648", 11);
	else
		write(fd, "0", 1);
}
