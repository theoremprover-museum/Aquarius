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
 *  io.c -- input/output routines
 *
 */

#include "header.h"

#define SYM_TAB_SIZE 50

static struct sym_ent *Sym_tab[SYM_TAB_SIZE];

#ifdef ROO

/*************
 *
 *    init_sym_tab_for_roo()
 *
 *    Insert dummy nodes so that all slaves will share table with master.
 *
 *************/

init_sym_tab_for_roo()
{
    int i;

    for (i=0; i < SYM_TAB_SIZE; i++) {
        Sym_tab[i] = get_sym_ent();
        Sym_tab[i]->sym_num = -1;
        str_copy("dummy_node", Sym_tab[i]->name);
        }
}  /* init_sym_tab_for_roo */

#endif

/*************
 *
 *   print_variable(fp, variable)
 *
 *************/

void print_variable(fp, t)
FILE *fp;
struct term *t;
{
    int i;

    if (t->sym_num != 0)
	fprintf(fp, "%s", sn_to_str(t->sym_num));
    else if (Flags[PROLOG_STYLE_VARIABLES].val) {
	fprintf(fp, "%c", (t->varnum % 26) + 'A');
	i = t->varnum / 26;
	if (i > 0)
	    fprintf(fp, "%d", i);
	}
    else {
	if (t->varnum <= 2)
	    fprintf(fp, "%c", 'x'+t->varnum);
	else if (t->varnum <= 5)
	    fprintf(fp, "%c", 'r'+t->varnum);
	else
	    fprintf(fp, "%c%d", 'v', t->varnum);
	}
}  /* print_variable */

/*************
 *
 *   str_print_variable(str, ip, variable)
 *
 *************/

void str_print_variable(str, ip, t)
char *str;
int *ip;
struct term *t;
{
    int i;
    char *s2, s3[100];

    if (t->sym_num != 0) {
        s2 = sn_to_str(t->sym_num);
	str_copy(s2, str += *ip);
	*ip += strlen(s2);
        }
    else if (Flags[PROLOG_STYLE_VARIABLES].val) {
	str[(*ip)++] = (t->varnum % 26) + 'A';
	i = t->varnum / 26;
	if (i > 0) {
	    sprintf(s3, "%d", i);
	    str_copy(s3, str+*ip);
	    *ip += strlen(s3);
	    }
	}
    else {
	if (t->varnum <= 2)
	    str[(*ip)++] = t->varnum + 'x';
	else if (t->varnum <= 5)
	    str[(*ip)++] = t->varnum + 'r';
	else {
	    str[(*ip)++] = 'v';
	    sprintf(s3, "%d", t->varnum);
	    str_copy(s3, str+*ip);
	    *ip += strlen(s3);
	    }
	}
}  /* str_print_variable */

/*************
 *
 *    print_term(file_ptr, term)  --  Print a term to a file.
 *
 *        Variables 0-5 are printed as x,y,z,u,v,w, and equalities
 *    and negated equalities are printed in infix.
 *
 *************/

void print_term(fp,t)
FILE *fp;
struct term *t;
{
    struct rel *r;
    struct term *t1;
    
    if (t == NULL)
	fprintf(fp, "(nil)");
    else if (t->type == NAME) {           /* name */
	if (t->sym_num == Nil_sym_num)
	    fprintf(fp, "[]");
	else
	    fprintf(fp, "%s", sn_to_str(t->sym_num));
	}
    else if (t->type == VARIABLE)               /* variable */
	print_variable(fp, t);
    else {  /* complex */
	if (t->sym_num == Cons_sym_num) {   /* list notation */
	    fprintf(fp, "[");
	    print_term(fp, t->farg->argval);
	    if (proper_list(t) == 0) {
		fprintf(fp, "|");
		print_term(fp, t->farg->narg->argval);
		fprintf(fp, "]");
		}
	    else {
		t1 = t->farg->narg->argval;
		while (t1->sym_num != Nil_sym_num) {
		    fprintf(fp, ",");
		    print_term(fp, t1->farg->argval);
		    t1 = t1->farg->narg->argval;
		    }
		fprintf(fp, "]");
		}
	    }   /* list notation */
	else if (t->sym_num == Eq_sym_num && t->varnum != TERM) {
	    /* (t1 = t2) or (t1 != t2) */
	    fprintf(fp, "(");
	    print_term(fp, t->farg->argval);
	    if (t->occ.lit != NULL && t->occ.lit->sign == 0)
		fprintf(fp, " != ");
	    else
		fprintf(fp, " = ");
	    print_term(fp, t->farg->narg->argval);
	    fprintf(fp, ")");
	    }
	else if (Flags[BIRD_PRINT].val && is_symbol(t, "a", 2))
	    bird_print(fp, t);
	else if (Flags[BIRD_PRINT].val && is_symbol(t, "j", 2)) {
	    print_term(fp, t->farg->argval);
	    fprintf(fp, " + ");
	    print_term(fp, t->farg->narg->argval);
	    }
	else if (Flags[BIRD_PRINT].val && is_symbol(t, "f", 2)) {
	    print_term(fp, t->farg->argval);
	    print_term(fp, t->farg->narg->argval);
	    }
	else if (Flags[BIRD_PRINT].val && is_symbol(t, "g", 1)) {
	    fprintf(fp, "-(");
	    print_term(fp, t->farg->argval);
	    fprintf(fp, ")");
	    }
	else {
	    fprintf(fp, "%s", sn_to_str(t->sym_num));
	    fprintf(fp, "(");
	    r = t->farg;
	    while(r != NULL) {
		print_term(fp, r->argval);
		r = r->narg;
		if(r != NULL)
		    fprintf(fp, ",");
		}
	    fprintf(fp, ")");
	    }
	}
}  /* print_term */

/*************
 *
 *    str_print_term(string, ip, term)  --  Print a term to a string.
 *
 *        Variables 0-5 are printed as x,y,z,u,v,w, and equalities
 *    and negated equalities are printed in infix.
 *
 *************/

void str_print_term(str, ip, t)
char *str;
int *ip;
struct term *t;
{
    struct rel *r;
    struct term *t1;
    char *s2;
    
