#ifndef H_LISTEDOUBLE
#define H_LISTEDOUBLE

struct node;
typedef struct dlist Dlist;
Dlist *dlist_new(void);
Dlist *dlist_append(Dlist *p_list, int data);
Dlist *dlist_prepend(Dlist *p_list, int data);
Dlist *dlist_insert(Dlist *p_list, int data, int position);
void dlist_delete(Dlist **p_list);
void dlist_display(Dlist *p_list);
Dlist *dlist_remove(Dlist *p_list, int data);
size_t dlist_length(Dlist *p_list);
#endif /* not H_LISTEDOUBLE */
