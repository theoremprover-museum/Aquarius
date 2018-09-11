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
 *  formula.c
 *
 *    This file has routines to input and output quantified formulas and
 *    to convert them to lists of clauses (Skolemization and CNF translation).
 *
 *    Well-formed-formulas:
 *        - an atom is a WFF
 *        - if F is a non-negated WFF, then -F is a WFF.
 *        - if F and G are WFFs, then (F <-> G) and (F -> G) are WFFs.
 *        - if F1, ..., Fn are WFFs, then (F1 & ... & Fn) and (F1 | ... | Fn)
 *          are WFFs.
 *        - if F is a WFF, Q1 ... Qn are quantifiers ("all" or "exists"),
 *          and X1 ... Xn are symbols, then (Q1 X1 ... Qn Xn F) is a WFF.
 *
 *    Note, double negations are not allowed, and all parentheses must be
 *    included:  --F is not a WFF, and (A & B -> C) is not a WFF.
 *
 */

/*
 *    In the first implementation, formulas were stored as terms.
 *    Later, a formula type was introduced, but I never got around
 *    to updating the parsing routine str_term.  Therefore, formulas
 *    are input to terms, then translated to the formula type in
 *    term_to_formula below.
 *
 */

#include "header.h"

#define F_TYPE fpa_id  /* for when formulas as kept as terms during input */
#define EQ_FORM 100    /* for when formulas as kept as terms during input */

static int Sk_func_num, Sk_const_num;  /* for creating new skolem symbols */

/*************
 *
 *    print_formula(fp, t) -- print a formula to a file.
 *
 *************/

void print_formula(fp, f)
FILE *fp;
struct formula *f;
{
    char op[MAX_NAME];
    struct formula *f1;
    
    if (f == NULL)
	fprintf(fp, "(nil)");
    else if (f->type == ATOM_FORM) {
	
	print_term(fp, f->t);
	}
    else if (f->type == NOT_FORM) {
	fprintf(fp, "-");
	print_formula(fp, f->first_child);
	}
    else if (f->type == AND_FORM && f->first_child == NULL)
	fprintf(fp, "TRUE");
    else if (f->type == OR_FORM && f->first_child == NULL)
	fprintf(fp, "FALSE");
    else if (f->type == QUANT_FORM) {
	fprintf(fp, "(");
	if (f->quant_type == ALL_QUANT)
	    fprintf(fp, "all ");
	else
	    fprintf(fp, "exists ");
	print_term(fp, f->t);
	fprintf(fp, " ");
	print_formula(fp, f->first_child);
	fprintf(fp, ")");
	}
    else {
	if (f->type == AND_FORM)
	    str_copy("& ", op);
	else if (f->type == OR_FORM)
	    str_copy("| ", op);
	else if (f->type == IMP_FORM)
	    str_copy("-> ", op);
	else if (f->type == IFF_FORM)
	    str_copy("<-> ", op);
	else
	    op[0] = '\0';
	
	fprintf(fp, "(");
	for (f1 = f->first_child; f1; f1 = f1->next) {
	    print_formula(fp, f1);
	    if (f1->next)
		fprintf(fp, " %s", op);
	    }
	fprintf(fp, ")");
	}
}  /* print_formula */

/*************
 *
 *    p_formula(f) -- print formula to standard output
 *
 *************/

void p_formula(f)
struct formula *f;
{
    print_formula(Fdout, f);
}  /* p_formula */

/*************
 *
 *    static void str_print_formula(str, ip, f)
 *
 *    Print a formula to a string and count the length of the string.
 *
 *************/

static void str_print_formula(str, ip, f)
char *str;
int *ip;
struct formula *f;
{

    char op[MAX_NAME];
    struct formula *f1;
    
    if (f == NULL) {
        str_copy("(nil)", str+*ip);
        *ip += 5;
        }
    else if (f->type == ATOM_FORM) {
	str_print_term(str, ip, f->t);
	}
    else if (f->type == NOT_FORM) {
	str_copy("-", str+*ip);
	*ip += 1;
	str_print_formula(str, ip, f->first_child);
	}
    else if (f->type == AND_FORM && f->first_child == NULL) {
	str_copy("TRUE", str+*ip);
	*ip += 4;
	}
    else if (f->type == OR_FORM && f->first_child == NULL) {
	str_copy("FALSE", str+*ip);
	*ip += 5;
	}
    else if (f->type == QUANT_FORM) {
	str_copy("(", str+*ip);
	*ip += 1;
	if (f->quant_type == ALL_QUANT) {
	    str_copy("all ", str+*ip);
	    *ip += 4;
	    }
	else {
	    str_copy("exists ", str+*ip);
	    *ip += 7;
	    }
	str_print_term(str, ip, f->t);
	str_copy(" ", str+*ip);
	*ip += 1;
	str_print_formula(str, ip, f->first_child);
	str_copy(")", str+*ip);
	*ip += 1;
	}
    else {
	if (f->type == AND_FORM)
	    str_copy(" & ", op);
	else if (f->type == OR_FORM)
	    str_copy(" | ", op);
	else if (f->type == IMP_FORM)
	    str_copy(" -> ", op);
	else if (f->type == IFF_FORM)
	    str_copy(" <-> ", op);
	else
	    op[0] = '\0';
	
	str_copy("(", str+*ip);
	*ip += 1;
	for (f1 = f->first_child; f1; f1 = f1->next) {
	    str_print_formula(str, ip, f1);
	    if (f1->next) {
		str_copy(op, str+*ip);
		*ip += strlen(op);
		}
	    }
	str_copy(")", str+*ip);
	*ip += 1;
	}

}  /* str_print_formula */

/*************
 *
 *    int sprint_formula(s, f) -- return length of s.
 *
 *************/

int sprint_formula(s, f)
char *s;
struct formula *f;
{
    int i;

    i = 0;

    str_print_formula(s, &i, f);

    s[i] = '\0';
    return(i);

}  /* sprint_formula */

/*************
 *
 *    static int str_form_term(buf, bufp, sft) 
 *	convert part of a string
 *    representing a formula into a term.
 *
 *    After it is parsed into a term, it will be translated to a formula type.
 *
 *    *bufp is an integer giving the current position in the string.
 *    *bufp is updated by this routine.
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. The pointer to struct term is returned through the 
 *	parameter sft.
 *
 *************/

static int str_form_term(buf, bufp, sft)
char buf[];
int *bufp;
struct term **sft;
{
    struct term *t1, *t2;
    struct rel *r1, *r2;
    char word[MAX_NAME], *s;
    int i, save_pos, sign, formula_type, quant_done;
	int tempint;
    
	*sft = NULL;				/* default */
    skip_white(buf, bufp);
    if (buf[*bufp] == '-' && buf[(*bufp)+1] != '>') {
	sign = 0;
	(*bufp)++;
	skip_white(buf, bufp);
	if (buf[*bufp] == '-') {
	    fprintf(Fdout, "ERROR, double negation:\n");
	    print_error(Fdout, buf, *bufp);
		*sft = NULL;
	    return(NO_TROUBLE);
	    }
	}
    else
	sign = 1;

    if (buf[*bufp] == '(') {
	(*bufp)++;    /* skip past open paren */
	i = 0;  /* count arguments */
	formula_type = 0;
	if (get_term(&t1) == TROUBLE)
		return(TROUBLE);
	t1->type = COMPLEX;
	r1 = NULL;
	save_pos = *bufp;  /* in case of error */
	while (buf[*bufp] != ')') {
	    i++;
	    save_pos = *bufp;
	    skip_white(buf, bufp);
	    /* '|' must be handled specially, because it is a delimiter */
	    if (buf[*bufp] == '|' && buf[(*bufp) + 1] == ' ') {
		(*bufp)++;  /* skip past '|' */
		skip_white(buf, bufp);
		if (get_term(&t2) == TROUBLE)
			return(TROUBLE);
		t2->type = NAME;
		if (str_to_sn("|", 0, &tempint) == TROUBLE)
			return(TROUBLE);
		t2->sym_num = tempint;
		}
	    else
		{
	        if (str_form_term(buf, bufp, &t2) == TROUBLE)
			return(TROUBLE);
		}
	    if (t2 == NULL)
		{
		*sft = NULL;
		return(NO_TROUBLE);  /* an error has already been handled */
		}
	    else {
		s = sn_to_str(t2->sym_num);
		if (i == 1) {
	if (t2->type == NAME && (str_ident(s,"all") || str_ident(s,"exists"))) {
			formula_type = QUANT_FORM;
			quant_done = 0;
			}
		    }
		else if (formula_type == QUANT_FORM) {
		    if (quant_done) {
			fprintf(Fdout, "ERROR, bad quantified formula:\n");
			print_error(Fdout, buf, save_pos);
			*sft = NULL;
			return(NO_TROUBLE);
			}
		    else if (i % 2 == 0 && t2->type != NAME) {
			fprintf(Fdout, "ERROR, variable name expected:\n");
			print_error(Fdout, buf, save_pos);
			*sft = NULL;
			return(NO_TROUBLE);
			}
else if (i % 2 == 1 && (str_ident(s,"all") == 0 && str_ident(s,"exists") == 0))
			quant_done = 1;
		    }
	else if (i == 2)
		{  /* must be AND, OR, IMP, or IFF, or =, or != */
		    if (str_ident(s, "&"))
			formula_type = AND_FORM;
		    else if (str_ident(s, "|"))
			formula_type = OR_FORM;
		    else if (str_ident(s, "->"))
			formula_type = IMP_FORM;
		    else if (str_ident(s, "<->"))
			formula_type = IFF_FORM;
		    else if (str_ident(s, "="))
			formula_type = EQ_FORM;
		    else if (str_ident(s, "!=")) {
			formula_type = EQ_FORM;
			sign = (sign ? 0 : 1);
			}
		    
		    if (formula_type == 0 || t2->type != NAME) {
	fprintf(Fdout, "ERROR, logical operator, '=', or '!=' expected:\n");
			print_error(Fdout, buf, save_pos);
			*sft = NULL;
			return(NO_TROUBLE);
			}
		    }
		else if (i > 3 && (formula_type == IMP_FORM ||
				   formula_type == IFF_FORM ||
				   formula_type == EQ_FORM)) {
			fprintf(Fdout, "ERROR, too many arguments:\n");
			print_error(Fdout, buf, save_pos);
			*sft = NULL;
			return(NO_TROUBLE);
			}
		else if (i % 2 == 0) {
		    if ((formula_type == AND_FORM && str_ident(s, "&") == 0) ||
		        (formula_type == OR_FORM  && str_ident(s, "|")  == 0)) {

			fprintf(Fdout, "ERROR, operators switched:\n");
			print_error(Fdout, buf, save_pos);
			*sft = NULL;
			return(NO_TROUBLE);
			}
		    }
		/* else ok: AND or OR, and odd-numbered argument */

		if (i % 2 == 1 || formula_type == QUANT_FORM) {
		    if (get_rel(&r2) == TROUBLE)
			return(TROUBLE);
		    r2->argval = t2;
		    if (r1 == NULL)
			t1->farg = r2;
		    else
			r1->narg = r2;
		    r1 = r2;
		    }
		else
		    free_term(t2);  /* free operator */
		}
	    }
	if (i < 3) {
	    fprintf(Fdout, "ERROR, too few arguments:\n");
	    print_error(Fdout, buf, save_pos);
		*sft = NULL;
	    return(NO_TROUBLE);
	    }
	else {
	    (*bufp)++;    /* skip past close paren */
	    skip_white(buf, bufp);
	    if (formula_type == EQ_FORM)
		t1->sym_num = Eq_sym_num;
	    else
	        t1->F_TYPE = formula_type;
	    if (sign)
	        SET_BIT(t1->bits, SCRATCH_BIT);
	    else
	        CLEAR_BIT(t1->bits, SCRATCH_BIT);
		*sft = t1;
	    return(NO_TROUBLE);
	    }
	}
    else {
	i = *bufp;
	while (is_delim(buf[i]) == 0)
	    i++;
	if (buf[i] == ' ') {  /* next thing is not an atom with arguments */
	    get_word(buf, bufp, word);
	    if (word[0] == '\0') {
		fprintf(Fdout, "ERROR, bad word:\n");
		print_error(Fdout, buf, *bufp);
		*sft = NULL;
		return(NO_TROUBLE);
		}
	    else {
		if (get_term(&t1) == TROUBLE)
			return(TROUBLE);
		t1->type = NAME;
		if (str_to_sn(word, 0, &tempint) == TROUBLE)
			return(TROUBLE);
		t1->sym_num = tempint;
		if (sign)
		    SET_BIT(t1->bits, SCRATCH_BIT);
		else
		    CLEAR_BIT(t1->bits, SCRATCH_BIT);
		*sft = t1;
		return(NO_TROUBLE);
		}
	    }
	else {  /* next thing is atom with arguments */
	    if (str_term(buf, bufp, &t1) == TROUBLE)
		return(TROUBLE);
	    if (t1 == NULL)
		{
		*sft = NULL;
		return(NO_TROUBLE);
		}
	    else {
		if (sign)
		    SET_BIT(t1->bits, SCRATCH_BIT);
		else
		    CLEAR_BIT(t1->bits, SCRATCH_BIT);
		*sft = t1;
		return(NO_TROUBLE);
		}
	    }
	}
}  /* str_form_term */

/*************
 *
 *   static int term_to_formula(t, ttf)
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter ttf.
 *
 *************/

static int term_to_formula(t, ttf)
struct term *t;
struct formula **ttf;
{
    struct formula *f1, *f2, *f3;
    struct rel *r;
	struct formula *temp;
	struct term *tempterm;

	*ttf = NULL;				/* default */
    /* handle negation at end */
    
