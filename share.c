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
 *  share.c -- routines to manage the shared data structures
 *
 */

#include "header.h"

#ifdef TURBO_C  /* PC */
#define TERM_TAB_SIZE 200
#else
#ifdef THINK_C  /* Macintosh */
#define TERM_TAB_SIZE 200 
#else
#define TERM_TAB_SIZE 2000
#endif
#endif

/* Hash table of shared terms */

static struct term_ptr *Term_tab[TERM_TAB_SIZE];

/* alphas and betas of pos eq lits are not shared, so they are put here */
/* so that back dedmodulation can find them                             */

static struct term_ptr *Bd_kludge;

#ifdef ROO

/*************
 *
 *    init_term_tab_for_roo()
 *
 *************/

void init_term_tab_for_roo()
{
    int i;

    for (i=0; i<TERM_TAB_SIZE; i++) {
        Term_tab[i] = get_term_ptr(); /* insert dummy node */
        Term_tab[i]->term = NULL;
	}

    Bd_kludge = get_term_ptr();
    Bd_kludge->term = NULL;
}  /* init_term_tab_for_roo */

#endif

/*************
 *
 *    int linear_vars(t, vars);
 *
 *************/

static int linear_vars(t, vars)
struct term *t;
int vars[];
{
    struct rel *r;
    int ok;

    if (t->type == NAME)
	return(1);
    else if (t->type == VARIABLE) {
	if (vars[t->varnum])
	    return(0);
	else {
	    vars[t->varnum] = 1;
	    return(1);
	    }
	}
    else {  /* COMPLEX */
	ok = 1;
	for (r = t->farg; r && ok; r = r->narg)
	    ok = linear_vars(r->argval, vars);
	return(ok);
	}
}  /* linear_vars */

/*************
 *
 *    mark_linear_ground(t) -- assume all arguments are already marked.
 *
 *************/

static void mark_linear_ground(t)
struct term *t;
{
    int ground, possibly_linear;
    struct rel *r;
    int vars[MAX_VARS], i;

    if (t->type == NAME) {
	SET_BIT(t->bits, GROUND_BIT);
	SET_BIT(t->bits, LINEAR_BIT);
	}
    else if (t->type == VARIABLE) {
	CLEAR_BIT(t->bits, GROUND_BIT);
	SET_BIT(t->bits, LINEAR_BIT);
	}
    else {  /* COMPLEX */
	ground = possibly_linear = 1;
	for (r = t->farg; r; r = r->narg) {
	    ground = ground && TP_BIT(r->argval->bits, GROUND_BIT);
	    possibly_linear = possibly_linear && TP_BIT(r->argval->bits, LINEAR_BIT);
	    }
	if (ground) {
	    SET_BIT(t->bits, GROUND_BIT);
	    SET_BIT(t->bits, LINEAR_BIT);
	    }
	else if (!possibly_linear) {
	    CLEAR_BIT(t->bits, GROUND_BIT);
	    CLEAR_BIT(t->bits, LINEAR_BIT);
	    }
	else if (t->farg == NULL || t->farg->narg == NULL) {
	    /* 0 or 1 argument */
	    CLEAR_BIT(t->bits, GROUND_BIT);
	    SET_BIT(t->bits, LINEAR_BIT);
	    }
	else {
	    CLEAR_BIT(t->bits, GROUND_BIT);
	    for (i = 0; i < MAX_VARS; i++)
		vars[i] = 0;
	    possibly_linear = linear_vars(t->farg->argval, vars);
	    for (r = t->farg->narg; r && possibly_linear; r = r->narg)
		possibly_linear = linear_vars(r->argval, vars);
	    if (possibly_linear)
		SET_BIT(t->bits, LINEAR_BIT);
	    else
		CLEAR_BIT(t->bits, LINEAR_BIT);
	    }
	}
/*
    if (t->type == COMPLEX) {
	print_term(Fdout, t);
	printf(" linear: %d, ", TP_BIT(t->bits, LINEAR_BIT));
	printf(" ground: %d.\n", TP_BIT(t->bits, GROUND_BIT));
	}
*/
}  /* mark_linear_ground */

/*************
 *
 *    int hash_term(term)
 *
 *        Return the hash value of a term.  It is assumed that
 *    the subterms are already integrated.  The hash value is
 *    constructed from the functor and the addresses of the
 *    subterms.
 *
 *************/