    if (t == NULL) {
        str_copy("(nil)", str+*ip);
	*ip += 5;
	}
    else if (t->type == NAME) {           /* name */
	if (t->sym_num == Nil_sym_num) {
	    str_copy("[]", str+*ip);
	    *ip += 2;
	    }
	else {
	    s2 = sn_to_str(t->sym_num);
	    str_copy(s2, str+*ip);
	    *ip += strlen(s2);
	    }
	}
    else if (t->type == VARIABLE)               /* variable */
	str_print_variable(str, ip, t);
    else {  /* complex */
	if (t->sym_num == Cons_sym_num) {   /* list notation */
	    str[(*ip)++] = '[';
	    str_print_term(str, ip, t->farg->argval);
	    if (proper_list(t) == 0) {
		str[(*ip)++] = '|';
		str_print_term(str, ip, t->farg->narg->argval);
		str[(*ip)++] = ']';
		}
	    else {
		t1 = t->farg->narg->argval;
		while (t1->sym_num != Nil_sym_num) {
		    str[(*ip)++] = ',';
		    str_print_term(str, ip, t1->farg->argval);
		    t1 = t1->farg->narg->argval;
		    }
		str[(*ip)++] = ']';
		}
	    }   /* list notation */
	else if (t->sym_num == Eq_sym_num && t->varnum != TERM) {
	    /* (t1 = t2) or (t1 != t2) */
	    str[(*ip)++] = '(';
	    str_print_term(str, ip, t->farg->argval);
	    if (t->occ.lit != NULL && t->occ.lit->sign == 0) {
		str_copy(" != ", str+*ip);
		*ip += 4;
		}
	    else {
		str_copy(" = ", str+*ip);
		*ip += 3;
		}
	    str_print_term(str, ip, t->farg->narg->argval);
	    str[(*ip)++] = ')';
	    }
	else if (Flags[BIRD_PRINT].val && is_symbol(t, "a", 2)  && 0)
	    /* bird_print(fp, t) */ ;
	else {
	    s2 = sn_to_str(t->sym_num);
	    str_copy(s2, str+*ip);
	    *ip += strlen(s2);
	    str[(*ip)++] = '(';
	    r = t->farg;
	    while(r != NULL) {
		str_print_term(str, ip, r->argval);
		r = r->narg;
		if(r != NULL)
		    str[(*ip)++] = ',';
		}
	    str[(*ip)++] = ')';
	    }
	}
}  /* str_print_term */

/*************
 *
 *    int sprint_term(s, t) -- return length of s.
 *
 *************/

int sprint_term(s, t)
char *s;
struct term *t;
{
    int i;

    i = 0;
    str_print_term(s, &i, t);
    s[i] = '\0';
    return(i);

}  /* sprint_term */

/*************
 *
 *    p_term(term)  --  print_term to the standard output.
 *
 *************/

void p_term(t)
struct term *t;
{
    print_term(Fdout, t);
}  /* p_term */

/*************
 *
 *    print_term_nl(fp, term)  --  print_term followed by newline
 *
 *************/

void print_term_nl(fp, t)
FILE *fp;
struct term *t;
{
    print_term(fp, t);
    fprintf(fp,"\n");
}  /* print_term_nl */


/*************
 *
 *    int proper_list(t)
 *
 *************/

int proper_list(t)
struct term *t;
{

    if (t->type == VARIABLE)
	return(0);
    else if (t->type == NAME)
	return(t->sym_num == Nil_sym_num);
    else if (t->sym_num != Cons_sym_num)
	return(0);
    else
	return(proper_list(t->farg->narg->argval));
}  /* proper_list */

/*************
 *
 *    int str_to_sn(string, arity, sts) 
 *	Return a symbol number for string/arity.
 *
 *        If the given string/arity is already in the global symbol table,
 *    then return symbol number; else, create a new symbol table entry and
 *    return a new symbol number
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE and the symbol number through
 *	the parameter sts.
 *
 *************/

int str_to_sn(s, arity, sts)
char *s;
int arity;
int *sts;
{
    struct sym_ent *p, *r;
    int i;

    for (i = 0; i < SYM_TAB_SIZE; i++) {
	p = Sym_tab[i];
#ifdef ROO
p = p->next;  /* skip dummy node */
#endif	
	while (p != NULL) {
	    if (str_ident(s, p->name) == 0)
	        p = p->next;
	    else if (p->arity == arity)
		{
		*sts = p->sym_num;
		return(NO_TROUBLE);
		}
	    else if (Flags[CHECK_ARITY].val)
		{
		*sts = 0;
		return(NO_TROUBLE);
		}
	    else
		p = p->next;
	    }	/* end of while */
	}	/* end of for */
	
    if (get_sym_ent(&r) == TROUBLE)
	return(TROUBLE);
    str_copy(s, r->name);
    r->arity = arity;
#ifdef ROO
    p4_lock(&(Glob->sym_tab_lock));
    r->sym_num = new_sym_num();
    i = r->sym_num % SYM_TAB_SIZE;
    r->next = Sym_tab[i]->next;
    Sym_tab[i]->next = r;
    p4_unlock(&(Glob->sym_tab_lock));
#else
    r->sym_num = new_sym_num();
    i = r->sym_num % SYM_TAB_SIZE;
    r->next = Sym_tab[i];
    Sym_tab[i] = r;
#endif
	*sts = r->sym_num;
    return(NO_TROUBLE);
}  /* str_to_sn */

/*************
 *
 *    void mark_evaluable_symbols(reading) 
 *	Go through symbol table, checking $ Symbols.
 *
 *	Penguin adds the parameter reading.
 *
 *************/

