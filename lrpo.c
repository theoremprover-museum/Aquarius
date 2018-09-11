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
 *  lrpo.c -- Lexicograpphic recursive path ordering (RPO with status)
 *
 */

#include "header.h"

/*************
 *
 *    int sym_precedence(sym_num_1, sym_num_2)
 *
 *    Return SAME_AS, GREATER_THAN, LESS_THAN, or NOT_COMPARABLE.
 *
 *************/

static int sym_precedence(sym_num_1, sym_num_2)
int sym_num_1;
int sym_num_2;
{
    int p1, p2;

    if (sym_num_1 == sym_num_2)
	return(SAME_AS);
    else {
	p1 = sn_to_node(sym_num_1)->lex_val;
	p2 = sn_to_node(sym_num_2)->lex_val;
	
	if (p1 == MAX_INT || p2 == MAX_INT)
	    return(NOT_COMPARABLE);
	else if (p1 > p2)
	    return(GREATER_THAN);
	else if (p1 < p2)
	    return(LESS_THAN);
	else
	    return(SAME_AS);
	}
}  /* sym_precedence */

/*************
 *
 *    int lrpo_status(sym_num)
 *
 *************/

static int lrpo_status(sym_num)
int sym_num;
{

    return(sn_to_node(sym_num)->lex_rpo_status);

}  /* lrpo_status */

/*************
 *
 *    struct rel *reverse_args(r1, r2) 
 *
 *    Reverse arguments of t (in place, don't copy).
 *
 *************/

static struct rel *reverse_args(r1, r2)
struct rel *r1;
struct rel *r2;
{
    struct rel *r3;

    if (r1 == NULL)
	return(r2);
    else {
	r3 = r1->narg;
	r1->narg = r2;
	return(reverse_args(r3,r1));
	}
}  /* reverse_args */

/*************
 *
 *    void reverse_rl(t)
 *
 *    Reverse arguments of each functor with rl status.  Recursive.
 *
 *************/

static void reverse_rl(t)
struct term *t;
{
    struct rel *r;

    if (t->type == COMPLEX) {
	for (r = t->farg; r; r = r->narg)
	    reverse_rl(r->argval);
	if (lrpo_status(t->sym_num) == LRPO_RL_STATUS)
	    t->farg = reverse_args(t->farg, (struct rel *) NULL);
	}
}  /* reverse_rl */

/*************
 *
 *    int lrpo_lex(t1, t2, ll) -- Is t1 > t2 ?
 *
 *    t1 and t2 have same functor and the functor has lr status.
 *    (If a functor really has rl status, its args have already been
 *    reversed.  This is true of all subterms of t1 and t2.)
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE and 0/1 through the 
 *	parameter ll.
 *
 *************/

static int lrpo_lex(t1, t2, ll)
struct term *t1;
struct term *t2;
int *ll;
{
    struct rel *r1, *r2;
    int rc;
	int tempint;

	*ll = 0;				/* default */

    /* First skip over any identical arguments. */
    /* (Same number of args, because same functor.) */

    for (r1 = t1->farg, r2 = t2->farg;
	 r1 != NULL && term_ident(r1->argval, r2->argval);
	 r1 = r1->narg, r2 = r2->narg) /* empty body */ ;

    if (r1 == NULL)
	rc = 0;  /* t1 and t2 identical */
    else {
	if (lrpo(r1->argval, r2->argval, &tempint) == TROUBLE)
		return(TROUBLE);
	if (tempint)
	{
	/* return (t1 > each remaining arg of t2) */
	r2 = r2->narg;
	while (r2 != NULL)
	{
	if (lrpo(t1, r2->argval, &tempint) == TROUBLE)
		return(TROUBLE);
	if (!tempint)
		break;
/* So that if lrpo fails, it does not advance the pointer.	*/
	r2 = r2->narg;
	}	/* end of while */
	rc = (r2 == NULL);
	}
    	else {
	/* return (there is a remaining arg of t1 s.t. arg == t2 or arg > t2) */
	rc = 0;
	for (r1 = r1->narg; r1 != NULL; r1 = r1->narg) {
	    	if (term_ident(r1->argval, t2))
			rc = 1;
		if (lrpo(r1->argval, t2, &tempint) == TROUBLE)
			return(TROUBLE);
		if (tempint)
			rc = 1;
	    }
	}
	}
	*ll = rc;
    return(NO_TROUBLE);

}  /* lrpo_lex */

