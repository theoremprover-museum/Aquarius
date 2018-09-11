/* Penguin */

/*
 *         Copyright (C) Argonne National Laboratory
 *
 *   Argonne does not guarantee this software in any manner and is
 *   not responsible for any damages that may result from its use.
 *   Furthermore, Argonne does not provide any formal support for this
 *   software.  This is an experimental program.  This software
 *   or any part of it may be freely copied and redistributed,
 *   provided that this paragraph is included in each source file.
 *
 */

/*
 *
 *    is.c -- This file contains routines for discrimination tree
 *    indexing for forward subsumption.
 *
 */

#include "header.h"

/*************
 *
 *    static int insert_is_tree(t, is, iit)
 *
 *	struct is_tree * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct is_tree through the
 *	parameter iit.
 *
 *************/

static int insert_is_tree(t, is, iit)
struct term *t;
struct is_tree *is;
struct is_tree **iit;
{
    struct rel *r;
    struct is_tree *i1, *prev, *i3;
    int varnum, sym;
	struct is_tree *temp;

	*iit = NULL;				/* default */

    if (t->type == VARIABLE) {
	i1 = is->u.kids;
	prev = NULL;
	varnum = t->varnum;
	while (i1 != NULL && i1->type == VARIABLE && i1->lab < varnum) {
	    prev = i1;
	    i1 = i1->next;
	    }
	if (i1 == NULL || i1->type != VARIABLE || i1->lab != varnum) {
	    if (get_is_tree(&i3) == TROUBLE)
		return(TROUBLE);
	    i3->type = VARIABLE;
	    i3->lab = t->varnum;
	    i3->next = i1;
	    if (prev == NULL)
		is->u.kids = i3;
	    else
		prev->next = i3;
		*iit = i3;
	    return(NO_TROUBLE);
	    }
	else  /* found node */
		{
		*iit = i1;
	    return(NO_TROUBLE);
		}
	}

    else {  /* NAME || COMPLEX */
	i1 = is->u.kids;
	prev = NULL;
	/* arities fixed: handle both NAME and COMPLEX */
	sym = t->sym_num;
	while (i1 != NULL && i1->type == VARIABLE) {  /* skip variables */
	    prev = i1;
	    i1 = i1->next;
	    }
	while (i1 != NULL && i1->lab < sym) {
	    prev = i1;
	    i1 = i1->next;
	    }
	if (i1 == NULL || i1->lab != sym) {
	    if (get_is_tree(&i3) == TROUBLE)
		return(TROUBLE);
	    i3->type = t->type;
	    i3->lab = sym;
	    i3->next = i1;
	    i1 = i3;
	    }
	else
	    i3 = NULL;  /* new node not required at this level */

	if (t->type == COMPLEX && t->sym_num != Ignore_sym_num) {
	    r = t->farg;
	    while (r != NULL) {
		if (insert_is_tree(r->argval, i1, &temp) == TROUBLE)
			return(TROUBLE);
		i1 = temp;
		r = r->narg;
		}
	    }
        if (i3 != NULL)  /* link in new subtree (possibly a leaf) */
	    if (prev == NULL)
		is->u.kids = i3;
	    else
		prev->next = i3;
	    
	*iit = i1;
	return(NO_TROUBLE);  /* i1 is leaf corresp. to end of input term */
	}
}  /* insert_is_tree */

/*************
 *
 *    int is_insert(t, root_is)
 *
 *    Insert a term into the discrimination tree index for
 *    forward subsumption.  (for finding more general terms)
 *
 *************/

int is_insert(t, root_is)
struct term *t;
struct is_tree *root_is;
{
    struct is_tree *i1;
    struct term_ptr *tp;

    if (insert_is_tree(t, root_is, &i1) == TROUBLE)
	return(TROUBLE);
    if (get_term_ptr(&tp) == TROUBLE)
	return(TROUBLE);
    tp->term = t;
    tp->next = i1->u.terms;
    i1->u.terms = tp;
return(NO_TROUBLE);
}  /* is_insert */

/*************
 *
 *    int end_term_is(t, is, path_p, eti)
 *
 *    Given a discrimination tree (or a subtree) and a term, return the 
 *    node in the tree that corresponds to the last symbol in t (or NULL
 *    if the node doesn't exist).  *path_p is a list that is extended by
 *    this routine.  It is a list of pointers to the
 *    nodes in path from the parent of the returned node up to imd. 
 *    (It is needed for deletions, because nodes do not have pointers to
 *    parents.) 
 *
 *	struct is_tree * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct is_tree through tha
 *	parameter eti.
 *
 *************/

