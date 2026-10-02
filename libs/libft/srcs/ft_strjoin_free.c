/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin_free.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rrollin <rrollin@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/31 14:40:37 by rrollin           #+#    #+#             */
/*   Updated: 2022/04/04 13:52:51 by rrollin          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strjoin_free(char const *s1, char const *s2, int to_free)
{
	char	*str = ft_strjoin(s1, s2);
	if (to_free & 1)
		free((char *)s1);
	if (to_free & 2)
		free((char *)s2);
	return (str);
}