    if (t->F_TYPE == 0) {
        if (get_formula(&f1) == TROUBLE)
		return(TROUBLE);
	f1->type = ATOM_FORM;
	if (copy_term(t, &tempterm) == TROUBLE)
		return(TROUBLE);
	f1->t = tempterm;
	f1->t->varnum = NORM_ATOM;
	}
    else if (t->F_TYPE == QUANT_FORM) {
	r = t->farg;
	f1 = f3 = NULL;
	while (r->narg) {
	    if (get_formula(&f2) == TROUBLE)
		return(TROUBLE);
	    f2->type = QUANT_FORM;
	    if (f3)
		f3->first_child = f2;
	    else
		f1 = f2;
	    f3 = f2;

	    if (str_ident(sn_to_str(r->argval->sym_num), "all"))
		f2->quant_type = ALL_QUANT;
	    else
		f2->quant_type = EXISTS_QUANT;
	    /* copy variable */
	    r = r->narg;
	   if (copy_term(r->argval, &tempterm) == TROUBLE)
		return(TROUBLE);
		f2->t = tempterm;
	    r = r->narg;
	    }
	if (f3)
	{
	   if (term_to_formula(r->argval, &temp) == TROUBLE)
		return(TROUBLE);
		f3->first_child = temp;
	}
	else
		{
	    if (term_to_formula(r->argval, &f1) == TROUBLE)
  /* no quantifiers */	return(TROUBLE);
		}
	}
    else {
        if (get_formula(&f1) == TROUBLE)
		return(TROUBLE);
	f1->type = t->F_TYPE;
	f3 = NULL;
	for (r = t->farg; r; r = r->narg) {
	    if (term_to_formula(r->argval, &f2) == TROUBLE)
		return(TROUBLE);
	    if (f3)
		f3->next = f2;
	    else
		f1->first_child = f2;
	    f3 = f2;
	    }
	}
	
    if ( ! TP_BIT(t->bits, SCRATCH_BIT) )
	{
	if (negate_formula(f1,&temp) == TROUBLE)
		return(TROUBLE);
	f1 = temp;
	}
    
	*ttf = f1;
    return(NO_TROUBLE);

}  /* term_to_formula */

/*************
 *
 *    int str_formula(buf, bufp, sf) -- convert a string
 *    into a formula.
 *
 *    *bufp is an integer giving the current position in the string.
 *    *bufp is updated by this routine.
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter sf.
 *
 *************/

int str_formula(buf, bufp, sf)
char buf[];
int *bufp;
struct formula **sf;
{
    struct term *t;
    struct formula *f;
    
	*sf = NULL;				/* default */
    if (str_form_term(buf, bufp, &t) == TROUBLE)
		return(TROUBLE);
    if (t)
	{
	if (term_to_formula(t, &f) == TROUBLE)
		return(TROUBLE);
	}
    else
	f = NULL;
	*sf = f;
    return(NO_TROUBLE);
    
}  /* str_formula */

/*************
 *
 *    int read_formula(fp, rcp, rf) -- read a formula from a file
 *
 *    The return code *rcp:
 *        0 - an error was encountered and reported; NULL is returned.
 *        1 - OK; if EOF was found instead of a formula, NULL is returned.
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter rf.
 *
 *************/

int read_formula(fp, rcp, rf)
FILE *fp;
int *rcp;
struct formula **rf;
{
    char buf[MAX_BUF];
    int p, rc;
    struct formula *f;
    struct term *t;
    
	*rf = NULL;				/* default */
    rc = read_buf(fp, buf);
    if (rc == 0) {  /* error */
	*rcp = 0;
	return(NO_TROUBLE);
	}
    else if (buf[0] == '\0') {  /* ok. EOF */
	*rcp = 1;
	return(NO_TROUBLE);
	}
    else {
	p = 0;
	if (str_form_term(buf, &p, &t) == TROUBLE)
		return(TROUBLE);
	if (t == NULL) {
	    *rcp = 0;
	    return(NO_TROUBLE);
	    }
	else {
	    skip_white(buf, &p);
	    if (buf[p] != '.') {
		fprintf(Fdout, "ERROR, text after formula:\n");
		print_error(Fdout, buf, p);
		*rcp = 0;
		return(NO_TROUBLE);
		}
	    if (term_to_formula(t, &f) == TROUBLE)
		return(TROUBLE);
	    if (contains_skolem_symbol(t)) {
	fprintf(Fdout, "\nERROR, input formula contains Skolem symbol:\n");
		print_formula(Fdout, f);
		fprintf(Fdout,".\n\n");
		zap_formula(f);
		*rcp = 0;
		return(NO_TROUBLE);
		}
	    else {
		if (term_to_formula(t, &f) == TROUBLE)
			return(TROUBLE);
		zap_term(t);
		*rcp = 1;
		*rf = f;
		return(NO_TROUBLE);
		}
	    }
	}
}  /* read_formula */

/*************
 *
 *    int read_formula_list(file_ptr, errors_ptr, rfl) 
 *
 *    Read and return a list of quantified formulas.
 *
 *    The list must be terminated either with the term `end_of_list.'
 *    or with an actual EOF.
 *    Set errors_ptr to point to the number of errors found.
 *
 *	struct formula_ptr * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula_ptr through the
 *	parameter rfl.
 *
 *************/

int read_formula_list(fp, ep, rfl)
FILE *fp;
int *ep;
struct formula_ptr **rfl;
{
    struct formula_ptr *p1, *p2, *p3;
    struct formula *f;
    int rc;

	*rfl = NULL;				/* default */
    *ep = 0;
    p3 = NULL;
    p2 = NULL;
    if (read_formula(fp, &rc, &f) == TROUBLE)
	return(TROUBLE);
    while (rc == 0) {
	(*ep)++;
	if (read_formula(fp, &rc, &f) == TROUBLE)
		return(TROUBLE);
	}

    /* keep going until f == NULL || f is end marker */

    while (f != NULL && (f->type != ATOM_FORM ||
		 str_ident(sn_to_str(f->t->sym_num), "end_of_list") == 0)) {
	if (get_formula_ptr(&p1) == TROUBLE)
		return(TROUBLE);
	p1->f = f;
	if (p2 == NULL)
	    p3 = p1;
	else
	    p2->next = p1;
	p2 = p1;
	if (read_formula(fp, &rc, &f) == TROUBLE)
		return(TROUBLE);
	while (rc == 0) {
	    (*ep)++;
	    if (read_formula(fp, &rc, &f) == TROUBLE)
		return(TROUBLE);
	    }
	}
    if (f != NULL)
	zap_formula(f);
	*rfl = p3;
    return(NO_TROUBLE);
}  /* read_formula_list */

/*************
 *
 *    print_formula_list(file_ptr, term_ptr) -- Print a list of quantified formulas.
 *
 *        The list is printed with periods after each quantified formula, and
 *    the list is terminated with `end_of_list.' so that it can
 *    be read with read_formula_list.
 *
 *************/

void print_formula_list(fp, p)
FILE *fp;
struct formula_ptr *p;
{
    while (p != NULL) {
	print_formula(fp, p->f); fprintf(fp, ".\n");
	p = p->next;
	}
    fprintf(fp, "end_of_list.\n");
}  /* print_formula_list */

/*************
 *
 *    int copy_formula(f, cf)
 *
 *    Copy a formula.  copy_term is used to copy atoms and quantified vars.
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter cf.
 *
 *************/

int copy_formula(f, cf)
struct formula *f;
struct formula **cf;
{
    struct formula *f_new, *f_sub, *f_prev, *f3;
	struct term *tempterm;
	struct formula *temp;

	*cf = NULL;				/* default */
    if (get_formula(&f_new) == TROUBLE)
	return(TROUBLE);
    f_new->type = f->type;

    if (f->type == ATOM_FORM)
	{
	if (copy_term(f->t, &tempterm) == TROUBLE)
		return(TROUBLE);
		f_new->t = tempterm;
	}
    else if (f->type == QUANT_FORM) {
	f_new->quant_type = f->quant_type;
	if (copy_term(f->t, &tempterm) == TROUBLE)
		return(TROUBLE);
		f_new->t = tempterm;
	if (copy_formula(f->first_child, &temp) == TROUBLE)
		return(TROUBLE);
		f_new->first_child = temp;
	}
    else {
	f_prev = NULL;
	for (f_sub = f->first_child; f_sub; f_sub = f_sub->next) {
	    if (copy_formula(f_sub, &f3) == TROUBLE)
		return(TROUBLE);
	    if (f_prev)
		f_prev->next = f3;
	    else
		f_new->first_child = f3;
	    f_prev = f3;
	    }
	}
	*cf = f_new;
    return(NO_TROUBLE);
	
}  /* copy_formula  */

/*************
 *
 *    void zap_formula(f)
 *
 *    Free a formula and all of its subformulas and subterms.
 *
 *************/

void zap_formula(f)
struct formula *f;
{
    struct formula *f1, *f2;

    if (f->type == ATOM_FORM)
	zap_term(f->t);
    else {
	f1 = f->first_child;
	while (f1) {
	    f2 = f1;
	    f1 = f1->next;
	    zap_formula(f2);
	    }
	if (f->type == QUANT_FORM)
	    zap_term(f->t);
	}
    free_formula(f);
}  /* zap_formula */

/*************
 *
 *    int negate_formula(f, nf)
 *
 *    f is changed to its negation.  (Do not move negation signs inward.)
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter nf.
 *
 *************/

int negate_formula(f, nf)
struct formula *f;
struct formula **nf;
{
    struct formula  *f1, *f_save;

	*nf = NULL;				/* default */
    /* save next pointer */
    f_save = f->next; f->next = NULL;
    
    if (f->type == NOT_FORM) {
	f1 = f->first_child;
	free_formula(f);
	}
    else {
	if (get_formula(&f1) == TROUBLE)
		return(TROUBLE);
	f1->type = NOT_FORM;
	f1->first_child = f;
	}
    /* restore next pointer */
    f1->next = f_save;
	*nf = f1;
    return(NO_TROUBLE);
}  /* negate_formula */

/*************
 *
 *    int nnf(f, nf)
 *
 *    f is changed into its negation normal form (NNF) by removing
 *    -> and <-> and moving negation signs all the way in.
 *
 *     (A <-> B) (not negated) rewrites to ((-a | b) & (-b | a)).
 *    -(A <-> B)               rewrites to ((a | b) & (-a | -b)).
 *
 *    because conjunctions are favored.
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter nf.
 *
 *************/

int nnf(f, nf)
struct formula *f;
struct formula **nf;
{
    struct formula *f1, *f2, *next, *prev, *fn;
	struct formula *temp, *temp2;

	*nf = NULL;				/* default */
    switch (f->type) {
      case ATOM_FORM: 
	*nf = f;
	return(NO_TROUBLE);  /* f is atomic */
      case IFF_FORM:
	if (get_formula(&f1) == TROUBLE)
		return(TROUBLE);
	f1->type = AND_FORM;
	f1->first_child = f;
	f1->next = f->next;

	if (copy_formula(f, &f2) == TROUBLE)
		return(TROUBLE);
	f2->type = OR_FORM;
	if (negate_formula(f2->first_child->next,&temp) == TROUBLE)
		return(TROUBLE);
	f2->first_child->next = temp;

	f->type = OR_FORM;
	if (negate_formula(f->first_child,&temp) == TROUBLE)
		return(TROUBLE);
	f->first_child = temp;
	f->next = f2;
	if (nnf(f1,&temp) == TROUBLE)
		return(TROUBLE);
	*nf = temp;
	return(NO_TROUBLE);
      case IMP_FORM:
	f->type = OR_FORM;
	if (negate_formula(f->first_child,&temp) == TROUBLE)
		return(TROUBLE);
	f->first_child = temp;
	if (nnf(f,&temp) == TROUBLE)
		return(TROUBLE);
	*nf = temp;
	return(NO_TROUBLE);
      case QUANT_FORM:
	if (nnf(f->first_child,&temp) == TROUBLE)
		return(TROUBLE);
	f->first_child = temp;
	*nf = f;
	return(NO_TROUBLE);
      case AND_FORM:
      case OR_FORM:
	prev = NULL;
	f1 = f->first_child;
	while(f1) {
	    next = f1->next;  f1->next = NULL;
	    if (nnf(f1,&f2) == TROUBLE)
		return(TROUBLE);
	    if (prev)
		prev->next = f2;
	    else
		f->first_child = f2;
	    prev = f2;
	    f1 = next;
	    }
	*nf = f;
	return(NO_TROUBLE);

      case NOT_FORM:
	fn = f->first_child;
	switch (fn->type) {
	  case ATOM_FORM: 
		*nf = f;
	    return(NO_TROUBLE);
	  case IFF_FORM:
	    if (copy_formula(fn, &f2) == TROUBLE)
		return(TROUBLE);
	    f2->type = OR_FORM;
	    fn->type = OR_FORM;
	    if (negate_formula(f2->first_child,&temp) == TROUBLE)
		return(TROUBLE);
		f2->first_child = temp;
	    if (negate_formula(f2->first_child->next,&temp) == TROUBLE)
		return(TROUBLE);
		f2->first_child->next = temp;
	    fn->next = f2;
	    f->type = AND_FORM;
	    f->first_child = fn;
	if (nnf(f,&temp) == TROUBLE)
		return(TROUBLE);
	*nf = temp;
	return(NO_TROUBLE);
	  case IMP_FORM:
	    fn->type = OR_FORM;
	    if (negate_formula(fn->first_child,&temp) == TROUBLE)
		return(TROUBLE);
		fn->first_child = temp;
	if (nnf(f,&temp) == TROUBLE)
		return(TROUBLE);
	*nf = temp;
	return(NO_TROUBLE);
	  case QUANT_FORM:
fn->quant_type = (fn->quant_type == ALL_QUANT ? EXISTS_QUANT : ALL_QUANT);
	if (negate_formula(fn->first_child,&temp) == TROUBLE)
		return(TROUBLE);
	if (nnf(temp,&temp2) == TROUBLE)
		return(TROUBLE);
		fn->first_child = temp2;
	    fn->next = f->next;
	    free_formula(f);
		*nf = fn;
	    return(NO_TROUBLE);
	  case AND_FORM:
	  case OR_FORM:
	    prev = NULL;
	    f1 = fn->first_child;
	    while(f1) {
		next = f1->next;  f1->next = NULL;
	if (negate_formula(f1,&temp) == TROUBLE)
		return(TROUBLE);
	if (nnf(temp,&f2) == TROUBLE)
		return(TROUBLE);
		if (prev)
		    prev->next = f2;
		else
		    fn->first_child = f2;
		prev = f2;
		f1 = next;
		}
	    fn->type = (fn->type == AND_FORM ? OR_FORM : AND_FORM);
	    fn->next = f->next;
	    free_formula(f);
		*nf = fn;
	    return(NO_TROUBLE);
	    
	  case NOT_FORM:    /* double negation */
	    f1 = fn->first_child;
	    f1->next = f->next;
	    free_formula(f);
	    free_formula(fn);
	if (nnf(f1,&temp) == TROUBLE)
		return(TROUBLE);
	*nf = temp;
	return(NO_TROUBLE);
	    }
	}
	*nf = NULL;	/* ERROR */
    return(NO_TROUBLE);  /* ERROR */
}  /* nnf */

