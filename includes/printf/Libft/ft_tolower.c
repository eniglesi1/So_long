/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_tolower.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eniglesi <eniglesi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/06 18:48:40 by eniglesi          #+#    #+#             */
/*   Updated: 2021/08/06 19:26:01 by eniglesi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

int	ft_tolower(char c)
{
	if (c > 64 && c < 91)
		return (c + 32);
	else
		return (c);
}
