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
 *  demod.c -- Demodulation (rewriting) routines.
 *
 */

#include "header.h"

/*************
 *
 *    int contract_lin(term, demods, context, demod_id_p,clrt)
 *
 *        Attempt to rewrite the top level of `term', using a
 *    sequential search of `demods'.  If success, term is freed; if fail,
 *    NULL is returned.
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter clrt.
 *
 *************/

static int contract_lin(t, demods, c, demod_id_p,clrt)
struct term *t;
int *demods;
struct context *c;
int *demod_id_p;
struct term **clrt;
{
    struct term *atom, *contractum, *t2, *t3, *beta;
    struct rel *alpha_rel;
    struct trail *tr;
    struct clause *p;
    struct list *d;
    int mult_flag, dummy, ok;
	int mok;

	*clrt = NULL;				/* default */
    tr = NULL;
    d = (struct list *) demods;
    if (d == NULL)
	return(NO_TROUBLE);
    p = d->first_cl;
    contractum = NULL;
    while (p != NULL && contractum == NULL) {
	atom = p->first_lit->atom;
	alpha_rel = (atom->varnum == CONDITIONAL_DEMOD ? atom->farg->narg
						   : atom->farg);
	tr = NULL;
	if (match(alpha_rel->argval, c, t, &tr, &mok) == TROUBLE)
		return(TROUBLE);
	if (mok)
	{
	    if (atom->varnum == CONDITIONAL_DEMOD) {
		if (apply_demod(atom->farg->argval, c, &dummy,&t2) == TROUBLE)
			return(TROUBLE);
		if (convenient_demod(t2,&t3) == TROUBLE)
			return(TROUBLE);
	ok = (t3->type == NAME && str_ident(sn_to_str(t3->sym_num),"$T"));
		zap_term_special(t3);
		}
	    else
		ok = 1;

	    if (ok) {
		beta = alpha_rel->narg->argval;
		mult_flag = 0;
		if (apply_demod(beta,c,&mult_flag,&contractum) == TROUBLE)
			return(TROUBLE);
		if (mult_flag) 
		    c->multiplier++;

		/* varnum == LEX_DEP_DEMOD means it's lex-dependent */
		if (p->first_lit->atom->varnum != LEX_DEP_DEMOD)
		    ok = 1;
		else if (Flags[LEX_RPO].val)
			{
		    if (lrpo_greater(t, contractum, &ok) == TROUBLE)
			return(TROUBLE);
			}
		else
		    ok = (lex_check(contractum, t) == LESS_THAN);

		if (ok) {
		    zap_term_special(t);
		    *demod_id_p = p->id;
		    }
		else {
		    zap_term_special(contractum);
		    contractum = NULL;
		    }
		}
	    clear_subst_1(tr);
	    }
	p = p->next_cl;
	}
    *clrt = contractum;	/* may be NULL */
    return(NO_TROUBLE);

}  /* contract_lin */

/*************
 *
 *    dollar_out_non_list(t) - Process $OUT(t).
 *
 *************/

static void dollar_out_non_list(t)
struct term *t;
{
    int i;

if (t->sym_num==Chr_sym_num && str_int(sn_to_str(t->farg->argval->sym_num),&i))
	fprintf(Fdout,"%c", i);
    else
	print_term(Fdout, t);
}  /* dollar_out_non_list */

/*************
 *
 *    dollar_out(t) - Process $OUT(t) or $OUT([t1,...,tn]).
 *
 *************/

static void dollar_out(t)
struct term *t;
{
    struct term *t1;

    if (proper_list(t)) {
	fprintf(Fdout,"\n");
        for (t1 = t; t1->sym_num != Nil_sym_num; t1 = t1->farg->narg->argval) {
	    if (t != t1)
		fprintf(Fdout," ");
	    dollar_out_non_list(t1->farg->argval);
	    }
	fprintf(Fdout,".\n\n");
	}
    else
	dollar_out_non_list(t);
}  /* dollar_out */

/*************
 *
 *    int dollar_contract(t,rt) - evaluate $EQ, $SUM, ...
 *
 *    If t is evaluated, it is deallocated.
 *    Return(NULL) if t cannot be evaluated.
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter rt.
 *
 *************/

static int dollar_contract(t,rt)
struct term *t;
struct term **rt;
{
    long i1, i2, i3;
    int b1, b3, op_code, op_type, s1t, s1f, s2t, s2f;
    char *s1, *s2, str[MAX_NAME];
    struct term *t1, *ta, *tb;
	int tempint;

	*rt = NULL;				/* default */
    /*
    if (t->type != COMPLEX)
        return(NULL);
    */
    op_code = sn_to_ec(t->sym_num);  /* get eval code */
    if (op_code < 1)
	return(NO_TROUBLE);
    switch(op_code) {
	case SUM_SYM:
	case PROD_SYM:
	case DIFF_SYM:
	case DIV_SYM:
	case MOD_SYM:
	case BIT_AND_SYM:
	case BIT_OR_SYM:
	case BIT_XOR_SYM:
	case SHIFT_RIGHT_SYM:
	case SHIFT_LEFT_SYM: op_type = 1; break; /* int x int -> int */
	case EQ_SYM:
	case NE_SYM:
	case LT_SYM:
	case LE_SYM:
	case GT_SYM:
	case GE_SYM:   op_type = 2; break; /* int x int -> bool */
	case AND_SYM:
	case OR_SYM:   op_type = 3; break; /* bool x bool -> bool */
	case TRUE_SYM:
	case NOT_SYM:  op_type = 4; break; /* bool -> bool */
	case IF_SYM:   op_type = 5; break; /* bool x term x term -> term */
	case LLT_SYM:
	case LLE_SYM:
	case LGT_SYM:
	case LGE_SYM:
	case LNE_SYM:
	case ID_SYM:  op_type = 6; break;          /* term x term -> bool */
	case NEXT_CL_NUM_SYM:  op_type = 7; break; /* -> int */
	case ATOMIC_SYM:
	case NUMBER_SYM:
	case GROUND_SYM:
	case VAR_SYM:  op_type = 8; break;         /* term -> bool */
	case T_SYM: return(NO_TROUBLE);
	case F_SYM: return(NO_TROUBLE);
	case OUT_SYM:  op_type = 9; break;      /* term -> same_term_with_output */
	case BIT_NOT_SYM:  op_type = 10; break; /* int -> int */
	default: fprintf(Fdout,"ERROR, dollar_contract, bad op_code: %d.\n",op_code); return(NO_TROUBLE);
	}