/*************
 *
 *    static void rename_free_formula(f, old_sn, new_sn)
 *
 *    Rename free occurrences of old_sn in NAMEs to new_sn.
 *    Recall that variables in formulas are really NAMEs.
 *
 *************/

static void rename_free_formula(f, old_sn, new_sn)
struct formula *f;
int old_sn;
int new_sn;
{
    struct formula *f1;
    
    if (f->type == ATOM_FORM)
	subst_sn_term(old_sn, f->t, new_sn, NAME);
    else if (f->type == QUANT_FORM) {
	if (old_sn != f->t->sym_num)
	    rename_free_formula(f->first_child, old_sn, new_sn);
	}
    else {
	for (f1 = f->first_child; f1; f1 = f1->next)
	    rename_free_formula(f1, old_sn, new_sn);
	}
	
}  /* rename_free_formula  */

/*************
 *
 *    static int skolem(f, vars, sss)
 *     
 *    Skolemize f w.r.t universally quantified vars.
 *    Called by skolemize.
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter sss.
 *
 *************/

static int skolem(f, vars, sss)
struct formula *f;
struct term *vars;
struct formula **sss;
{
    struct formula *f1, *f2, *prev, *next;
    struct rel *end, *r2;
    int sn;
	struct formula *temp;

	*sss = NULL;				/* default */

    if (f->type == NOT_FORM && f->first_child->type != ATOM_FORM) {
	fprintf(Fdout,"ERROR, skolem gets negated non-atom: ");
	print_formula(Fdout, f);
	fprintf(Fdout,"\n");
	}
    else if (f->type == IMP_FORM || f->type == IFF_FORM) {
	fprintf(Fdout,"ERROR, skolem gets: ");
	print_formula(Fdout, f);
	fprintf(Fdout,"\n");
	}
    else if (f->type == AND_FORM || f->type == OR_FORM) {
	prev = NULL;
	f1 = f->first_child;
	while(f1) {
	    next = f1->next;  f1->next = NULL;
	    if (skolem(f1, vars, &f2) == TROUBLE)
		return(TROUBLE);
	    if (prev)
		prev->next = f2;
	    else
		f->first_child = f2;
	    prev = f2;
	    f1 = next;
	    }
	}
    else if (f->type == QUANT_FORM) {
	if (f->quant_type == ALL_QUANT) {
	    if (occurs_in(f->t, vars)) {
		/*
		  rename current variable, because we are already in the
		  scope of a universally quantified var with that name.
		  */
		if (new_var_name(&sn) == TROUBLE)
			return(TROUBLE);
		rename_free_formula(f->first_child, f->t->sym_num, sn);
		f->t->sym_num = sn;
		}
	    if (get_rel(&r2) == TROUBLE)
		return(TROUBLE);
	    r2->argval = f->t;

	    /* Install variable at end of vars. */
	    for (end = vars->farg; end && end->narg; end = end->narg);
	    if (end)
		end->narg = r2;
	    else
		vars->farg = r2;

	    if (skolem(f->first_child, vars, &temp) == TROUBLE)
		return(TROUBLE);
		f->first_child = temp;

	    /* Remove variable from vars. */

	    free_rel(r2);
	    if (end)
		end->narg = NULL;
	    else
		vars->farg = NULL;
	    }
	else {  /* existential quantifier */
	    /*
	      must skolemize subformula first to avoid problem in
	      Ax...Ey...Ex F(x,y).
	      */
	    if (skolem(f->first_child, vars, &temp) == TROUBLE)
			return(TROUBLE);
		f->first_child = temp;
	    
	    if (gen_sk_sym(vars) == TROUBLE)
	 /* fills in sym_num and assigns type */
		return(TROUBLE);
	    if (subst_free_formula(f->t, f->first_child, vars) == TROUBLE)
		return(TROUBLE);
	    vars->type = COMPLEX; /* so that occurs_in above works */

	    f1 = f->first_child;
	    zap_term(f->t);
	    free_formula(f);
	    f = f1;
	    }
	}
	*sss = f;
    return(NO_TROUBLE);
}  /* skolem */

/*************
 *
 *    int skolemize(f,sko) -- Skolemize a formula
 *
 *    This routine assumes that f is in negation normal form.
 *    The existential quantifiers are deleted.
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter sko.
 *
 *************/

int skolemize(f,sko)
struct formula *f;
struct formula **sko;
{
    struct term *vars;
	struct formula *temp;

	*sko = NULL;				/* default */
    if (get_term(&vars) == TROUBLE)
	return(TROUBLE);
    vars->type = COMPLEX;
    if (skolem(f, vars, &temp) == TROUBLE)
	return(TROUBLE);
    free_term(vars);
	*sko = temp;
    return(NO_TROUBLE);

}  /* skolemize */

/*************
 *
 *    int anti_skolemize(f,asko) -- Anti-Skolemize a formula
 *
 *    The dual of skolemize:  universal quantifiers are removed.
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter asko.
 *
 *************/

int anti_skolemize(f,asko)
struct formula *f;
struct formula **asko;
{
struct formula *temp, *temp1, *temp2, *temp3;

	*asko = NULL;				/* default */

	if (negate_formula(f,&temp) == TROUBLE)
		return(TROUBLE);
	if (nnf(temp,&temp1) == TROUBLE)
		return(TROUBLE);
	if (skolemize(temp1,&temp2) == TROUBLE)
		return(TROUBLE);
	if (negate_formula(temp2,&temp3) == TROUBLE)
		return(TROUBLE);
	if (nnf(temp3,asko) == TROUBLE)		/* it sets *asko */
		return(TROUBLE);

	return(NO_TROUBLE);
}  /* anti_skolemize */

/*************
 *
 *    static int subst_free_term(var, t, sk)
 *
 *    Substitute free occurrences of var in t with copies of sk.
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

static int subst_free_term(var, t, sk)
struct term *var;
struct term *t;
struct term *sk;
{
    struct rel *r;
	struct term *tempterm;

    if (t->type != COMPLEX)
	return(NO_TROUBLE);
    else {
	r = t->farg;
	for (r = t->farg; r; r = r->narg) {
	    if (term_ident(var, r->argval)) {
		zap_term(r->argval);
		if (copy_term(sk, &tempterm) == TROUBLE)
			return(TROUBLE);
		r->argval = tempterm;
		}
	    else
		if (subst_free_term(var, r->argval, sk) == TROUBLE)
			return(TROUBLE);
	    }
	}
return(NO_TROUBLE);
}  /* subst_free_term */

/*************
 *
 *    int subst_free_formula(var, f, sk)
 *    Substitute free occurrences of var in f with copies of sk.
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int subst_free_formula(var, f, sk)
struct term *var;
struct formula *f;
struct term *sk;
{
    struct formula *f1;
    
    if (f->type == ATOM_FORM)
	{
	if (subst_free_term(var, f->t, sk) == TROUBLE)
		return(TROUBLE);
	}
    else if (f->type == QUANT_FORM) {
	if (!term_ident(f->t, var))
	    if (subst_free_formula(var, f->first_child, sk) == TROUBLE)
		return(TROUBLE);
	}
    else {
	for (f1 = f->first_child; f1; f1 = f1->next)
	    if (subst_free_formula(var, f1, sk) == TROUBLE)
		return(TROUBLE);
	}
	return(NO_TROUBLE);
}  /* subst_free_formula  */

/*************
 *
 *    int gen_sk_sym(t) -- generate a fresh skolem symbol for term t.
 *
 *    Assign type field as well as sym_num field to term t.
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int gen_sk_sym(t)
struct term *t;
{
    int arity;
    struct rel *r;
    char s1[MAX_NAME], s2[MAX_NAME];
	int tempint;

    arity = 0;
    r = t->farg;
    while (r != NULL) {
	arity++;
	r = r->narg;
	}

    if (arity == 0) {
	t->type = NAME;
	int_str(++Sk_const_num, s1);
        cat_str("$c", s1, s2, MAX_NAME);
	}
    else {
	t->type = COMPLEX;
	int_str(++Sk_func_num, s1);
        cat_str("$f", s1, s2, MAX_NAME);
	}

    if (str_to_sn(s2, arity, &tempint) == TROUBLE)
	return(TROUBLE);
		t->sym_num = tempint;
    if (mark_as_skolem(t->sym_num) == TROUBLE)
	return(TROUBLE);

return(NO_TROUBLE);
}  /* gen_sk_sym */

/*************
 *
 *    int skolem_symbol(sn) -- Is sn the symbol number of a skolem symbol?
 *
 *    Check if it is "$cn" or "$fn" for integer n.
 *    Do not check the skolem flag in the symbol node.
 *
 *************/

int skolem_symbol(sn)
int sn;
{
    char *s;
    int dummy;

    s = sn_to_str(sn);
    return(*s == '$' &&
	   (*(s+1) == 'c' | *(s+1) == 'f') &&
	   str_int(s+2,&dummy));
}  /* skolem_symbol */

/*************
 *
 *    int contains_skolem_symbol(t)
 *
 *    Check if any of the NAMEs in t are  "$cn" or "$fn", for integer n.
 *
 *************/

int contains_skolem_symbol(t)
struct term *t;     
{
    struct rel *r;

    if (t->type == VARIABLE)
	return(0);
    else if (t->type == NAME)
	return(skolem_symbol(t->sym_num));
    else {  /* COMPLEX */
	if (skolem_symbol(t->sym_num))
	    return(1);
	else {
	    for (r = t->farg; r; r = r->narg)
		if (contains_skolem_symbol(r->argval))
		    return(1);
	    return(0);
	    }
	}
}  /* contains_skolem_symbol */

/*************
 *
 *    int new_var_name(nvn) -- return a sym_num for a new VARIABLE symbol
 *
 *    Check and make sure that the new symbol does not occur in the
 *     symbol table.
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE and the sym_num through the
 *	parameter nvn.
 *
 *************/

int new_var_name(nvn)
int *nvn;
{
    char s1[MAX_NAME], s2[MAX_NAME];
    static int var_num;
    char c[2];
	int tempint;

    c[0] = (Flags[PROLOG_STYLE_VARIABLES].val ? 'X' : 'x');
    c[1] = '\0';

    int_str(++var_num, s1);
    cat_str(c, s1, s2, MAX_NAME);
    while (in_sym_tab(s2)) {
	int_str(++var_num, s1);
	cat_str(c, s1, s2, MAX_NAME);
	}

    if (str_to_sn(s2, 0, &tempint) == TROUBLE)
	return(TROUBLE);

*nvn = tempint;
return(NO_TROUBLE);
  
}  /* new_var_name */

/*************
 *
 *    int new_functor_name(arity, nfn) -- return a sym_num for a new symbol.
 *
 *   Check and make sure that the new symbol does not occur in the symbol table.
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE and the sym_num through the
 *	parameter nfn.
 *
 *	Penguin modifies it in such a way that each Penguin introduces its
 *	own symbols.
 *
 *************/

int new_functor_name(arity, nfn)
int arity;
int *nfn;
{
    char s1[MAX_NAME], s2[MAX_NAME];
	char my_s[MAX_NAME], s3[MAX_NAME], s4[MAX_NAME];	/* Penguin */

    static int functor_num;
	int tempint;

    int_str(++functor_num, s1);
	int_str(Whoami, my_s);			/* Penguin */
	cat_str("k", my_s, s3, MAX_NAME);
	cat_str(s3, "_", s4, MAX_NAME);
    cat_str(s4, s1, s2, MAX_NAME);
    while (in_sym_tab(s2)) {
	int_str(++functor_num, s1);
	cat_str(s4, s1, s2, MAX_NAME);
	}

    if (str_to_sn(s2, arity, &tempint) == TROUBLE)
	return(TROUBLE);

	*nfn = tempint;
	return(NO_TROUBLE);
  
}  /* new_functor_name */

/*************
 *
 *    static int uq_all(f, vars) -- called by unique_all
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

static int uq_all(f, vars)
struct formula *f;
struct term *vars;
{
    struct rel *r1;
    struct formula *f1;
    int sn;

    switch (f->type) {
      case ATOM_FORM: break;
      case NOT_FORM:
      case AND_FORM:
      case OR_FORM:
	for (f1 = f->first_child; f1; f1 = f1->next)
	    if (uq_all(f1, vars) == TROUBLE)
		return(TROUBLE);
	break;
      case QUANT_FORM:
	if (occurs_in(f->t, vars)) {
	    /*
	      rename current variable, because already have
	      a quantified var with that name.
	      */
	    if (new_var_name(&sn) == TROUBLE)
		return(TROUBLE);
	    rename_free_formula(f->first_child, f->t->sym_num, sn);
	    f->t->sym_num = sn;
	    }
	else {
	    if (get_rel(&r1) == TROUBLE)
		return(TROUBLE);
	    r1->argval = f->t;
	    r1->narg = vars->farg;
	    vars->farg = r1;
	    }
	
	/* recursive call on quantified formula */
	if (uq_all(f->first_child, vars) == TROUBLE)
		return(TROUBLE);
	break;
	}