static int hash_term(t)
struct term *t;
{
    struct rel *r;
    int hashval;
    
    if (t->type == NAME)   /* name */
	hashval = t->sym_num;
    else if (t->type == VARIABLE)  /* variable */
	hashval = t->sym_num + t->varnum + 25;
    else {  /* complex */
	hashval = t->sym_num;
	r = t->farg;
	while (r != NULL) {
	    hashval >>= 1;  /* shift right one bit */
	    hashval ^= (int) r->argval; /* exclusive or */
	    r = r->narg;
	    }
	}
    return(abs(hashval) % TERM_TAB_SIZE);
}  /* hash_term */

/*************
 *
 *    int term_compare(t1, t2)
 *
 *        Special purpose term comparison for term integration.
 *    Succeed iff functors are identical and subterm pointers are
 *    identical.  (Note that this routine is not recursive.)
 *    (For general purpose routine, see `term_ident'.)
 *
 *************/

static int term_compare(t1, t2)
struct term *t1;
struct term *t2;
{
    struct rel *r1, *r2;
    
    if (t1->type != t2->type)
	return(0);
    else if (t1->type == NAME) /* name */
	return(t1->sym_num == t2->sym_num);
    else if (t1->type == VARIABLE) /* variable */
	return(t1->varnum == t2->varnum && t1->sym_num == t2->sym_num);
    else if (t1->sym_num != t2->sym_num)
	return(0);  /* both complex with different functors */
    else {
	r1 = t1->farg;
	r2 = t2->farg;
	while (r1 != NULL && r2 != NULL) {
	    if (r1->argval != r2->argval)
		return(0);
	    else {
		r1 = r1->narg;
		r2 = r2->narg;
		}
	    }
	return(r1 == NULL && r2 == NULL);
	}
}  /* term_compare */

/*************
 *
 *    int integrate_term(term,t1)
 *
 *        Incorporate a term into the shared data structures.
 *    Either ther term itself is integrated and returned, or
 *    the term is deallocated and a previously integrated copy
 *    is returned.  A good way to invoke it is
 *
 *           t = integrate(t)
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter t1. Thus a good way to invoke is
 *
 *		if (integrate(t,&t) == TROUBLE)
 *			return(TROUBLE);
 *
 *************/

int integrate_term(t,t1)
struct term *t;
struct term **t1;
{
    int hashval, found;
    struct term_ptr *p, *prev;
    struct rel *r, *r2;
	struct term *st;
    

    if (t->type == COMPLEX) {  /* complex term */
	r = t->farg;
	while (r != NULL) {
	    if (integrate_term(r->argval, &st) == TROUBLE)
		{
			*t1 = NULL;
			return(TROUBLE);
		}
		else r->argval = st;
	    r = r->narg;
	    }	/* end of while */
	}	/* end of complex term */
    
    hashval = hash_term(t);

    p = Term_tab[hashval];
    prev = NULL;

#ifdef ROO
    prev = p;  /* skip over dummy node */
    p = p->next;
#endif

    found = 0;
    while (found == 0 && p != NULL)
	if (term_compare(t, p->term))
	    found = 1;
	else {
	    prev = p;
	    p = p->next;
	    }
    
    if (found) {    /* p->term is integrated copy */
	if (t == p->term) {
	    output_stats(Fdout, 4);
	    fprintf(Fderr, "ABEND, integrate_term, already integrated.\007\n");
	    fprintf(Fdout, "ABEND, integrate_term, already integrated: ");
	    print_term_nl(Fdout, t);
		*t1 = NULL;
	    return(TROUBLE);
	    }
	if (t->type == COMPLEX) { /* if complex, free rels */
	    r = t->farg;
	    while (r != NULL) {
		r2 = r;
		r = r->narg;
		free_rel(r2);
		}
	    }
	free_term(t);
	*t1 = p->term;
	return(NO_TROUBLE);
	}
    else {    /* not in bucket, so insert at end of bucket and return */
	if (t->type == COMPLEX) { /* if complex, set up containment lists */
	    r = t->farg;
	    while (r != NULL) {
		r->argof = t;
		r->nocc = r->argval->occ.rel;
		r->argval->occ.rel = r;
		r = r->narg;
		}
	    }
	if (get_term_ptr(&p) == TROUBLE)
		{
		*t1 = NULL;
		return(TROUBLE);
		}
	p->term = t;
	p->next = NULL;
	if (prev == NULL)
	    Term_tab[hashval] = p;
	else
	    prev->next = p;

	mark_linear_ground(t);
        if (Flags[INDEX_FOR_BACK_DEMOD].val)
		{
	    if (fpa_insert(t, Parms[FPA_TERMS].val, Fpa_back_demod)==TROUBLE)
			{
			*t1 = NULL;
			return(TROUBLE);
			}
		}
	*t1 = t;
	return(NO_TROUBLE);
	}
}  /* integrate_term */