static int end_term_is(t, is, path_p, eti)
struct term *t;
struct is_tree *is;
struct term_ptr **path_p;
struct is_tree **eti;
{
    struct rel *r;
    struct is_tree *i1;
    struct term_ptr *isp;
    int varnum, sym;
	struct is_tree *temp;

	*eti = NULL;				/* default */
    /* add current node to the front of the path list. */

    if (get_term_ptr(&isp) == TROUBLE)
	return(TROUBLE);
    isp->term = (struct term *) is;
    isp->next = *path_p;
    *path_p = isp;

    if (t->type == VARIABLE) {
	i1 = is->u.kids;
	varnum = t->varnum;
	while (i1 != NULL && i1->type == VARIABLE && i1->lab < varnum) 
	    i1 = i1->next;

	if (i1 == NULL || i1->type != VARIABLE || i1->lab != varnum)
	    return(NO_TROUBLE);
	else   /* found node */
		{
		*eti = i1;
	    return(NO_TROUBLE);
		}
	}

    else {  /* NAME || COMPLEX */
	i1 = is->u.kids;
	sym = t->sym_num;  /* arities fixed: handle both NAME and COMPLEX */
	while (i1 != NULL && i1->type == VARIABLE)  /* skip variables */
	    i1 = i1->next;
	while (i1 != NULL && i1->lab < sym)
	    i1 = i1->next;

	if (i1 == NULL || i1->lab != sym)
	    return(NO_TROUBLE);
	else {
	    if (t->type == NAME || t->sym_num == Ignore_sym_num)
		{
		*eti = i1;
		return(NO_TROUBLE);
		}
	    else {
		r = t->farg;
		while (r != NULL && i1 != NULL) {
		    if (end_term_is(r->argval, i1, path_p, &temp) == TROUBLE)
			return(TROUBLE);
			i1 = temp;
		    r = r->narg;
		    }
		*eti = i1;
		return(NO_TROUBLE);
		}
	    }
	}
}  /* end_term_is */

/*************
 *
 *    int is_delete(t, root_is)
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int is_delete(t, root_is)
struct term *t;
struct is_tree *root_is;
{
    struct is_tree *end, *i2, *i3, *parent;
    struct term_ptr *tp1, *tp2;
    struct term_ptr *isp1, *path;

    /* First find the correct leaf.  path is used to help with  */
    /* freeing nodes, because nodes don't have parent pointers. */

    path = NULL;
    if (end_term_is(t, root_is, &path, &end) == TROUBLE)
	return(TROUBLE);
    if (end == NULL) {
	output_stats(Fdout, 4);
	fprintf(Fderr, "ABEND, is_delete, can't find end.\007\n");
	fprintf(Fdout, "ABEND, is_delete, can't find end: ");
	print_term_nl(Fdout, t);
	return(TROUBLE);
	}

    /* Free the pointer in the leaf-list */

    tp1 = end->u.terms;
    tp2 = NULL;
    while(tp1 != NULL && tp1->term != t) {
	tp2 = tp1;
	tp1 = tp1->next;
	}
    if (tp1 == NULL) {
	output_stats(Fdout, 4);
	fprintf(Fdout, "ABEND, is_delete, can't find term.\007\n");
	fprintf(Fdout, "ABEND, is_delete, can't find term: ");
	print_term_nl(Fdout, t);
	return(TROUBLE);
	}
    if (tp2 == NULL)
	end->u.terms = tp1->next;
    else
	tp2->next = tp1->next;
    free_term_ptr(tp1);

    if (end->u.terms == NULL) {
        /* free tree nodes from bottom up, using path to get parents */
	end->u.kids = NULL;  /* probably not necessary */
	isp1 = path;
	while (end->u.kids == NULL && end != root_is) {
	    parent = (struct is_tree *) isp1->term;
	    isp1 = isp1->next;
	    i2 = parent->u.kids;
	    i3 = NULL;
	    while (i2 != end) {
		i3 = i2;
		i2 = i2->next;
		}
	    if (i3 == NULL)
		parent->u.kids = i2->next;
	    else
		i3->next = i2->next;
#ifdef ROO
	    add_to_time_node_list((void *) i2, IS_TREE);
#else
	    free_is_tree(i2);
#endif
	    end = parent;
	    }
	}

    /* free path list */

    while (path != NULL) {
	isp1 = path;
	path = path->next;
	free_term_ptr(isp1);
	}