    switch (op_type) {
	case 1:  /* int x int -> int */
	    ta = t->farg->argval;
	    tb = t->farg->narg->argval;
	    if (ta->type != NAME || tb->type != NAME)
		return(NO_TROUBLE);

	    s1 = sn_to_str(ta->sym_num);
	    s2 = sn_to_str(tb->sym_num);
	    if (str_long(s1, &i1) == 0 || str_long(s2, &i2) == 0)
		return(NO_TROUBLE);

            if ((op_code == DIV_SYM || op_code == MOD_SYM) && i2 == 0) {
		output_stats(Fdout, 4);
		fprintf(Fderr, "ABEND, integer divide by 0.\007\n");
		fprintf(Fdout,"ABEND, integer divide by 0: ");
		print_term(Fdout, t),
		fprintf(Fdout,"\n");
		return(TROUBLE);
		}
	    switch (op_code) {
		case SUM_SYM:   i3 = i1 + i2; break;
		case PROD_SYM:  i3 = i1 * i2; break;
		case DIFF_SYM:  i3 = i1 - i2; break;
		case DIV_SYM:   i3 = i1 / i2; break;
		case MOD_SYM:   i3 = i1 % i2; break;
		case BIT_AND_SYM:      i3 = i1 & i2; break;
		case BIT_OR_SYM:       i3 = i1 | i2; break;
		case BIT_XOR_SYM:      i3 = i1 ^ i2; break;
		case SHIFT_RIGHT_SYM:  i3 = i1 >> i2; break;
		case SHIFT_LEFT_SYM:   i3 = i1 << i2; break;
		}
	    zap_term_special(t);
	    if (get_term(&t1) == TROUBLE)
		return(TROUBLE);
	    t1->type = NAME;
	    long_str(i3, str);
	    if (str_to_sn(str, 0, &tempint) == TROUBLE)
		return(TROUBLE);
		t1->sym_num = tempint;
		*rt = t1;
	    return(NO_TROUBLE);
	case 2:  /* int x int -> bool */
	    ta = t->farg->argval;
	    tb = t->farg->narg->argval;
	    if (ta->type != NAME || tb->type != NAME)
		return(NO_TROUBLE);

	    s1 = sn_to_str(ta->sym_num);
	    s2 = sn_to_str(tb->sym_num);
	    if (str_long(s1, &i1) == 0 || str_long(s2, &i2) == 0)
		return(NO_TROUBLE);
	    switch (op_code) {
		case EQ_SYM:    b3 = i1 == i2; break;
		case NE_SYM:    b3 = i1 != i2; break;
		case LT_SYM:    b3 = i1 <  i2; break;
		case LE_SYM:    b3 = i1 <= i2; break;
		case GT_SYM:    b3 = i1 >  i2; break;
		case GE_SYM:    b3 = i1 >= i2; break;
		}
            t->occ.lit = NULL; /* in case t is a literal */
	    zap_term_special(t);
	    if (get_term(&t1) == TROUBLE)
		return(TROUBLE);
	    t1->type = NAME;
	    if (str_to_sn(b3 ? "$T" : "$F", 0, &tempint) == TROUBLE)
		return(TROUBLE);
		t1->sym_num = tempint;
		*rt = t1;
	    return(NO_TROUBLE);
	case 3:  /* bool x bool -> bool */
	    s1 = sn_to_str(t->farg->argval->sym_num);
	    s2 = sn_to_str(t->farg->narg->argval->sym_num);
	    s1t = str_ident(s1,"$T");
	    s1f = str_ident(s1,"$F");
	    s2t = str_ident(s2,"$T");
	    s2f = str_ident(s2,"$F");
	    if ((s1t == 0 && s1f == 0) || (s2t == 0 && s2f == 0))  
		return(NO_TROUBLE);
	    switch (op_code) {
		case AND_SYM:   b3 = s1t && s2t; break;
		case OR_SYM:    b3 = s1t || s2t; break;
		}
            t->occ.lit = NULL; /* in case t is a literal */
	    zap_term_special(t);
	    if (get_term(&t1) == TROUBLE)
		return(TROUBLE);
	    t1->type = NAME;
	    if (str_to_sn(b3 ? "$T" : "$F", 0, &tempint) == TROUBLE)
		return(TROUBLE);
		t1->sym_num = tempint;
		*rt = t1;
	    return(NO_TROUBLE);
	case 4:  /* bool -> bool  $NOT(x), $TRUE(x) */
	    s1 = sn_to_str(t->farg->argval->sym_num);
	    s1t = str_ident(s1,"$T");
	    s1f = str_ident(s1,"$F");
	    if (s1t == 0 && s1f == 0)
		return(NO_TROUBLE);
            t->occ.lit = NULL; /* in case t is a literal */
	    zap_term_special(t);
	    if (get_term(&t1) == TROUBLE)
		return(TROUBLE);
	    t1->type = NAME;
	    switch (op_code) {
case NOT_SYM:  if (str_to_sn(s1t ? "$F" : "$T", 0, &tempint) == TROUBLE)
			return(TROUBLE);
			t1->sym_num = tempint;
                               break;
case TRUE_SYM: if (str_to_sn(s1t ? "$T" : "$F", 0, &tempint) == TROUBLE)
			return(TROUBLE);
			t1->sym_num = tempint;
                               break;
		}
		*rt = t1;
	    return(NO_TROUBLE);
	case 5:  /* bool x term x term -> term   $IF(x,y,z) */
	    s1 = sn_to_str(t->farg->argval->sym_num);
	    s1t = str_ident(s1,"$T");
	    s1f = str_ident(s1,"$F");
	    if (s1t == 0 && s1f == 0)
		return(NO_TROUBLE);
	    if (s1t)
		t1 = t->farg->narg->argval;
	    else
		t1 = t->farg->narg->narg->argval;
	    t1->fpa_id++;  /* one more pointer to t1 */
	    zap_term_special(t);  /* one less pointer to t */
		*rt = t1;
	    return(NO_TROUBLE);
	case 6:  /* term x term -> bool (lexical comparisons) */
	    ta = t->farg->argval;
	    tb = t->farg->narg->argval;
	    b1 = lex_check(ta, tb);
	    switch (op_code) {
		case ID_SYM: b3 =  (b1 == SAME_AS); break;
		case LNE_SYM: b3 = (b1 != SAME_AS); break;
		case LLT_SYM: b3 = (b1 == LESS_THAN); break;
		case LLE_SYM: b3 = (b1 == LESS_THAN || b1 == SAME_AS); break;
		case LGT_SYM: b3 = (b1 == GREATER_THAN); break;
		case LGE_SYM: b3 = (b1 == GREATER_THAN || b1 == SAME_AS); break;
		}
	    t->occ.lit = NULL; /* in case t is a literal */
	    zap_term_special(t);
	    if (get_term(&t1) == TROUBLE)
		return(TROUBLE);
	    t1->type = NAME;
	    if (str_to_sn(b3 ? "$T" : "$F", 0, &tempint) == TROUBLE)
		return(TROUBLE);
		t1->sym_num = tempint;
		*rt = t1;
	    return(NO_TROUBLE);
	case 7:  /* -> int */
	    int_str(next_cl_num(), str);
	    if (str_to_sn(str, 0, &tempint) == TROUBLE)
		return(TROUBLE);
		t1->sym_num = tempint;
		*rt = t;
	    return(NO_TROUBLE);
	case 8:  /* term -> bool (metalogical properties) */
	    ta = t->farg->argval;
	    switch (op_code) {
		case ATOMIC_SYM: b3 = ta->type == NAME; break;
		case NUMBER_SYM:
		    b3 = ( ta->type == NAME && 
			   str_long(sn_to_str(ta->sym_num), &i1));
		    break;
		case VAR_SYM: b3 = ta->type == VARIABLE; break;
		case GROUND_SYM: b3 = ground(ta); break;
		}

	    t->occ.lit = NULL; /* in case t is a literal */
	    zap_term_special(t);
	    if (get_term(&t1) == TROUBLE)
		return(TROUBLE);
	    t1->type = NAME;
	    if (str_to_sn(b3 ? "$T" : "$F", 0, &tempint) == TROUBLE)
		return(TROUBLE);
		t1->sym_num = tempint;
		*rt = t1;
	    return(NO_TROUBLE);
	case 9:  /* term -> same_term_with_output */
	    dollar_out(t->farg->argval);
	    return(NO_TROUBLE);
	case 10:  /* int -> int */
	    s1 = sn_to_str(t->farg->argval->sym_num);
	    if (str_long(s1, &i1) == 0)
		return(NO_TROUBLE);
	    switch (op_code) {
	        case BIT_NOT_SYM: i3 = ~i1;
		break;
		}
	    zap_term_special(t);
	    if (get_term(&t1) == TROUBLE)
		return(TROUBLE);
	    t1->type = NAME;
	    long_str(i3, str);
            if (str_to_sn(str, 0, &tempint) == TROUBLE)
		return(TROUBLE);
		t1->sym_num = tempint;
		*rt = t1;
	    return(NO_TROUBLE);
	}
    fprintf(Fdout,"ERROR, dollar_contract, bad op_type: %d.\n", op_type);
    return(NO_TROUBLE);
}  /* dollar_contract */