/*************
 *
 *    int disintegrate_term(term)
 *
 *       Remove a previously integrated term from the shared data
 *    structures.  A warning is printed if the term has a list of
 *    superterms.
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int disintegrate_term(t)
struct term *t;
{
    int hashval;
    struct rel *r1, *r2, *r3;
    struct term_ptr *p1, *p2;
    
    if (t->occ.rel != NULL) {
	fprintf(Fderr, "WARNING, disintegrate_term, contained term.\n");
	printf("WARNING, disintegrate_term, contained term: ");
	print_term_nl(Fdout, t);
	}
    else {
	hashval = hash_term(t);
	p1 = Term_tab[hashval];
	p2 = NULL;

#ifdef ROO
	p2 = p1;  /* skip over dummy node */
	p1 = p1->next;
#endif
	while (p1 != NULL && p1->term != t) {
	    p2 = p1;
	    p1 = p1->next;
	    }
	if (p1 == NULL)  {
	    output_stats(Fdout, 4);
	    fprintf(Fderr, "ABEND, disintegrate_term cannot find term.\007\n");
	    fprintf(Fdout, "ABEND, disintegrate_term cannot find term: ");
	    print_term_nl(Fdout, t);
	    return(TROUBLE);
	    }
	else {
	    if (p2 == NULL)
		Term_tab[hashval] = p1->next;
	    else
		p2->next = p1->next;
	    free_term_ptr(p1);
            if (Flags[INDEX_FOR_BACK_DEMOD].val)
	    if (fpa_delete(t, Parms[FPA_TERMS].val, Fpa_back_demod)==TROUBLE)
			return(TROUBLE);
	    if (t->type == COMPLEX) {
		r1 = t->farg;
		while (r1 != NULL) {
		    r2 = r1->argval->occ.rel;
		    r3 = NULL;
		    while (r2 != NULL && r2 != r1) {
		        r3 = r2;
		        r2 = r2->nocc;
		        }
		    if (r2 == NULL) {
			output_stats(Fdout, 4);
	fprintf(Fderr, "ABEND, disintegrate_term bad containment.\007\n");
		   fprintf(Fdout, "ABEND, disintegrate_term bad containment: ");
		        print_term_nl(Fdout, t);
		        return(TROUBLE);
		        }
		    else {
		        if (r3 == NULL)
		            r1->argval->occ.rel = r1->nocc;
		        else
		            r3->nocc = r1->nocc;
		        if (r1->argval->occ.rel == NULL)
		            if (disintegrate_term(r2->argval)==TROUBLE)
				return(TROUBLE);
		        }
		    r3 = r1;
		    r1 = r1->narg;
		    free_rel(r3);
		    }
		}
	    free_term(t);
	    }
	}
return(NO_TROUBLE);
}  /* disintegrate_term */

/*************
 *
 *    zap_term(term)
 *
 *        Deallocate a nonshared term.  A warning is printed
 *    if the term or any of its subterms contains a list of superterms.
 *
 *************/

void zap_term(t)
struct term *t;
{
    struct rel *r1, *r2;
    
    if (t->occ.rel != NULL) {
	fprintf(Fderr, "WARNING, zap_term, contained term.\n");
	fprintf(Fdout,"WARNING, zap_term, contained term: ");
	print_term_nl(Fdout, t);
	printf("\n");
	}
    else {
	if (t->type == COMPLEX) { /* complex term */
	    r1 = t->farg;
	    while (r1 != NULL) {
		zap_term(r1->argval);
		r2 = r1;
		r1 = r1->narg;
		free_rel(r2);
		}
	    }
	free_term(t);
	}
}  /* zap_term */

