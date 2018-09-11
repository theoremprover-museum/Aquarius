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
 *  weight.c -- Routines to weigh clauses, literals and terms.
 *  (also some routines that handle lexical ordering (not LRPO)).
 *
 */

#include "header.h"

/*************
 *
 *    static struct is_tree *weight_retrieve(t, wt_index)
 *
 *************/

static struct is_tree *weight_retrieve(t, wt_index)
struct term *t;
struct is_tree *wt_index;
{
    struct is_tree *is;

    if (!wt_index)
	return(NULL);
    else {
	is = wt_index->u.kids;
	while (is && ((t->type != is->type) || (t->type != VARIABLE && (t->sym_num != is->lab))))
	    is = is->next;
	return(is);
	}
}  /* weight_retrieve */

/*************
 *
 *    int weight(term, wt_index) -- Return the weight a term.
 *
 *************/

int weight(t, wt_index)
struct term *t;
struct is_tree *wt_index;
{
    struct is_tree *is;
    struct term_ptr *p;
    struct rel *r;
    int wt, w1, max;

    is = weight_retrieve(t, wt_index);
    if (is)
	p = is->u.terms;
    else
	p = NULL;

    wt = 0;
    while (p != NULL && wt_match(t, p->term->farg->argval, &wt, wt_index) == 0) {
	p = p->next;
	wt = 0;
	}

    if (p != NULL)  /* we have a match */
	return(wt + p->term->farg->narg->argval->fpa_id);
    else if (is_atom(t) && t->varnum == ANSWER)
	return(0);  /* default weight of answer atom */
    else if (t->type == VARIABLE || t->type == NAME)
	return(1);  /* default weight of symbol */
    else {   /* compute default weight of term or atom */
	/* 
	    if (flag is set)
	        weight of t is (max or weights of args) + 1
	    else
		weight of t is (sum weights of subterms) + 1
	*/
	if (is_atom(t))
	    max = Flags[ATOM_WT_MAX_ARGS].val;
	else
	    max = Flags[TERM_WT_MAX_ARGS].val;
	wt = 0;
	r = t->farg;
	while (r != NULL) {
	    w1 = weight(r->argval, wt_index);
	    if (max)
		wt = (w1 > wt ? w1 : wt);
	    else
	        wt += w1;
	    r = r->narg;
	    }
	return(wt + 1);
	}
}  /* weight */

/*************
 *
 *    int weight_match(term, template, wtp, wt_index)
 *
 *        Attempt to match a term with a weight template.  If
 *    successful, add the weight of the term to *wtp, and
 *    return(1); else return(0).
 *
 *************/

int wt_match(t, template, wtp, wt_index)
struct term *t;
struct term *template;
int *wtp;
struct is_tree *wt_index;
{
    struct rel *r1,*r2;
    int go;

    if (t->type != template->type)
	return(0);
    else if (t->type == VARIABLE)
	return(1);
    else if (t->type == NAME)
	return(t->sym_num == template->sym_num);
    else {  /* complex */
	if (t->sym_num != template->sym_num)
	    return(0);
	else {
	    go = 1;
	    r1 = t->farg;
	    r2 = template->farg;
	    while (go && r1 != NULL && r2 != NULL) {
		if (r2->argval->type == NAME && TP_BIT(r2->argval->bits, SCRATCH_BIT)) 
		    /* term is a multiplier */
		    *wtp += r2->argval->fpa_id * weight(r1->argval,wt_index);
		else
		    go = wt_match(r1->argval, r2->argval, wtp, wt_index);
		r1 = r1->narg;
		r2 = r2->narg;
		}
	    return(go && r1 == NULL && r2 == NULL);
	    }
	}
}  /* wt_match */

/*************
 *
 *    static void set_wt_term(term, warn_ptr) --
 *
 *    Mark multipliers with SCRATCH_BIT,
 *    and store the multipliers in fpa_id field.
 *
 *************/

