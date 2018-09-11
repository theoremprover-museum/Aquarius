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
 *  paramod.c -- Paramodulation inference rules.
 *
 */

#include "header.h"

/*************
 *
 *    int apply_substitute(t, into_term, into_subst, beta, from_subst, as)
 *
 *    This routine is similar to apply, except that when it reaches the into
 *    term, the appropriate instance of beta is returned.
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter as.
 *
 *************/

static int apply_substitute(t, into_term, into_subst, beta, from_subst, as)
struct term *t;
struct term *into_term;
struct context *into_subst;
struct term *beta;
struct context *from_subst;
struct term **as;
{
    struct term *t2;
    struct rel *r1, *r2, *r3;
	struct term *temp;

	*as = NULL;				/* default */
    if (t == into_term)
	{
	 if (apply(beta, from_subst, &temp) == TROUBLE)
		return(TROUBLE);
	*as = temp;
	return(NO_TROUBLE);
	}
    else if (t->type != COMPLEX) {
	if (Flags[PARA_ALL].val == 0) {
	    output_stats(Fdout, 4);
	    fprintf(Fderr, "ABEND, apply_subst, t not COMPLEX.\007\n");
	    fprintf(Fdout, "ABEND, apply_subst, t not COMPLEX: ");
	    print_term_nl(Fdout, t);
	    return(TROUBLE);
	    }
	if (apply(t, into_subst, &temp) == TROUBLE)
		return(TROUBLE);
	*as = temp;
	return(NO_TROUBLE);
	}
    else {
	if (get_term(&t2) == TROUBLE)
		return(TROUBLE);
	t2->type = COMPLEX;
	t2->sym_num = t->sym_num;
	r3 = NULL;
	r1 = t->farg;
	while (r1 != NULL ) {
	    if (get_rel(&r2) == TROUBLE)
		return(TROUBLE);
	    if (r3 == NULL)
		t2->farg = r2;
	    else
		r3->narg = r2;
	    /* if we are on the path to the into term || PARA_ALL */
	    if (r1->path || Flags[PARA_ALL].val)
		{
if (apply_substitute(r1->argval,into_term,into_subst,beta,from_subst,&temp)==TROUBLE)
		return(TROUBLE);
		r2->argval = temp;
		}
	    else
		{
	        if (apply(r1->argval, into_subst, &temp) == TROUBLE)
			return(TROUBLE);
		r2->argval = temp;
		}
	    r3 = r2;
	    r1 = r1->narg;
	    }	/* end of while */
	*as = t2;
	return(NO_TROUBLE);
	}
}  /* apply_substitute */

/*************
 *
 *    int build_bin_para(alpha,from_subst,into_term,into_lit,into_subst,bbp)
 *
 *    Construct a binary paramodulant.
 *	struct clause * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct clause through the 
 *	parameter bbp.
 *
 *************/

static int build_bin_para(alpha,from_subst,into_term,into_lit,into_subst,bbp)
struct term *alpha;
struct context *from_subst;
struct term *into_term;
struct literal *into_lit;
struct context *into_subst;
struct clause **bbp;
{
    struct clause *paramodulant;
    struct literal *lit, *new, *prev;
    struct term *from_atom, *beta;
    struct int_ptr *ip0, *ip1, *ip2;
	struct term *temp;

	*bbp = NULL;				/* default */
    if (get_clause(&paramodulant) == TROUBLE)
	return(TROUBLE);
    prev = NULL;

    from_atom = alpha->occ.rel->argof;  /* find beta */
    if (from_atom->farg->argval == alpha)
	beta = from_atom->farg->narg->argval;  /* beta is second arg */
    else
	beta = from_atom->farg->argval;  /* beta is first arg */