return(NO_TROUBLE);
}  /* uq_all */

/*************
 *
 *    int unique_all(f) -- make all universally quantified variables unique
 *
 *    It is assumed that f is in negation normal form and is Skolemized (no
 *    existential quantifiers).
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int unique_all(f)
struct formula *f;
{
    struct term *vars;
    struct rel *r1, *r2;

    if (get_term(&vars) == TROUBLE)
	return(TROUBLE);
    vars->type = COMPLEX;
    if (uq_all(f, vars) == TROUBLE)
	return(TROUBLE);
    r1 = vars->farg;
    while (r1 != NULL) {
	r2 = r1;
	r1 = r1->narg;
	free_rel(r2);
	}
    free_term(vars);
return(NO_TROUBLE);
}  /* unique_all */

/*************
 *
 *    static mark_free_var_term(v, t) -- mark free occurrences of v in t 
 *
 *    Each free NAME in t with sym_num == v->sym_num is marked as
 *    a VARIABLE by setting the type field to VARIABLE.
 *
 *************/

static void mark_free_var_term(v, t)
struct term *v;
struct term *t;
{
    struct rel *r;
    struct term *t1;

    if (t->type != COMPLEX)
	return;
    else {
	r = t->farg;
        for (r = t->farg; r; r = r->narg) {
	    t1 = r->argval;
            if (t1->type == NAME) {
		if (t1->sym_num == v->sym_num) {
		    t1->type = VARIABLE;
		    /*
		      bug fix 31-Jan-91. WWM.  The following line was added
		      because term-ident (called if simplify_fol) does not
		      check sym_num field for vars.  It is a trick.
		    */
		    t1->varnum = t1->sym_num;
		    }
                }
            else
		mark_free_var_term(v, t1);
	    }
	}
}  /* mark_free_var_term */

/*************
 *
 *    static void mark_free_var_formula(v, f)
 *
 *************/

static void mark_free_var_formula(v, f)
struct term *v;
struct formula *f;
{
    struct formula *f1;
    
    if (f->type == ATOM_FORM)
	mark_free_var_term(v, f->t);
    else {
	for (f1 = f->first_child; f1; f1 = f1->next)
	    mark_free_var_formula(v, f1);
	}
}  /* mark_free_var_formula */

/*************
 *
 *    struct term *zap_quant(f)
 *
 *    Delete quantifiers and mark quantified variables.
 *
 *    It is assumed that f is skolemized nnf with unique universally
 *    quantified variables.  For each universal quantifier,
 *    mark all occurrences of the quantified variable by setting the type field
 *    to VARIABLE, then delete the quantifier.
 *    All QUANT_FORM nodes are deleted as well.
 *
 *************/

struct formula *zap_quant(f)
struct formula *f;
{

    struct formula *f1, *f2, *prev, *next;

    switch (f->type) {
      case ATOM_FORM:
	break;
      case NOT_FORM:
      case AND_FORM:
      case OR_FORM:
        prev = NULL;
        f1 = f->first_child;
        while(f1) {
            next = f1->next;  f1->next = NULL;
            f2 = zap_quant(f1);
            if (prev)
                prev->next = f2;
            else
                f->first_child = f2;
            prev = f2;
            f1 = next;
            }
	break;
      case QUANT_FORM:
	mark_free_var_formula(f->t, f->first_child);
	f1 = f->first_child;
	f1->next = f->next;
	free_formula(f);
	f = zap_quant(f1);
	break;
	}
    return(f);
}  /* zap_quant */

/*************
 *
 *    static void flatten_top_2(f, start, end_p) -- called by flatten_top.
 *
 *************/

static void flatten_top_2(f, start, end_p)
struct formula *f;
struct formula *start;
struct formula **end_p;
{
    struct formula *f1, *f2;

    f1 = f->first_child;
    while (f1) {
	f2 = f1;
	f1 = f1->next;
	if (f2->type == f->type) {
	    flatten_top_2(f2, start, end_p);
	    free_formula(f2);
	    }
	else {
	    if (*end_p)
		(*end_p)->next = f2;
	    else
		start->first_child = f2;
	    *end_p = f2;
	    }
	}
}  /* flatten_top_2 */

/*************
 *
 *    void flatten_top(f) -- flatten conjunctions or disjunctions
 *
 *    f is flattened with respect to f_type.  Subtrees of type f_type below
 *    a node of the oppposite type are not flattened.  For example, in
 *    (a or (b and (c or (d or e)))), the formula (c or (d or e)) is never
 *    flattened.
 *
 *************/

void flatten_top(f)
struct formula *f;
{
    struct formula *end;
    
    if (f->type == AND_FORM || f->type == OR_FORM) {
	end = NULL;
	flatten_top_2(f, f, &end);
	if (end)
	    end->next = NULL;
	else
	    f->first_child = NULL;
	}
}  /* flatten_top */

/*************
 *
 *    static int distribute(f, df) -- distribute OR over AND.
 *
 *    f is an OR node whose subterms are in CNF.  This routine returns
 *    a CNF of f.
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter df.
 *
 *************/

static int distribute(f, df)
struct formula *f;
struct formula **df;
{
    struct formula *f_new, *f1, *f2, *f3, *f4, *f_prev, *f_save;
    int i, j;
	struct formula *temp;

	*df = NULL;

    f_save = f->next; f->next = NULL;

    if (f->type != OR_FORM)
	{
	*df = f;
	return(NO_TROUBLE);
	}
    else {

	flatten_top(f);
	if (Flags[SIMPLIFY_FOL].val) {
	    conflict_tautology(f);
	    f = subsume_disj(f);
	    }
	if (f->type != OR_FORM)
	{
		*df = f;
	    return(NO_TROUBLE);
	}
	else {
	    
	    /* find first AND subformula */
	    i = 1;
	    f_prev = NULL;
	  for (f1 = f->first_child; f1 && f1->type != AND_FORM; f1 = f1->next) {
		i++;
		f_prev = f1;
		}
	    if (f1 == NULL)
		{
		*df = f;
		return(NO_TROUBLE);  /* nothing to distribute */
		}
	    else {
		/* unhook AND */
		if (f_prev)
		    f_prev->next = f1->next;
		else
		    f->first_child = f1->next;
		f2 = f1->first_child;
		f_new = f1;
		f_prev = NULL;
		while (f2) {
		    f3 = f2->next;
		    if (f3)
			{
			if (copy_formula(f, &f1) == TROUBLE)
				return(TROUBLE);
			}
		    else
			f1 = f;
		    if (i == 1) {
			f2->next = f1->first_child;
			f1->first_child = f2;
			}
		    else {
			j = 1;
			for (f4 = f1->first_child; j < i-1; f4 = f4->next)
			    j++;
			f2->next = f4->next;
			f4->next = f2;
			}
		    if (distribute(f1, &temp) == TROUBLE)
			return(TROUBLE);
			f1 = temp;
		    if (f_prev)
			f_prev->next = f1;
		    else
			f_new->first_child = f1;
		    f_prev = f1;
		    f2 = f3;
		    }
		f_new->next = f_save;
		flatten_top(f_new);
		if (Flags[SIMPLIFY_FOL].val) {
		    conflict_tautology(f_new);
		    f_new = subsume_conj(f_new);
		    }
		*df = f_new;
		return(NO_TROUBLE);
		}
	    }
	}
}  /* distribute */

/*************
 *
 *    int cnf(f, nf) -- convert nnf f to conjunctive normal form.
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter nf.
 *
 *************/

int cnf(f, nf)
struct formula *f;
struct formula **nf;
{
    struct formula *f1, *f2, *f_prev, *f_next, *f_save;
	struct formula *temp;

	*nf = NULL;				/* default */

    f_save = f->next; f->next = NULL;

    if (f->type == AND_FORM || f->type == OR_FORM) {
	/* first convert subterms to CNF */
	f_prev = NULL;
	f1 = f->first_child;
	while(f1) {
	    f_next = f1->next;
	    if (cnf(f1,&f2) == TROUBLE)
		return(TROUBLE);
	    if (f_prev)
		f_prev->next = f2;
	    else
		f->first_child = f2;
	    f_prev = f2;
	    f1 = f_next;
	    }

	if (f->type == AND_FORM) {
	    flatten_top(f);
	    if (Flags[SIMPLIFY_FOL].val) {
		conflict_tautology(f);
		f = subsume_conj(f);
		}
	    }
	else
		{
	    if (distribute(f, &temp) == TROUBLE)
		return(TROUBLE);
	/* flatten and simplify in distribute */
		f = temp;
		}
	}

    f->next = f_save;
	*nf = f;
    return(NO_TROUBLE);

}  /* cnf */

/*************
 *
 *    int dnf(f, ddd)) -- convert f to disjunctive normal form.
 *
 *	struct formula * in Otter, int in Penguin,as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter ddd.
 *
 *************/

int dnf(f, ddd)
struct formula *f;
struct formula **ddd;
{
struct formula *temp, *temp1, *temp2, *temp3;

	*ddd = NULL;				/* default */

	if (negate_formula(f,&temp) == TROUBLE)
		return(TROUBLE);
	if (nnf(temp,&temp1) == TROUBLE)
		return(TROUBLE);
	if (cnf(temp1,&temp3) == TROUBLE)
		return(TROUBLE);
	if (negate_formula(temp3,&temp2) == TROUBLE)
		return(TROUBLE);
	if (nnf(temp2,ddd) == TROUBLE)		/* it sets *ddd */
		return(TROUBLE);

return(NO_TROUBLE);
}  /* dnf */    
    
/*************
 *
 *    static int rename_syms_term(t, fr)
 *
 *    Called from rename_syms_formula.
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE.
 *
 *************/

static int rename_syms_term(t, fr)
struct term *t;
struct formula *fr;
{
    struct rel *r;
    int sn;

    if (t->type == NAME) {
	if (var_name(sn_to_str(t->sym_num))) {
	    fprintf(Fderr,"\nWARNING, the following formula has constant '%s', whose\nname may be misinterpreted by the user as a variable.\n", sn_to_str(t->sym_num));
	    print_formula(Fderr, fr);  fprintf(stderr, "\n");
#if 0  /* replaced 18 June 91 WWM */
	    if (new_functor_name(0, &sn) == TROUBLE) /* with arity 0 */
		return(TROUBLE);
	    subst_sn_formula(t->sym_num, fr, sn, NAME);
#endif	    
	    }
	}
    else if (t->type == VARIABLE) {
	if (!var_name(sn_to_str(t->sym_num))) {
	    if (new_var_name(&sn) == TROUBLE)
		return(TROUBLE);
	    subst_sn_formula(t->sym_num, fr, sn, VARIABLE);
	    }
	}
    else {
	r = t->farg;
	while(r != NULL) {
	    if (rename_syms_term(r->argval, fr) == TROUBLE)
		return(TROUBLE);
	    r = r->narg;
	    }
	}
return(NO_TROUBLE);
}  /* rename_syms_term */

/*************
 *
 *    int rename_syms_formula(f, fr)
 *
 *    Rename VARIABLEs so that they conform to the rule for clauses.
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int rename_syms_formula(f, fr)
struct formula *f;
struct formula *fr;
{
    struct formula *f1;
    
    if (f->type == ATOM_FORM)
	{
	if (rename_syms_term(f->t, fr) == TROUBLE)
		return(TROUBLE);
	}
    else {
	for (f1 = f->first_child; f1; f1 = f1->next)
	    if (rename_syms_formula(f1, fr) == TROUBLE)
		return(TROUBLE);
	}
return(NO_TROUBLE);
}  /* rename_syms_formula() */

/*************
 *
 *    void subst_sn_term(old_sn, t, new_sn, type)
 *
 *************/

void subst_sn_term(old_sn, t, new_sn, type)
int old_sn;
struct term *t;
int new_sn;
int type;
{
    struct rel *r;

    if (t->type == NAME) {
	if (type == NAME && t->sym_num == old_sn)
	    t->sym_num = new_sn;
	}
    else if (t->type == VARIABLE) {
	if (type == VARIABLE && t->sym_num == old_sn)
	    t->sym_num = new_sn;
	}
    else {
	for (r = t->farg; r; r = r->narg)
	    subst_sn_term(old_sn, r->argval, new_sn, type);
	}
}  /* subst_sn_term */

/*************
 *
 *    void subst_sn_formula(old_sn, f, new_sn, type)
 *
 *************/

void subst_sn_formula(old_sn, f, new_sn, type)
int old_sn;
struct formula *f;
int new_sn;
int type;
{
    struct formula *f1;
    
    if (f->type == ATOM_FORM)
	subst_sn_term(old_sn, f->t, new_sn, type);
    else {
	for (f1 = f->first_child; f1; f1 = f1->next)
	    subst_sn_formula(old_sn, f1, new_sn, type);
	}
}  /* subst_sn_formula */

/*************
 *
 *    int gen_subsume_prop(c, d) -- does c gen_subsume_prop d?
 *
 *    This is generalized propositional subsumption.  If given
 *    quantified formulas, they are treated as atoms (formula_ident
 *    determines outcome).
 *
 *************/

int gen_subsume_prop(c, d)
struct formula *c;
struct formula *d;
{
    struct formula *f;

    /* The order of these tests is important.  For example, if */
    /* the last test is moved to the front, c=(p|q) will not   */
    /* subsume d=(p|q|r).                                      */
    