return(NO_TROUBLE);
}  /* is_delete */
    
/*************
 *
 *    int is_retrieve(term, subst, index_tree, position, irtptr)
 *
 *        Return the first or next list of terms that subsumes `term'.
 *    Also return the substitution.  Return NULL if there are
 *    none or no more.  All terms in the returned list of terms
 *    are identical.
 *
 *    if (term != NULL)
 *        {This is the first call, so return the first, and also
 *        return a position for subsequent calls}
 *    else if (position != NULL)
 *        {return the next term, and update the position}
 *    else 
 *        {there are no more terms that subsume}
 *
 *    If you don't want the entire set of subsuming terms, then
 *    cancel the position with `free_is_pos_list(position)'.
 *
 *	struct term_ptr * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term_ptr through the
 *	parameter irtptr.
 *
 *************/

int is_retrieve(t, subst, is, is_pos, irtptr)
struct term *t;
struct context *subst;
struct is_tree *is;
struct is_pos **is_pos;
struct term_ptr **irtptr;
{
    struct rel *rel_stack[MAX_FS_TERM_DEPTH];
    struct is_tree *i1;
    struct is_pos *pos, *ip2;
    int top, found, backup, varnum, j, reset, sym;

    if (t != NULL) {  /* first call */
	pos = NULL;
	top = -1;
	i1 = is->u.kids;
	backup = 0;
	}
    else if (*is_pos != NULL) {  /* continuation with more to try */
	pos = *is_pos;  /* must remember to set is_pos on return */
	backup = 1;
	}
    else  /* continuation with nothing more to try */
	{
	*irtptr = NULL;
	return(NO_TROUBLE);
	}

    while (1) {  /* loop until a leaf is found or done with tree */
	if (backup) {
	    if (pos == NULL)
		{
		*irtptr = NULL;
		return(NO_TROUBLE);
		}
	    else {  /* pop top of stack (most recent variable node)
		       and restore state */
		top = pos->stack_pos;
		for (j = 0; j <= top; j++)
		    rel_stack[j] = pos->rel_stack[j];
		i1 = pos->is;
		t = subst->terms[i1->lab];
		if (pos->reset)  /* undo variable binding */
		    subst->terms[i1->lab] = NULL;
		i1 = i1->next;
		ip2 = pos;
		pos = pos->next;
		free_is_pos(ip2);
		}
	    }

	/* at this point, i1 is the next node to try */

	found = 0;
	/* first try to match input term t with a variable node */
	while (found == 0 && i1 != NULL && i1->type == VARIABLE) {
	    varnum = i1->lab;
	    if (subst->terms[varnum] == NULL) { /*if not bound, bind it */
		subst->terms[varnum] = t;
		found = 1;
		reset = 1;
		}
	    else {  /* bound variable, succeed iff identical */
		found = term_ident(subst->terms[varnum], t);
		reset = 0;
		}

	    if (found) {  /* save state */
		if (get_is_pos(&ip2) == TROUBLE)
			{
			*irtptr = NULL;
			return(TROUBLE);
			}
		ip2->next = pos;
		pos = ip2;
		pos->is = i1;
		pos->reset = reset;
		for (j = 0; j <= top; j++)
		    pos->rel_stack[j] = rel_stack[j];
		pos->stack_pos = top;
		}	/* end of save state */
	    else  /* try next variable */
		i1 = i1->next;
	    }

	backup = 0;
	if (found == 0) {  /* couldn't match t with (another) variable */
	    if (t->type == VARIABLE)
		backup = 1;  /* because we can't instantiate given term */
	    else {  /* NAME or COMPLEX */
		sym = t->sym_num;
		while (i1 != NULL && i1->lab < sym)
		    i1 = i1->next;
		if (i1 == NULL || i1->lab != sym)
		    backup = 1;
		else if (t->type == COMPLEX && t->sym_num != Ignore_sym_num) {
		    top++;
		    if (top >= MAX_FS_TERM_DEPTH) {
			output_stats(Fdout, 4);
	fprintf(Fdout, "ABEND, is_retrieve, increase MAX_FS_TERM_DEPTH.\007\n");
	fprintf(Fdout, "ABEND, is_retrieve, increase MAX_FS_TERM_DEPTH.\n");
			*irtptr = NULL;
			return(TROUBLE);
			}
		    rel_stack[top] = t->farg;  /* save pointer to subterms */
		    }
	        } 
	    }

	if (backup == 0) {  /* get next term from rel_stack */
	    while (top >= 0 && rel_stack[top] == NULL)
		top--;
	    if (top == -1) {  /* found a term */
		*is_pos = pos;
		*irtptr = i1->u.terms;
		return(NO_TROUBLE);
		}
	    else {  /* pop a term and continue */
		t = rel_stack[top]->argval;
		rel_stack[top] = rel_stack[top]->narg;
		i1 = i1->u.kids;
		}
	    }
	}  /* end of while(1) loop */

}  /* is_retrieve */