    lit = into_lit->container->first_lit;
    while (lit != NULL) {  /* go through literals of into clause */
	if (get_literal(&new) == TROUBLE)
		return(TROUBLE);
	new->container = paramodulant;
	if (prev == NULL)
	    paramodulant->first_lit = new;
	else
	    prev->next_lit = new;
	prev = new;
	new->sign = lit->sign;
	if (lit == into_lit || Flags[PARA_ALL].val)
		{
if (apply_substitute(lit->atom,into_term,into_subst,beta,from_subst,&temp)==TROUBLE)
		return(TROUBLE);
		new->atom = temp;
		}
	else {
	    if (apply(lit->atom, into_subst, &temp) == TROUBLE)
		return(TROUBLE);
		new->atom = temp;
		}
	new->atom->occ.lit = new;
	new->atom->varnum = lit->atom->varnum;  /* copy type of atom */
	lit = lit->next_lit;
	}

    lit = from_atom->occ.lit->container->first_lit;

    while (lit != NULL) {  /* go through literals of from clause */
	if (lit->atom != from_atom) {  /* omit instance of from literal */
	    if (get_literal(&new) == TROUBLE)
		return(TROUBLE);
	    new->container = paramodulant;
	    if (paramodulant->first_lit == NULL)
		paramodulant->first_lit = new;
	    else
		prev->next_lit = new;
	    prev = new;
	    new->sign = lit->sign;
	    if (apply(lit->atom, from_subst, &temp) == TROUBLE)
		return(TROUBLE);
		new->atom = temp;
	    new->atom->occ.lit = new;
	    new->atom->varnum = lit->atom->varnum;  /* copy type of atom */
	    }
	lit = lit->next_lit;
	}
    
    if (get_int_ptr(&ip0) == TROUBLE)
	return(TROUBLE);
	/* rule: to be filled in by caller */
    if (get_int_ptr(&ip1) == TROUBLE)
	return(TROUBLE);
    if (get_int_ptr(&ip2) == TROUBLE)
	return(TROUBLE);
    ip1->i = from_atom->occ.lit->container->id;
    ip2->i = into_lit->container->id;
    ip0->next = ip1;
    ip1->next = ip2;
    paramodulant->parents = ip0;
	*bbp = paramodulant;
    return(NO_TROUBLE);
}  /* build_bin_para */

/*************
 *
 *    int para_from_up(t, into_term, into_subst, alpha, from_subst)
 *
 *    We are paramodulating from the given clause, and a clashable into term
 *    has been found.  This routine recursively goes through the clashable
 *    superterms of the into term.
 *
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

static int para_from_up(t, into_term, into_subst, alpha, from_subst)
struct term *t;
struct term *into_term;
struct context *into_subst;
struct term *alpha;
struct context *from_subst;
{
    struct clause *paramodulant;
    struct rel *r;
    struct int_ptr *ip;
	int pp, pfu;

	pp = NO_PROOF;				/* default */
	pfu = NO_PROOF;				/* default */
    if (t->type == COMPLEX && t->varnum != 0) {  /* it's an atom */
#ifdef ROO
	if (alpha->occ.rel->argof->occ.lit->container->giv_cl_seq_no >=
	    t->occ.lit->container->giv_cl_seq_no &&
	    (Flags[PARA_INTO_UNITS_ONLY].val == 0 ||
	             num_literals(t->occ.lit->container) == 1)) {
#else
	if (Flags[PARA_INTO_UNITS_ONLY].val == 0 ||
		     num_literals(t->occ.lit->container) == 1) {
#endif
if (build_bin_para(alpha,from_subst,into_term,t->occ.lit,into_subst,&paramodulant)==TROUBLE)
		return(TROUBLE);
	    /* fill in derivation info */
	    ip = paramodulant->parents;
	    ip->i = PARA_FROM_RULE;
	    ip->next->i = alpha->occ.rel->argof->occ.lit->container->id;
	    ip->next->next->i = t->occ.lit->container->id;
	    Stats[CL_GENERATED]++;
	    CLOCK_STOP(PARA_FROM_TIME)
	    pp = pre_process(paramodulant,0,Sos);	/* Penguin */
	    if (pp == PROOF || pp == TROUBLE)
			return(pp);
	    CLOCK_START(PARA_FROM_TIME)
	    }
	}
    else {
	r = t->occ.rel;
	while (r != NULL) {
	   if (r->clashable) {
	       r->path = 1;  /* mark path from into_term up to atom */
  pfu = para_from_up(r->argof, into_term, into_subst, alpha, from_subst);
	if (pfu == PROOF || pfu == TROUBLE)
		return(pfu);
	       r->path = 0;  /* remove mark */
	       }
	    r = r->nocc;
	    }
	}
return(NO_PROOF);
}  /* para_from_up() */