    if (c->type == OR_FORM) {  /* return(each c_i subsumes d) */
	for (f = c->first_child; f && gen_subsume_prop(f, d); f = f->next);
	return(f == NULL);
	}
    else if (d->type == AND_FORM) {  /* return(c subsumes each d_i) */
	for (f = d->first_child; f && gen_subsume_prop(c, f); f = f->next);
	return(f == NULL);
	}
    else if (c->type == AND_FORM) {  /* return(one c_i subsumes d) */
	for (f = c->first_child; f && ! gen_subsume_prop(f, d); f = f->next);
	return(f != NULL);
	}
    else if (d->type == OR_FORM) {  /* return(c subsumes one d_i) */
	for (f = d->first_child; f && ! gen_subsume_prop(c, f); f = f->next);
	return(f != NULL);
	}
    else  /* c and d are NOT, ATOM, or QUANT */
	return(formula_ident(c, d));

}  /* gen_subsume_prop */

/*************
 *
 *    struct formula *subsume_conj(c)
 *
 *    Given a conjunction, discard weaker conjuncts.
 *    This is like deleting subsumed clauses.
 *    The result is equivalent.
 *
 *************/

struct formula *subsume_conj(c)
struct formula *c;
{
    struct formula *f1, *f2, *f3, *prev;

    if (c->type != AND_FORM  || c->first_child == NULL)
	return(c);
    else {
	/* start with second child */
	prev = c->first_child;
	f1 = prev->next;
	while (f1) {
	    /* first do forward subsumption of part already processed */
	    f2 = c->first_child;
	    while (f2 != f1 && ! gen_subsume_prop(f2, f1))
	        f2 = f2->next;;
	    if (f2 != f1) {  /* delete f1 */
		prev->next = f1->next;
		zap_formula(f1);
		f1 = prev;
		}
	    else {
		/* back subsumption on part already processed */
		/* delete all previous that are subsumed by f1 */
		f2 = c->first_child;
		prev = NULL;
		while (f2 != f1) {
		    if (gen_subsume_prop(f1, f2)) {
			if (prev == NULL)
			    c->first_child = f2->next;
			else
			    prev->next = f2->next;
			f3 = f2;
			f2 = f2->next;
			zap_formula(f3);
			}
		    else {
			prev = f2;
			f2 = f2->next;
			}
		    }
		}
	    prev = f1;
	    f1 = f1->next;
	    }
	/* If just one child left, replace input formula with child. */
	if (c->first_child->next == NULL) {
	    f1 = c->first_child;
	    f1->next = c->next;
	    free_formula(c);
	    return(f1);
	    }
	else
	    return(c);
	}
}  /* subsume_conj */

/*************
 *
 *    struct formula *subsume_disj(c)
 *
 *    Given a disjunction, discard stronger disjuncts.
 *    The result is equivalent.  This the dual of
 *    normal clause subsumption.
 *
 *************/

struct formula *subsume_disj(c)
struct formula *c;
{
    struct formula *f1, *f2, *f3, *prev;

    if (c->type != OR_FORM  || c->first_child == NULL)
	return(c);
    else {
	/* start with second child */
	prev = c->first_child;
	f1 = prev->next;
	while (f1) {
	    /* delete f1 if it subsumes anything previous */
	    f2 = c->first_child;
	    while (f2 != f1 && ! gen_subsume_prop(f1, f2))
	        f2 = f2->next;;
	    if (f2 != f1) {  /* delete f1 */
		prev->next = f1->next;
		zap_formula(f1);
		f1 = prev;
		}
	    else {
		/* delete all previous that subsume f1 */
		f2 = c->first_child;
		prev = NULL;
		while (f2 != f1) {
		    if (gen_subsume_prop(f2, f1)) {
			if (prev == NULL)
			    c->first_child = f2->next;
			else
			    prev->next = f2->next;
			f3 = f2;
			f2 = f2->next;
			zap_formula(f3);
			}
		    else {
			prev = f2;
			f2 = f2->next;
			}
		    }
		}
	    prev = f1;
	    f1 = f1->next;
	    }
	/* If just one child left, replace input formula with child. */
	if (c->first_child->next == NULL) {
	    f1 = c->first_child;
	    f1->next = c->next;
	    free_formula(c);
	    return(f1);
	    }
	else
	    return(c);
	}
}  /* subsume_disj */

/*************
 *
 *    int formula_ident(f1, f2)
 *
 *    Do not permute ANDs, ORs, or like quantifiers.
 *
 *************/

int formula_ident(f, g)
struct formula *f, *g;
{
    struct formula *f1, *g1;

    if (f->type != g->type)
	return(0);
    else if (f->type == ATOM_FORM)
	return(term_ident(f->t, g->t));
    else if (f->type == QUANT_FORM) {
	if (f->quant_type != g->quant_type || ! term_ident(f->t, g->t))
	    return(0);
	else
	    return(formula_ident(f->first_child, g->first_child));
	}
    else {  /* AND_FORM || OR_FORM || IFF_FORM || IMP_FORM || NOT_FORM */
	for (f1 = f->first_child, g1 = g->first_child; f1 && g1;
	     f1 = f1->next, g1 = g1->next)
	    if (! formula_ident(f1, g1))
		return(0);
	return(f1 == NULL && g1 == NULL);
	}
}  /* formula_ident */

/*************
 *
 *    conflict_tautology(f)
 *
 *    If f is an AND_FORM, reduce to empty disjunction (FALSE)
 *    if conflicting conjuncts occur.
 *    If f is an OR_FORM,  reduce to empty conjunction (TRUE) 
 *    if conflicting disjuncts occur.
 *
 *************/

void conflict_tautology(f)
struct formula *f;
{
    struct formula *f1, *f2, *a1, *a2;
    int f1_sign, f2_sign;

    /* note possible return from inner loop */

    if (f->type == AND_FORM || f->type == OR_FORM) {
	for (f1 = f->first_child; f1; f1 = f1->next) {
	    f1_sign = (f1->type != NOT_FORM);
	    a1 = (f1_sign ? f1 : f1->first_child);
	    for (f2 = f1->next; f2; f2 = f2->next) {
		f2_sign = (f2->type != NOT_FORM);
		if (f1_sign != f2_sign) {
		    a2 = (f2_sign ? f2 : f2->first_child);
		    if (formula_ident(a1, a2)) {
			f1 = f->first_child;
			while (f1) {
			    f2 = f1;
			    f1 = f1->next;
			    zap_formula(f2);
			    }
			f->first_child = NULL;
			/* switch types */
			f->type = (f->type == AND_FORM ? OR_FORM : AND_FORM);
			return;
			}
		    }
		}
	    }
	}
}  /* conflict_tautology */

/*************
 *
 *   void ts_and_fs(f)
 *
 *   Simplify if f is AND or OR, and an immediate subformula is
 *   TRUE (empty AND) or FALSE (empty OR).
 *
 *************/

void ts_and_fs(f)
struct formula *f;
{
    struct formula *f1, *f2, *f_prev;
    int f_type;

    f_type = f->type;
    if (f_type != AND_FORM && f_type != OR_FORM)
	return;
    else {
	f_prev = NULL;
	f1 = f->first_child;
	while (f1 != NULL) {
	    if ((f1->type == AND_FORM || f1->type == OR_FORM) &&
		                              f1->first_child == NULL) {
		if (f_type != f1->type) {
		    f->type = f1->type;
		    f1 = f->first_child;
		    while (f1) {
			f2 = f1;
			f1 = f1->next;
			zap_formula(f2);
			}
		    f->first_child = NULL;
		    /* switch types */
		    f->type = (f->type == AND_FORM ? OR_FORM : AND_FORM);
		    return;
		    }
		else {
		    if (f_prev == NULL)
			f->first_child = f1->next;
		    else
			f_prev->next = f1->next;

		    f2 = f1;
		    f1 = f1->next;
		    free_formula(f2);
		    }
		}
	    else {
		f_prev = f1;
		f1 = f1->next;
		}
	    }
	}
}  /* ts_and_fs */

/*************
 *
 *     static int set_vars_term_2(term, sn)
 *
 *     Called from set_vars_cl_2.
 *
 *************/

static int set_vars_term_2(t, sn)
struct term *t;
int sn[];
{
    struct rel *r;
    int i, rc;
    
    if (t->type == COMPLEX) {
	r = t->farg;
	rc = 1;
	while (rc && r != NULL) {
	    rc = set_vars_term_2(r->argval, sn);
	    r = r->narg;
	    }
	return(rc);
	}
    else if (t->type == NAME)
	return(1);
    else {
	i = 0;
	while (i < MAX_VARS && sn[i] != -1 && sn[i] != t->sym_num)
	    i++;
	if (i == MAX_VARS)
	    return(0);
	else {
	    if (sn[i] == -1)
		sn[i] = t->sym_num;
	    t->varnum = i;
	    /*  include following to destroy input variable names 
            t->sym_num = 0;
	    */
	    return(1);
	    }
	}
}  /* set_vars_term_2 */

/*************
 *
 *    static int set_vars_cl_2(cl) -- give variables var_nums
 *
 *    This is different from set_vars_cl bacause variables have
 *    already been identified:  type==VARIABLE.  Identical
 *    variables have same sym_num.
 *
 *************/

static int set_vars_cl_2(cl)
struct clause *cl;
{
    struct literal *lit;
    int sn[MAX_VARS];
    int i;

    for (i=0; i<MAX_VARS; i++)
	sn[i] = -1;
    lit = cl->first_lit;
    while (lit != NULL) {
	if (set_vars_term_2(lit->atom, sn))
	    lit = lit->next_lit;
	else
	    return(0);
	}
    return(1);
}  /* set_vars_cl_2 */

/*************
 *
 *    static int disj_to_clause(f,dtc)
 *
 *	struct clause * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct clause through the
 *	parameter dtc.
 *
 *************/

static int disj_to_clause(f,dtc)
struct formula *f;
struct clause **dtc;
{
    struct formula *f1, *f2;
    struct clause *c;
    struct literal *lit, *prev;

	*dtc = NULL;				/* default */
    if (get_clause(&c) == TROUBLE)
	return(TROUBLE);
    if (f->type == ATOM_FORM || f->type == NOT_FORM) {
	if (get_literal(&lit) == TROUBLE)
		return(TROUBLE);
	lit->sign = (f->type == ATOM_FORM);
	lit->atom = (f->type == ATOM_FORM ? f->t : f->first_child->t);
	if (f->type == NOT_FORM)
	    free_formula(f->first_child);
	free_formula(f);
	lit->atom->occ.lit = lit;
	lit->container = c;
	mark_literal(lit);  /* atoms have varnum > 0 */
	c->first_lit = lit;
	}
    else {  /* OR_FORM */
	prev = NULL;
	f1 = f->first_child;
	while (f1) {
	    f2 = f1;
	    f1 = f1->next;
	    
	    if (get_literal(&lit) == TROUBLE)
		return(TROUBLE);
	    lit->sign = (f2->type == ATOM_FORM);
	    lit->atom = (f2->type == ATOM_FORM ? f2->t : f2->first_child->t);
	    if (f2->type == NOT_FORM)
		free_formula(f2->first_child);
	    free_formula(f2);
	    lit->atom->occ.lit = lit;
	    lit->container = c;
	    mark_literal(lit);  /* atoms have varnum > 0 */
	    
	    if (prev == NULL) 
		c->first_lit = lit;
	    else
		prev->next_lit = lit;
	    prev = lit;
	    }
	free_formula(f);
	}

    if (set_vars_cl_2(c) == 0) {
	output_stats(Fdout, 4);
fprintf(Fderr,"ABEND, too many variables in clause, max is %d.\007\n",MAX_VARS);
fprintf(Fdout,"ABEND, too many variables in clause, max is %d:\n",MAX_VARS);
	print_clause(Fdout, c);
	return(TROUBLE);
	}
    cl_merge(c);  /* merge identical literals */
	*dtc = c;
    return(NO_TROUBLE);
}  /* disj_to_clause */

/*************
 *
 *    static int cnf_to_list(f,ctl)
 *
 *    Convert a CNF formula to a list of clauses.
 *   This includes assigning variable numbers to the varnum fileds of VARIABLES.
 *    An ABEND occurs if a clause has too many variables.
 *	struct list * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct list through the
 *	parameter ctl.
 *
 *************/

static int cnf_to_list(f,ctl)
struct formula *f;
struct list **ctl;
{
    struct formula *f1, *f2;
    struct list *l;
    struct clause *c;

	*ctl = NULL;				/* default */
    if (get_list(&l) == TROUBLE)
	return(TROUBLE);
    if (f->type != AND_FORM) {
	if (disj_to_clause(f,&c) == TROUBLE)
		return(TROUBLE);
	append_cl(l, c);
	}
    else {  /* OR_FORM || ATOM_FORM || NOT_FORM */
	f1 = f->first_child;
	while (f1) {
	    f2 = f1;
	    f1 = f1->next;
	    if (disj_to_clause(f2,&c) == TROUBLE)  /* zaps f2 */
		return(TROUBLE);
	    append_cl(l, c);
	    }
	free_formula(f);
	}
	*ctl = l;
    return(NO_TROUBLE);
}  /* cnf_to_list */

/*************
 *
 *    int clausify(f,cfy) -- Skolem/CNF tranformation.
 *
 *    Convert a quantified formula to a list of clauses.
 *	struct list * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct list through the
 *	parameter cfy.
 *
 *************/

int clausify(f,cfy)
struct formula *f;
struct list **cfy;
{
    struct list *l;
	struct formula *temp;

	*cfy = NULL;				/* default */
    if (nnf(f,&temp) == TROUBLE)
	return(TROUBLE);
	f = temp;
    if (skolemize(f,&temp) == TROUBLE)
	return(TROUBLE);
	f = temp;
    if (unique_all(f) == TROUBLE)
	return(TROUBLE);
    f = zap_quant(f);
    if (rename_syms_formula(f, f) == TROUBLE)
	return(TROUBLE);
    if (cnf(f,&temp) == TROUBLE)
	return(TROUBLE);
	f = temp;
    if (cnf_to_list(f,&l) == TROUBLE)
	return(TROUBLE);
	*cfy = l;
    return(NO_TROUBLE);

}  /* clausify */