void mark_evaluable_symbols(reading)
int reading;
/* reading == 1 if mark_evaluable_symbols() is called by read_all_input(), */
/* reading == 0 if mark_evaluable_symbols() is called by move_messages():  */
/* the call to mark_evaluable_symbols() in read_all_input() is executed by */
/* the Penguins which reads the input, while the call to mark_evaluable_   */
/* symbols() in move_messages() is executed by the Penguins which receive  */
/* the input via messages.						   */
{
    struct sym_ent *p;
    int i, error;
    char *n;

    for (i = 0; i < SYM_TAB_SIZE; i++) {
	p = Sym_tab[i];
#ifdef ROO
p = p->next;  /* skip dummy node */
#endif	
	while (p != NULL) {
	    n = p->name;
	    if (n[0] == '$' && p->sym_num != Cons_sym_num &&
		               p->sym_num != Nil_sym_num &&
			       p->sym_num != Eq_sym_num &&
			       p->sym_num != Ignore_sym_num &&
			       p->sym_num != Conditional_demodulator_sym_num &&
			       p->sym_num != Chr_sym_num &&
			       initial_str("$ANS", n) == 0 &&
			       initial_str("$Ans", n) == 0 &&
			       initial_str("$ans", n) == 0 &&
                               str_ident(n, "$NUCLEUS") == 0 &&
                               str_ident(n, "$BOTH") == 0 &&
                               str_ident(n, "$LINK") == 0 &&
                               str_ident(n, "$SATELLITE") == 0) {
		error = 0;
		if (str_ident(n, "$SUM"))
		    p->eval_code = SUM_SYM;
		else if (str_ident(n, "$PROD"))
		    p->eval_code = PROD_SYM;
		else if (str_ident(n, "$DIFF"))
		    p->eval_code = DIFF_SYM;
		else if (str_ident(n, "$DIV"))
		    p->eval_code = DIV_SYM;
		else if (str_ident(n, "$MOD"))
		    p->eval_code = MOD_SYM;
		else if (str_ident(n, "$EQ"))
		    p->eval_code = EQ_SYM;
		else if (str_ident(n, "$NE"))
		    p->eval_code = NE_SYM;
		else if (str_ident(n, "$LT"))
		    p->eval_code = LT_SYM;
		else if (str_ident(n, "$LE"))
		    p->eval_code = LE_SYM;
		else if (str_ident(n, "$GT"))
		    p->eval_code = GT_SYM;
		else if (str_ident(n, "$GE"))
		    p->eval_code = GE_SYM;
		else if (str_ident(n, "$AND"))
		    p->eval_code = AND_SYM;
		else if (str_ident(n, "$OR"))
		    p->eval_code = OR_SYM;
	        else if (str_ident(n, "$NOT"))
		    p->eval_code = NOT_SYM;
	        else if (str_ident(n, "$IF"))
		    p->eval_code = IF_SYM;
	        else if (str_ident(n, "$LLT"))
		    p->eval_code = LLT_SYM;
	        else if (str_ident(n, "$LLE"))
		    p->eval_code = LLE_SYM;
	        else if (str_ident(n, "$LGT"))
		    p->eval_code = LGT_SYM;
	        else if (str_ident(n, "$LGE"))
		    p->eval_code = LGE_SYM;
	        else if (str_ident(n, "$ID"))
		    p->eval_code = ID_SYM;
	        else if (str_ident(n, "$LNE"))
		    p->eval_code = LNE_SYM;
	        else if (str_ident(n, "$T"))
		    p->eval_code = T_SYM;
	        else if (str_ident(n, "$F"))
		    p->eval_code = F_SYM;
	        else if (str_ident(n, "$NEXT_CL_NUM"))
		    p->eval_code = NEXT_CL_NUM_SYM;
	        else if (str_ident(n, "$ATOMIC"))
		    p->eval_code = ATOMIC_SYM;
	        else if (str_ident(n, "$NUMBER"))
		    p->eval_code = NUMBER_SYM;
	        else if (str_ident(n, "$VAR"))
		    p->eval_code = VAR_SYM;
	        else if (str_ident(n, "$GROUND"))
		    p->eval_code = GROUND_SYM;
	        else if (str_ident(n, "$TRUE"))
		    p->eval_code = TRUE_SYM;
	        else if (str_ident(n, "$OUT"))
		    p->eval_code = OUT_SYM;
	        else if (str_ident(n, "$BIT_AND"))
		    p->eval_code = BIT_AND_SYM;
	        else if (str_ident(n, "$BIT_OR"))
		    p->eval_code = BIT_OR_SYM;
	        else if (str_ident(n, "$BIT_XOR"))
		    p->eval_code = BIT_XOR_SYM;
	        else if (str_ident(n, "$BIT_NOT"))
		    p->eval_code = BIT_NOT_SYM;
	        else if (str_ident(n, "$SHIFT_LEFT"))
		    p->eval_code = SHIFT_LEFT_SYM;
	        else if (str_ident(n, "$SHIFT_RIGHT"))
		    p->eval_code = SHIFT_RIGHT_SYM;
		else if (!p->skolem) {
			if (reading)
			{
		    Stats[INPUT_ERRORS]++;
fprintf(Fdout,"ERROR, unrecognized $ symbol: %s, arity %d, somewhere in the input.\n",n,p->arity);
			}
			else /* receiving from Penguin which read the input */
				p->skolem = 1;
		    }
		
		switch (p->eval_code) {
		    case SUM_SYM:
		    case PROD_SYM:
		    case DIFF_SYM:
		    case DIV_SYM:
		    case MOD_SYM:
		    case EQ_SYM:
		    case NE_SYM:
		    case LT_SYM:
		    case LE_SYM:
		    case GT_SYM:
		    case GE_SYM:
		    case AND_SYM:
		    case LLT_SYM:
		    case LLE_SYM:
		    case LGT_SYM:
		    case LGE_SYM:
		    case ID_SYM:
		    case LNE_SYM:
		    case BIT_AND_SYM:
		    case BIT_OR_SYM:
		    case BIT_XOR_SYM:
		    case SHIFT_LEFT_SYM:
		    case SHIFT_RIGHT_SYM:
		    case OR_SYM : error = (p->arity != 2); break;
		    case TRUE_SYM :
		    case BIT_NOT_SYM:
		    case NOT_SYM : error = (p->arity != 1); break; 
		    case IF_SYM : error = (p->arity != 3); break; 
		    case T_SYM :
		    case F_SYM :
		    case NEXT_CL_NUM_SYM : error = (p->arity != 0); break; 
		    case ATOMIC_SYM :
		    case NUMBER_SYM :
		    case VAR_SYM : error = (p->arity != 1); break; 
		    case GROUND_SYM : error = (p->arity != 1); break; 
		    case OUT_SYM : error = (p->arity != 1); break; 
		    }

		if (error) {
		    Stats[INPUT_ERRORS]++;
fprintf(Fdout,"ERROR, $ symbol: %s, has wrong arity: %d, somewhere in the input.\n", n, p->arity);
		    }
		else if (p->eval_code != 0)
		    Internal_flags[DOLLAR_PRESENT] = 1;
		}
	    p = p->next;
	    }
	}
}  /* mark_evaluable_symbols */

/*************
 *
 *    print_syms(file_ptr) -- Display the symbol list.
 *
 *************/

void print_syms(fp)
FILE *fp;
{
    struct sym_ent *p;
    int i;

    for (i = 0; i < SYM_TAB_SIZE; i++) {
	p = Sym_tab[i];
#ifdef ROO
p = p->next;  /* skip dummy node */
#endif	
	while (p != NULL) {
	    fprintf(fp, "%d  %s/%d, lex_val=%d\n", p->sym_num, p->name, p->arity, p->lex_val);
	    p = p->next;
	    }
	}
}  /* print_syms */

/*************
 *
 *    p_syms()
 *
 *************/

void p_syms()
{
    print_syms(Fdout);
}  /* p_syms */