/*************
 *
 *    int para_from_alpha(alpha)
 *
 *    We are paramodulating from the given clause.  This routine
 *    paramodulates from an alpha.
 *
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

static int para_from_alpha(alpha)
struct term *alpha;
{
    struct context *into_subst, *from_subst;
    struct term *into_term;
    struct fpa_tree *ut;
    struct trail *tr;
	int uok;
	int pfu;

	pfu = NO_PROOF;				/* default */
    if (get_context(&into_subst) == TROUBLE)
	return(TROUBLE);
    into_subst->multiplier = 0;
    if (get_context(&from_subst) == TROUBLE)
	return(TROUBLE);
    from_subst->multiplier = 1;

    if (alpha->type == VARIABLE && Flags[PARA_FROM_VARS].val == 0)
	;  /* do nothing */
    else {	/* else do something */
	if (alpha->type == VARIABLE)
	{
	    if (build_for_all(Fpa_clash_terms,&ut)) /* get all terms in index */
		return(TROUBLE);
	}
        else {
if (build_tree(alpha,UNIFY,Parms[FPA_TERMS].val,Fpa_clash_terms,&ut) == TROUBLE)
			return(TROUBLE);
	}
	into_term = next_term(ut, 0); 
	while (into_term != NULL) {
	    tr = NULL;
	if (unify(into_term, into_subst, alpha, from_subst, &tr, &uok)==TROUBLE)
		return(TROUBLE);
	if (uok)
	{	/* unify succeeds */
pfu = para_from_up(into_term, into_term, into_subst, alpha, from_subst);
	if (pfu == PROOF || pfu == TROUBLE)
		return(pfu);
		clear_subst_1(tr);
		}	/* end of if unify succeeds */
	    into_term = next_term(ut, 0);
	    }	/* end of while */
	}	/* end of else do something */

    free_context(into_subst);
    free_context(from_subst);
return(NO_PROOF);
}  /* para_from_alpha() */

/*************
 *
 *    int para_from(giv_cl) -- binary paramodulation from the given clause
 *
 *    Paramodulants are given to the routine pre_process.
 *
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

int para_from(giv_cl)
struct clause *giv_cl;
{
    struct literal *from_lit;
	int pfa;

    CLOCK_START(PARA_FROM_TIME)

    if (Flags[PARA_FROM_UNITS_ONLY].val == 0 || num_literals(giv_cl) == 1) {

	from_lit = giv_cl->first_lit;
	while (from_lit != NULL) {
	    if (from_lit->atom->varnum == POS_EQ) {
		if (Flags[PARA_FROM_LEFT].val)
		{
	   pfa = para_from_alpha(from_lit->atom->farg->argval);
		if (pfa == PROOF || pfa == TROUBLE)
			return(pfa);
		}
		if (Flags[PARA_FROM_RIGHT].val)
		{
	pfa = para_from_alpha(from_lit->atom->farg->narg->argval);
		if (pfa == PROOF || pfa == TROUBLE)
			return(pfa);
		}
		}
	    from_lit = from_lit->next_lit;
	    }
	}

    CLOCK_STOP(PARA_FROM_TIME)
return(NO_PROOF);
}  /* para_from() */