/*************
 *
 *    int clausify_formula_list(fp,cfl)
 *
 *    Clausify a set of formulas, and return a list of clauses.
 *    The set of formulas is deallocated.
 *	struct list * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct list through the
 *	parameter cfl.
 *
 *************/

int clausify_formula_list(fp,cfl)
struct formula_ptr *fp;
struct list **cfl;
{
    struct list *l, *l1;
    struct formula_ptr *fp1, *fp2;

	*cfl = NULL;				/* default */
    if (get_list(&l) == TROUBLE)
	return(TROUBLE);
    fp1 = fp;
    while (fp1 != NULL) {
	if (clausify(fp1->f,&l1) == TROUBLE)
		return(TROUBLE);
	append_lists(l, l1);
	fp2 = fp1;
	fp1 = fp1->next;
	free_formula_ptr(fp2);
	}
	*cfl = l;
    return(NO_TROUBLE);
}  /* clausify_formula_list */

/*************
 *
 *    int negation_inward(f, ni)
 *
 *    If f is a negated conjunction, disjunction, or quantified formula,
 *    move the negation sign in one level.
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter ni.
 *
 *************/

int negation_inward(f, ni)
struct formula *f;
struct formula **ni;
{
    struct formula *f1, *f2, *prev, *f_save;
	struct formula *temp;
    
	*ni = NULL;				/* default */

    if (f->type == NOT_FORM) {
	f1 = f->first_child;
if (f1->type == AND_FORM || f1->type == OR_FORM || f1->type == QUANT_FORM) {
	    f_save = f->next;
	    if (negate_formula(f,&temp) == TROUBLE)
		return(TROUBLE);
		f = temp;
	    f->next = f_save;
	    
	    if (f->type == AND_FORM || f->type == OR_FORM) {
		/* apply DeMorgan's laws */
		f->type = (f->type == AND_FORM ? OR_FORM : AND_FORM);
		f1 = f->first_child;
		prev = NULL;
		while (f1) {
		    f2 = f1;
		    f1 = f1->next;
		    if (negate_formula(f2,&temp) == TROUBLE)
			return(TROUBLE);
			f2 = temp;
		    if (prev)
			prev->next = f2;
		    else
			f->first_child = f2;
		    prev = f2;
		    }
		}
	    else {  /* QUANT_FORM */
        f->quant_type = (f->quant_type==ALL_QUANT ? EXISTS_QUANT : ALL_QUANT);
		if (negate_formula(f->first_child,&temp) == TROUBLE)
			return(TROUBLE);
		f->first_child = temp;
		}
	    
	    }
	}
	*ni = f;
    return(NO_TROUBLE);
}  /* negation_inward */

/*************
 *
 *    int expand_imp(f, ei)
 *
 *    Change (P -> Q) to (-P | Q).
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter ei.
 *
 *************/

int expand_imp(f, ei)
struct formula *f;
struct formula **ei;
{
struct formula *temp;

	*ei = NULL;				/* default */

    if (f->type != IMP_FORM)
	{
	*ei = f;
	return(NO_TROUBLE);
	}
    else {
	f->type = OR_FORM;
	if (negate_formula(f->first_child,&temp) == TROUBLE)
		return(TROUBLE);
	f->first_child = temp;
	*ei = f;
	return(NO_TROUBLE);
	}
}  /* expand_imp */

/*************
 *
 *    int iff_to_conj(f, itc)
 *
 *    Change (P <-> Q) to ((P -> Q) & (Q -> P)).
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter itc.
 *
 *************/

int iff_to_conj(f, itc)
struct formula *f;
struct formula **itc;
{
    struct formula *f1, *f2, *f_save;

	*itc = NULL;				/* default */
    if (f->type != IFF_FORM)
	{
	*itc = f;
	return(NO_TROUBLE);
	}
    else {
	f_save = f->next;

	if (copy_formula(f, &f1) == TROUBLE)
		return(TROUBLE);
	f->type = f1->type = IMP_FORM;

	/* flip args of f1 */
	
	f2 = f1->first_child;
	f1->first_child = f2->next;
	f2->next = NULL;
	f1->first_child->next = f2;

        f->next = f1;
	f1->next = NULL;		

        /* build conjunction */
        if (get_formula(&f2) == TROUBLE)
		return(TROUBLE);
	f2->type = AND_FORM;
	f2->first_child = f;

	f2->next = f_save;
	*itc = f2;
	return(NO_TROUBLE);
	}
}  /* iff_to_conj */

/*************
 *
 *    int iff_to_disj(f, itd)
 *
 *    Change (P <-> Q) to ((P & Q) | (-Q & -P)).
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter itd.
 *
 *************/

int iff_to_disj(f, itd)
struct formula *f;
struct formula **itd;
{
    struct formula *f1, *f2, *f_save;
	struct formula *temp;

	*itd = NULL;				/* default */
    if (f->type != IFF_FORM)
	{
	*itd = f;
	return(NO_TROUBLE);
	}
    else {
	f_save = f->next;

	if (copy_formula(f, &f1) == TROUBLE)
		return(TROUBLE);
	f->type = f1->type = AND_FORM;
	if (negate_formula(f1->first_child->next,&temp) == TROUBLE)
		return(TROUBLE);
	f1->first_child->next = temp;
	if (negate_formula(f1->first_child,&temp) == TROUBLE)
		return(TROUBLE);
	f1->first_child = temp;

        f->next = f1;
	f1->next = NULL;		

        /* build disjunction */
        if (get_formula(&f2) == TROUBLE)
		return(TROUBLE);
	f2->type = OR_FORM;
	f2->first_child = f;

	f2->next = f_save;
	*itd = f2;
	return(NO_TROUBLE);
	}
}  /* iff_to_disj */

/*************
 *
 *    int nnf_cnf(f, nc)
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter nc.
 *
 *************/

int nnf_cnf(f, nc)
struct formula *f;
struct formula **nc;
{
struct formula *temp;

	*nc = NULL;				/* default */

	if (nnf(f,&temp) == TROUBLE)
		return(TROUBLE);

    if (cnf(temp, nc) == TROUBLE)	/* it sets nc */
	return(TROUBLE);

	return(NO_TROUBLE);
}  /* nnf_cnf */

/*************
 *
 *    int nnf_dnf(f, nd)
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter nd.
 *
 *************/

int nnf_dnf(f, nd)
struct formula *f;
struct formula **nd;
{
struct formula *temp;

	*nd = NULL;				/* default */

	if (nnf(f,&temp) == TROUBLE)
		return(TROUBLE);

	if (dnf(temp,nd) == TROUBLE)	/* it sets nd */
		return(TROUBLE);

    return(NO_TROUBLE);
}  /* nnf_dnf */

/*************
 *
 *    int nnf_skolemize(f, nnfsko)
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter nnfsko.
 *
 *************/

int nnf_skolemize(f, nnfsko)
struct formula *f;
struct formula **nnfsko;
{
struct formula *temp;

	*nnfsko = NULL;				/* default */

	if (nnf(f,&temp) == TROUBLE)
		return(TROUBLE);
	if (skolemize(temp,nnfsko) == TROUBLE)	/* it sets nnfsko */
		return(TROUBLE);

	return(NO_TROUBLE);

}  /* nnf_skolemize */

/*************
 *
 *    int clausify_formed(f,cf)
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter cf.
 *
 *************/

int clausify_formed(f,cf)
struct formula *f;
struct formula **cf;
{
struct formula *temp;

	*cf = NULL;				/* default */
    if (nnf(f,&temp) == TROUBLE)
	return(TROUBLE);
	f = temp;
    if (skolemize(f,&temp) == TROUBLE)
	return(TROUBLE);
	f = temp;
    if (unique_all(f) == TROUBLE)
	return(TROUBLE);
    f = zap_quant(f);
    if (rename_syms_formula(f, f) == TROUBLE)
	return(TROUBLE);
    if (cnf(f, &temp) == TROUBLE)
	return(TROUBLE);
	f = temp;
	*cf = f;
    return(NO_TROUBLE);
}  /* clausify_formed */

/*************
 *
 *    int rms_conflict_tautology(f)
 *
 *    If f is an AND_FORM, reduce to empty disjunction (FALSE)
 *    if conflicting conjuncts occur.
 *    If f is an OR_FORM,  reduce to empty conjunction (TRUE) 
 *    if conflicting disjuncts occur.
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int rms_conflict_tautology(f)
struct formula *f;
{
    struct formula *f1, *f2;
	int tempint;
    
    /* note possible return from inner loop */
    
    if (f->type == AND_FORM) {
	for (f1 = f->first_child; f1; f1 = f1->next) {
	    for (f2 = f1->next; f2; f2 = f2->next) {
		if (gen_conflict(f1,f2,&tempint) == TROUBLE)
			return(TROUBLE);
		if (tempint) {
		    f1 = f->first_child;
		    while (f1) {
			f2 = f1;
			f1 = f1->next;
			zap_formula(f2);
			}
		    f->first_child = NULL;
		    /* switch types */
		    f->type = OR_FORM;
		    return(NO_TROUBLE);
		    }
		}
	    }
	}

    else if (f->type == OR_FORM) {
	for (f1 = f->first_child; f1; f1 = f1->next) {
	    for (f2 = f1->next; f2; f2 = f2->next) {
		if (gen_tautology(f1,f2,&tempint) == TROUBLE) 
			return(TROUBLE);
		if (tempint)
		{
		    f1 = f->first_child;
		    while (f1) {
			f2 = f1;
			f1 = f1->next;
			zap_formula(f2);
			}
		    f->first_child = NULL;
		    /* switch types */
		    f->type = AND_FORM;
		    return(NO_TROUBLE);
		    }
		}
	    }
	}
return(NO_TROUBLE);
}  /* rms_conflict_tautology */

/*************
 *
 *    int rms_subsume_conj(c,rsc)
 *
 *    Given a conjunction, discard weaker conjuncts.
 *    This is like deleting subsumed clauses.
 *    The result is equivalent.
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter rsc.
 *
 *************/

int rms_subsume_conj(c,rsc)
struct formula *c;
struct formula **rsc;
{
    struct formula *f1, *f2, *f3, *prev;
	int tempint;

	*rsc = NULL;				/* default */
    if (c->type != AND_FORM  || c->first_child == NULL)
	{
	*rsc = c;
	return(NO_TROUBLE);
	}
    else {
	/* start with second child */
	prev = c->first_child;
	f1 = prev->next;
	while (f1) {
	    /* first do forward subsumption of part already processed */
	    f2 = c->first_child;
		if (gen_subsume(f2,f1,&tempint) == TROUBLE)
			return(TROUBLE);
	    while (f2 != f1 && !tempint)
		{
	        f2 = f2->next;
		if (gen_subsume(f2,f1,&tempint) == TROUBLE)
			return(TROUBLE);
		}
	    if (f2 != f1) {  /* delete f1 */
		prev->next = f1->next;
		zap_formula(f1);
		f1 = prev;
		}
	    else {
		/* back subsumption on part already processed */
		/* delete all previous that are subsumed by f1 */
		f2 = c->first_child;
		prev = NULL;
		while (f2 != f1)
		{
		if (gen_subsume(f1,f2,&tempint) == TROUBLE)
			return(TROUBLE);
		    if (tempint)
			{
			if (prev == NULL)
			    c->first_child = f2->next;
			else
			    prev->next = f2->next;
			f3 = f2;
			f2 = f2->next;
			zap_formula(f3);
			}
		    else {
			prev = f2;
			f2 = f2->next;
			}
		    }
		}
	    prev = f1;
	    f1 = f1->next;
	    }
	/* If just one child left, replace input formula with child. */
	if (c->first_child->next == NULL) {
	    f1 = c->first_child;
	    f1->next = c->next;
	    free_formula(c);
		*rsc = f1;
	    return(NO_TROUBLE);
	    }
	else
		{
		*rsc = c;
	    return(NO_TROUBLE);
		}
	}
}  /* rms_subsume_conj */

/*************
 *
 *    int rms_subsume_disj(c,rsd)
 *
 *    Given a disjunction, discard stronger disjuncts.
 *    The result is equivalent.  This the dual of
 *    normal clause subsumption.
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter rsd.
 *
 *************/

int rms_subsume_disj(c,rsd)
struct formula *c;
struct formula **rsd;
{
    struct formula *f1, *f2, *f3, *prev;
	int tempint;

	*rsd = NULL;				/* default */
    if (c->type != OR_FORM  || c->first_child == NULL)
	{
	*rsd = c;
	return(NO_TROUBLE);
	}
    else {
	/* start with second child */
	prev = c->first_child;
	f1 = prev->next;
	while (f1) {
	    /* delete f1 if it subsumes anything previous */
	    f2 = c->first_child;
		if (gen_subsume(f1,f2,&tempint) == TROUBLE)
			return(TROUBLE);
	    while (f2 != f1 && !tempint)
		{
	        f2 = f2->next;;
		if (gen_subsume(f1,f2,&tempint) == TROUBLE)
			return(TROUBLE);
		}
	    if (f2 != f1) {  /* delete f1 */
		prev->next = f1->next;
		zap_formula(f1);
		f1 = prev;
		}
	    else {
		/* delete all previous that subsume f1 */
		f2 = c->first_child;
		prev = NULL;
		while (f2 != f1) {
		if (gen_subsume(f2,f1,&tempint) == TROUBLE)
			return(TROUBLE);
		    if (tempint)
		    {
			if (prev == NULL)
			    c->first_child = f2->next;
			else
			    prev->next = f2->next;
			f3 = f2;
			f2 = f2->next;
			zap_formula(f3);
			}
		    else {
			prev = f2;
			f2 = f2->next;
			}
		    }
		}
	    prev = f1;
	    f1 = f1->next;
	    }
	/* If just one child left, replace input formula with child. */
	if (c->first_child->next == NULL) {
	    f1 = c->first_child;
	    f1->next = c->next;
	    free_formula(c);
		*rsd = f1;
	    return(NO_TROUBLE);
	    }
	else
		{
		*rsd = c;
	    return(NO_TROUBLE);
		}
	}
}  /* rms_subsume_disj */

