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
 *  imd.c -- This file contains routines for discrimination
 *  tree indexing for demodulation.
 *
 */

#include "header.h"

/*************
 *
 *    static int insert_imd_tree(t, imd, iit)  --  called by imd_insert
 *
 *	struct imd_tree * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct imd_tree through the
 *	parameter iit.
 *
 *************/

static int insert_imd_tree(t, imd, iit)
struct term *t;
struct imd_tree *imd;
struct imd_tree **iit;
{
    struct rel *r;
    struct imd_tree *i1, *i2, *i3;
    int varnum, sym;
	struct imd_tree *temp;

	*iit = NULL;				/* default */
	
    if (t->type == VARIABLE) {
	i1 = imd->kids;
	i2 = NULL;
	varnum = t->varnum;
	while (i1 != NULL && i1->type == VARIABLE && i1->lab < varnum) {
	    i2 = i1;
	    i1 = i1->next;
	    }
	if (i1 == NULL || i1->type != VARIABLE || i1->lab != varnum) {
	    if (get_imd_tree(&i3) == TROUBLE)
		return(TROUBLE);
	    i3->type = VARIABLE;
	    i3->lab = varnum;
	    i3->next = i1;
	    if (i2 == NULL)
		imd->kids = i3;
	    else
		i2->next = i3;
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
	i1 = imd->kids;
	i2 = NULL;
	sym = t->sym_num;  /* arities fixed: handle both NAME and COMPLEX */
	while (i1 != NULL && i1->type == VARIABLE) {  /* skip variables */
	    i2 = i1;
	    i1 = i1->next;
	    }
	while (i1 != NULL && i1->lab < sym) {
	    i2 = i1;
	    i1 = i1->next;
	    }
	if (i1 == NULL || i1->lab != sym) {
	    if (get_imd_tree(&i3) == TROUBLE)
		return(TROUBLE);
	    i3->type = t->type;
	    i3->lab = sym;
	    i3->next = i1;
	    i1 = i3;
	    }
	else
	    i3 = NULL;  /* new node not required at this level */

	if (t->type == COMPLEX) {
	    r = t->farg;
	    while (r != NULL) {
		if (insert_imd_tree(r->argval, i1, &temp) == TROUBLE)
			return(TROUBLE);
		i1 = temp;
		r = r->narg;
		}
	    }
	if (i3 != NULL)  /* link in new subtree (possibly a leaf) */
	    if (i2 == NULL)
		imd->kids = i3;
	    else
		i2->next = i3;

	*iit = i1;
	return(NO_TROUBLE);  /* i1 is leaf corresp. to end of input term */
	}
}  /* insert_imd_tree */

/*************
 *
 *    int imd_insert(demod, imd)
 *
 *    Insert the left argument of demod into the  discrimination
 *    tree index for demodulation.
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int imd_insert(demod, imd)
struct clause *demod;
struct imd_tree *imd;
{
    struct imd_tree *i1;
    struct term *atom, *alpha, *beta;
    struct term_ptr *tp;
    int max;

    atom = demod->first_lit->atom;
    if (atom->varnum != CONDITIONAL_DEMOD) {
	alpha = atom->farg->argval;
	beta = atom->farg->narg->argval;
	}
    else {  /* CONDITIONAL(cond, alpha, beta) */
	alpha = atom->farg->narg->argval;
	beta = atom->farg->narg->narg->argval;
	}

    if (term_ident(alpha, beta)) {
fprintf(Fderr,"\nWARNING, instance of x=x cannot be inserted into demod_imd index: ");
	print_clause(Fderr, demod);
fprintf(Fdout,"\nWARNING, instance of x=x cannot be inserted into demod_imd index: ");
	print_clause(Fdout, demod);
	}
    else {
	if (insert_imd_tree(alpha, imd, &i1) == TROUBLE)
		return(TROUBLE);
	if (get_term_ptr(&tp) == TROUBLE)
		return(TROUBLE);
	tp->term = atom;
	tp->next = i1->atoms;
	if ((max = biggest_var(alpha)) == -1)
	    i1->max_vnum = 0;  /* in case i->max_vnum is an unsigned char */
	else
	    i1->max_vnum = max;
	i1->atoms = tp;
	   
	}
return(NO_TROUBLE);
}  /* imd_insert */