/*************
 *
 *    int demod(term,contract_proc,demods,count,context,histp,drt)
 *
 *    Demodulate a term.
 *
 *        The demodulated term is returned, and the given term
 *    becomes garbage, so a good way to invoke is `t = demod(t, demods, ...'.
 *    A context must be allocated before the call--the same one is used
 *    for all subterms--this saves allocating and deallocating at
 *    each subterm.  `count' is pointer to the maximum number of 
 *    rewrites that will be applied.  `contract_proc' is a pointer to the
 *    routine that looks for demodulators and does the rewriting.
 *    The type of `demods' depends on `contract_proc'.
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter drt.
 *
 *************/

static int demod(t,contract_proc,demods,count,c,histp,drt)
struct term *t;
int (*contract_proc)();
int *demods;
long *count;
struct context *c;
struct int_ptr **histp;
struct term **drt;
{
    struct rel *r;
    struct term *t1;
    struct int_ptr *ip;
    int demod_id;
	struct term *temp;

	*drt = NULL;				/* default */
    if (t->type == VARIABLE || TP_BIT(t->bits, SCRATCH_BIT))
	{
	*drt = t;
	return(NO_TROUBLE);
	}
/* don't try to demodulate if a variable or if already fully demodulated */
    else if (t->type == COMPLEX) {
     
	/* if $IF, evaluate right now! */

	if (Internal_flags[DOLLAR_PRESENT] && sn_to_ec(t->sym_num) == IF_SYM) {
	    /* first reduce condition */
if (demod(t->farg->argval,contract_proc,demods,count,c,histp,&temp)==TROUBLE)
		return(TROUBLE);
		t->farg->argval = temp;
	    /* now evaluate $IF */
	    if (dollar_contract(t,&t1) == TROUBLE)
		return(TROUBLE);
	    if (t1 != NULL) {
		(*count)--;
	   if (demod(t1,contract_proc,demods,count,c,histp,&temp)==TROUBLE)
		return(TROUBLE);
		*drt = temp;
		return(NO_TROUBLE);
		}
	    }


        /* fully demodulate subterms */

	r = t->farg;
	while (r != NULL && *count > 0) {
if (demod(r->argval, contract_proc, demods, count, c, histp,&temp)==TROUBLE)
		return(TROUBLE);
		r->argval = temp;
	    r = r->narg;
	    }
	}
    