/*************
 *
 *    print_term_tab(file_ptr) -- Print the table of integrated terms.
 *
 *************/

void print_term_tab(fp)
FILE *fp;
{
    int i;
    struct term_ptr *p;
    
    for(i=0; i<TERM_TAB_SIZE; i++)
	if(Term_tab[i] != NULL) {
	    fprintf(fp, "bucket %d: ",i);
	    p = Term_tab[i];

#ifdef ROO
	    p = p->next;   /* skip over dummy node */
#endif
	    
	    while(p != NULL) {
	       print_term(fp, p->term);
	       fprintf(fp, " ");
	       p = p->next;
	       }
	    fprintf(fp, "\n");
	    }
}  /* print_term_tab */

/*************
 *
 *    p_term_tab()
 *
 *************/

void p_term_tab()
{
    print_term_tab(Fdout);
}  /* p_term_tab */

/*************
 *
 *    test_terms(file_ptr)
 *
 *        Print ther list of integrated terms.  For each term, list its
 *    subterms and superterms.
 *
 *************/

void test_terms(fp)
FILE *fp;
{
    int i;
    struct term_ptr *p;
    struct rel *r;
    
    for(i=0; i<TERM_TAB_SIZE; i++)
	if(Term_tab[i] != NULL) {
	    fprintf(fp, "    bucket %d:\n",i);
	    p = Term_tab[i];
#ifdef ROO
	    p = p->next;   /* skip over dummy node */
#endif
	    while(p != NULL) {
	       print_term(fp, p->term);
	       fprintf(fp, " containing terms: ");
	       r = p->term->occ.rel;
	       while (r != NULL) {
		   print_term(fp, r->argof);
		   fprintf(fp, " ");
		   r = r->nocc;
		   }
	       fprintf(fp, "\n");
	       p = p->next;
	       }
	    }
}  /* test_terms */

/*************
 *
 *    int all_instances(atom, ai)
 *
 *    Get all terms (in table of shared terms) that can be rewritten
 *    with demodulator (atom).  Handles lex-dependent demod correctly.
 *
 *	struct term_ptr * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term_ptr through the
 *	parameter ai.
 *
 *************/

int all_instances(atom, ai)
struct term *atom;
struct term_ptr **ai;
{
    struct term *alpha, *beta, *t;
    struct term_ptr *tp, *tp1, *instances;
    struct context *subst;
    struct trail *tr;
    int i, lex_dependent, ok;
	int mok;

	*ai = NULL;				/* default */
    alpha = atom->farg->argval;
    beta = atom->farg->narg->argval;
    lex_dependent = (atom->varnum == LEX_DEP_DEMOD);
    instances = NULL;
    if (get_context(&subst) == TROUBLE)
	return(TROUBLE);
    subst->multiplier = 1;
    for (i = 0; i <= TERM_TAB_SIZE; i++) {
	tp = (i == TERM_TAB_SIZE ? Bd_kludge : Term_tab[i]);
#ifdef ROO
	tp = tp->next;   /* skip over dummy node */
#endif
	while (tp != NULL) {
	    tr = NULL;
	    if (match(alpha, subst, tp->term, &tr, &mok) == TROUBLE)
		return(TROUBLE);
		if (mok)
		 {	/* if match successful */
		if (lex_dependent == 0)
		    ok = 1;
		else {
		    if (apply(beta, subst, &t) == TROUBLE)
			return(TROUBLE);
		    if (Flags[LEX_RPO].val)
			{
			if (lrpo_greater(tp->term, t, &ok) == TROUBLE)
				return(TROUBLE);
			}
		    else
		        ok = (lex_check(t, tp->term) == LESS_THAN);
		    zap_term(t);
		    }

		if (ok) {
		    if (get_term_ptr(&tp1) == TROUBLE)
			return(TROUBLE);
		    tp1->term = tp->term;
		    tp1->next = instances;
		    instances = tp1;
		    }

		clear_subst_1(tr);
		}	/* end of if match successful */
	    tp = tp->next;
	    }
	}
    free_context(subst);
	*ai = instances;
    return(NO_TROUBLE);
}  /* all_instances */