static void set_wt_term(t,warn_ptr)
struct term *t;
int *warn_ptr;
{
    struct rel *r;
    int n;
    char *s;

    if (t->type == NAME) {
	s = sn_to_str(t->sym_num);
	if (!Flags[SUPPRESS_WEIGHT_WARNING].val && str_int(s, &n)) {
	    (*warn_ptr)++;
	    }
	
        if (*s == '*' && str_int(s+1, &n)) {
	    /* this is a trick to mark a multiplier */
	    SET_BIT(t->bits, SCRATCH_BIT);
	    t->fpa_id = n;
	    }
	}
    else if (t->type == COMPLEX) {
	r = t->farg;
	while (r != NULL) {
	    set_wt_term(r->argval, warn_ptr);
	    r = r->narg;
	    }
	}
}  /* set_wt_term */

/*************
 *
 *    static int set_wt_template(template, warn_ptr)
 *
 *        Make sure that the template is OK, and mark the multipliers
 *    and the adder.  Return 1 for success and 0 for failure.
 *    Eamample weight templates:  weight(f(*1,f(*3,a)),5),
 *    weight(x,-100), (all variables have weight -100),
 *    weight(f(x,g(a,x)),30) (x matches any variable, and the
 *    two occurrences of x don't have to match the same variable.
 *
 *************/

static int set_wt_template(t, warn_ptr)
struct term *t;
int *warn_ptr;
{
    int n;

    /* first make sure that template is ok; if ok, str_int gets adder */
    if (t->type != COMPLEX || str_ident(sn_to_str(t->sym_num), "weight") == 0
	|| t->farg == NULL || t->farg->narg == NULL
	|| t->farg->narg->narg != NULL || t->farg->narg->argval->type != NAME
	|| str_int(sn_to_str(t->farg->narg->argval->sym_num), &n) == 0) {
	return(0);
	}
    else {
	/* stash adder in fpa_id field */
	t->farg->narg->argval->fpa_id = n;
	set_wt_term(t->farg->argval, warn_ptr);
	return(1);
	}
}  /* set_wt_template */

/*************
 *
 *    static int weight_insert(t, wt_index)
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

static int weight_insert(t, wt_index)
struct term *t;
struct is_tree *wt_index;
{
    struct is_tree *is;
    struct term_ptr *tp, *new_tp;
    struct term *t1;

    if (get_term_ptr(&new_tp) == TROUBLE)
	return(TROUBLE);
    new_tp->term = t;

    is = weight_retrieve(t->farg->argval, wt_index);

    if (is) {
	/* Put new template at end of list. */
	tp = is->u.terms;
	while (tp->next)
	    tp = tp->next;
	tp->next = new_tp;
	}
    else {
	t1 = t->farg->argval;
	if (get_is_tree(&is) == TROUBLE)
	return(TROUBLE);
	is->type = t1->type;
	if (t1->type == VARIABLE)
	    is->lab = t1->varnum;
	else
	    is->lab = t1->sym_num;
	is->u.terms = new_tp;
	is->next = wt_index->u.kids;
	wt_index->u.kids = is;
	}
    return(NO_TROUBLE);
}  /* weight_insert */

/*************
 *
 * int set_wt_list(wt_list, wt_index, error_ptr) 
 *	 Set a list of weight termplates.  
 * 
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE
 *
 *************/

int set_wt_list(wt_list, wt_index, ep)
struct term_ptr *wt_list;
struct is_tree *wt_index;
int *ep;
{
    struct term_ptr *p;
    int warn;

    *ep = 0;
    p = wt_list;
    warn = 0;
    while (p != NULL) {
	if (set_wt_template(p->term, &warn) == 0) {
	    fprintf(Fdout, "ERROR, weight template: ");
	    print_term_nl(Fdout, p->term);
	    (*ep)++;
	    }
	else
	    if (weight_insert(p->term, wt_index) == TROUBLE)
		return(TROUBLE);
	p = p->next;
	}
    
    if (warn > 0) {
	fprintf(Fderr, "\nWarning: You have integers in your weight templates.  The integers are\n");
	fprintf(Fderr, "not being used as multipliers as they would have been in Otter 2.0\n");
	fprintf(Fderr, "and earlier versions.  The new way to specify a multiplier n is *n.  The\n");
	fprintf(Fderr, "change was made so that you would be able to weigh integers and terms con-\n");
	fprintf(Fderr, "taining integers.  Ignore this message if you really wish to weigh those\n");
	fprintf(Fderr, "integers.  To suppress this message, set(suppress_weight_warning).\007\007\n\n");
	}
return(NO_TROUBLE);
}  /* set_wt_list */