    if (*count > 0) {
	if ((*contract_proc)(t, demods, c, &demod_id, &t1) == TROUBLE)
		return(TROUBLE);
	if (t1 != NULL) {
	    (*count)--;
	    if (*histp != NULL) {
		if (get_int_ptr(&ip) == TROUBLE)
			return(TROUBLE);
		ip->i = demod_id;
		(*histp)->next = ip;
		*histp = ip;
		}
	    if (demod(t1,contract_proc,demods,count,c,histp,&temp)==TROUBLE)
		return(TROUBLE);
		t = temp;
	    }
	else if (Internal_flags[DOLLAR_PRESENT]) {
	    if (dollar_contract(t,&t1) == TROUBLE)
		return(TROUBLE);
	    if (t1 != NULL) {
		(*count)--;
		t = t1;
		}
	    }
	SET_BIT(t->bits, SCRATCH_BIT);
	}
	*drt = t;
    return(NO_TROUBLE);
}  /* demod */

/*************
 *
 *    int left_most_one_step(t, contract_proc, demods, c, histp,lmos)
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter lmos.
 *
 *************/

static int left_most_one_step(t, contract_proc, demods, c, histp,lmos)
struct term *t;
int (*contract_proc)();
int *demods;
struct context *c;
struct int_ptr **histp;
struct term **lmos;
{
    struct term *t1;
    struct int_ptr *ip;
    struct rel *r;
    int demod_id;

	*lmos = NULL;				/* default */
    if (t->type == VARIABLE || TP_BIT(t->bits, SCRATCH_BIT))
	return(NO_TROUBLE);
    else {
	if ((*contract_proc)(t, demods, c, &demod_id, &t1) == TROUBLE)
		return(TROUBLE);
	if (t1 != NULL) {
	    if (*histp != NULL) {
		if (get_int_ptr(&ip) == TROUBLE)
			return(TROUBLE);
		ip->i = demod_id;
		(*histp)->next = ip;
		*histp = ip;
		}
	    }
	else {
	    if (Internal_flags[DOLLAR_PRESENT]) {
		if (dollar_contract(t,&t1) == TROUBLE)
			return(TROUBLE);
		}
	    }
	
	if (t1 != NULL)
		{
		*lmos = t1;
	    return(NO_TROUBLE);
		}
	else {
	    r = t->farg;
	    while (r != NULL) {
if (left_most_one_step(r->argval,contract_proc,demods,c,histp,&t1)==TROUBLE)
		return(TROUBLE);
		if (t1 != NULL) {
		    r->argval = t1;
			*lmos = t;
		    return(NO_TROUBLE);
		    }
		SET_BIT(r->argval->bits, SCRATCH_BIT);
		r = r->narg;
		}
		*lmos = NULL;
	    return(NO_TROUBLE);
	    }
	}
}  /* left_most_one_step */

/*************
 *
 *    int demod_out_in(term, contract_proc, demods, count, subst, histp,doi)
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the 
 *	parameter doi.
 *
 *************/

static int demod_out_in(t, contract_proc, demods, count, c, histp,doi)
struct term *t;
int (*contract_proc)();
int *demods;
long *count;
struct context *c;
struct int_ptr **histp;
struct term **doi;
{
    struct term *t1;

	*doi = NULL;				/* default */
    if (left_most_one_step(t,contract_proc,demods,c,histp,&t1) == TROUBLE)
		return(TROUBLE);
    while (t1 != NULL) {
	(*count)--;
	if (*count <= 0)
		{
		*doi = t1;
	    return(NO_TROUBLE);
		}
	else {
	    t = t1;
if (left_most_one_step(t,contract_proc,demods,c,histp,&t1) == TROUBLE)
		return(TROUBLE);
	    }
	}
	*doi = t;
    return(NO_TROUBLE);
}  /* demod_out_in */

/*************
 *
 *    int un_share_special(term)
 *
 *        Given a term in which some of the subterms (not the term 
 *    itself) may be referenced more than once (fpa_id > 0),
 *    transform it into a term in which all of the subterms
 *    are referenced exactly once (a normal nonintegrated term)
 *    by copying the appropriate subterms.
 *    Also clear the SCRATCH bit in all subterms visited.
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

static int un_share_special(t)
struct term *t;
{
    struct rel *r;
	struct term *temp;

    CLEAR_BIT(t->bits, SCRATCH_BIT);
    if (t->type != COMPLEX)
	return(NO_TROUBLE);
    else {
	r = t->farg;
	while (r != NULL) {
	    if (r->argval->fpa_id != 0) {
		r->argval->fpa_id--;
		if (copy_term(r->argval, &temp) == TROUBLE)
			return(TROUBLE);
		r->argval = temp;
		}
	    else
		if (un_share_special(r->argval) == TROUBLE)
			return(TROUBLE);
	    r = r->narg;
	    }
	}
return(NO_TROUBLE);
}  /* un_share_special */

/*************
 *
 *    int convenient_demod(t,cd)
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter cd.
 *
 *************/

int convenient_demod(t,cd)
struct term *t;
struct term **cd;
{
    struct term *t1;
    struct context *c;
    struct int_ptr *hist;
    long limit;

	*cd = NULL;				/* default */
    limit = Parms[DEMOD_LIMIT].val;

    limit = (limit == 0 ? MAX_LONG_INT : limit);

    hist = NULL;  /* so that history will not be kept */

    if (get_context(&c) == TROUBLE)
	return(TROUBLE);

    if (Flags[DEMOD_LINEAR].val)
	{
if (demod(t,contract_lin,(int *) Demodulators,&limit,c,&hist,&t1)==TROUBLE)
	return(TROUBLE);
	}
    else
	{
if (demod(t,contract_imd,(int *) Demod_imd,&limit,c,&hist,&t1) == TROUBLE)
	return(TROUBLE);
	}

    free_context(c);
	*cd = t1;
    return(NO_TROUBLE);

}  /* convenient_demod */

/*************
 *
 *    zap_term_special(term)  --  Special term deletion.
 *
 *        Deletion of nonintegrated term in which the term and
 *    some of its subterms might be referenced more than once.
 *    term->fpa_id is a count of the number of extra references.
 *    If we get to a term with more than one reference, the
 *    decrement the number of references; else recurse on the
 *    subterms and free the node.
 *
 *************/

void zap_term_special(t)
struct term *t;
{
    struct rel *r1, *r2;
    