/*************
 *
 *    int fs_retrieve(t,  c, is, fs_pos, frtptr)
 *
 *    Get the first or next term that subsumes t.   (t != NULL)
 *    for first call, and (t == NULL) for subsequent calls.
 *
 *    If you want to stop calls before a NULL is returned, please
 *    call canc_fs_pos(fs_pos, context).
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter frtptr.
 *
 *************/

int fs_retrieve(t, subst, is, fs_pos, frtptr)
struct term *t;
struct context *subst;
struct is_tree *is;
struct fsub_pos **fs_pos;
struct term **frtptr;
{
    struct term_ptr *tp;
    struct is_pos *i_pos;
    struct fsub_pos *f_pos;

    if (t != NULL) {  /* if first call */
	if (is_retrieve(t, subst, is, &i_pos, &tp) == TROUBLE)
		{
		*frtptr = NULL;
		return(TROUBLE);
		}
	if (tp == NULL)
		{
		*frtptr = NULL;
	    return(NO_TROUBLE);
		}
	else {	/* tp not NULL */
	    if (get_fsub_pos(&f_pos) == TROUBLE)
		{
		*frtptr = NULL;
		return(TROUBLE);
		}
	    f_pos->pos = i_pos;
	    f_pos->terms = tp;
	    *fs_pos = f_pos;
	    *frtptr = tp->term;
		return(NO_TROUBLE);
	    }	/* end of tp not NULL */
	}	/* end of if first call */
    else {  /* subsequent call */
	f_pos = *fs_pos;
	tp = f_pos->terms->next;
	if (tp != NULL) {  /* if any more terms in current leaf */
	    f_pos->terms = tp;
		*frtptr = tp->term;
	    return(NO_TROUBLE);
	    }
	else {  /* try for another leaf */
if (is_retrieve((struct term *) NULL, subst, is, &(f_pos->pos), &tp) == TROUBLE)
		{
		*frtptr = NULL;
		return(TROUBLE);
		}
	    if (tp == NULL) {
		free_fsub_pos(f_pos);
		*frtptr = NULL;
		return(NO_TROUBLE);
		}
	    else {
		f_pos->terms = tp;
		*frtptr = tp->term;
		return(NO_TROUBLE);
		}
	    }	/* end of else try for another leaf */
	}	/* end of else subsequent call */
}  /* fs_retrieve */

/*************
 *
 *    canc_fs_pos(pos, subst)
 *
 *************/

void canc_fs_pos(pos, subst)
struct fsub_pos *pos;
struct context *subst;
{
    int i;

    if (pos->pos != NULL) {
	for (i = 0; i < MAX_VARS; i++)
	    subst->terms[i] = NULL;
	}

    free_is_pos_list(pos->pos);
    free_fsub_pos(pos);
}  /* canc_fs_pos */

/*************
 *
 *    print_is_tree(fp, is)
 *
 *        Display an index-subsumption tree.
 *
 *************/

void print_is_tree(fp, is)
FILE *fp;
struct is_tree *is;
{
    fprintf(fp, "don't know how to print is tree %x\n", is);
}  /* print_is_tree */

/*************
 *
 *    p_is_tree(is)
 *
 *************/

void p_is_tree(is)
struct is_tree *is;
{
    print_is_tree(Fdout, is);
}  /* p_is_tree */