/*************
 *
 *    void weight_index_delete(wt_index)
 *
 *************/

void weight_index_delete(wt_index)
struct is_tree *wt_index;
{
    struct is_tree *is1, *is2;
    struct term_ptr *tp1, *tp2;

    if (wt_index) {
	is1 = wt_index->u.kids;
	while (is1) {
	    tp1 = is1->u.terms;
	    while (tp1) {
		/* Do not free template; it belongs to Weight_list. */
		tp2 = tp1;
		tp1 = tp1->next;
		free_term_ptr(tp2);
		}
	    is2 = is1;
	    is1 = is1->next;
	    free_is_tree(is2);
	    }
	free_is_tree(wt_index);
	}
	
}  /* weight_index_delete */

/*************
 *
 *    lex_compare_sym_nums(n1, n2)
 *
 *************/

static int lex_compare_sym_nums(n1, n2)
int n1;
int n2;
{
    int v1, v2;

    if (n1 == n2)
	return(SAME_AS);
    else {
	v1 = sn_to_node(n1)->lex_val;
	v2 = sn_to_node(n2)->lex_val;
	if (v1 < v2)
	    return(LESS_THAN);
	if (v1 > v2)
	    return(GREATER_THAN);
	else if (n1 < n2)
	    return(LESS_THAN);
	else  /* n1 > n2 */
	    return(GREATER_THAN);
	}
}  /* lex_compare_sym_nums */

/*************
 *
 *    int lex_order(t1, t2)
 *
 *    Return SAME_AS, GREATER_THAN, LESS_THAN, or NOT_COMPARABLE.
 *
 *    A variable is comparable only to an identical variable (nonground terms can still
 *    be compared: if a < b, f(a,x) < f(b,y).)
 *    For pairs of nonvariables, use the lex_val field of the symbol_table node;
 *    if identical, use the sym_num's of the terms.
 *
 *************/

static int lex_order(t1, t2)
struct term *t1;
struct term *t2;
{
    struct rel *r1, *r2;
    int i, t1_special, t2_special;

    /* The following handles special unary functions that are to be */
    /* ignored during lex_check.  For example, when using lex-dependent */
    /* demodulation to sort AC expressions, you can make the canonical */
    /* form be a + -a + b + -b + c + -c.                              */

    if (Internal_flags[SPECIAL_UNARY_PRESENT]) {
	t1_special = (t1->type == COMPLEX && sn_to_node(t1->sym_num)->special_unary);
	t2_special = (t2->type == COMPLEX && sn_to_node(t2->sym_num)->special_unary);
	
	if (t1_special && !t2_special)
	    if (term_ident(t1->farg->argval, t2))
		return(GREATER_THAN);
	    else
		return(lex_order(t1->farg->argval, t2));
	else if (!t1_special && t2_special)
	    if (term_ident(t2->farg->argval, t1))
		return(LESS_THAN);
	    else
		return(lex_order(t1, t2->farg->argval));
	}

    /* end of special_unary code */

    if (t1->type == VARIABLE)
	if (t2->type == VARIABLE)
	    return(t1->varnum == t2->varnum ? SAME_AS : NOT_COMPARABLE);
	else
	    return(occurs_in(t1, t2) ? LESS_THAN : NOT_COMPARABLE);
    else if (t2->type == VARIABLE)
	return(occurs_in(t2, t1) ? GREATER_THAN : NOT_COMPARABLE);
    else if (t1->sym_num == t2->sym_num) {
	r1 = t1->farg;
	r2 = t2->farg;
	i = SAME_AS;
	while (r1 != NULL && (i = lex_order(r1->argval,r2->argval)) == SAME_AS) {
	    r1 = r1->narg;
	    r2 = r2->narg;
	    }
	return(i);
	}
    else
	return(lex_compare_sym_nums(t1->sym_num, t2->sym_num));
}  /* lex_order */