    if (t->occ.rel != NULL) {
	fprintf(Fdout,"WARNING, zap_term_special called with contained term: ");
	print_term(Fdout, t);
	fprintf(Fdout,"\n");
	}
    else if (t->fpa_id != 0)
	t->fpa_id--;
    else {
	if (t->type == COMPLEX) { /* complex term */
	    r1 = t->farg;
	    while (r1 != NULL) {
		zap_term_special(r1->argval);
		r2 = r1;
		r1 = r1->narg;
		free_rel(r2);
		}
	    }
	free_term(t);
	}
}  /* zap_term_special */

/*************
 *
 *    int apply_demod(term, context, mult_flag_ptr,ad) -- Special purpose apply.
 *
 *         If `term' is a variable instantiated to some term t2,
 *    then increment the reference count of t2, and return t2;
 *    else create a new node and recurse on any subterms. 
 *
 *    mult_flag_ptr is a pointer to flag for incrementing multiplier
 *    for demodulators that have a variable on the right that doesn't
 *    occur on the left:  If an uninstantiated variable is encountered,
 *    then the flag is set;  when finished applying to beta, if the
 *    flag is set, then the multiplier is incremented.
 *    (It may be the case that demods of this type are not allowed.)
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter ad.
 *
 *************/

int apply_demod(t, c, pf,ad)
struct term *t;
struct context *c;
int *pf;
struct term **ad;
{
    struct term *t2;
    struct rel *r1, *r2, *r3;
	struct term *temp;

	*ad = NULL;				/* default */
    if (t->type == VARIABLE && c->terms[t->varnum] != NULL) {  /* bound var */
	t2 = c->terms[t->varnum];
	t2->fpa_id++;  /* count of extra references to a term */
	*ad = t2;
	return(NO_TROUBLE);
	}
    
    if (t->type == VARIABLE) {  /* unboud variable */
        if (!Flags[ORDER_EQ].val) {
	    /* order_eq causes clauses to be renumbered before demod.  */
	    /* If not renumbered, new vars in beta may map to existing var. */
	    output_stats(Fdout, 4);
fprintf(Fderr,"ABEND, must set order_eq, because beta has variable not in alpha.\007\n");
fprintf(Fdout,"ABEND, must set order_eq, because beta has variable not in alpha.");
	    return(TROUBLE);
	    }
	if (get_term(&t2) == TROUBLE)
		return(TROUBLE);
	t2->type = VARIABLE;
	t2->varnum = c->multiplier * MAX_VARS + t->varnum;
	*pf = 1;  /* when finished applying to beta, increment multiplier */
	*ad = t2;
	return(NO_TROUBLE);
	}
    else if (t->type == NAME) {  /* name */
	if (get_term(&t2) == TROUBLE)
		return(TROUBLE);
	t2->type = NAME;
	t2->sym_num = t->sym_num;
	*ad = t2;
	return(NO_TROUBLE);
	}
    else {  /* complex term */
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
	    if (apply_demod(r1->argval, c, pf,&temp) == TROUBLE)
		return(TROUBLE);
		r2->argval = temp;
	    r3 = r2;
	    r1 = r1->narg;
	    }
	*ad = t2;
	return(NO_TROUBLE);
	}
}  /* apply_demod */

/*************
 *
 *    int demod_cl(c,changed) -- demodulate a clause
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE
 *
 *	Penguin adds the parameter changed to say whether the clause has
 *	been demodulated or not.
 *
 *************/

int demod_cl(c,changed)
struct clause *c;
int *changed;
{
    struct literal *lit;
    struct term *atom;
    int linear, out_in, hist;
    long limit_save, limit;
    struct context *subst;
    struct int_ptr *ip_save, *ip_send, *ip_d;
    int *d;
	struct term *temp;

    linear = Flags[DEMOD_LINEAR].val;
    limit = Parms[DEMOD_LIMIT].val;
    out_in = Flags[DEMOD_OUT_IN].val;
    limit_save = limit = (limit == 0 ? MAX_LONG_INT : limit);
    hist = Flags[DEMOD_HISTORY].val;
    if (c->parents == NULL)
	{
	if (get_int_ptr(&ip_save) == TROUBLE)
		return(TROUBLE);
	ip_d = ip_save;
	}
    else {
	ip_save = c->parents;
	while (ip_save->next != NULL)
	    ip_save = ip_save->next;
	ip_d = NULL;
	}
    /* ip_save saves position to insert "d" if any demodulation occurs */
    ip_send = (hist ? ip_save : NULL);
    if (get_context(&subst) == TROUBLE)
	return(TROUBLE);
    subst->multiplier = 1;
    if (linear)
	d = (int *) Demodulators;
    else
	d = (int *) Demod_imd;

    lit = c->first_lit;
    while (lit != NULL && limit > 0) {
	atom = lit->atom;
	atom->occ.lit = NULL;  /* reset at end of loop */
	if (out_in) {
	    if (linear)
		{
if (demod_out_in(atom,contract_lin,d,&limit,subst,&ip_send,&temp) == TROUBLE)
		return(TROUBLE);
		atom = temp;
		}
	    else
		{
if (demod_out_in(atom,contract_imd,d,&limit,subst,&ip_send,&temp) == TROUBLE)
		return(TROUBLE);
		atom = temp;
		}
	    }
	else {
	    if (linear)
		{
	if (demod(atom,contract_lin,d,&limit,subst,&ip_send,&temp) == TROUBLE)
		return(TROUBLE);
		atom = temp;
		}
	    else
		{
	if (demod(atom,contract_imd,d,&limit,subst,&ip_send,&temp) == TROUBLE)
		return(TROUBLE);
		atom = temp;
		}
	    }
	if (un_share_special(atom) == TROUBLE)
		return(TROUBLE);
	if (atom->varnum == TERM) {  /* if the atom itself was changed */
	    lit->atom = atom;
	    mark_literal(lit);
	    }
	atom->occ.lit = lit;
	lit = lit->next_lit;
	}

    if (subst->multiplier != 1) {
	/* new variables were introduced */
	if (renumber_vars(c) == 0) {
	    output_stats(Fdout, 4);
fprintf(Fderr,"ABEND, demod_cl, demodulation introduced too many variables.\007\n");
fprintf(Fdout,"ABEND, demod_cl, demodulation introduced too many variables:\n");
	    print_clause(Fdout, c);
	    return(TROUBLE);
	    }
	
	}

    if (limit <= 0) {
	fprintf(Fderr, "WARNING, demod_limit.\n");
	fprintf(Fdout,"WARNING, demod_limit:");
	print_clause(Fdout, c);
	}
    /* if some demodulation occured, insert "d" into parent list */
    if (limit_save > limit) {
	if (ip_d != NULL) {
	    c->parents = ip_d;
	    }
	else {
	    if (get_int_ptr(&ip_d) == TROUBLE)
		return(TROUBLE);
	    ip_d->next = ip_save->next;
	    ip_save->next = ip_d;
	    }
	ip_d->i = DEMOD_RULE;
	*changed = 1;
	}
    else {
		if (ip_d != NULL)
			free_int_ptr(ip_d);
		*changed = 0;
	}
    Stats[REWRITES] += (limit_save - limit);
    free_context(subst);
return(NO_TROUBLE);
}  /* demod_cl */