/*************
 *
 *    int end_term_imd(t, imd, path_p, eti)
 *
 *    Given a discrimination tree (or a subtree) and a term, return the
 *    node in the tree that corresponds to the last symbol in t (or NULL
 *    if the node doesn't exist).  *path_p is a list that is extended by
 *    this routine.  It is a list of pointers to the
 *    nodes in path from the parent of the returned node up to imd.
 *    (It is needed for deletions, because nodes do not have pointers to
 *    parents.)
 *
 *	struct imd_tree * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct imd_tree through the
 *	parameter eti.
 *
 *************/

static int end_term_imd(t, imd, path_p, eti)
struct term *t;
struct imd_tree *imd;
struct term_ptr **path_p;
struct imd_tree **eti;
{
    struct rel *r;
    struct imd_tree *i1;
    struct term_ptr *imdp;
    int varnum, sym;
    struct imd_tree *temptree;

	*eti = NULL;				/* default */

    /* add current node to the front of the path list. */

    if (get_term_ptr(&imdp) == TROUBLE)
	return(TROUBLE);
    imdp->term = (struct term *) imd;
    imdp->next = *path_p;
    *path_p = imdp;

    if (t->type == VARIABLE) {
	i1 = imd->kids;
	varnum = t->varnum;
	while (i1 != NULL && i1->type == VARIABLE && i1->lab < varnum) 
	    i1 = i1->next;

	if (i1 == NULL || i1->type != VARIABLE || i1->lab != varnum)
	{
		*eti = NULL;
	    return(NO_TROUBLE);
	}
	else   /* found node */
	{
		*eti = i1;
	    return(NO_TROUBLE);
	}
	}

    else {  /* NAME || COMPLEX */
	i1 = imd->kids;
	sym = t->sym_num;  /* arities fixed: handle both NAME and COMPLEX */
	while (i1 != NULL && i1->type == VARIABLE)  /* skip variables */
	    i1 = i1->next;
	while (i1 != NULL && i1->lab < sym)
	    i1 = i1->next;

	if (i1 == NULL || i1->lab != sym)
	{
		*eti = NULL;
	    return(NO_TROUBLE);
	}
	else {
	    if (t->type == NAME)
		{
		*eti = i1;
		return(NO_TROUBLE);
		}
	    else {
		r = t->farg;
		while (r != NULL && i1 != NULL) {
		    if (end_term_imd(r->argval,i1,path_p,&temptree) == TROUBLE)
			return(TROUBLE);
			i1 = temptree;
		    r = r->narg;
		    }
		*eti = i1;
		return(NO_TROUBLE);
		}
	    }
	}
}  /* end_term_imd */

/*************
 *
 *    int imd_delete(demod, root_imd)
 *
 *  Delete the left argument of demod from the demodulation discrimination tree.
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int imd_delete(demod, root_imd)
struct clause *demod;
struct imd_tree *root_imd;
{
    struct imd_tree *end, *i2, *i3, *parent;
    struct term_ptr *tp1, *tp2;
    struct term_ptr *imdp, *path;
    struct term *atom, *alpha;

    /* First find the correct leaf.  path is used to help with  */
    /* freeing nodes, because nodes don't have parent pointers. */

    path = NULL;
    atom = demod->first_lit->atom;

    if (atom->varnum == CONDITIONAL_DEMOD)
	alpha = atom->farg->narg->argval;
    else
	alpha = atom->farg->argval;

    if (end_term_imd(alpha, root_imd, &path, &end) == TROUBLE)
	return(TROUBLE);

    if (end == NULL) {
	output_stats(Fdout, 4);
	fprintf(Fderr, "ABEND, imd_delete, can't find alpha.\007\n");
	fprintf(Fdout, "ABEND, imd_delete, can't find alpha: ");
	print_term_nl(Fdout, alpha);
	return(TROUBLE);
	}

    tp1 = end->atoms;
    tp2 = NULL;
    while (tp1 != NULL && tp1->term != atom) {
	tp2 = tp1;
	tp1 = tp1->next;
	}

    if (tp1 == NULL) {
	output_stats(Fdout, 4);
	fprintf(Fderr, "ABEND, imd_delete, can't find atom.\007\n");
	fprintf(Fdout, "ABEND, imd_delete, can't find atom: ");
	print_term_nl(Fdout, atom);
	return(TROUBLE);
	}

    if (tp2 == NULL)
	end->atoms = tp1->next;
    else
	tp2->next = tp1->next;
    free_term_ptr(tp1);

    if (end->atoms == NULL) {
	/* free tree nodes from bottom up, using path to get parents */
	imdp = path;
	while (end->kids == NULL && end != root_imd) {
	    parent = (struct imd_tree *) imdp->term;
	    imdp = imdp->next;
	    i2 = parent->kids;
	    i3 = NULL;
	    while (i2 != end) {
		i3 = i2;
		i2 = i2->next;
		}
	    if (i3 == NULL)
		parent->kids = i2->next;
	    else
		i3->next = i2->next;
#ifdef ROO
	    add_to_time_node_list((void *) i2, IMD_TREE);
#else
	    free_imd_tree(i2);
#endif
	    end = parent;
	    }
	}

    /* free path list */

    while (path != NULL) {
	imdp = path;
	path = path->next;
	free_term_ptr(imdp);
	}