/*************
 *
 *    int free_occurrence(v, f)
 *
 *    Does v have a free occurrence in f?
 *
 *************/

int free_occurrence(v, f)
struct term *v;
struct formula *f;
{
    struct formula *f1;
    int free;

    switch (f->type) {
      case ATOM_FORM:
	free = occurs_in(v, f->t);
	break;
      case NOT_FORM:
      case AND_FORM:
      case OR_FORM:
	for (free = 0, f1 = f->first_child; f1 && ! free; f1 = f1->next)
	    free = free_occurrence(v, f1);
	break;
      case QUANT_FORM:
	if (term_ident(v, f->t))
	    return(0);
	else
	    return(free_occurrence(v, f->first_child));
	break;
	}
    return(free);

}  /* free_occurrence */

/*************
 *
 *    int rms_distribute_quants(f,rdq)
 *
 *    f is universally quantified formula.
 *    Child is conjunction in RMS.
 *    Distribute quantifier to conjuncts.
 *    Return a RMS of f.
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter rdq.
 *
 *************/

int rms_distribute_quants(f_quant,rdq)
struct formula *f_quant;
struct formula **rdq;
{
    struct formula *f_conj, *f1, *f2, *f3;
	struct formula *temp;
	struct term *tempterm;

	*rdq = NULL;				/* default */
    f_conj = f_quant->first_child;
    f3 = NULL;
    f1 = f_conj->first_child;
    while (f1) {
	if (get_formula(&f2) == TROUBLE)
		return(TROUBLE);
	f2->type = QUANT_FORM;
	f2->quant_type = ALL_QUANT;
	f2->first_child = f1;
	if (copy_term(f_quant->t, &tempterm) == TROUBLE)
		return(TROUBLE);
	f2->t = tempterm;
	f1 = f1->next;
	f2->first_child->next = NULL;
	if (rms_quantifiers(f2,&temp) == TROUBLE)
		return(TROUBLE);
	f2 = temp;
	 /* indirect recursive call */
        if (f3)
	    f3->next = f2;
	else
	    f_conj->first_child = f2;
	f3 = f2;
	}

    zap_term(f_quant->t);
    free_formula(f_quant);

    flatten_top(f_conj);
    if (rms_conflict_tautology(f_conj) == TROUBLE)
	return(TROUBLE);
    if (rms_subsume_conj(f_conj,&temp) == TROUBLE)
	return(TROUBLE);
	*rdq = temp;
    return(NO_TROUBLE);

}  /* rms_distribute_quants */

/*************
 *
 *    static int separate_free(v, f, free, not_free)
 *
 *	void in Otter, int in penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

static int separate_free(v, f, p_free, p_not_free)
struct term *v;
struct formula *f;
struct formula **p_free;
struct formula **p_not_free;
{
    struct formula *f1, *not_free, *f2, *f3, *prev;

    not_free = f2 = f3 = prev = NULL;
    f1 = f->first_child;
    while (f1) {
        f2 = f1;
	f1 = f1->next;

	if (!free_occurrence(v, f2)) {
	    f2->next = NULL;
	    if (not_free)
		f3->next = f2;
	    else
		not_free = f2;
	    f3 = f2;

	    if (prev == NULL)
		f->first_child = f1;
	    else
		prev->next = f1;
	    }
        else
	    prev = f2;
	}

    if (f->first_child == NULL) {
	*p_free = NULL;
	free_formula(f);
	}
    else if (f->first_child->next == NULL) {
	*p_free = f->first_child;
	free_formula(f);
	}
    else
	*p_free = f;
    
    if (not_free == NULL)
	*p_not_free = NULL;
    else if (not_free->next == NULL)
	*p_not_free = not_free;
    else {
	if (get_formula(&f1) == TROUBLE)
		return(TROUBLE);
	f1->type = OR_FORM;
	f1->first_child = not_free;
	*p_not_free = f1;
	}
return(NO_TROUBLE);
}  /* separate_free */

/*************
 *
 *    int rms_push_free(f,rpf)
 *
 *    f is universally quantifierd formula.
 *    The child of f is a (simple) disjunction in RMS.
 *    Reduce scopes based on free variables.
 *    Result is in RMS, either a quantified formula or a disjunction.
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter rpf.
 *
 *************/

int rms_push_free(f,rpf)
struct formula *f;
struct formula **rpf;
{
    struct formula *f2, *free, *not_free;
	struct formula *temp;

	*rpf = NULL;			/* default */
    if (separate_free(f->t, f->first_child, &free, &not_free) == TROUBLE)
	return(TROUBLE);
    
    if (!free) {  /* var doesn't occur free in any subformula. */
	output_stats(Fdout, 4);
	fprintf(Fderr, "ABEND, rms_push_free has extra quantifier.\007\n");
	fprintf(Fdout, "ABEND, rms_push_free has extra quantifier.\n");
	return(TROUBLE);
	}
    
    if (not_free) {

	f->first_child = free;
	if (rms_quantifiers(f,&temp) == TROUBLE)
		return(TROUBLE);
	f = temp;
	f->next = NULL;
	if (not_free->type == OR_FORM) {
	    /* Install f as last disjunct. */
	    for (f2 = not_free->first_child; f2->next; f2 = f2->next);
	    f2->next = f;
	    f2 = not_free;
	    }
	else {
	    if (get_formula(&f2) == TROUBLE)
		return(TROUBLE);
	    f2->type = OR_FORM;
	    f2->first_child = not_free;
	    not_free->next = f;
	    }
	/* f2 is disjunction */
	if (rms_conflict_tautology(f2) == TROUBLE)
		return(TROUBLE);
	if (rms_subsume_disj(f2,&temp) == TROUBLE)
		return(TROUBLE);
	*rpf = temp;
	return(NO_TROUBLE);
	}
    else
	{
	*rpf = f;
	return(NO_TROUBLE);
	}

}  /* rms_push_free */

/*************
 *
 *    int rms_quantifiers(f,rq)
 *
 *    f is a quantified formula whose child is in RMS.
 *    This function returns a RMS of f.
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter rq.
 *
 *************/

int rms_quantifiers(f,rq)
struct formula *f;
struct formula **rq;
{
    struct formula *f1, *f2, *f_save;
    int negate_flag;
	struct formula *temp;

	*rq = NULL;				/* default */
    f_save = f->next;
    f->next = NULL;

    if (!free_occurrence(f->t, f->first_child)) {
	f1 = f->first_child;
	zap_term(f->t);
	free_formula(f);
	f1->next = f_save;
	*rq = f1;
	return(NO_TROUBLE);
	}

    if (f->quant_type == EXISTS_QUANT) {
	if (negate_formula(f,&temp) == TROUBLE)
		return(TROUBLE);
	if (nnf(temp,&f) == TROUBLE)
		return(TROUBLE);
	negate_flag = 1;
	/* If f is an OR with and AND child, call rms to make conjunction. */
	if (f->first_child->type == OR_FORM) {
	    for(f1 = f->first_child->first_child;
		f1 && f1->type != AND_FORM;
		f1 = f1->next);
	    if (f1)
		{
		if (rms(f->first_child,&temp) == TROUBLE)
			return(TROUBLE);
		f->first_child = temp;
		}
	    }
	}
    else
	negate_flag = 0;

    /* Now, "all" is the quantifier, and child is RMS. */

    if (f->first_child->type == AND_FORM)
	{
	if (rms_distribute_quants(f,&temp) == TROUBLE)
		return(TROUBLE);
	f = temp;
	}
    else if (f->first_child->type == OR_FORM)
	{
	if (rms_push_free(f,&temp) == TROUBLE)
		return(TROUBLE);
	f = temp;
	}
    
    /* else atomic or negated atomic, so do nothing */

    /* f is now not necessarily QUANT_FORM. */

    if (negate_flag) {
	if (negate_formula(f,&temp) == TROUBLE)
		return(TROUBLE);
	if (nnf(temp,&f) == TROUBLE)
		return(TROUBLE);
	if (f->type == QUANT_FORM)
	    f2 = f->first_child;
	else
	    f2 = f;
	/* If f2 is an OR with and AND child, call rms to make conjunction. */
	if (f2->type == OR_FORM) {
	    for(f1 = f2->first_child;
		f1 && f1->type != AND_FORM;
		f1 = f1->next);
	    if (f1) {
		if (f == f2)
			{
		   if (rms(f2,&temp) == TROUBLE)
			return(TROUBLE);
			f = temp;
			}
		else
			{
		    if (rms(f2,&temp) == TROUBLE)
			return(TROUBLE);
			f->first_child = temp;
			}
		}
	    }
	}

    f->next= f_save;
	*rq = f;
    return(NO_TROUBLE);

}  /* rms_quantifiers */

/*************
 *
 *    static int rms_distribute(f,rd) -- rms_distribute OR over AND.
 *
 *    f is an OR node whose subterms are in Reduced MiniScope (RMS).
 *    This routine returns a RMS of f.
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter rd.
 *
 *************/

static int rms_distribute(f,rd)
struct formula *f;
struct formula **rd;
{
    struct formula *f_new, *f1, *f2, *f3, *f4, *f_prev, *f_save;
    int i, j;
	struct formula *temp;

	*rd = NULL;				/* default */
    f_save = f->next; f->next = NULL;

    if (f->type != OR_FORM)
	{
	*rd = f;
	return(NO_TROUBLE);
	}
    else {

	flatten_top(f);
	if (rms_conflict_tautology(f) == TROUBLE)
		return(TROUBLE);
	if (rms_subsume_disj(f,&temp) == TROUBLE)
		return(TROUBLE);
	f = temp;
	if (f->type != OR_FORM)
		{
		*rd = f;
	    return(NO_TROUBLE);
		}
	else {
	    
	    /* find first AND subformula */
	    i = 1;
	    f_prev = NULL;
	    for (f1 = f->first_child; f1 && f1->type != AND_FORM; f1 = f1->next) {
		i++;
		f_prev = f1;
		}
	    if (f1 == NULL)
		{
		*rd = f;
		return(NO_TROUBLE);  /* nothing to rms_distribute */
		}
	    else {
		/* unhook AND */
		if (f_prev)
		    f_prev->next = f1->next;
		else
		    f->first_child = f1->next;
		f2 = f1->first_child;
		f_new = f1;
		f_prev = NULL;
		while (f2) {
		    f3 = f2->next;
		    if (f3)
			{
			if (copy_formula(f, &f1) == TROUBLE)
				return(TROUBLE);
			}
		    else
			f1 = f;
		    if (i == 1) {
			f2->next = f1->first_child;
			f1->first_child = f2;
			}
		    else {
			j = 1;
			for (f4 = f1->first_child; j < i-1; f4 = f4->next)
			    j++;
			f2->next = f4->next;
			f4->next = f2;
			}
		    if (rms_distribute(f1,&temp) == TROUBLE)
			return(TROUBLE);
			f1 = temp;
		    if (f_prev)
			f_prev->next = f1;
		    else
			f_new->first_child = f1;
		    f_prev = f1;
		    f2 = f3;
		    }
		f_new->next = f_save;
		flatten_top(f_new);
		if (rms_conflict_tautology(f_new) == TROUBLE)
			return(TROUBLE);
		if (rms_subsume_conj(f_new,&temp) == TROUBLE)
			return(TROUBLE);
		*rd = temp;
		return(NO_TROUBLE);
		}
	    }
	}
}  /* rms_distribute */

/*************
 *
 *    int rms(f,rrr) -- convert f to Reduced MiniScope (RMS)
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	rrr parameter.
 *
 *************/

int rms(f,rrr)
struct formula *f;
struct formula **rrr;
{
    struct formula *f1, *f2, *f_prev, *f_next, *f_save;
	struct formula *temp;

	*rrr = NULL;				/* default */
    f_save = f->next; f->next = NULL;

    if (f->type == AND_FORM || f->type == OR_FORM) {
	/* first convert subterms to RMS */
	f_prev = NULL;
	f1 = f->first_child;
	while(f1) {
	    f_next = f1->next;
	    if (rms(f1, &f2) == TROUBLE)
		return(TROUBLE);
	    if (f_prev)
		f_prev->next = f2;
	    else
		f->first_child = f2;
	    f_prev = f2;
	    f1 = f_next;
	    }

	if (f->type == AND_FORM) {
	    flatten_top(f);
	    if (rms_conflict_tautology(f) == TROUBLE)
		return(TROUBLE);
	    if (rms_subsume_conj(f,&temp) == TROUBLE)
		return(TROUBLE);
		f = temp;
	    }
	else
		{
	    if (rms_distribute(f,&temp) == TROUBLE)
	 /* flatten and simplify in distribute */
		return(TROUBLE);
		f = temp;
		}
	}

    else if (f->type == QUANT_FORM) {
	if (rms(f->first_child,&temp) == TROUBLE)
		return(TROUBLE);
	f->first_child = temp;
	if (rms_quantifiers(f,&temp) == TROUBLE)
		return(TROUBLE);
	f = temp;
	}
    
    /* else f is atomic or negated atomic, so do nothing; */

    f->next = f_save;
	*rrr = f;
    return(NO_TROUBLE);

}  /* rms */

/*************
 *
 *    static void introduce_var_term(t, v, vnum)
 *
 *************/

static void introduce_var_term(t, v, vnum)
struct term *t;
struct term *v;
int vnum;
{
    struct rel *r;
    
    switch (t->type) {
      case NAME:
	if (term_ident(t,v)) {
	    t->type = VARIABLE;
	    t->varnum = vnum;
	    t->sym_num = 0;
	    }
	break;
      case VARIABLE:
	break;
      case COMPLEX:
	for (r = t->farg; r; r = r->narg)
	    introduce_var_term(r->argval, v, vnum);
	break;
	}
    
}  /* introduce_var_term */