/*************
 *
 *    int back_demod(d,c,ct,lst) - back demodulate with d
 *
 *    But don't back demodulate d or c
 *
 *	Penguin adds the parameter ct, which back_demod() inherits from
 *	post_process() and which can be IR for Input Read clause, i.e. an
 *	input clause read at the Penguin which reads the input, or IM for
 *	Input Message, i.e. an input clause received as message by a
 *	Penguin which did not read the input or 0 in all other cases.
 *	See comment at the call to pre_process().
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

#ifdef ROO

int back_demod(d,c,ct,lst)
struct clause *d;
struct clause *c;
int ct;
struct list *lst;
{
    struct term *atom;
    struct term_ptr *tp, *tp2;
    struct clause_ptr *cp, *cp2;
    struct int_ptr *ip0, *ip1;
    struct clause *c1, *c2;
	int pp;
    
	pp = NO_PROOF;				/* default */
    atom = d->first_lit->atom;

    if (Flags[INDEX_FOR_BACK_DEMOD].val)
	{
	if (all_instances_fpa(atom,&tp) == TROUBLE)
		return(TROUBLE);
	}
    else
	{
	if (all_instances(atom, &tp) == TROUBLE)
		return(TROUBLE);
	}

    cp = NULL;
    while (tp != NULL) {
if (all_cont_cl(tp->term, &cp) == TROUBLE)
		return(TROUBLE);
	tp2 = tp;
	tp = tp->next;
	free_term_ptr(tp2);
	}

    /* clause ID's are decreasing. */
    while (cp != NULL) {
	c1 = cp->c;
	if (c1 != d && c1 != c) {  /* don't back demodulate c or d */
	    if (c1->container == Demodulators)
	        c1->first_lit->atom->varnum = POS_EQ;  /* in case lex-dep */
	    if (c1->container == Demodulators &&
		c1->parents != NULL &&
		c1->parents->i == NEW_DEMOD_RULE)

                /* mark to be deleted, but don't back demodulate, */
		/* because the non-demodulator copy will be back  */
		/* demodulated.                                   */

		roo_delete(c1);
	    
	    else {
		Stats[CL_BACK_DEMOD]++;
		if (Flags[PRINT_BACK_DEMOD].val) {
		fprintf(Fdout,"%u: %d:   >> Back demodulating ",xx_clock(),Pid);
		print_ids(Fdout,c1);
		fprintf(Fdout," with ");
		print_ids(Fdout,d);
		fprintf(Fdout,".\n");
		    fflush(Fdout);
		    }
		if (cl_copy(c1, &c2) == TROUBLE)
			return(TROUBLE);
		if (c2->pid == Whoami)
			c2->bt = MAX_INT;
/* Penguin:								*/
/* cl_copy() copies the birth time also, whereas in this context we need to */
/* reset it to MAX_INT, because c2 is like a raw critical pair and its birth */
/* time will be set by cl_integrate() in pre_process(), if c2 is not deleted */ 
/* by proc_gen() called by pre_process().				*/
		if (get_int_ptr(&ip0) == TROUBLE)
			return(TROUBLE);
		ip0->i = BACK_DEMOD_RULE;
		if (get_int_ptr(&ip1) == TROUBLE)
			return(TROUBLE);
		ip1->i = c1->id;
		c2->parents = ip0; ip0->next = ip1;
		roo_delete(c1);
		CLOCK_STOP(BACK_DEMOD_TIME)
if (Flags[PROCESS_INPUT].val && Flags[POST_PROC_NS_BEFORE_SEND].val && ct==IR)
		pp = pre_process(c2, IR, lst);
		else
		pp = pre_process(c2, 0, lst);
/* Penguin:								*/
/* Clause c2 is a raw clause newly generated by back_demodulation, 	 */
/* and therefore pre_process() should be called with the second parameter ct */
/* set to 0. However, if we are back_demodulating an input clause, 	*/
/* ct == IR, BEFORE sending the input clause to the other Penguins, its	*/
/* reduced form should also be regarded as input clause.		*/
/* We check for POST_PROC_NS_BEFORE_SEND to be on, because back_demod() is */
/* done during post-processing and post-processing of input clauses, treated */
/* in this respect similar to new_settlers, is done before sending 	*/
/* only if POST_PROC_NS_BEFORE_SEND is on. If it is off, post_processing is */
/* done after sending.							*/
		if (pp == PROOF || pp == TROUBLE)
			return(pp);
		CLOCK_START(BACK_DEMOD_TIME)
		}
	    }
	cp2 = cp;
	cp = cp->next;
	free_clause_ptr(cp2);
	}
return(NO_PROOF);
}  /* back_demod() */

#else  /* not ROO */

int back_demod(d,c,ct,lst)
struct clause *d;
struct clause *c;
int ct;
struct list *lst;
{
    struct term *atom;
    struct term_ptr *tp, *tp2;
    struct clause_ptr *cp, *cp2;
    struct int_ptr *ip0, *ip1;
    struct clause *c1, *c2;
    int ok;
	int pp;
    