return(NO_TROUBLE);
}  /* imd_delete */

/*************
 *
 *    int contract_imd(t_in, demods, subst, demod_id_p,ci)
 *
 *    Attempt to contract (rewrite one step) a term (t_in) using demodulators
 *    in a discrimination tree index (demods).  NULL is returned if t_in
 *    cannot be contracted.  subst is an empty substitution.
 *    If success, *demod_id_p is set to the ID of the rewrite rule.
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter ci.
 *
 *************/

int contract_imd(t_in, demods, subst, demod_id_p,ci)
struct term *t_in;
int *demods;
struct context *subst;
int *demod_id_p;
struct term **ci;
{
    struct rel *rel_stack[MAX_AL_TERM_DEPTH];
    struct imd_tree *imd, *i1;
    struct imd_pos *pos, *ip2;
    struct term *t, *t2, *t3, *atom, *replacement;
    struct term_ptr *tp;
    int top, found, backup, varnum, j, reset, mult_flag, sym, ok, dummy;

	*ci = NULL;				/* default */
    imd = (struct imd_tree *) demods;
    if (imd == NULL)
	return(NO_TROUBLE);
    pos = NULL;
    top = -1;
    backup = 0;
    i1 = imd->kids;
    t = t_in;

    while(1) {
	if (backup) {
	    if (pos == NULL)
		{
		*ci = NULL;
		return(NO_TROUBLE);
		}
	    else {  /* pop top of stack (most recent variable node)
		       and restore state */
		top = pos->stack_pos;
		for (j = 0; j <= top; j++)
		    rel_stack[j] = pos->rel_stack[j];
		i1 = pos->imd;
		t = subst->terms[i1->lab];
		if (pos->reset)  /* undo variable binding */
		    subst->terms[i1->lab] = NULL;
		i1 = i1->next;
		ip2 = pos;
		pos = pos->next;
		free_imd_pos(ip2);
		}	/* end of pop top of stack and restore state */
	    }	/* end of if backup */

	/* at this point, i1 is the next node to try */

	found = 0;
	/* first try to match input term t with a variable node */
	while (found == 0 && i1 != NULL && i1->type == VARIABLE) {
	    varnum = i1->lab;
	    if (subst->terms[varnum] == NULL) { /*if not bound, bind it */
		subst->terms[varnum] = t;
		found = 1;
		reset = 1;
		}	/* end of if not bound bind it */
	    else {  /* bound variable, succeed iff identical */
		found = term_ident(subst->terms[varnum], t);
		reset = 0;
		}	/* end of bound variable, succeed iff identical */

	    if (found) {  /* save state */
		if (get_imd_pos(&ip2) == TROUBLE)
			return(TROUBLE);
		ip2->next = pos;
		pos = ip2;
		pos->imd = i1;
		pos->reset = reset;
		for (j = 0; j <= top; j++)
		    pos->rel_stack[j] = rel_stack[j];
		pos->stack_pos = top;
		}	/* end of save state */
	    else  /* try next variable */
		i1 = i1->next;
	    }	/* end of while */

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
		else if (t->type == COMPLEX) {	/* else if COMPLEX */
		    top++;
		    if (top >= MAX_AL_TERM_DEPTH) {
			output_stats(Fdout, 4);
fprintf(Fderr, "ABEND, contract_imd, increase MAX_AL_TERM_DEPTH.\007\n");
	fprintf(Fdout, "ABEND, contract_imd, increase MAX_AL_TERM_DEPTH.\n");
			return(TROUBLE);
			}
		    rel_stack[top] = t->farg;  /* save pointer to subterms */
		    }	/* end of else if COMPLEX */
		}	/* end of NAME or COMPLEX */
	    }	/* end of couldn't match t with (another) variable */

	if (backup == 0) {  /* get next term from rel_stack */
	    while (top >= 0 && rel_stack[top] == NULL)
		top--;

	    if (top == -1) {  /* found potential demods */
		tp = i1->atoms;
		ok = 0;
		while(tp != NULL && ok == 0) {
		    atom = tp->term;
		    mult_flag = 0;
		    if (atom->varnum == LEX_DEP_DEMOD)
			{	/* if LEX_DEP_DEMOD */
if (apply_demod(atom->farg->narg->argval,subst,&mult_flag,&replacement)==TROUBLE)
		return(TROUBLE);
			if (Flags[LEX_RPO].val)
				{
			    if (lrpo_greater(t_in,replacement,&ok) == TROUBLE)
				return(TROUBLE);
				}
			else
			    ok = (lex_check(replacement, t_in) == LESS_THAN);
			if (ok == 0) {
			    zap_term_special(replacement);
			    tp = tp->next;
			    	}
			}	/* end of if LEX_DEP_DEMOD */
		    else if (atom->varnum == CONDITIONAL_DEMOD)
			{	/* else if CONDITIONAL_DEMOD */
			/* apply subst to condition, then demodulate */
	if (apply_demod(atom->farg->argval, subst, &dummy,&t2)==TROUBLE)
			return(TROUBLE);
			if (convenient_demod(t2, &t3) == TROUBLE)
				return(TROUBLE);
	ok = (t3->type == NAME && str_ident(sn_to_str(t3->sym_num),"$T"));
			zap_term_special(t3);
			if (ok)
			{
if (apply_demod(atom->farg->narg->narg->argval,subst,&mult_flag,&replacement)==TROUBLE)
			return(TROUBLE);
			}
			else
			    tp = tp->next;
			}	/* end of else if CONDITIONAL_DEMOD */
		    else {  /* regular demodulator */
if (apply_demod(atom->farg->narg->argval,subst,&mult_flag,&replacement)==TROUBLE)
			return(TROUBLE);
			ok = 1;
			}	/* end of regular demodulator */
		     
		    }	/* end of while */
		    
		if (ok) {
		    if (mult_flag)
			subst->multiplier++;
		    for (j = 0; j <= i1->max_vnum; j++) /* clear substitution */
			subst->terms[j] = NULL;
		    free_imd_pos_list(pos);
		    zap_term_special(t_in);
		    *demod_id_p = tp->term->occ.lit->container->id;
			*ci = replacement;
		    return(NO_TROUBLE);
		    }
		else  /* failed lex_checks, so prepare to back up */
		    backup = 1;
		}	/* end of found potential demods */

	    else {  /* pop a term and continue */
		t = rel_stack[top]->argval;
		rel_stack[top] = rel_stack[top]->narg;
		i1 = i1->kids;
		}	/* end of pop a term and continue */
	    }	/* end of get next term from rel_stack */
	}  /* end of while(1) loop */

}  /* contract_imd */