/*************
 *
 *    free_sym_tab() -- free all symbols in the symbol table
 *
 *    Note for ROO:  dummy nodes are freed as well.
 *
 *************/

void free_sym_tab()
{
    struct sym_ent *p1, *p2;
    int i;

    for (i = 0; i < SYM_TAB_SIZE; i++) {
	p1 = Sym_tab[i];
	while (p1 != NULL) {
	    p2 = p1;
	    p1 = p1->next;
	    free_sym_ent(p2);
	    }
	Sym_tab[i] = NULL;
	}
}  /* free_sym_tab */

/*************
 *
 *    char *sn_to_str(sym_num)  --  given a symbol number, return the name
 *
 *************/

char *sn_to_str(sym_num)
int sym_num;
{
    struct sym_ent *p;

    p = Sym_tab[sym_num % SYM_TAB_SIZE];
#ifdef ROO
p = p->next;  /* skip dummy node */
#endif	
    while (p != NULL && p->sym_num != sym_num)
	p = p->next;
    if (p == NULL)
	return("");
    else
	return(p->name);
}  /* sn_to_str */

/*************
 *
 *    int sn_to_arity(sym_num)  --  given a symbol number, return the arity
 *
 *************/

int sn_to_arity(sym_num)
int sym_num;
{
    struct sym_ent *p;

    p = Sym_tab[sym_num % SYM_TAB_SIZE];
#ifdef ROO
p = p->next;  /* skip dummy node */
#endif	
    while (p != NULL && p->sym_num != sym_num)
	p = p->next;
    if (p == NULL)
	return(-1);
    else
	return(p->arity);
}  /* sn_to_arity */

/*************
 *
 *    int sn_to_ec(sym_num) - given a symbol number, return the evaluation code
 *
 *************/

int sn_to_ec(sym_num)
int sym_num;
{
    struct sym_ent *p;

    p = Sym_tab[sym_num % SYM_TAB_SIZE];
#ifdef ROO
p = p->next;  /* skip dummy node */
#endif	
    while (p != NULL && p->sym_num != sym_num)
	p = p->next;
    if (p == NULL)
	return(-1);
    else
	return(p->eval_code);
}  /* sn_to_ec */

/*************
 *
 *    int set_to_predicate(sym_num)		Penguin
 *
 *	Given a symbol number, turns its predicate field in Sym_tab on
 *
 *************/

int set_to_predicate(sym_num)
int sym_num;
{
    struct sym_ent *p;

    p = Sym_tab[sym_num % SYM_TAB_SIZE];
#ifdef ROO
p = p->next;  /* skip dummy node */
#endif	
    while (p != NULL && p->sym_num != sym_num)
	p = p->next;
    if (p == NULL)
	return(-1);
    else
	{
	p->predicate = 1;
	return(1);
	}
}  /* set_to_predicate */

/*************
 *
 *    int sn_to_node(sym_num) 
 *	Given a symbol number, return the symbol table node
 *
 *************/

struct sym_ent *sn_to_node(sym_num)
int sym_num;
{
    struct sym_ent *p;

    p = Sym_tab[sym_num % SYM_TAB_SIZE];
#ifdef ROO
p = p->next;  /* skip dummy node */
#endif	
    while (p != NULL && p->sym_num != sym_num)
	p = p->next;
    return(p);  /* possibly NULL */
}  /* sn_to_node */

/*************
 *
 *    int in_sym_tab(s)  --  is s in the symbol table?
 *
 *************/

int in_sym_tab(s)
char *s;
{
    struct sym_ent *p;
    int i;

    for (i = 0; i < SYM_TAB_SIZE; i++) {
	p = Sym_tab[i];
#ifdef ROO
p = p->next;  /* skip dummy node */
#endif	
	while (p != NULL) {
	    if (str_ident(p->name, s))
		return(1);
	    p = p->next;
	    }
	}
    return(0);
}  /* in_sym_tab */

/*************
 *
 *    int mark_as_skolem(sym_num)
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int mark_as_skolem(sym_num)
int sym_num;
{
    struct sym_ent *se;

    se = sn_to_node(sym_num);

    if (se == NULL) {
	output_stats(Fdout, 4);
fprintf(Fderr, "ABEND, mark_as_skolem, no symbol for %d.\007\n",sym_num);
	fprintf(Fdout, "ABEND, mark_as_skolem, no symbol for %d.\n",sym_num);
	return(TROUBLE);
	}
    else
	se->skolem = 1;
return(NO_TROUBLE);
}  /* mark_as_skolem */

/*************
 *
 *    int is_skolem(sym_num,is)
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE and 0/1 through the
 *	parameter is.
 *
 *************/

int is_skolem(sym_num,is)
int sym_num;
int *is;
{
    struct sym_ent *se;

	*is = 0;				/* default */
    se = sn_to_node(sym_num);

    if (se == NULL) {
	output_stats(Fdout, 4);
	fprintf(Fderr, "ABEND, is_skolem, no symbol for %d.\007\n",sym_num);
	fprintf(Fdout, "ABEND, is_skolem, no symbol for %d.\n",sym_num);
	return(TROUBLE);
	}
    else
	{
	*is = se->skolem;
	return(NO_TROUBLE);
	}
}  /* is_skolem */

/*************
 *
 *    str_copy(s, t)  --  Copy a string.
 *
 *************/
    
void str_copy(s, t)
char *s;
char *t;
{
    while (*t++ = *s++);
}  /* str_copy */

/*************
 *
 *     int str_ident(s, t) --  Identity of strings
 *
 *************/

int str_ident(s, t)
char *s;
char *t;
{
    for ( ; *s == *t; s++, t++)
	if (*s == '\0') return(1);
    return(0);
}  /* str_ident */

/*************
 *
 *     int initial_str(s, t)  --  Is s an initial substring of t?
 *
 *************/

int initial_str(s, t)
char *s;
char *t;
{
    for ( ; *s == *t; s++, t++)
	if (*s == '\0') return(1);
    return(*s == '\0');
}  /* initial_str */
    
/*************
 *
 *     int read_buf(file_ptr, buffer)
 *
 *    Read characters into buffer until one of the following:
 *        1.  '.' is reached ('.' goes into the buffer)
 *        2.  EOF is reached:  buf[0] = '\0' (an error occurs if
 *                 any nonwhite space precedes EOF)
 *        3.  MAX_BUF - 2 characters have been read (an error occurs)
 *
 *    If error occurs, return(0), else return(1).
 *
 *************/