	pp = NO_PROOF;				/* default */
    atom = d->first_lit->atom;

    if (Flags[INDEX_FOR_BACK_DEMOD].val)
	{
	if (all_instances_fpa(atom,&tp) == TROUBLE)
		return(TROUBLE);
	}
    else
	{
	if (all_instances(atom, &tp) == TROUBLE)
		return(TROUBLE);
	}

    cp = NULL;
    while (tp != NULL) {
	if (all_cont_cl(tp->term, &cp) == TROUBLE)
		return(TROUBLE);
	tp2 = tp;
	tp = tp->next;
	free_term_ptr(tp2);
	}

    while(cp != NULL) {
	c1 = cp->c;
	ok = 0;
	if (c1 == d || c1 == c)
	    ok = 0;  /* don't back demodulate yourself */
	else if (c1->container == Usable) {
	    ok = 1;
	    if (un_index_lits_all(c1) == TROUBLE)
		return(TROUBLE);
	    if (un_index_lits_clash(c1) == TROUBLE)
		return(TROUBLE);
	    Stats[USABLE_SIZE]--;		/* Penguin */
	    }
	else if (c1->container == Sos) {
	    ok = 1;
	    if (un_index_lits_all(c1) == TROUBLE)
		return(TROUBLE);
	    Stats[SOS_SIZE]--;
	    }
	else if (c1->container == Demodulators)
		{
		Stats[DEMODULATORS_SIZE]--;		/* Penguin */
	    if (Flags[DEMOD_LINEAR].val == 0)
		if (imd_delete(c1, Demod_imd) == TROUBLE)
			return(TROUBLE);
	    if (c1->parents != NULL && c1->parents->i == NEW_DEMOD_RULE)
		{ 
		/* just delete it.  this works because list is decreasing; */
		/* dynamic demodulator has id greater than parent. */
		rem_from_list(c1);
	        hide_clause(c1);
		ok = 0;
		}
	    else {
	        c1->first_lit->atom->varnum = POS_EQ;  /* in case lex-dep */
	        ok = 1;
		}
	    }
	if (ok) {
	    Stats[CL_BACK_DEMOD]++;
	    if (Flags[PRINT_BACK_DEMOD].val)
		{
		fprintf(Fdout,"   >> Back demodulating ");
		print_ids(Fdout,c1);
		fprintf(Fdout," with ");
		print_ids(Fdout,d);
		fprintf(Fdout,".\n");
		}
	    rem_from_list(c1);
	    if (cl_copy(c1, &c2) == TROUBLE)
		return(TROUBLE);
	    if (c2->pid == Whoami)
		c2->bt = MAX_INT;
/* cl_copy() copies the birth time also, whereas in this context we need to */
/* reset it to MAX_INT, because c2 is like a raw critical pair and its birth */
/* time will be set by cl_integrate() in pre_process(), if c2 is not deleted */ 
/* by proc_gen() called by pre_process().				*/
	    if (get_int_ptr(&ip0) == TROUBLE)
		return(TROUBLE);
		ip0->i = BACK_DEMOD_RULE;
	    if (get_int_ptr(&ip1) == TROUBLE)
		return(TROUBLE);
		ip1->i = c1->id;
	    c2->parents = ip0; ip0->next = ip1;
	    hide_clause(c1);
	    CLOCK_STOP(BACK_DEMOD_TIME)
	    CLOCK_STOP(POST_PROC_TIME)
if (Flags[PROCESS_INPUT].val && Flags[POST_PROC_NS_BEFORE_SEND].val && ct==IR)
		pp = pre_process(c2, IR, lst);
		else
		pp = pre_process(c2, 0, lst);
/* Clause c2 is a raw clause newly generated by back_demodulation, 	 */
/* and therefore pre_process() should be called with the second parameter ct */
/* set to 0. However, if we are back_demodulating an input clause, 	*/
/* ct == IR, BEFORE sending the input clause to the other Penguins, its	*/
/* reduced form should also be regarded as input clause.		*/
/* We check for POST_PROC_NS_BEFORE_SEND to be on, because back_demod() is */
/* done during post-processing and post-processing of input clauses, treated */
/* in this respect similar to new_settlers, is done before sending 	*/
/* only if POST_PROC_NS_BEFORE_SEND is on. If it is off, post_processing is */
/* done after sending.							*/
	    if (pp == PROOF || pp == TROUBLE)
			return(pp);
	    CLOCK_START(POST_PROC_TIME)
	    CLOCK_START(BACK_DEMOD_TIME)
	    }
	cp2 = cp;
	cp = cp->next;
	free_clause_ptr(cp2);
	}
return(NO_PROOF);
}  /* back_demod */

#endif  /* not ROO */

/*************
 *
 *    int lit_t_f_reduce(c) -- evaluate evaluable literals 
 *
 *    delete any false literals, $F or -$T.
 *
 *    return:  0 -- clause evaluated successfully.
 *             1 -- clause evaluated successfully to TRUE (clause/lits
 *                  not deallocated).
 *
 *************/

int lit_t_f_reduce(c)
struct clause *c;
{
    struct literal *lit, *prev_lit, *next_lit;
    int atom_true, atom_false;
    char *s;

    lit = c->first_lit;
    prev_lit = NULL;
    while (lit != NULL) {
	next_lit = lit->next_lit;
	if (lit->atom->varnum == EVALUABLE) {
	    s = sn_to_str(lit->atom->sym_num);
	    atom_true = str_ident(s, "$T");
	    atom_false = str_ident(s, "$F");
	    if (atom_true || atom_false) {
		if ((atom_true && lit->sign ) ||
		    (atom_false && lit->sign == 0 ))
		    return(1);  /* lit is true, so clause is true */
		else if ((atom_false && lit->sign) ||
		         (atom_true && lit->sign == 0 )) {
		    /* lit is false, so delete it */
		    if (prev_lit == NULL) {
			c->first_lit = next_lit;
			lit->atom->occ.lit = NULL;
			zap_term(lit->atom);
			free_literal(lit);
			lit = NULL;
			}
		    else {
			prev_lit->next_lit = next_lit;
			lit = prev_lit;
			}
		    }
		}
	    }
	prev_lit = lit;
	lit = next_lit;
	}
    return(0);
}  /* lit_t_f_reduce */