/*************
 *
 *    int num_occurrences(t_arg, t) -- How many times does t_arg occur
 *    as an argument of t?
 *
 *************/

static int num_occurrences(t_arg, t)
struct term *t_arg;
struct term *t;
{
    struct rel *r;
    int i;

    for (i = 0, r = t->farg; r != NULL; r = r->narg)
       if (term_ident(r->argval, t_arg))
	   i++;

    return(i);

}  /* num_occurrences */

/*************
 *
 *   int set_multiset_diff(t1, t2, smd)
 *
 *   Construct the multiset difference, then return the set of that.
 *   Result must be deallocated by caller with zap_term.
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter smd.
 *
 *************/

static int set_multiset_diff(t1, t2, smd)
struct term *t1;
struct term *t2;
struct term **smd;
{
    struct term *t_result;
    struct rel *prev, *curr, *r1, *r;
    int i;
	struct term *tempterm;

	*smd = NULL;				/* default */
    if (get_term(&t_result) == TROUBLE)
	return(TROUBLE);
    prev = NULL;
    i = 0;
    for (r1 = t1->farg; r1 != NULL; r1 = r1->narg) {
	/* First check if a preceeding occurrence of this arg has */
	/* already been done. */

for (r=t1->farg; r!=r1 && term_ident(r->argval,r1->argval)==0; r=r->narg)
	    /* empty body */ ;

if (r==r1 && num_occurrences(r1->argval,t1) > num_occurrences(r1->argval,t2))
	{
	    i++;
	    if (get_rel(&curr) == TROUBLE)
		return(TROUBLE);
	    if (copy_term(r1->argval, &tempterm) == TROUBLE)
		return(TROUBLE);
		curr->argval = tempterm;
	    if (prev == NULL)
	        t_result->farg = curr;
	    else
                prev->narg = curr;
	    prev = curr;
	    }
	}
	    
    t_result->type = (i == 0 ? NAME : COMPLEX) ;
    /* note that no sym_num is assigned; this should be ok */
	*smd = t_result;
    return(NO_TROUBLE);

}  /* set_multiset_diff */

/*************
 *
 *   int lrpo_multiset(t1, t2, lm) -- Is t1 > t2 in the lrpo multiset ordering?
 *
 *   t1 and t2 have functors with the same precedence.
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE, 0/1 through the parameter lm.
 *
 *************/

static int lrpo_multiset(t1, t2, lm)
struct term *t1;
struct term *t2;
int *lm;
{
    struct term *s1, *s2;
    struct rel *r1, *r2;
    int ok;

	*lm = 0;				/* default */
    if (set_multiset_diff(t1, t2, &s1) == TROUBLE)
	return(TROUBLE);
	 /* builds and returns a set */
    if (set_multiset_diff(t2, t1, &s2) == TROUBLE)
	return(TROUBLE);
	 /* builds and returns a set */

    /*
    return (s2 not empty and foreach arg a2 of s2
	    there is an arg a1 of s1 such that lrpo(a1, a2)).
    */

    if (s2->farg == NULL)
       ok = 0;
    else {
	for (r2 = s2->farg, ok = 1; r2 != NULL && ok; r2 = r2->narg) {
	    for (r1 = s1->farg, ok = 0; r1 != NULL && ok == 0; r1 = r1->narg)
		if (lrpo(r1->argval, r2->argval, &ok) == TROUBLE)
			return(TROUBLE);
	    }
	}

    zap_term(s1);
    zap_term(s2);

	*lm = ok;
    return(NO_TROUBLE);

}  /* lrpo_multiset */