int read_buf(fp, buf)
FILE *fp;
char buf[];
{
    int c, i, j;
    
    i = 0;
    c = getc(fp);
    while (c != '.' && c != EOF && i < MAX_BUF - 2) {
	if (c == '\n' || c == '\t')
	    c = ' ';
	if (c == '%') {  /* comment--flush rest of line */
	    c = getc(fp);
	    while (c != '\n' && c != EOF)
		c = getc(fp);
	    c = ' ';
	    }
	buf[i++] = c;
	c = getc(fp);
	}
    if (c == '.') {
	buf[i++] = '.';
	buf[i] = '\0';
	return(1);
	}
    else if (c == EOF) {
	j = 0;
	buf[i] = '\0';
	skip_white(buf, &j);
	if (i != j) {
	    fprintf(Fdout, "ERROR, text after last period: %s\n", buf);
	    buf[0] = '\0';
	    return(0);
	    }
	else {
	    buf[0] = '\0';
	    return(1);
	    }
	}
    else {
	buf[i] = '\0';
fprintf(Fdout, "ERROR, input string more than %d characters, increase MAX_BUF : %s\n", MAX_BUF, buf);
	/* now flush and discard */
	c = getc(fp);
	while (c != EOF && c != '.')
	    c = getc(fp);
	return(0);
	}
}  /* read_buf */

/*************
 *
 *    skip_white(buffer, position)
 *
 *        Advance the buffer to the next nonwhite position.
 *
 *************/

void skip_white(buf, bufp)
char buf[];
int *bufp;
{
    char c;
    c = buf[*bufp];
    while (c == ' ' || c == '\t' || c == '\n')
	c = buf[++(*bufp)];
}  /* skip_white */

/*************
 *
 *    int is_delim(c)
 *
 *************/

int is_delim(c)
char c;
{
    return(c == '(' || c == ',' || c == ')' || c == '.'  || 
	   c == '|' ||
	   c == ' ' || c == '\t' ||
	   c == '[' || c == ']' ||
	   c == '\n');
}

/*************
 *
 *    get_word(buffer, position, word)
 *
 *************/

void get_word(buf, bufp, word)
char buf[];
int *bufp;
char word[];
{
    int i;
    char c;
    i = 0;
    skip_white(buf, bufp);
    c = buf[*bufp];
    while (i < MAX_NAME-1 && is_delim(c) == 0) {
	word[i++] = c;
	c = buf[++(*bufp)];
	}
    word[i] = '\0';
    if (is_delim(c))
	skip_white(buf, bufp);
    else {
	fprintf(Fdout, "ERROR, word too big, max is %d; ", MAX_NAME-1);
	word[0] = '\0';
	}
}  /* get_word */

/*************
 *
 *    int str_term(buffer, position, st) -- parse buffer and build term
 *
 *    If a syntax error is found, print message and return(NULL).
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter st.
 *
 *************/

int str_term(buf, bufp, st)
char buf[];
int *bufp;
struct term **st;
{
    struct term *t1, *t2, *t3;
    struct rel *r1, *r2;
    char word[MAX_NAME];
    int i, save_pos;
	int tempint;
	struct term *tempterm;
    
	*st = NULL;				/* default */
    get_word(buf, bufp, word);
    if (word[0] == '\0') {
	if (buf[*bufp] == '[') {  /* list notation */
	    (*bufp)++;  /* skip past '[' */
	    if (buf[(*bufp)] == ']') {
		if (get_term(&t1) == TROUBLE)
		return(TROUBLE);
		t1->type = NAME;
		t1->sym_num = Nil_sym_num;
		(*bufp)++;  /* skip "]" */
		skip_white(buf,bufp);
		*st = t1;
		return(NO_TROUBLE);
		}
	    else {
		if (str_term(buf, bufp, &t1) == TROUBLE)
			return(TROUBLE);
		if (t1 == NULL)
		    return(NO_TROUBLE);
		else {
		    if (buf[*bufp] == '|') {   /* [t1|t2] */
			(*bufp)++;  /* skip past '|' */
			skip_white(buf,bufp);
			if (str_term(buf, bufp, &t2) == TROUBLE)
				return(TROUBLE);
			if (t2 == NULL)
			    return(NO_TROUBLE);
			else if (buf[*bufp] != ']') {
			    fprintf(Fdout, "ERROR, bad list:\n");
			    print_error(Fdout, buf, *bufp);
			    return(NO_TROUBLE);
			    }
			else {
			    if (get_term(&t3) == TROUBLE)
				return(TROUBLE);
			    if (get_rel(&r1) == TROUBLE)
				return(TROUBLE);
			    if (get_rel(&r2) == TROUBLE)
				return(TROUBLE);
			    t3->farg = r1;
			    r1->narg = r2;
			    r1->argval = t1;
			    r2->argval = t2;
			    t3->sym_num = Cons_sym_num;
			    t3->type = COMPLEX;
			    (*bufp)++;  /* skip past ']' */
			    skip_white(buf, bufp);
				*st = t3;
			    return(NO_TROUBLE);
			    }
			}
		    else if (buf[*bufp] == ',' || buf[*bufp] == ']')
			{   /* [t1,t2,...,tn] */
			if (get_term(&t3) == TROUBLE)
				return(TROUBLE);
			t3->sym_num = Cons_sym_num;
			t3->type = COMPLEX;
			if (get_rel(&r1) == TROUBLE)
				return(TROUBLE);
			if (get_rel(&r2) == TROUBLE)
				return(TROUBLE);
			t3->farg = r1;
			r1->narg = r2;
			r1->argval = t1;
			if (get_term(&t2) == TROUBLE)
				return(TROUBLE);
			r2->argval = t2;
			while (buf[*bufp] == ',') { 
			    (*bufp)++;
			    if (str_term(buf, bufp, &t1) == TROUBLE)
				return(TROUBLE);
			    if (t1 == NULL)
				return(NO_TROUBLE);
			    else {
				t2->type = COMPLEX;
			        t2->sym_num = Cons_sym_num;
				if (get_rel(&r1) == TROUBLE)
					return(TROUBLE);
				if (get_rel(&r2) == TROUBLE)
					return(TROUBLE);
				t2->farg = r1;
				r1->narg = r2;
				r1->argval = t1;
				if (get_term(&tempterm) == TROUBLE)
					return(TROUBLE);
				r2->argval = tempterm;
				t2 = r2->argval;
				}
			    }
			if (buf[*bufp] != ']') {
			    fprintf(Fdout, "ERROR, bad list:\n");
			    print_error(Fdout, buf, *bufp);
			    return(NO_TROUBLE);
			    }
			else {
			    t2->type = NAME;
			    t2->sym_num = Nil_sym_num;
			    (*bufp)++;  /* skip past ']' */
			    skip_white(buf, bufp);
				*st = t3;
			    return(NO_TROUBLE);
			    }
			}
		    else {
			fprintf(Fdout, "ERROR, bad list:\n");
			print_error(Fdout, buf, *bufp);
			return(NO_TROUBLE);
			}
		    }
		}
	    }  /* list notation */
	else {
	    fprintf(Fdout, "ERROR, bad word:\n");
	    print_error(Fdout, buf, *bufp);
	    return(NO_TROUBLE);
	    }
	}
    else {
	if (get_term(&t1) == TROUBLE)
		return(TROUBLE);
	if (buf[*bufp] != '(') {
	    t1->type = NAME;  /* decide later if variable */
	    if (str_to_sn(word, 0, &tempint) == TROUBLE)
		return(TROUBLE);
		t1->sym_num = tempint;
	    if (t1->sym_num == 0) {
	        fprintf(Fdout, "ERROR, multiple arities :%s:\n", word);
	        print_error(Fdout, buf, *bufp);
	        return(NO_TROUBLE);
	        }
	    else
		{
		*st = t1;
	        return(NO_TROUBLE);
		}
	    }
	else {
	    t1->type = COMPLEX;
	    r1 = NULL;
	    i = 0;  /* count subterms to get arity */
	    save_pos = *bufp;  /* in case of error */
	    while (buf[*bufp] != ')') {
		i++;
		(*bufp)++;    /* skip past comma or open paren */
		if (str_term(buf, bufp, &t2) == TROUBLE)
			return(TROUBLE);
		if (t2 == NULL)
		    return(NO_TROUBLE);
		else if (buf[*bufp] != ',' && buf[*bufp] != ')') {
		    fprintf(Fdout, "ERROR, comma or ) expected:\n");
		    print_error(Fdout, buf, *bufp);
		    return(NO_TROUBLE);
		    }
		else {
		    if (get_rel(&r2) == TROUBLE)
			return(TROUBLE);
		    r2->argval = t2;
		    if (r1 == NULL)
			t1->farg = r2;
		    else
			r1->narg = r2;
		    r1 = r2;
		    }
		}
	    (*bufp)++;    /* skip past close paren */
	    skip_white(buf, bufp);
	    if (str_to_sn(word, i, &tempint) ==  TROUBLE)  /* functor */
		return(TROUBLE);
		t1->sym_num = tempint;
	    if (t1->sym_num == 0) {
		fprintf(Fdout, "ERROR, multiple arities :%s:\n", word);
		print_error(Fdout, buf, save_pos);
		return(NO_TROUBLE);
		}
	    else
		{
		*st = t1;
		return(NO_TROUBLE);
		}
	    }
	}
}  /* str_term */