/*************
 *
 *    int para_into_terms(t, into_lit, from_subst, into_subst)
 *
 *    We are paramodulating into the given clause.  This routine recursively
 *    goes through the clashable subterms of the given literal.
 *
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

static int para_into_terms(into_term, into_lit, from_subst, into_subst)
struct term *into_term;
struct literal *into_lit;
struct context *from_subst;
struct context *into_subst;
{
    struct term *alpha;
    struct trail *tr;
    struct fpa_tree *ut;
    struct clause *paramodulant;
    struct rel *r;
    struct int_ptr *ip;
	int uok;
	int pp, pit;

	pp = NO_PROOF;				/* default */
	pit = NO_PROOF;				/* default */
    if (into_term->type == COMPLEX) {
	r = into_term->farg;
	while (r != NULL) {
	    if (r->clashable) {
		r->path = 1;  /* mark path to into term */
pit = para_into_terms(r->argval,into_lit,from_subst,into_subst);
		if (pit == PROOF || pit == TROUBLE)
			return(pit);
		r->path = 0;  /* remove mark */
		}
	    r = r->narg;
	    }
	}

    /* no need to check if variable and `no para into vars' */
    /* because the clashability flag handles it */

    if (into_term->type == VARIABLE)
	{
if (build_for_all(Fpa_alphas,&ut) == TROUBLE) /* get all terms in index */
	return(TROUBLE);
	}
    else
	{
if (build_tree(into_term,UNIFY,Parms[FPA_TERMS].val,Fpa_alphas,&ut)==TROUBLE)
		return(TROUBLE);
	}

    alpha = next_term(ut, 0); 
    while (alpha != NULL) {
	tr = NULL;
#ifdef ROO
	if (into_lit->container->giv_cl_seq_no >=
	    alpha->occ.rel->argof->occ.lit->container->giv_cl_seq_no &&
	    unify(into_term, into_subst, alpha, from_subst, &tr)) {
#else
if (unify(into_term, into_subst, alpha, from_subst, &tr, &uok) == TROUBLE) 
	return(TROUBLE);
	if (uok) {
#endif

if (build_bin_para(alpha,from_subst,into_term,into_lit,into_subst,&paramodulant)==TROUBLE)
		return(TROUBLE);
	    /* fill in derivation info */
	    ip = paramodulant->parents;
	    ip->i = PARA_INTO_RULE;
	    ip->next->i = into_lit->container->id;
	    ip->next->next->i = alpha->occ.rel->argof->occ.lit->container->id;
	    clear_subst_1(tr);
	    Stats[CL_GENERATED]++;
	    CLOCK_STOP(PARA_INTO_TIME)
	    pp = pre_process(paramodulant,0,Sos);	/* Penguin */
	    if (pp == PROOF || pp == TROUBLE)
			return(pp);
	    CLOCK_START(PARA_INTO_TIME)
	    }
	alpha = next_term(ut, 0);
	}
return(NO_PROOF);
}  /* para_into_terms */

/*************
 *
 *    int para_into(giv_cl) -- binary paramodulation into the given clause
 *
 *    Paramodulants are given to the routine pre_process.
 *
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

int para_into(giv_cl)
struct clause *giv_cl;
{
    struct literal *into_lit;
    struct context *into_subst, *from_subst;
    struct rel *r;
	int pit;

	pit = NO_PROOF;				/* default */
    CLOCK_START(PARA_INTO_TIME)

    if (Flags[PARA_INTO_UNITS_ONLY].val == 0 || num_literals(giv_cl) == 1) {

	/* Substitutions are allocated here instead of in */
	/* para_into_terms to save procedure calls.       */

	if (get_context(&into_subst) == TROUBLE)
		return(TROUBLE);
	into_subst->multiplier = 0;
	if (get_context(&from_subst) == TROUBLE)
		return(TROUBLE);
	from_subst->multiplier = 1;
	into_lit = giv_cl->first_lit;

	while (into_lit != NULL) {
	    if (into_lit->atom->varnum != ANSWER) {  /* if not answer literal */
		r = into_lit->atom->farg;
		while (r != NULL) {
		    if (r->clashable) {
			r->path = 1;  /* mark path to into term */
pit = para_into_terms(r->argval, into_lit, from_subst, into_subst);
		if (pit == PROOF || pit == TROUBLE)
			return(pit);
			r->path = 0;  /* remove mark */
			}
		    r = r->narg;
		    }
		}
	    into_lit = into_lit->next_lit;
	    }
	free_context(into_subst);
	free_context(from_subst);
	}
    CLOCK_STOP(PARA_INTO_TIME)
return(NO_PROOF);
}  /* para_into */