/*************
 *
 *    static int introduce_var(f, t, vnum)
 *
 *    In formula f, replace all free occurrences of t with a variable
 *    (set type to VARIABLE) with number vnum.
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

static int introduce_var(f, t, vnum)
struct formula *f;
struct term *t;
int vnum;
{
    struct formula *f1;
    
    switch (f->type) {
      case ATOM_FORM:
	introduce_var_term(f->t, t, vnum);
	break;
      case AND_FORM:
      case OR_FORM:
      case NOT_FORM:
	for (f1 = f->first_child; f1; f1 = f1->next)
	    introduce_var(f1, t, vnum);
	break;
      case QUANT_FORM:
	if (!term_ident(t, f->t))
	    introduce_var(f->first_child, t, vnum);
	break;
      default:
	
	output_stats(Fdout, 4);
	fprintf(Fderr, "ABEND, introduce_var, bad formula.\007\n");
	fprintf(Fdout, "ABEND, introduce_var, bad formula.");
	return(TROUBLE);
	break;
	}
    return(NO_TROUBLE);
}  /* introduce_var */

/*************
 *
 *    int renumber_unique(f,vnum,ru)
 *
 *    f is NNF, and all quantifiers are unique.
 *    This function renumbers variables, starting with *vnum_p and
 *    removes quantifiers.
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE.
 *
 *************/

int renumber_unique(f,vnum_p,ru)
struct formula *f;
int *vnum_p;
struct formula **ru;
{
    struct formula *f1, *f2, *f_prev, *f_next;
	struct formula *temp;

	*ru = NULL;				/* default */
    switch (f->type) {
      case ATOM_FORM:
	*ru = f;
	return(NO_TROUBLE);
	break;
      case AND_FORM:
      case OR_FORM:
      case NOT_FORM:
	f_prev = NULL;
	f1 = f->first_child;
	while(f1) {
	    f_next = f1->next;
	    if (renumber_unique(f1,vnum_p,&f2) == TROUBLE)
			return(TROUBLE);
	    if (f_prev)
		f_prev->next = f2;
	    else
		f->first_child = f2;
	    f_prev = f2;
	    f1 = f_next;
	    }
	*ru = f;
	return(NO_TROUBLE);
	break;
      case QUANT_FORM:
	f1 = f->first_child;
	if (introduce_var(f1, f->t, *vnum_p) == TROUBLE)
		return(TROUBLE);
	(*vnum_p)++;
	if (*vnum_p == MAX_VARS) {
	    output_stats(Fdout, 4);
	    fprintf(Fderr,"ABEND, renumber_unique, too many vars.\007\n");
	    fprintf(Fdout,"ABEND, renumber_unique, too many vars.\n");
	    return(TROUBLE);
	    }
	f1->next = f->next;
	f->first_child = NULL;
	zap_formula(f);
	if (renumber_unique(f1,vnum_p,&temp) == TROUBLE)
		return(TROUBLE);
	*ru = temp;
	return(NO_TROUBLE);
	break;
	}
    
    output_stats(Fdout, 4);
    fprintf(Fderr, "ABEND, renumber_unique, bad formula.\007\n");
    fprintf(Fdout, "ABEND, renumber_unique, bad formula.");
    return(TROUBLE);
	    
}  /* renumber_unique */

/*************
 *
 *    int gen_subsume_rec(c, cs, d, ds, tr_p, gsr) 
 *	 does c gen_subsume_rec d?
 *
 *    This is generalized subsumption on quantified formulas.  It is 
 *    not as complete as the Prolog version, because there is no
 *    backtracking to try alternatives in cases 3 and 4 below.
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE and 0/1 through the
 *	parameter gsr.
 *
 *************/

int gen_subsume_rec(c, cs, d, ds, tr_p, gsr)
struct formula *c;
struct formula *d;
struct context *cs;
struct context *ds;
struct trail **tr_p;
int *gsr;
{
    struct formula *f;
	int tempint, uok;

	*gsr = 0;

    /* The order of these tests is important.  For example, if */
    /* the last test is moved to the front, c=(p|q) will not   */
    /* subsume d=(p|q|r).                                      */
    
    if (c->type == OR_FORM)   /* return(each c_i subsumes d) */
	{
	f = c->first_child;
	if (f)
	if (gen_subsume_rec(f, cs, d, ds, tr_p, &tempint) == TROUBLE)
		return(TROUBLE);
	while (f && tempint)
	{
	f = f->next;
	if (f)
	if (gen_subsume_rec(f, cs, d, ds, tr_p, &tempint) == TROUBLE)
		return(TROUBLE);
	}	/* end of while */
	*gsr = (f == NULL);
	return(NO_TROUBLE);
	}	/* end of OR_FORM */
    else if (d->type == AND_FORM)   /* return(c subsumes each d_i) */
	{
	f = d->first_child;
	if (f)
	if (gen_subsume_rec(c, cs, f, ds, tr_p, &tempint) == TROUBLE)
		return(TROUBLE);
	while (f && tempint)
	{
	f = f->next;
	if (f)
	if (gen_subsume_rec(c, cs, f, ds, tr_p, &tempint) == TROUBLE)
		return(TROUBLE);
	}	/* end of while */
	*gsr = (f == NULL);
	return(NO_TROUBLE);
	}	/* end of AND_FORM */
    else if (c->type == AND_FORM)   /* return(one c_i subsumes d) */
	{
	f = c->first_child;
	if (f)
	if (gen_subsume_rec(f, cs, d, ds, tr_p, &tempint) == TROUBLE)
		return(TROUBLE);
	while (f && !tempint)
	{
	f = f->next;
	if (f)
	if (gen_subsume_rec(f, cs, d, ds, tr_p, &tempint) == TROUBLE)
		return(TROUBLE);
	}	/* end of while */
	*gsr = (f != NULL);
	return(NO_TROUBLE);
	}
    else if (d->type == OR_FORM)   /* return(c subsumes one d_i) */
	{
	f = d->first_child;
	if (f)
	if (gen_subsume_rec(c, cs, f, ds, tr_p, &tempint) == TROUBLE)
		return(TROUBLE);
	while (f && !tempint)
	{
	f = f->next;
	if (f)
	if (gen_subsume_rec(c, cs, f, ds, tr_p, &tempint) == TROUBLE)
		return(TROUBLE);
	}	/* end of while */
	*gsr = (f != NULL);
	return(NO_TROUBLE);
	}
    else if (c->type != d->type)
	{
	*gsr = 0;
	return(NO_TROUBLE);
	}
    else if (c->type == NOT_FORM)
	{
if (unify(c->first_child->t, cs, d->first_child->t, ds, tr_p, &uok)==TROUBLE)
	return(TROUBLE);
	*gsr = uok;
	return(NO_TROUBLE);
	}
    else  /* both ATOMs */
	{
	if (unify(c->t, cs, d->t, ds, tr_p, &uok) == TROUBLE)
	return(TROUBLE);
	*gsr = uok;
	return(NO_TROUBLE);
	}

}  /* gen_subsume_rec */

/*************
 *
 *    int gen_subsume(c,d,gs) -- generalized subsumption on RMS formulas.
 *
 *    In Otter, if 1 is returned, (c -> d) holds.
 *	In Penguin, it returns 1 through the parameter gs.
 *
 *************/

int gen_subsume(c,d,gs)
struct formula *c;
struct formula *d;
int *gs;
{
    struct formula *c1, *d1;
    int result, i;
    struct context *cs, *ds;
    struct trail *tr;
	struct formula *temp, *temp1;

	*gs = 0;				/* default */
    Sk_const_num = Sk_func_num = 0;
    i = 6;
	if (copy_formula(c,&temp1) == TROUBLE)
		return(TROUBLE);
	if (skolemize(temp1,temp) == TROUBLE)
		return(TROUBLE);
    if (renumber_unique(temp,&i,&c1) == TROUBLE)
		return(TROUBLE);
    i = 6;
	if (copy_formula(d,&temp1) == TROUBLE)
		return(TROUBLE);
	if (anti_skolemize(temp1,&temp) == TROUBLE)
		return(TROUBLE);
    if (renumber_unique(temp,&i,&d1) == TROUBLE)
		return(TROUBLE);

    if (get_context(&cs) == TROUBLE)
	return(TROUBLE);
    if (get_context(&ds) == TROUBLE)
	return(TROUBLE);
    tr = NULL;

    if (gen_subsume_rec(c1, cs, d1, ds, &tr, &result) == TROUBLE)
	return(TROUBLE);
    clear_subst_1(tr);
    free_context(cs);
    free_context(ds);
    zap_formula(c1);
    zap_formula(d1);
	*gs = result;
    return(NO_TROUBLE);
}  /* gen_subsume */

/*************
 *
 *    int gen_conflict(c,d,gc)
 *
 *    Try to show (c & d) inconsistent by showing (c -> -d).
 *
 *    In Otter, if 1 is returned, (c & d) is inconsistent.
 *	In Penguin, it returns 1 through the parameter gc.
 *
 *************/

int gen_conflict(c,d,gc)
struct formula *c;
struct formula *d;
int *gc;
{
    struct formula *c1, *d1;
    int result, i;
    struct context *cs, *ds;
    struct trail *tr;
	struct formula *temp, *temp1, *temp2;

	*gc = 0;				/* default */
    Sk_const_num = Sk_func_num = 0;
    i = 6;
	if (copy_formula(c,&temp1) == TROUBLE)
		return(TROUBLE);
	if (skolemize(temp1,&temp) == TROUBLE)
		return(TROUBLE);
    if (renumber_unique(temp,&i,&c1) == TROUBLE)
		return(TROUBLE);
    i = 6;
    /* can skip nnf of negate_formula, because anti-skolemize re-negates */
	if (copy_formula(d,&temp1) == TROUBLE)
		return(TROUBLE);
	if (negate_formula(temp1,&temp2) == TROUBLE)
		return(TROUBLE);
	if (anti_skolemize(temp2,&temp) == TROUBLE)
		return(TROUBLE);
	if (renumber_unique(temp,&i,&d1)==TROUBLE)
		return(TROUBLE);
    if (get_context(&cs) == TROUBLE)
	return(TROUBLE);
    if (get_context(&ds) == TROUBLE)
	return(TROUBLE);
    tr = NULL;

    if (gen_subsume_rec(c1, cs, d1, ds, &tr, &result) == TROUBLE)
	return(TROUBLE);
    clear_subst_1(tr);
    free_context(cs);
    free_context(ds);
    zap_formula(c1);
    zap_formula(d1);
	*gc = result;
    return(NO_TROUBLE);
}  /* gen_conflict */

/*************
 *
 *    int gen_tautology(c,d,gt)
 *
 *    Try to show (c | d) a tautology by showing (-c -> d).
 *
 *    In Otter, if 1 is returned, (c | d) is a tautology.
 *	In Penguin, it returns 1 through the parameter gt.
 *
 *************/

int gen_tautology(c,d,gt)
struct formula *c;
struct formula *d;
int *gt;
{
    struct formula *c1, *d1;
    int result, i;
    struct context *cs, *ds;
    struct trail *tr;
	struct formula *temp, *temp1, *temp2, *temp0;

	*gt = 0;				/* default */
    Sk_const_num = Sk_func_num = 0;
    i = 6;
if (copy_formula(c,&temp0) == TROUBLE)
	return(TROUBLE);
if (negate_formula(temp0,&temp2) == TROUBLE)
	return(TROUBLE);
if (nnf(temp2,&temp1) == TROUBLE)
	return(TROUBLE);
if (skolemize(temp1,&temp) == TROUBLE)
		return(TROUBLE);
	if (renumber_unique(temp,&i,&c1) == TROUBLE)
		return(TROUBLE);
    i = 6;
if (copy_formula(d,&temp0) == TROUBLE)
	return(TROUBLE);
if (anti_skolemize(temp0,&temp) == TROUBLE)
		return(TROUBLE);
	if (renumber_unique(temp,&i,&d1) == TROUBLE)
		return(TROUBLE);

    if (get_context(&cs) == TROUBLE)
	return(TROUBLE);
    if (get_context(&ds) == TROUBLE)
	return(TROUBLE);
    tr = NULL;

    if (gen_subsume_rec(c1, cs, d1, ds, &tr, &result) == TROUBLE)
	return(TROUBLE);
    clear_subst_1(tr);
    free_context(cs);
    free_context(ds);
    zap_formula(c1);
    zap_formula(d1);
	*gt = result;
    return(NO_TROUBLE);
}  /* gen_tautology */

/*************
 *
 *    int rms_cnf(f,rc)
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter rc.
 *
 *************/

int rms_cnf(f,rc)
struct formula *f;
struct formula **rc;
{
struct formula *temp;

	*rc = NULL;				/* default */
	if (nnf(f,&temp) == TROUBLE)
		return(TROUBLE);
	if (rms(temp,rc) == TROUBLE)	/* it sets rc */
		return(TROUBLE);

	return(NO_TROUBLE);
}  /* rms_cnf */

/*************
 *
 *    int rms_dnf(f,rd)
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter rd.
 *
 *************/

int rms_dnf(f,rd)
struct formula *f;
struct formula **rd;
{
struct formula *temp, *temp1, *temp2, *temp3;

	*rd = NULL;				/* default */

	if (negate_formula(f,&temp2) == TROUBLE)
		return(TROUBLE);
	if (nnf(temp2,&temp1) == TROUBLE)
		return(TROUBLE);
	if (rms(temp1,&temp) == TROUBLE)
		return(TROUBLE);
	if (negate_formula(temp,&temp3) == TROUBLE)
		return(TROUBLE);
	if (nnf(temp3,rd) == TROUBLE)	/* it sets rd */
		return(TROUBLE);

	return(NO_TROUBLE);
}  /* rms_dnf */