/*************
 *
 *    int str_atom(buf, bufp, signp, sa)
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter sa.
 *
 *************/

int str_atom(buf, bufp, signp, sa)
char buf[];
int *bufp;
int *signp;
struct term **sa;
{
    struct term *t, *t1, *t2;
    struct rel *r1, *r2;
    char *s;
    int save_pos;
	struct term *temp;
    
	*sa = NULL;				/* default */
    skip_white(buf, bufp);

    if (buf[*bufp] != '(') {
	*signp = 1;
	if (str_term(buf, bufp, &temp) == TROUBLE)
		return(TROUBLE);
	*sa = temp;
	return(NO_TROUBLE);
	}
    else {
	(*bufp)++;    /* skip past open paren */
	save_pos = *bufp;  /* in case of error */
	if (str_term(buf, bufp, &t1) == TROUBLE)
		return(TROUBLE);
	if (t1 == NULL)
	    return(NO_TROUBLE);  /* an error has already been handled */
	else {
            skip_white(buf, bufp);
	    if (buf[*bufp] == ')') {
		fprintf(Fdout, "ERROR, '=' or '!=' expected:\n");
		print_error(Fdout, buf, *bufp);
		return(NO_TROUBLE);
		}
	    else {
		if (str_term(buf, bufp, &t2) == TROUBLE)
			return(TROUBLE);
		if (t2 == NULL)
		    return(NO_TROUBLE);
		else {
		    s = sn_to_str(t2->sym_num);
		    *signp = -1;
		    if (str_ident(s, "="))
			*signp = 1;
		    else if (str_ident(s, "!="))
			*signp = 0;
		    
		    if (*signp == -1 || t2->type != NAME) {
			fprintf(Fdout, "ERROR, '=' or '!=' expected:\n");
			print_error(Fdout, buf, *bufp);
			return(NO_TROUBLE);
			}
		    else {
			free_term(t2);
			if (buf[*bufp] == ')') {
			    fprintf(Fdout, "ERROR, bad equality:\n");
			    print_error(Fdout, buf, save_pos);
			    return(NO_TROUBLE);
			    }
			else {
			    if (str_term(buf, bufp, &t2) == TROUBLE)
				return(TROUBLE);
			    if (t2 == NULL)
				return(NO_TROUBLE);
			    else if (buf[*bufp] != ')') {
				fprintf(Fdout, "ERROR, bad equality:\n");
				print_error(Fdout, buf, save_pos);
				return(NO_TROUBLE);
				}
			    else {
				(*bufp)++;  /* skip past ')' */
				skip_white(buf, bufp);
				if (get_term(&t) == TROUBLE)
					return(TROUBLE);
				t->type = COMPLEX;
				t->sym_num = Eq_sym_num;
				if (get_rel(&r1) == TROUBLE)
					return(TROUBLE);
				if (get_rel(&r2) == TROUBLE)
					return(TROUBLE);
				t->farg = r1;
				r1->narg = r2;
				r1->argval = t1;
				r2->argval = t2;
				*sa = t;
				return(NO_TROUBLE);
				}
			    }
			}
		    }
		}
	    }
	}

}  /* str_atom */

/*************
 *
 *    int set_vars(term)
 *
 *        Decide which of the names are really variables, and make
 *    into variables.  (This routine is used only on input terms.)
 *    Preserve the user's variable names by keeping the pointer into
 *    the symbol list.
 *
 *    If too many variabls, return(0); else return(1).
 *
 *************/

int set_vars(t)
struct term *t;
{
    char *varnames[MAX_VARS];
    int i;
    
    for (i=0; i<MAX_VARS; i++)
	varnames[i] = NULL;
    
    return(set_vars_term(t, varnames));
}  /* set_vars */

/*************
 *
 *     int set_vars_term(term, varnames)
 *
 *************/