/*************
 *
 *    int lex_order_vars(t1, t2)
 *
 *    Similar to lex_order, except that variables are lowest, and are ordered by number.
 *
 *
 *************/

static int lex_order_vars(t1, t2)
struct term *t1;
struct term *t2;
{
    struct rel *r1, *r2;
    int i, t1_special, t2_special;

    /* The following handles special unary functions that are to be */
    /* ignored during lex_check.  For example, when using lex-dependent */
    /* demodulation to sort AC expressions, you can make the canonical */
    /* form be a + -a + b + -b + c + -c.                              */

    t1_special = (t1->type == COMPLEX && sn_to_node(t1->sym_num)->special_unary);
    t2_special = (t2->type == COMPLEX && sn_to_node(t2->sym_num)->special_unary);
    if (t1_special && t2_special == 0)
	if (term_ident(t1->farg->argval, t2))
	    return(GREATER_THAN);
	else
	    return(lex_order_vars(t1->farg->argval, t2));
    else if (t1_special == 0 && t2_special)
	if (term_ident(t2->farg->argval, t1))
	    return(LESS_THAN);
	else
	    return(lex_order_vars(t1, t2->farg->argval));

    /* end of special_unary code */

    else if (t1->type == VARIABLE)
	if (t2->type == VARIABLE)
	    if (t1->varnum == t2->varnum)
		return(SAME_AS);
	    else
		return(t1->varnum > t2->varnum ? GREATER_THAN : LESS_THAN);
	else
	    return(LESS_THAN);

    else if (t2->type == VARIABLE)
	return(GREATER_THAN);

    else if (t1->sym_num == t2->sym_num) {
	r1 = t1->farg;
	r2 = t2->farg;
	i = SAME_AS;
	while (r1 != NULL && (i = lex_order_vars(r1->argval,r2->argval)) == SAME_AS) {
	    r1 = r1->narg;
	    r2 = r2->narg;
	    }
	return(i);
	}
    else
	return(lex_compare_sym_nums(t1->sym_num, t2->sym_num));
}  /* lex_order_vars */

/*************
 *
 *    int term_order(t1, t2)
 *
 *    Return SAME_AS, GREATER_THAN, LESS_THAN, or NOT_COMPARABLE.
 *
 *************/

static int term_order(t1, t2)
struct term *t1;
struct term *t2;
{
    int i1, i2;

    i1 = weight(t1, Weight_terms_index);
    i2 = weight(t2, Weight_terms_index);

    if (i1 > i2)
	return(GREATER_THAN);
    else if (i1 < i2)
	return(LESS_THAN);
    else
	return(lex_order(t1, t2));
}  /* term_order */

/*************
 *
 *    int lex_check(t1, t2)
 *
 *    Return SAME_AS, GREATER_THAN, LESS_THAN, or NOT_COMPARABLE.
 *
 *    Consult a flag to see if variables should be considered.
 *
 *************/

int lex_check(t1, t2)
struct term *t1;
struct term *t2;
{
    if (Flags[LEX_ORDER_VARS].val)
	return(lex_order_vars(t1, t2));
    else
	return(lex_order(t1, t2));
}  /* lex_check */

/*************
 *
 *    get_var_multiset(t, a)
 *
 *    Get (or continue getting) multiset of variables in t by
 *    Filling in array a.
 *
 *************/

static void get_var_multiset(t, a)
struct term *t;
int a[];
{
    struct rel *r;

    if (t->type == VARIABLE)
	a[t->varnum]++;
    else if (t->type == COMPLEX) {
	r = t->farg;
	while (r != NULL) {
	    get_var_multiset(r->argval, a);
	    r = r->narg;
	    }
	}
}  /* get_var_multiset */