/*************
 *
 *   int lrpo(t1, t2, lr) - Is t1 > t2 in the lexicographic recursive
 *                      path ordering?
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE and 0/1 through the
 *	parameter lr.
 *
 *************/

int lrpo(t1, t2, lr)
struct term *t1;
struct term *t2;
int *lr;
{
    int p;
    struct rel *r;
	int tempint;

	*lr = 0;				/* default */
    if (t1->type == VARIABLE)
	/* variable never greater than anything */
	return(NO_TROUBLE);  
    else if (t2->type == VARIABLE)
	/* t1 > variable iff t1 properly contains that variable */
	{
	*lr = occurs_in(t2, t1);
	return(NO_TROUBLE);
	}
    else if (t1->sym_num == t2->sym_num &&
             lrpo_status(t1->sym_num) != LRPO_MULTISET_STATUS)
	{
	if (lrpo_lex(t1, t2, &tempint) == TROUBLE)
		return(TROUBLE);
	*lr = tempint;
	return(NO_TROUBLE);
	}
    else {
	p = sym_precedence(t1->sym_num, t2->sym_num);
	if (p == SAME_AS)
	{
		if (lrpo_multiset(t1,t2,&tempint) == TROUBLE)
			return(TROUBLE);
		*lr = tempint;
		return(NO_TROUBLE);
	}
	else if (p == GREATER_THAN) {
	    /* return (t1 > each arg of t2) */
		r = t2->farg;
		while (r != NULL)
		{
		if (lrpo(t1, r->argval, &tempint) == TROUBLE)
			return(TROUBLE);
		if (!tempint)
			break;
/* So that if lrpo() fails, it does not advance the pointer. */
		r = r->narg;
		}	/* end of while */
		

		*lr = (r == NULL);
		return(NO_TROUBLE);
	    }
	else {  /* LESS_THAN or NOT_COMPARABLE */
	    /* return (there is an arg of t1 s.t. arg == t2 or arg > t2) */
	    for (r = t1->farg; r != NULL; r = r->narg) 
		{
		if (term_ident(r->argval, t2))
			{
			*lr = 1;
		    return(NO_TROUBLE);
			}
		if (lrpo(r->argval, t2, &tempint) == TROUBLE)
			return(TROUBLE);
		if (tempint)
			{
			*lr = 1;
			return(NO_TROUBLE);
			}
		}	/* end of for */
		*lr = 0;
	    return(NO_TROUBLE);
	    }
	}
}  /* lrpo */

/*************
 *
 *   int lrpo_greater(t1, t2, lg) - Is t1 > t2 in the lexicographic
 *                              recursive path ordering?
 *
 *    Time this routine.
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE, 0/1 through the parameter lg.
 *
 *************/

int lrpo_greater(t1, t2, lg)
struct term *t1;
struct term *t2;
int *lg;
{
    int rc;

	*lg = 0;				/* default */
    CLOCK_START(LRPO_TIME)
    /* Reverse args of all functors with rl status. */
    reverse_rl(t1);
    reverse_rl(t2);
    if (lrpo(t1,t2,&rc) == TROUBLE)
	return(TROUBLE);
    /* UN-reverse args of all functors with rl status. */
    reverse_rl(t1);
    reverse_rl(t2);
    CLOCK_STOP(LRPO_TIME)
	*lg = rc;
    return(NO_TROUBLE);

}  /* lrpo_greater */

/*************
 *
 *    int order_equalities_lrpo(c,should)
 *   
 *    For each equality literal (pos or neg), flip args if the right
 *    side is heavier.
 *
 *    return:
 *
 *      0 - should not be a demodulator
 *      1 - should be a demodulator
 *      2 - should be a conditional demodulator
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE and 0/1/2 through the
 *	parameter should.
 *
 *************/