/*************
 *
 *    print_imd_tree(file_pointer, imd_tree, level)
 *
 *        Display an imd tree.  Level == 0 on initial call.
 *
 *************/

void print_imd_tree(fp, imd, level)
FILE *fp;
struct imd_tree *imd;
int level;
{
    struct imd_tree *i1;
    int i;

    fprintf(fp, "%x ", imd);
    for (i = 0; i < level; i++)
	fprintf(fp, "  ");
    if (imd->type == 0)
	fprintf(fp, "start of index-match-demodulate tree");
    else if (imd->type == VARIABLE)
	fprintf(fp, "v%d ", imd->lab);
    else
	fprintf(fp, "%s ", sn_to_str((int) imd->lab));

    if (imd->atoms != NULL) {
	fprintf(fp, " demod=");
	print_term(fp, imd->atoms->term);
	}
    fprintf(fp, "\n");

    i1 = imd->kids;
    while (i1 != NULL) {
	print_imd_tree(fp, i1, level + 1);
	i1 = i1->next;
	}

}  /* print_imd_tree */

/*************
 *
 *    p_imd_tree(imd_tree)
 *
 *        Display an imd tree.  Level == 0 on initial call.
 *
 *************/

void p_imd_tree(imd)
struct imd_tree *imd;
{
    print_imd_tree(Fdout, imd, 0);
}  /* p_imd_tree */