int set_vars_term(t, varnames)
struct term *t;
char *varnames[];
{
    struct rel *r;
    int i, rc;
    
    if (t->type == COMPLEX) {
	r = t->farg;
	rc = 1;
	while (rc && r != NULL) {
	    rc = set_vars_term(r->argval, varnames);
	    r = r->narg;
	    }
	return(rc);
	}
    else if (var_name(sn_to_str(t->sym_num)) == 0)
	return(1);
    else {
	i = 0;
	t->type = VARIABLE;
	while (i < MAX_VARS && varnames[i] != NULL &&
	       varnames[i] != sn_to_str(t->sym_num))
	    i++;
	if (i == MAX_VARS)
	    return(0);
	else {
	    if (varnames[i] == NULL)
		varnames[i] = sn_to_str(t->sym_num);
	    t->varnum = i;
	    return(1);
/* t->sym_num = 0;  include this to destroy input variable names */
	    }
	}
}  /* set_vars_term */

/*************
 *
 *    int var_name(string) -- Decide if a string represents a variable.
 *
 *        return("string is a variable")
 *
 *	Modified by Penguin to recognize that the special strings
 *	HALT, TROUBLE and E_STOP are not variables in
 *	Prolog style.
 *
 *************/

int var_name(s)
char *s;
{
    if (Flags[PROLOG_STYLE_VARIABLES].val)
	{
	if ((*s >= 'A' && *s <= 'Z') || *s == '_')
		if (str_ident(s,"HALT"))
			return(0);
		else if (str_ident(s,"TROUBLE"))
			return(0);
		else if (str_ident(s,"E_STOP"))
			return(0);
		else return(1);
	else return(0);
	}
    else
        return(*s >= 'u' && *s <= 'z');
}  /* var_name() */

/*************
 *
 *    int read_term(file_ptr, retcd_ptr, rt) -- Read and return then next term.
 *
 *        It is assumed that the next term in the file is terminated
 *    with a period.   NULL is returned if EOF is reached first.
 *
 *    If an error is found, return(0); else return(1).
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter rt.
 *
 *************/

int read_term(fp, rcp, rt)
FILE *fp;
int *rcp;
struct term **rt;
{
    char buf[MAX_BUF];
    int p, rc;
    struct term *t;
    
	*rt = NULL;				/* default */

    rc = read_buf(fp, buf);
    if (rc == 0) {  /* error */
	*rcp = 0;
	*rt = NULL;
	return(NO_TROUBLE);
	}
    else if (buf[0] == '\0') {  /* ok. EOF */
	*rcp = 1;
	*rt = NULL;
	return(NO_TROUBLE);
	}
    else {
	p = 0;
	if (str_term(buf, &p, &t) == TROUBLE)
		return(TROUBLE);
	if (t == NULL) {
	    *rcp = 0;
		*rt = NULL;
	    return(NO_TROUBLE);
	    }
	else {
	    skip_white(buf, &p);
	    if (buf[p] != '.') {
		fprintf(Fdout, "ERROR, text after term:\n");
		print_error(Fdout, buf, p);
		*rcp = 0;
		*rt = NULL;
		return(NO_TROUBLE);
		}
	    else {
		if (set_vars(t)) {
		    *rcp = 1;
			*rt = t;
		    return(NO_TROUBLE);
		    }
		else {
		   fprintf(Fdout, "ERROR, too many variables, max is %d:\n%s\n",
				       MAX_VARS, buf);
		    *rcp = 0;
			*rt = NULL;
		    return(NO_TROUBLE);
		    }
		}
		    
	    }
	}
}  /* read_term */

/*************
 *
 *    int read_list(file_ptr, errors_ptr, integrate, rl)
 *
 *    Read and return a list of terms.
 *
 *    The list must be terminated either with the term `end_of_list.'
 *    or with an actual EOF.
 *    Set errors_ptr to point to the number of errors found.
 *
 *	struct term_ptr * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term_ptr through the
 *	parameter rl.
 *
 *************/

int read_list(fp, ep, integrate, rl)
FILE *fp;
int *ep;
int integrate;
struct term_ptr **rl;
{
    struct term_ptr *p1, *p2, *p3;
    struct term *t;
    int rc;
	struct term *temp;

	*rl = NULL;				/* default */
    *ep = 0;
    p3 = NULL;
    p2 = NULL;
    if (read_term(fp, &rc, &t) == TROUBLE)
	return(TROUBLE);
    while (rc == 0) {
	(*ep)++;
	if (read_term(fp, &rc, &t) == TROUBLE)
		return(TROUBLE);
	}

    /* keep going until t == NULL || t is end marker */

    while (t != NULL && (t->type != NAME ||
			 str_ident(sn_to_str(t->sym_num), "end_of_list") == 0)) {
	if (integrate)
	{
	   if (integrate_term(t, &temp) == TROUBLE)
		return(TROUBLE);
		t = temp;
	}
	if (get_term_ptr(&p1) == TROUBLE)
		return(TROUBLE);
	p1->term = t;
	if (p2 == NULL)
	    p3 = p1;
	else
	    p2->next = p1;
	p2 = p1;
	if (read_term(fp, &rc, &t) == TROUBLE)
		return(TROUBLE);
	while (rc == 0) {
	    (*ep)++;
	    if (read_term(fp, &rc, &t) == TROUBLE)
		return(TROUBLE);
	    }
	}
    if (t == NULL)
	{
	*rl = p3;
	return(NO_TROUBLE);
	}
    else {
	zap_term(t);
	*rl = p3;
	return(NO_TROUBLE);
	}
}  /* read_list */

/*************
 *
 *    print_list(file_ptr, term_ptr) -- Print a list of terms.
 *
 *        The list is printed with periods after each term, and
 *    the list is terminated with `end_of_list.' so that it can
 *    be read with read_list.
 *
 *************/

void print_list(fp, p)
FILE *fp;
struct term_ptr *p;
{
    while (p != NULL) {
	print_term(fp, p->term); fprintf(fp, ".\n");
	p = p->next;
	}
    fprintf(fp, "end_of_list.\n");
}  /* print_list */

/*************
 *
 *    print_error(fp, buf, pos)
 *
 *************/

void print_error(fp, buf, pos)
FILE *fp;
char buf[];
int pos;
{
    int i;

    fprintf(fp, "%s\n", buf);
    for (i = 0; i < pos; i++)
	fprintf(fp, "-");
    fprintf(fp, "^\n");
}  /* print_error */

/*************
 *
 *    int is_symbol(t, str, arity) -- Does t have leading function symbol str with arity.
 *
 *************/