int order_equalities_lrpo(c, should)
struct clause *c;
int *should;
{
    struct rel *r1, *r2;
    struct term *alpha, *beta;
    struct literal *l;
    int first_bigger, second_bigger, numlit;

	*should = 0;				/* default */
    alpha = NULL;

    for (l = c->first_lit, numlit = 0; l != NULL; l = l->next_lit) {
	numlit++;
	if (l->atom->varnum == POS_EQ || l->atom->varnum == NEG_EQ) {
	    r1 = l->atom->farg;
	    r2 = r1->narg;
	    alpha = r1->argval;
	    beta  = r2->argval;

            if (lrpo_greater(alpha, beta, &first_bigger) == TROUBLE)
		return(TROUBLE);
	    if (first_bigger)
		second_bigger = 0;
	    else {
		if (lrpo_greater(beta, alpha, &second_bigger) == TROUBLE)
			return(TROUBLE);
		if (second_bigger) {
		    r1->argval = beta;
		    r2->argval = alpha;
		    }
		}
	    }
	}
    

    /* following could be improved, but I think it is correct. */
    if (numlit != 1 || alpha == NULL || term_ident(alpha, beta))
	*should = 0;
    else if (first_bigger || second_bigger)
	*should = 1;  /* it can be an unconditional demodulator */
    else if (Flags[DYNAMIC_DEMOD_LEX_DEP].val &&
	     var_subset(r2->argval, r1->argval))
	*should = 2;  /* it can be a conditional demodulator */
    else
	*should = 0;
return(NO_TROUBLE);
}  /* order_equalities_lrpo */

/*************
 *
 *   int make_clause_into_term(c, mcit)		Penguin only
 *
 *   Construct a term with the atoms of the clause as subterms.
 *   Result must be deallocated by caller with zap_term.
 *	It returns TROUBLE/NO_TROUBLE and the pointer to struct term
 *	through the parameter mcit.
 *
 *************/

static int make_clause_into_term(c, mcit)
struct clause *c;
struct term **mcit;
{
    struct term *t_result, *t;
    struct literal *lit;
    struct rel *prev, *curr;
    int i;

	*mcit = NULL;				/* default */
    if (get_term(&t_result) == TROUBLE)
	return(TROUBLE);
    prev = NULL;
    i = 0;
    for (lit = c->first_lit; lit != NULL; lit = lit->next_lit)
	{
	    i++;
	    if (get_rel(&curr) == TROUBLE)
		return(TROUBLE);
	    if (copy_term(lit->atom, &t) == TROUBLE)
		return(TROUBLE);
      	    reverse_rl(t);
    /* Reverse args of all functors with rl status. */
	    curr->argval = t;
	    if (prev == NULL)
	        t_result->farg = curr;
	    else
                prev->narg = curr;
	    prev = curr;
	}
	    
    t_result->type = (i == 0 ? NAME : COMPLEX) ;
    /* note that no sym_num is assigned; this should be ok */
	*mcit = t_result;
    return(NO_TROUBLE);

}  /* make_clause_into_term */

/*******************
*
*	int greater_cl(c1,c2,gc)		Penguin
*
*	It returns TROUBLE/NO_TROUBLE, it returns 1 if c1 > c2, 0 otherwise,
*	through the parameter gc.
*
*******************/

int greater_cl(c1,c2,gc)
struct clause *c1, *c2;
int *gc;
{
struct term *t1, *t2;
int r;

	*gc = 0;				/* default */
if (make_clause_into_term(c1, &t1) == TROUBLE)
	return(TROUBLE);
if (make_clause_into_term(c2, &t2) == TROUBLE)
	return(TROUBLE);
/* It returns a term whose root has no sym_num and whose children are the */
/* atoms, i.e. the literals stripped of sign, in the clause given as 	  */
/* parameter. Also, it has already called reverse_rl() to reverse all args*/
/* of functors with rl status in the term of the clause.		  */

if (lrpo_multiset(t1,t2, &r) == TROUBLE)
	return(TROUBLE);

/* It compares the two as two multisets of terms, so that the two clauses  */
/* are compared by comparing the multisets of their atoms.		   */

zap_term(t1);
zap_term(t2);
	*gc = r;
return(NO_TROUBLE);
} /* greater_cl() */