/*************
 *
 *    int var_subset(t1, t2)
 *
 *    True if vars(t1) is a subset of vars(t2)
 *
 *************/

int var_subset(t1, t2)
struct term *t1;
struct term *t2;
{
    int t1_vars[MAX_VARS], t2_vars[MAX_VARS], i;

    for (i = 0; i < MAX_VARS; i++)
	t1_vars[i] = t2_vars[i] = 0;

    get_var_multiset(t1, t1_vars);
    get_var_multiset(t2, t2_vars);

    /* now make sure every variable in t1 is in t2 */

    for (i = 0; i < MAX_VARS; i++)
	if (t2_vars[i] == 0 && t1_vars[i] != 0)
	    return(0);
    
    return(1);

}  /* var_subset */

/*************
 *
 *    int sym_occur(sym_num, t)
 *
 *    True if sym_num is the symbol number of one of the constants
 *    or functors in t.
 *
 *************/

static int sym_occur(sym_num, t)
int sym_num;
struct term *t;
{
    struct rel *r;
    int found;

    if (t->type == VARIABLE)
	return(0);
    else if (t->sym_num == sym_num)
	return(1);  /* NAME or COMPLEX */
    else if (t->type == NAME)
	return(0);
    else {  /* complex with different sym_num */
	r = t->farg;
	found = 0;
	while (r != NULL && found == 0) {
	    found = sym_occur(sym_num, r->argval);
	    r = r->narg;
	    }
	return(found);
	}
}  /* sym_occur */

/*************
 *
 *    sym_elim(alpha, beta)
 *
 *    True if alpha is complex, all args of alpha are unique vars, functor 
 *    of alpha doesn't occur in beta, and subset(vars(beta),vars(alpha)) .
 *    (If true, alpha = beta can be made into a symbol-eliminating
 *    demodulator.)
 *
 *************/

static int sym_elim(alpha, beta)
struct term *alpha;
struct term *beta;
{
    struct rel *r;
    struct term *t1;
    int i, a[MAX_VARS], ok;

    if (alpha->type == VARIABLE)
	return(0);
    else {
	if (alpha->type == NAME)
	    ok = 0;
	else {
	    /* check for list of unique vars */
	    for (i = 0; i < MAX_VARS; i++)
		a[i] = 0;
	    ok = 1;
	    r = alpha->farg;
	    while (r != NULL && ok) {
		t1 = r->argval;
		ok = (t1->type == VARIABLE && a[t1->varnum] == 0);
		a[t1->varnum] = 1;
		r = r->narg;
		}
	    }
	if (ok == 0)
	    return(0);
	else { /* check that functor of alpha doesn't occur in beta */
	       /* and that vars(beta) is a subset of vars(alpha)    */
	    return(sym_occur(alpha->sym_num, beta) == 0 && var_subset(beta, alpha));
	    }
	}
}  /* sym_elim */

/*************
 *
 *    int order_equalities(c)
 *   
 *    For each equality literal (pos or neg), flip args if the right
 *    side is heavier.
 *
 *    return:
 *
 *      0 - should not be a demodulator
 *      1 - should be a demodulator
 *      2 - should be a lex-dependent demodulator
 *
 *************/

int order_equalities(c)
struct clause *c;
{
    struct literal *l;
    struct rel *r1, *r2;
    struct term *alpha, *beta;
    int rc;

