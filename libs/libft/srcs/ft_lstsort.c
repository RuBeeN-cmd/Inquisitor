#include "list.h"

void	ft_lstsort(t_list *lst, int (*cmp)(void *, void *))
{
	t_list	*i;
	t_list	*j;
	void	*temp;

	if (!lst || !cmp)
		return ;
	i = lst;
	while (i)
	{
		j = i->next;
		while (j)
		{
			if (cmp(i->content, j->content) > 0)
			{
				temp = i->content;
				i->content = j->content;
				j->content = temp;
			}
			j = j->next;
		}
		i = i->next;
	}
}