/*************
 *
 *    int check_input_demod(c, cid)
 *
 *    Check if it is a valid demodulator, possibly flipping and
 *    making lex_dependent.
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE and 0/1 through the
 *	parameter cid.
 *
 *************/

int check_input_demod(c, cid)
struct clause *c;
int *cid;
{
    struct term *atom, *alpha, *beta;
	int tempint, tempint2;

	*cid = 0;				/* default */
    if (num_literals(c) != 1)
	return(NO_TROUBLE);
    else {	/* else it is unit */
	atom = c->first_lit->atom;
        if (atom->varnum == CONDITIONAL_DEMOD)
		{	/* if conditional_demod */
	    alpha = atom->farg->narg->argval;
	    beta = atom->farg->narg->narg->argval;
	    *cid = (term_ident(alpha, beta) == 0);
		return(NO_TROUBLE);
	    }	/* end of if conditional_demod */
	else if (atom->varnum != POS_EQ)
	    return(NO_TROUBLE);		/* *cid == 0 by default */
	else {	/* else POS_EQ */
	    alpha = atom->farg->argval;
	    beta = atom->farg->narg->argval;
	    if (term_ident(alpha, beta))
		return(NO_TROUBLE);
	    else {	/* it can be a demodulator */
	        if (Flags[LEX_RPO].val == 0) {
		    if (term_ident_x_vars(alpha, beta)) {
			fprintf(Fdout,"lex dependent demodulator: ");
			print_clause(Fdout, c);
			atom->varnum = LEX_DEP_DEMOD;
			}
		    }
		else {	/* LEX_RPO is on */
		    if (lrpo_greater(alpha, beta, &tempint) == TROUBLE)
			return(TROUBLE);
			if (tempint)
			;  /* do nothing */
		    else {	/* alpha is not greater than beta */
			if (lrpo_greater(beta, alpha, &tempint2) == TROUBLE)
				return(TROUBLE);
			if (tempint2) {	/* beta is greater than alpha */
			/* flip args */
printf(Fdout,"Flipping following input demodulator due to lrpo ordering: ");
			print_clause(Fdout, c);
			atom->farg->argval = beta;
			atom->farg->narg->argval = alpha;
			}	/* end of beta is greater than alpha */
		    else {	/* alpha and beta are not comparable */
			fprintf(Fdout,"LRPO dependent demodulator: ");
			print_clause(Fdout, c);
			atom->varnum = LEX_DEP_DEMOD;
			}	/* end of alpha and beta are not comparable */
			}	/* end of alpha is not greater than beta */
			}	/* end of LEX_RPO is on */
		    }	/* end of it can be a demodulator */
		/* now ok if new vars in beta
	if (var_subset(atom->farg->narg->argval,atom->farg->argval) == 0) {
		    fprintf(Fdout,"beta has a variable not in alpha: ");
		    return(NO_TROUBLE);
		    }
		else
		*/
       	        *cid = 1;
		return(NO_TROUBLE);
		}	/* end of POS_EQ */
	    }	/* end of it is unit */

}  /* check_input_demod */

/*************
 *
 *    int new_demod(c, demod_flag)
 *
 *    Make an equality unit into a demodulator.
 *    It has already been decided (in order_equalities) that c should
 *    be a demodulator.  (Don't flip or back demodulate.)
 *
 *    demod_flag:
 *       1: regular demodulator
 *       2: lex-dependent
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int new_demod(c, demod_flag)
struct clause *c;
int demod_flag;
{
    struct clause *d;
    struct int_ptr *ip0, *ip1;

    Stats[NEW_DEMODS]++;
    if (cl_copy(c, &d) == TROUBLE)
	return(TROUBLE);
    if (cl_integrate(d,ND,NONE) == TROUBLE)
		return(TROUBLE);
/* Penguin: the second parameter set to ND informs cl_integrate() that	*/
/* d is a copy of an already existing clause, so that the id of d is 	*/
/* set by cl_integrate(), but pid, lid and dest are kept as those of c. */
    if (demod_flag == 2)
	d->first_lit->atom->varnum = LEX_DEP_DEMOD;
    if (Flags[BACK_DEMOD].val) {
	/* Back demod (if set) occurs in post_process. */
	/* bits indicates presence of new demodulator */
	SET_BIT(c->first_lit->atom->bits, NEW_DEMOD_BIT);
	}
    if (get_int_ptr(&ip0) == TROUBLE)
	return(TROUBLE);
	ip0->i = NEW_DEMOD_RULE;
    if (get_int_ptr(&ip1) == TROUBLE)
	return(TROUBLE);
	ip1->i = c->id;
    d->parents = ip0; ip0->next = ip1;
    if (Flags[PRINT_NEW_DEMOD].val) {
	fprintf(Fdout,"---> New Demodulator: ");
	if (demod_flag == 2)
	    fprintf(Fdout,"(lex-dependent) ");
	print_clause(Fdout, d);
	}

#ifdef ROO

    /* It is important for roo_delete that indexing occurs */
    /* before insertion into Demodulators.              */

    if (Flags[DEMOD_LINEAR].val == 0) {
	if (Demod_imd == NULL)
	    if (get_imd_tree(&Demod_imd) == TROUBLE)
		return(TROUBLE);
        p4_lock(&(Glob->imd_index_lock));
	imd_insert(d, Demod_imd);
        p4_unlock(&(Glob->imd_index_lock));
	}

    p4_lock(&(Glob->list_move_lock));
    append_cl(Demodulators, d);
    p4_unlock(&(Glob->list_move_lock));

#else

    if (Flags[DEMOD_LINEAR].val == 0) {
	if (Demod_imd == NULL)
	    if (get_imd_tree(&Demod_imd) == TROUBLE)
		return(TROUBLE);
	if (imd_insert(d, Demod_imd) == TROUBLE)
		return(TROUBLE);
	}

    append_cl(Demodulators, d);
	Stats[DEMODULATORS_SIZE]++;		/* Penguin */

#endif
return(NO_TROUBLE);
}  /* new_demod */