    if (num_literals(c) == 1 && c->first_lit->atom->varnum == POS_EQ) {
	/* handle equality units differently from equality lits in nonunits */
	r1 = c->first_lit->atom->farg;
	r2 = r1->narg;
	alpha = r1->argval;
	beta  = r2->argval;

	if (term_ident(alpha, beta))
	    return(0);
	else if (Flags[SYMBOL_ELIM].val) {
	    if (sym_elim(alpha, beta))
		return(1);
	    else if (sym_elim(beta, alpha)) {  /* flip */
		r1->argval = beta;
		r2->argval = alpha;
		return(1);
		}
	    }

	if (occurs_in(beta, alpha))
	    return(1);
	else if (occurs_in(alpha, beta)) {  /* flip */
	    r1->argval = beta;
	    r2->argval = alpha;
	    return(1);
	    }

        rc = term_order(alpha, beta);
	if (rc == LESS_THAN || rc == GREATER_THAN) {
	    if (rc == LESS_THAN) {  /* flip */
		r1->argval = beta;
		r2->argval = alpha;
		}
	    if (var_subset(r2->argval, r1->argval) &&
		    (Flags[DYNAMIC_DEMOD_ALL].val ||
		     weight(r2->argval, Weight_terms_index) <= 1))
		return(1);
	    else
		return(0);
	    }
	else {  /* they are incomparable */
	    if (Flags[DYNAMIC_DEMOD_LEX_DEP].val == 0)
		rc = 0;
	    else if (var_subset(beta, alpha))
		rc = 2;  /* lex_dependent */
	    else if (var_subset(alpha, beta)) {  /* flip */
		r1->argval = beta;
		r2->argval = alpha;
		rc = 2;  /* lex_dependent */
		}
	    else
		rc = 0;

	    if (Flags[DYNAMIC_DEMOD_ALL].val == 0 || term_ident_x_vars(alpha, beta) == 0)
		rc = 0;  /* should not be a demodulator */

	    return(rc);
	    }
	}
	
    else {  /* not an equality unit */
	l = c->first_lit;
	while (l != NULL) {
	    if (l->atom->varnum == POS_EQ || l->atom->varnum == NEG_EQ) {
		r1 = l->atom->farg;
		r2 = r1->narg;
		alpha = r1->argval;
		beta  = r2->argval;
		
		if (term_order(alpha, beta) == LESS_THAN) {
		    r1->argval = beta;
		    r2->argval = alpha;
		    }
		}
	    l = l->next_lit;
	    }
	return(0);
	}
}  /* order_equalities */

/*************
 *
 *    int term_ident_x_vars(term1, term2) -- Compare two terms, ignoring variables
 *
 *        If identical except for vars, return(1); else return(0).  The bits
 *    field is not checked.
 *
 *************/

int term_ident_x_vars(t1, t2)
struct term *t1;
struct term *t2;
{
    struct rel *r1, *r2;

    if (t1->type != t2->type)
	return(0);
    else if (t1->type == COMPLEX) {
	if (t1->sym_num != t2->sym_num)
	    return(0);
	else {
	    r1 = t1->farg;
	    r2 = t2->farg;
	    while (r1 && term_ident_x_vars(r1->argval,r2->argval)) {
		r1 = r1->narg;
		r2 = r2->narg;
		}
	    return(r1 == NULL);
	    }
	}
    else if (t1->type == VARIABLE)
	return(1);
    else  /* NAME */
	return(t1->sym_num == t2->sym_num);
}  /* term_ident_x_vars */  

/*************
 *
 *    int new_function(eq, lst) - see if a new function should be introduced.
 *
 *    eq is an equality unit.  Try to introduce a new function.
 *    A new function and two new equalities introduced if each argument has 
 *    at least one variable not occurring in the other side.  The new function
 *    is a function of the common variables.
 *
 *    For example, f(x,y) = g(x,z) introduces f(x,y) = k1(x) and g(x,z) = k1(x).
 *
 *    In Otter return 1 if success,
 *	in Penguin returns 1 if success, PROOF or TROUBLE if pre_process()
 *	returns such special codes.
 *
 *************/

int new_function(eq, lst)
struct clause *eq;
struct list *lst;
{
    static int new_function_lex_val;  /* decremented each time */

    int t1_vars[MAX_VARS], t2_vars[MAX_VARS], common_vars[MAX_VARS];
    int i, j, t1_ok, t2_ok;

    struct term *atom, *t, *t1, *t2;
    struct rel *r, *prev, *r1, *r2;
    struct int_ptr *ip1, *ip2;
    struct clause *c;
    struct literal *l;
	int pp;
	struct term *temp;
	int tempint;