/*************
 *
 *    int all_instances_fpa(atom,aifpa)
 *
 *    Get all terms (in table of shared terms) that can be rewritten
 *    with demodulator (atom).  Handles lex-dependent demod correctly.
 *	struct term_ptr * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term_ptr through the
 *	parameter aifpa.
 *
 *************/

int all_instances_fpa(atom,aifpa)
struct term *atom;
struct term_ptr **aifpa;
{
    struct term *alpha, *beta, *t, *found;
    struct term_ptr *tp1, *instances;
    struct context *subst;
    struct trail *tr;
    int lex_dependent, ok;
    struct fpa_tree *ut;
	int mok;

	*aifpa = NULL;				/* default */
    alpha = atom->farg->argval;
    beta = atom->farg->narg->argval;
    lex_dependent = (atom->varnum == LEX_DEP_DEMOD);
    instances = NULL;
    if (get_context(&subst) == TROUBLE)
	return(TROUBLE);
    subst->multiplier = 1;
    
if (build_tree(alpha,INSTANCE,Parms[FPA_TERMS].val,Fpa_back_demod,&ut)==TROUBLE)
		return(TROUBLE);

    found = next_term(ut, 0);

    while (found != NULL) {
	tr = NULL;
	if (match(alpha, subst, found, &tr, &mok) == TROUBLE)
		return(TROUBLE);

		if (mok)
		 {
	    if (lex_dependent == 0)
		ok = 1;
	    else {
		if (apply(beta, subst, &t) == TROUBLE)
			return(TROUBLE);
		if (Flags[LEX_RPO].val)
		{
		    if (lrpo_greater(found, t, &ok) == TROUBLE)
			return(TROUBLE);
		}
		else
		    ok = (lex_check(t, found) == LESS_THAN);
		zap_term(t);
		}
	    
	    if (ok) {
		if (get_term_ptr(&tp1) == TROUBLE)
			return(TROUBLE);
		tp1->term = found;
		tp1->next = instances;
		instances = tp1;
		}
	    
	    clear_subst_1(tr);
	    }
	found = next_term(ut, 0);
	}
    free_context(subst);
	*aifpa = instances;
    return(NO_TROUBLE);
}  /* all_instances_fpa */

/*************
 *
 *    int bd_kludge_insert(t)
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int bd_kludge_insert(t)
struct term *t;
{
    struct term_ptr *tp;

    if (Flags[INDEX_FOR_BACK_DEMOD].val)
        if (fpa_insert(t, Parms[FPA_TERMS].val, Fpa_back_demod)==TROUBLE)
		return(TROUBLE);

    if (get_term_ptr(&tp) == TROUBLE)
	return(TROUBLE);

    tp->term = t;

#ifdef ROO
    tp->next = Bd_kludge->next;
    Bd_kludge->next = tp;
#else
    tp->next = Bd_kludge;
    Bd_kludge = tp;
#endif
return(NO_TROUBLE);
}  /* bd_kludge_insert */

/*************
 *
 *    int bd_kludge_delete(t)
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int bd_kludge_delete(t)
struct term *t;
{
    struct term_ptr *tp1, *tp2;


    if (Flags[INDEX_FOR_BACK_DEMOD].val)
	{
        if (fpa_delete(t, Parms[FPA_TERMS].val, Fpa_back_demod)==TROUBLE)
		return(TROUBLE);
	}
    tp1 = Bd_kludge;
    tp2 = NULL;
    while (tp1 != NULL && tp1->term != t) {
	tp2 = tp1;
	tp1 = tp1->next;
	}
    if (tp1 == NULL) {
	fprintf(Fderr, "WARNING, bd_kludge_delete, term not found.\n");
	fprintf(Fdout,"WARNING, bd_kludge_delete, term not found: ");
	print_term_nl(Fdout, t);
	}
    else if (tp2 != NULL)
/* t has been found and tp2 points to its predecessor in the list Bd_kludge. */
	tp2->next = tp1->next;
    else
/* t has been found, tp2 == NULL, because t is the first term in Bd_kludge. */
	Bd_kludge = tp1->next;

if (tp1 != NULL)		/* Penguin! Otter didn't check! */
    free_term_ptr(tp1);

return(NO_TROUBLE);
}  /* bd_kludge_delete */