int is_symbol(t, str, arity)
struct term *t;
char *str;
int arity;
{

    return((t->type == COMPLEX || t->type == NAME) &&
	   sn_to_arity(t->sym_num) == arity &&
	   str_ident(sn_to_str(t->sym_num), str));
}  /* arity */

/*************
 *
 *    bird_print(fp, t)
 *
 *************/

void bird_print(fp, t)
FILE *fp;
struct term *t;
{
    struct rel *r;
    
    if (t == NULL)
	fprintf(fp, "(nil)");
    else if (is_symbol(t, "a", 2) == 0) {
	/* t is not of the form a(_,_), so print in prefix */
	if (t->type == NAME)            /* name */
	    fprintf(fp, "%s", sn_to_str(t->sym_num));
	else if (t->type == VARIABLE)               /* variable */
	    print_variable(fp, t);
	else {  /* complex */
	    fprintf(fp, "%s", sn_to_str(t->sym_num));
	    fprintf(fp, "(");
	    r = t->farg;
	    while(r != NULL) {
		bird_print(fp, r->argval);
		r = r->narg;
		if(r != NULL)
		    fprintf(fp, ",");
		}
	    fprintf(fp, ")");
	    }
	}
    else {  /* t has form a(_,_), so print in bird notation */
	if (is_symbol(t->farg->narg->argval, "a", 2)) {
	    bird_print(fp, t->farg->argval);
	    fprintf(fp, " (");
	    bird_print(fp, t->farg->narg->argval);
	    fprintf(fp, ")");
	    }
	else {
	    bird_print(fp, t->farg->argval);
	    fprintf(fp, " ");
	    bird_print(fp, t->farg->narg->argval);
	    }
	}
}  /* bird_print */

/*************
 *
 *    int str_int(string, int_ptr) -- Translate a string to an integer.
 *
 *        String has optional '+' or '-' as first character.
 *    Return(1) iff success.
 *
 *************/

int str_int(s, np)
char s[];
int *np;
{
    int i, sign, n;

    i = 0;
    sign = 1;
    if (s[0] == '+' || s[0] == '-') {
	if (s[0] == '-')
	    sign = -1;
	i = 1;
	}
    if (s[i] == '\0')
	return(0);
    else {
	n = 0;
	for( ; s[i] >= '0' && s[i] <= '9'; i++)
	    n = n * 10 + s[i] - '0';
	*np = n * sign;
	return(s[i] == '\0');
	}
}  /* str_int */

/*************
 *
 *    int str_long(string, long_ptr) -- Translate a string to a long.
 *
 *        String has optional '+' or '-' as first character.
 *    Return(1) iff success.
 *
 *************/

int str_long(s, np)
char s[];
long *np;
{
    int i, sign;
    long n;

    i = 0;
    sign = 1;
    if (s[0] == '+' || s[0] == '-') {
	if (s[0] == '-')
	    sign = -1;
	i = 1;
	}
    if (s[i] == '\0')
	return(0);
    else {
	n = 0;
	for( ; s[i] >= '0' && s[i] <= '9'; i++)
	    n = n * 10 + s[i] - '0';
	*np = n * sign;
	return(s[i] == '\0');
	}
    }  /* str_long */

/*************
 *
 *    int_str(int, str) -- translate an integer to a string
 *
 *************/

void int_str(i, s)
int i;
char s[];
{
    int j, sign;

    if ((sign = i) < 0)
	i = -i;
    
    j = 0;
    if (i == 0)
        s[j++] = '0';
    else {
	while (i > 0) {
            s[j++] = i % 10 + '0';
            i = i / 10;
	    }
        }
    if (sign < 0)
	s[j++] = '-';
    s[j] = '\0';
    reverse(s);
}  /* int_str */

/*************
 *
 *    long_str(int, str) -- translate a long to a string
 *
 *************/

void long_str(i, s)
long i;
char s[];
{
    int j;
    long sign;

    if ((sign = i) < 0)
	i = -i;
    
    j = 0;
    if (i == 0)
        s[j++] = '0';
    else {
	while (i > 0) {
            s[j++] = i % 10 + '0';
            i = i / 10;
	    }
        }
    if (sign < 0)
	s[j++] = '-';
    s[j] = '\0';
    reverse(s);
}  /* long_str */

/*************
 *
 *    reverse(s) -- reverse a string
 *
 *************/

void reverse(s)
char s[];
{
    int i, j;
    char temp;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
	temp = s[i];
	s[i] = s[j];
	s[j] = temp;
	}
}  /* reverse */

/*************
 *
 *    cat_str(s1, s2, s3, lim)
 *
 *	Penguin adds the parameter lim and modifies the function to check
 *	array boundaries.
 *
 *************/

void cat_str(s1,s2,s3,lim)
char *s1;
char *s2;
char *s3;
int lim;
{
    int i, j;

    for (i = 0, j = 0; i < lim - 1 && j < lim && s1[j] != '\0'; j++, i++)
        s3[i] = s1[j];

    for (j = 0; i < lim - 1 && j < lim && s2[j] != '\0'; j++, i++)
	s3[i] = s2[j];

    s3[i] = '\0';
}  /* cat_str */

/*****************
*
*	void check_beq_lex_val()		Penguin
*
*	It checks whether any predicate symbol in the symbol table has
*	been given a lex_value different from the default MAX_INT.
*	If that is the case, it sets the lex_val of buil-in equality
*	Eq_sym_num to the smallest lex_val among the predicates.
*	It is called by read_all_input() after having checked that
*	Eq_sym_num has not been given a lex_val different from MAX_INT
*	by the user.
*
********************/

void check_beq_lex_val()
{
struct sym_ent *p, *beqp;
int i, min_pred_lex_val;

min_pred_lex_val = MAX_INT;

for (i = 0; i < SYM_TAB_SIZE; i++)
	{
	p = Sym_tab[i];
	while (p != NULL)
	{
	if (p->predicate == 1 && p->lex_val < min_pred_lex_val)
			min_pred_lex_val = p->lex_val;
	p = p->next;
	} /* end of while */
	} /* end of for */

if (min_pred_lex_val != MAX_INT)
	{
	beqp = sn_to_node(Eq_sym_num);
	beqp->lex_val = min_pred_lex_val;
	for (i = 0; i < SYM_TAB_SIZE; i++)
		{
		p = Sym_tab[i];
		while (p != NULL)
		{
		if (p->predicate == 1 && p->lex_val < MAX_INT && p != beqp)
			p->lex_val++;
/* It increases because equality has been inserted at the bottom of the	*/
/* precedence among predicates.						*/
		p = p->next;
		} /* end of while */
		} /* end of for */
	} /* end of if */
} /* check_beq_lex_val */