	pp = NO_PROOF;				/* default */
    atom = eq->first_lit->atom;
    t1 = atom->farg->argval;
    t2 = atom->farg->narg->argval;

    for (i = 0; i < MAX_VARS; i++)
	 t1_vars[i] = t2_vars[i] = 0;

    get_var_multiset(t1, t1_vars);
    get_var_multiset(t2, t2_vars);

    t1_ok = t2_ok = 0;

    for (i = 0; i < MAX_VARS; i++) {
	if (t1_vars[i] == 0 && t2_vars[i] != 0)
	    t2_ok = 1;
	else if (t2_vars[i] == 0 && t1_vars[i] != 0)
	    t1_ok = 1;
	common_vars[i] = (t1_vars[i] != 0 && t2_vars[i] != 0);
	}

    if (t1_ok == 0 || t2_ok == 0)
	return(0);
    else {
	/* each has a var not in the other */
	if (get_term(&t) == TROUBLE)
		return(TROUBLE);
	prev = NULL;
	j = 0;  /* conut for arity */
	for (i = 0; i < MAX_VARS; i++) {
	    if (common_vars[i]) {
		j++;
		if (get_rel(&r) == TROUBLE)
			return(TROUBLE);
		if (get_term(&temp) == TROUBLE)
			return(TROUBLE);
		r->argval = temp;
		r->argval->type = VARIABLE;
		r->argval->varnum = i;
		if (prev == NULL)
		    t->farg = r;
		else
		    prev->narg = r;
		prev = r;
		}	/* end of if common_vars */
	    }	/* end of for */
	if (j == 0)
	    t->type = NAME;
	else
	    t->type = COMPLEX;
	if (new_functor_name(j, &tempint) == TROUBLE)
		return(TROUBLE);
		t->sym_num = tempint;

	/* new functions should be light */
	sn_to_node(t->sym_num)->lex_val = --new_function_lex_val;

	for (i = 1; i <= 2; i++) {
	    if (get_clause(&c) == TROUBLE)
		return(TROUBLE);
	    if (get_literal(&l) == TROUBLE)
		return(TROUBLE);
	    l->sign = 1;
	    if (get_term(&temp) == TROUBLE)
		return(TROUBLE);
		l->atom = temp;
	    l->atom->type = COMPLEX;
	    l->atom->sym_num = atom->sym_num;
	    l->atom->varnum = atom->varnum;
	    c->first_lit = l;
	    l->container = c;
	    l->atom->occ.lit = l;
	    if (get_int_ptr(&ip1) == TROUBLE)
		return(TROUBLE);
	    if (get_int_ptr(&ip2) == TROUBLE)
		return(TROUBLE);
	    c->parents = ip1;
	    ip1->next = ip2;
	    ip1->i = NEW_FUNCTION_RULE;
	    ip2->i = eq->id;
	    if (get_rel(&r1) == TROUBLE)
		return(TROUBLE);
	    if (get_rel(&r2) == TROUBLE)
		return(TROUBLE);
	    l->atom->farg = r1;
	    r1->narg = r2;
	    if (i == 1) {
		if (copy_term(t1, &temp) == TROUBLE)
			return(TROUBLE);
		r1->argval = temp;
	        if (copy_term(t, &temp) == TROUBLE)
			return(TROUBLE);
		r2->argval = temp;
		}
	    else {
		if (copy_term(t2, &temp) == TROUBLE)
			return(TROUBLE);
		r1->argval = temp;
	        r2->argval = t;
		}
            Stats[CL_GENERATED]++;
	    /* pre_process clock is already on, so turn it off */
	    CLOCK_STOP(PRE_PROC_TIME)
	    pp = pre_process(c, 0, lst);
		if (pp == PROOF || pp == TROUBLE)
			return(pp);
	    CLOCK_START(PRE_PROC_TIME)
	    }
	return(1);
	}
}  /* new_function */

