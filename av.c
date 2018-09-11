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
 *  av.c -- This file has routines for memory management.
 *
 */

#include "header.h"

/* Size of chunk allocated by malloc */

#ifdef TURBO_C
#define TP_ALLOC_SIZE 8180
#define ARG_TYPE size_t
#else
#ifdef THINK_C
#define TP_ALLOC_SIZE 8180
#define ARG_TYPE size_t
#else
#define TP_ALLOC_SIZE 32700
#define ARG_TYPE unsigned
#endif
#endif

static char *Alloc_block;    /* location returned by most recent malloc */
static char *Alloc_pos;      /* current position in block */

/*  a list of available nodes for each type of structure */

static struct term *term_avail;
static struct rel *rel_avail;
static struct sym_ent *sym_ent_avail;
static struct term_ptr *term_ptr_avail;
static struct formula_ptr_2 *formula_ptr_2_avail;
static struct fpa_tree *fpa_tree_avail;
static struct fpa_head *fpa_head_avail;
static struct context *context_avail;
static struct trail *trail_avail;
static struct imd_tree *imd_tree_avail;
static struct imd_pos *imd_pos_avail;
static struct is_tree *is_tree_avail;
static struct is_pos *is_pos_avail;
static struct fsub_pos *fsub_pos_avail;
static struct literal *literal_avail;
static struct clause *clause_avail;
static struct list *list_avail;
static struct clash_nd *clash_nd_avail;
static struct clause_ptr *clause_ptr_avail;
static struct int_ptr *int_ptr_avail;


static struct link_node *link_node_avail;
static struct ans_lit_node *ans_lit_node_avail;
static struct formula_box *formula_box_avail;
static struct formula *formula_avail;
static struct formula_ptr *formula_ptr_avail;

#ifdef ROO
static struct time_node *time_node_avail;
#endif

static int Malloc_calls;  /* number of calls to malloc */

/* # of gets, frees, and size of avail list for each type of structure */

static long term_gets, term_frees, term_avails;
static long rel_gets, rel_frees, rel_avails;
static long sym_ent_gets, sym_ent_frees, sym_ent_avails;
static long term_ptr_gets, term_ptr_frees, term_ptr_avails;
static long formula_ptr_2_gets, formula_ptr_2_frees, formula_ptr_2_avails;
static long fpa_tree_gets, fpa_tree_frees, fpa_tree_avails;
static long fpa_head_gets, fpa_head_frees, fpa_head_avails;
static long context_gets, context_frees, context_avails;
static long trail_gets, trail_frees, trail_avails;
static long imd_tree_gets, imd_tree_frees, imd_tree_avails;
static long imd_pos_gets, imd_pos_frees, imd_pos_avails;
static long is_tree_gets, is_tree_frees, is_tree_avails;
static long is_pos_gets, is_pos_frees, is_pos_avails;
static long fsub_pos_gets, fsub_pos_frees, fsub_pos_avails;
static long literal_gets, literal_frees, literal_avails;
static long clause_gets, clause_frees, clause_avails;
static long list_gets, list_frees, list_avails;
static long clash_nd_gets, clash_nd_frees, clash_nd_avails;
static long clause_ptr_gets, clause_ptr_frees, clause_ptr_avails;
static long int_ptr_gets, int_ptr_frees, int_ptr_avails;

static long link_node_gets, link_node_frees, link_node_avails;
static long ans_lit_node_gets, ans_lit_node_frees, ans_lit_node_avails;
static long formula_box_gets, formula_box_frees, formula_box_avails;
static long formula_gets, formula_frees, formula_avails;
static long formula_ptr_gets, formula_ptr_frees, formula_ptr_avails;

#ifdef ROO
static long time_node_gets, time_node_frees, time_node_avails;
#endif

/*************
 *
 *    int tp_alloc(n,rb)
 *
 *    Allocate n contiguous bytes, aligned on pointer boundry.
 *	*char in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *	In Penguin it returns a pointer to char through the parameter rb.
 *
 *************/

int tp_alloc(n,rb)
int n;
char **rb;
{
    int scale;
    
    /* if n is not a multiple of sizeof(int *), then round up so that it is */
    
    scale = sizeof(int *);
    if (n % scale != 0)
	n = n + (scale - (n % scale));
    
    if (Alloc_block == NULL || Alloc_block + TP_ALLOC_SIZE - Alloc_pos < n) {
        /* try to malloc a new block */
	if (n > TP_ALLOC_SIZE) {
	    output_stats(Fdout, 4);
	    fprintf(Fdout, "ABEND, tp_alloc, request too big: %d\n", n);
	    fprintf(Fderr, "ABEND, tp_alloc, request too big: %d\007\n", n);
	    return(TROUBLE);
	    }
	else if (Parms[MAX_MEM].val != 0 &&
		 ((Malloc_calls+1)*TP_ALLOC_SIZE)/1024 > Parms[MAX_MEM].val) {
	    fprintf(Fdout, "\nsearch stopped in tp_alloc by max_mem option.\n");
	  fprintf(Fderr, "search stopped in tp_alloc by max_mem option.\007\n");
	    if (Flags[FREE_ALL_MEM].val) {
		/* freeing memory can require additional memory */
		fprintf(Fdout, "    (free_all_mem cleared).\n");
		Flags[FREE_ALL_MEM].val = 0;
		}
	    cleanup();
	    return(TROUBLE);
	    }
	else {

#ifdef ROO	    
Alloc_pos = Alloc_block = (char *) p4_shmalloc((ARG_TYPE) TP_ALLOC_SIZE);
            Glob->proc_data[Pid].mallocs++;
#else	    
	    Alloc_pos= Alloc_block = (char *) malloc((ARG_TYPE) TP_ALLOC_SIZE);
#endif	    
	    Malloc_calls++;
            Stats[K_MALLOCED] = (Malloc_calls * (TP_ALLOC_SIZE / 1024.));
	    if (Alloc_pos == NULL) {
		output_stats(Fdout, 4);
	fprintf(Fdout, "\nABEND, malloc returns NULL (out of memory).\n");
	fprintf(Fderr, "ABEND, malloc returns NULL (out of memory).\007\n");
		return(TROUBLE);
		}
	    }
        }
    *rb = Alloc_pos;
    Alloc_pos += n;
    return(NO_TROUBLE);
}  /* tp_alloc */

/*************
*
*   int get_term(p1)
*
*	struct term * in Otter, int in Penguin as it returns TROUBLE/NO_TROUBLE.
*	It returns the pointer to struct term through the parameter p1.
*
**************/

int get_term(p1)
struct term **p1;
{
    struct term *p;
	char *c;
    
    term_gets++;
    if (term_avail == NULL)
	{
	if (tp_alloc(sizeof(struct term),&c) == TROUBLE)
		return(TROUBLE);
	else p = (struct term *) c;
	}
    else {
	term_avails--;
	p = term_avail;
	term_avail = (struct term *) term_avail->farg;
	}
    p->sym_num = 0;
    p->farg = NULL;
    p->occ.rel = NULL;
    p->varnum = 0;
    p->bits = 0;
    p->fpa_id = 0;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_term */

/*************
 *
 *    free_term()
 *
 *************/

void free_term(p)
struct term *p;
{
    term_frees++;
    term_avails++;
    p->farg = (struct rel *) term_avail;
    term_avail = p;
}  /* free_term */

/*************
*
*    int get_rel(p1)
*
*	struct rel * in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
*	It returns the pointer to struct rel through the parameter p1.
*
**************/

int get_rel(p1)
struct rel **p1;
{
    struct rel *p;
	char *c;
    
    rel_gets++;
    if (rel_avail == NULL)
	if (tp_alloc(sizeof(struct rel), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct rel *) c;
    else {
	rel_avails--;
	p = rel_avail;
	rel_avail = rel_avail->narg;
	}
    p->argval = NULL;
    p->argof = NULL;
    p->narg = NULL;
    p->nocc = NULL;
    p->path = 0;
    p->clashable = 0;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_rel */

/*************
 *
 *    free_rel()
 *
 *************/

void free_rel(p)
struct rel *p;
{
    rel_frees++;
    rel_avails++;
    p->narg = rel_avail;
    rel_avail = p;
}  /* free_rel */

/*************
*
*    int get_sym_ent(p1)
*
*	struct sym_ent * in Otter, int in Penguin, as it returns TROUBLE/
*	NO_TROUBLE. It returns the pointer to struct sym_ent through the
*	parameter p1.
*
**************/

int get_sym_ent(p1)
struct sym_ent **p1;
{
    struct sym_ent *p;
	char *c;
    
    sym_ent_gets++;
    if (sym_ent_avail == NULL)
	if (tp_alloc(sizeof(struct sym_ent), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct sym_ent *) c;
    else {
	sym_ent_avails--;
	p = sym_ent_avail;
	sym_ent_avail = sym_ent_avail->next;
	}
    p->eval_code = 0;
    p->lex_val = MAX_INT;
    p->skolem = 0;
    p->special_unary = 0;
    p->lex_rpo_status = LRPO_MULTISET_STATUS;
    p->predicate = 0;	/* Penguin: default is function */
    p->next = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_sym_ent */

/*************
 *
 *    free_sym_ent()
 *
 *************/

void free_sym_ent(p)
struct sym_ent *p;
{
    sym_ent_frees++;
    sym_ent_avails++;
    p->next = sym_ent_avail;
    sym_ent_avail = p;
}  /* free_sym_ent */

/*************
 *
 *    int get_term_ptr(p1)
 *
 *	struct term_ptr * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term_ptr through the
 *	parameter p1.
 *
 *************/

int get_term_ptr(p1)
struct term_ptr **p1;
{
    struct term_ptr *p;
	char *c;
    
    term_ptr_gets++;
    if (term_ptr_avail == NULL)
	if (tp_alloc(sizeof(struct term_ptr), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct term_ptr *) c;
    else {
	term_ptr_avails--;
	p = term_ptr_avail;
	term_ptr_avail = term_ptr_avail->next;
	}
    p->term = NULL;
    p->next = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_term_ptr */

/*************
 *
 *    free_term_ptr()
 *
 *************/

void free_term_ptr(p)
struct term_ptr *p;
{
    term_ptr_frees++;
    term_ptr_avails++;
    p->next = term_ptr_avail;
    term_ptr_avail = p;
}  /* free_term_ptr */

/*************
 *
 *    int get_formula_ptr_2(p1)
 *
 *	struct formula_ptr_2 * in Otter, int in Penguin, as it returns
 *	TROUBLE/NO_TROUBLE. It returns the pointer to struct formula_ptr_2
 *	through the parameter p1.
 *
 *************/

int get_formula_ptr_2(p1)
struct formula_ptr_2 **p1;
{
    struct formula_ptr_2 *p;
	char *c;
    
    formula_ptr_2_gets++;
    if (formula_ptr_2_avail == NULL)
	if (tp_alloc(sizeof(struct formula_ptr_2), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct formula_ptr_2 *) c;
    else {
	formula_ptr_2_avails--;
	p = formula_ptr_2_avail;
	formula_ptr_2_avail = formula_ptr_2_avail->next;
	}
    p->f = NULL;
    p->next = NULL;
    p->prev = NULL;
    p->left = NULL;
    p->right = NULL;
    p->up = NULL;
    p->down = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_formula_ptr_2 */

/*************
 *
 *    free_formula_ptr_2()
 *
 *************/

void free_formula_ptr_2(p)
struct formula_ptr_2 *p;
{
    formula_ptr_2_frees++;
    formula_ptr_2_avails++;
    p->next = formula_ptr_2_avail;
    formula_ptr_2_avail = p;
}  /* free_formula_ptr_2 */

/*************
 *
 *    int get_fpa_tree(p1)
 *
 *	struct fpa_tree * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct fpa_tree through the
 *	parameter p1.
 *
 *************/

int get_fpa_tree(p1)
struct fpa_tree **p1;
{
    struct fpa_tree *p;
	char *c;
    
    fpa_tree_gets++;
    if (fpa_tree_avail == NULL)
	if (tp_alloc(sizeof(struct fpa_tree), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct fpa_tree *) c;
    else {
	fpa_tree_avails--;
	p = fpa_tree_avail;
	fpa_tree_avail = fpa_tree_avail->left;
	}
    p->terms = NULL;
    p->left = NULL;
    p->right = NULL;
    p->left_term = NULL;
    p->right_term = NULL;
    p->path = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_fpa_tree */

/*************
 *
 *    free_fpa_tree()
 *
 *************/

void free_fpa_tree(p)
struct fpa_tree *p;
{
    fpa_tree_frees++;
    fpa_tree_avails++;
    p->left = fpa_tree_avail;
    fpa_tree_avail = p;
}  /* free_fpa_tree */

/*************
 *
 *    int get_fpa_head(p1)
 *
 *	struct fpa_head * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to fpa_head through the parameter p1.
 *
 *************/

int get_fpa_head(p1)
struct fpa_head **p1;
{
    struct fpa_head *p;
	char *c;
    
    fpa_head_gets++;
    if (fpa_head_avail == NULL)
	if (tp_alloc(sizeof(struct fpa_head), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct fpa_head *) c;
    else {
	fpa_head_avails--;
	p = fpa_head_avail;
	fpa_head_avail = fpa_head_avail->next;
	}
    p->terms = NULL;
    p->next = NULL;
    p->path = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_fpa_head */

/*************
 *
 *    free_fpa_head()
 *
 *************/

void free_fpa_head(p)
struct fpa_head *p;
{
    fpa_head_frees++;
    fpa_head_avails++;
    p->next = fpa_head_avail;
    fpa_head_avail = p;
}  /* free_head */

/*************
 *
 *    int get_context(p1)
 *
 *	struct context * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct context through the
 *	parameter p1.
 *
 *************/

int get_context(p1)
struct context **p1;
{
    struct context *p;
	char *c;
    int i;
    static int count=0;
    
    context_gets++;
    if (context_avail == NULL) {
	if (tp_alloc(sizeof(struct context), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct context *) c;
	for (i=0; i<MAX_VARS; i++) {
	    p->terms[i] = NULL;
	    p->status[i] = 0;
	    }
        p->built_in_multiplier = count++;  /* never change */
	}
    else {
	context_avails--;
	p = context_avail;
	context_avail = context_avail->contexts[0];
	}
    p->multiplier = -1;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_context */

/*************
 *
 *    free_context()
 *
 *************/

void free_context(p)
struct context *p;
{

    /*B
    int i;
    for (i=0; i<MAX_VARS; i++) {
	if (p->terms[i] != NULL) {
	    printf("ERROR, context %x, var &d not null.\n",p->contexts[i], i);
	    print_term_nl(Fdout, p->terms[i]);
	    p->terms[i] = NULL;
	    }
	}
    E*/
    context_frees++;
    context_avails++;
    p->contexts[0] = context_avail;
    context_avail = p;
}  /* free_context */

/*************
 *
 *    int get_trail(p1)
 *
 *	struct trail * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct trail through the
 *	parameter p1.
 *
 *************/

int get_trail(p1)
struct trail **p1;
{
    struct trail *p;
	char *c;
    
    trail_gets++;
    if (trail_avail == NULL)
	if (tp_alloc(sizeof(struct trail), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct trail *) c;
    else {
	trail_avails--;
	p = trail_avail;
	trail_avail = trail_avail->next;
	}
    p->next = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_trail */

/*************
 *
 *    free_trail()
 *
 *************/

void free_trail(p)
struct trail *p;
{
    trail_frees++;
    trail_avails++;
    p->next = trail_avail;
    trail_avail = p;
}  /* free_trail */

/*************
 *
 *    int get_imd_tree(p1)
 *
 *	struct imd_tree * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct imd_tree through the
 *	parameter p1.
 *
 *************/

int get_imd_tree(p1)
struct imd_tree **p1;
{
    struct imd_tree *p;
	char *c;
    
    imd_tree_gets++;
    if (imd_tree_avail == NULL)
	if (tp_alloc(sizeof(struct imd_tree), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct imd_tree *) c;
    else {
	imd_tree_avails--;
	p = imd_tree_avail;
	imd_tree_avail = imd_tree_avail->next;
	}
    p->next = NULL;
    p->kids = NULL;
    p->type = 0;
    p->lab = 0;
    p->atoms = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_imd_tree */

/*************
 *
 *    free_imd_tree()
 *
 *************/

void free_imd_tree(p)
struct imd_tree *p;
{
    imd_tree_frees++;
    imd_tree_avails++;
    p->next = imd_tree_avail;
    imd_tree_avail = p;
}  /* free_imd_tree */

/*************
 *
 *    int get_imd_pos(p1)
 *
 *	struct imd_pos * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct imd_pos through the
 *	parameter p1.
 *
 *************/

int get_imd_pos(p1)
struct imd_pos **p1;
{
    struct imd_pos *p;
	char *c;
    
    imd_pos_gets++;
    if (imd_pos_avail == NULL)
	if (tp_alloc(sizeof(struct imd_pos), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct imd_pos *) c;
    else {
	imd_pos_avails--;
	p = imd_pos_avail;
	imd_pos_avail = imd_pos_avail->next;
	}
    p->next = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_imd_pos */

/*************
 *
 *    free_imd_pos()
 *
 *************/

void free_imd_pos(p)
struct imd_pos *p;
{
    imd_pos_frees++;
    imd_pos_avails++;
    p->next = imd_pos_avail;
    imd_pos_avail = p;
}  /* free_imd_pos */

/*************
 *
 *    int get_is_tree(p1)
 *
 *	struct is_tree * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct is_tree through the
 *	parameter p1.
 *
 *************/

int get_is_tree(p1)
struct is_tree **p1;
{
    struct is_tree *p;
	char *c;
    
    is_tree_gets++;
    if (is_tree_avail == NULL)
	if (tp_alloc(sizeof(struct is_tree), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct is_tree *) c;
    else {
	is_tree_avails--;
	p = is_tree_avail;
	is_tree_avail = is_tree_avail->next;
	}
    p->next = NULL;
    p->type = 0;
    p->lab = 0;
    p->u.kids = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_is_tree */

/*************
 *
 *    free_is_tree()
 *
 *************/

void free_is_tree(p)
struct is_tree *p;
{
    is_tree_frees++;
    is_tree_avails++;
    p->next = is_tree_avail;
    is_tree_avail = p;
}  /* free_is_tree */

/*************
 *
 *    int get_is_pos(p1)
 *
 *	struct is_pos * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct is_pos through the
 *	parameter p1.
 *
 *************/

int get_is_pos(p1)
struct is_pos **p1;
{
    struct is_pos *p;
	char *c;

    is_pos_gets++;
    if (is_pos_avail == NULL)
	if (tp_alloc(sizeof(struct is_pos), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct is_pos *) c;
    else {
	is_pos_avails--;
	p = is_pos_avail;
	is_pos_avail = is_pos_avail->next;
	}
    p->next = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_is_pos */

/*************
 *
 *    free_is_pos()
 *
 *************/

void free_is_pos(p)
struct is_pos *p;
{
    is_pos_frees++;
    is_pos_avails++;
    p->next = is_pos_avail;
    is_pos_avail = p;
}  /* free_is_pos */

/*************
 *
 *    int get_fsub_pos(p1)
 *
 *	struct fsub_pos * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct fsub_pos through the
 *	parameter p1.
 *
 *************/

int get_fsub_pos(p1)
struct fsub_pos **p1;
{
    struct fsub_pos *p;
	char *c;
    
    fsub_pos_gets++;
    if (fsub_pos_avail == NULL)
	if (tp_alloc(sizeof(struct fsub_pos), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct fsub_pos *) c;
    else {
	fsub_pos_avails--;
	p = fsub_pos_avail;
	fsub_pos_avail = (struct fsub_pos *) fsub_pos_avail->terms;
	}
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_fsub_pos */

/*************
 *
 *    free_fsub_pos()
 *
 *************/

void free_fsub_pos(p)
struct fsub_pos *p;
{
    fsub_pos_frees++;
    fsub_pos_avails++;
    p->terms = (struct term_ptr *) fsub_pos_avail;
    fsub_pos_avail = p;
}  /* free_fsub_pos */

/*************
 *
 *    int get_literal(p1)
 *
 *	struct literal * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct literal through the
 *	parameter p1.
 *
 *************/

int get_literal(p1)
struct literal **p1;
{
    struct literal *p;
	char *c;
    
    literal_gets++;
    if (literal_avail == NULL)
	if (tp_alloc(sizeof(struct literal), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct literal *) c;
    else {
	literal_avails--;
	p = literal_avail;
	literal_avail = literal_avail->next_lit;
	}
    p->container = NULL;
    p->next_lit = NULL;
    p->sign = 0;
    p->atom = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_literal */

/*************
 *
 *    free_literal()
 *
 *************/

void free_literal(p)
struct literal *p;
{
    literal_frees++;
    literal_avails++;
    p->next_lit = literal_avail;
    literal_avail = p;
}  /* free_literal */

/*************
 *
 *    int get_clause(p1)
 *
 *	struct clause * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct clause through the
 *	parameter p1.
 *
 *************/

int get_clause(p1)
struct clause **p1;
{
    struct clause *p;
	char *c;
    
    clause_gets++;
    if (clause_avail == NULL)
	if (tp_alloc(sizeof(struct clause), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct clause *) c;
    else {
	clause_avails--;
	p = clause_avail;
	clause_avail = clause_avail->next_cl;
	}
    p->id = 0;
    p->lid = MAX_INT;
/* In Otter, the id of a clause is initialized to 0 and set to a number	*/
/* i, 0 < i < MAX_INT, when the clause is integrated (cl_integrate()).	*/
/* In Penguin, the lid of a clause is initialized to MAX_INT and set to a*/
/* number i, 0 < i < MAX_INT, when the clause is integrated.		*/
/* In this way a newly generated clause has lid infinity, implemented as */
/* MAX_INT, when compared to ``older'' clauses in distributed subsumption.*/
    p->pid = MAX_INT;
/* Penguin: the pid field of a clause is initialized to MAX_INT.	*/
/* Initialization to 0 would not be correct, since 0 is in general the	*/
/* id of a node.							*/
/* The pid field is set to a value i, 0 < i < No_of_nodes-1, when the	*/
/* clauses settles down at node i (decide_allocation()).		*/
    p->bt = MAX_INT;
/* Penguin: the birth time field of a clause is initialized to MAX_INT	*/
/* for the same reason as for lid.					*/
    p->dest = NONE;
/* As long as it is not used as a message, the clause does not have a	*/
/* destination.								*/
    p->parents = NULL;
    p->container = NULL;
    p->next_cl = NULL;
    p->prev_cl = NULL;
    p->first_lit = NULL;
    p->weight = 0;
    p->type = NOT_SPECIFIED;
#ifdef ROO
    p->giv_cl_seq_no = 0;
#endif
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_clause */

/*************
 *
 *    free_clause()
 *
 *************/

void free_clause(p)
struct clause *p;
{
    clause_frees++;
    clause_avails++;
    p->next_cl = clause_avail;
    clause_avail = p;
}  /* free_clause */

/*************
 *
 *    int get_list(p1)
 *
 *	struct list * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to atruct list through the
 *	parameter p1.
 *
 *************/

int get_list(p1)
struct list **p1;
{
    struct list *p;
	char *c;
    
    list_gets++;
    if (list_avail == NULL)
	if (tp_alloc(sizeof(struct list), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct list *) c;
    else {
	list_avails--;
	p = list_avail;
	list_avail = (struct list *) list_avail->first_cl;
	}
    p->first_cl = NULL;
    p->last_cl = NULL;
    p->name[0] = '\0';
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_list */

/*************
 *
 *    free_list()
 *
 *************/

void free_list(p)
struct list *p;
{
    list_frees++;
    list_avails++;
    p->first_cl = (struct clause *) list_avail;
    list_avail = p;
}  /* free_list */

/*************
 *
 *    int get_clash_nd(p1)
 *
 *	struct clash_nd * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct clash_nd through the
 *	parameter p1.
 *
 *************/

int get_clash_nd(p1)
struct clash_nd **p1;
{
    struct clash_nd *p;
	char *c;
    
    clash_nd_gets++;
    if (clash_nd_avail == NULL)
	if (tp_alloc(sizeof(struct clash_nd), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct clash_nd *) c;
    else {
	clash_nd_avails--;
	p = clash_nd_avail;
	clash_nd_avail = clash_nd_avail->next;
	}
    p->next = NULL;
    p->prev = NULL;
    p->evaluable = 0;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_clash_nd */

/*************
 *
 *    free_clash_nd()
 *
 *************/

void free_clash_nd(p)
struct clash_nd *p;
{
    clash_nd_frees++;
    clash_nd_avails++;
    p->next = clash_nd_avail;
    clash_nd_avail = p;
}  /* free_clash_nd */

/*************
 *
 *    int get_clause_ptr(p1)
 *
 *	struct clause_ptr * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct clause_ptr through the
 *	parameter p1.
 *
 *************/

int get_clause_ptr(p1)
struct clause_ptr **p1;
{
    struct clause_ptr *p;
	char *c;
    
    clause_ptr_gets++;
    if (clause_ptr_avail == NULL)
	if (tp_alloc(sizeof(struct clause_ptr), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct clause_ptr *) c;
    else {
	clause_ptr_avails--;
	p = clause_ptr_avail;
	clause_ptr_avail = clause_ptr_avail->next;
	}
    p->next = NULL;
    p->c = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_clause_ptr */

/*************
 *
 *    free_clause_ptr()
 *
 *************/

void free_clause_ptr(p)
struct clause_ptr *p;
{
    clause_ptr_frees++;
    clause_ptr_avails++;
    p->next = clause_ptr_avail;
    clause_ptr_avail = p;
}  /* free_clause_ptr */

/*************
 *
 *    int get_int_ptr(p1)
 *
 *	struct int_ptr * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct int_ptr through the
 *	parameter p1.
 *
 *************/

int get_int_ptr(p1)
struct int_ptr **p1;
{
    struct int_ptr *p;
	char *c;
    
    int_ptr_gets++;
    if (int_ptr_avail == NULL)
	if (tp_alloc(sizeof(struct int_ptr), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct int_ptr *) c;
    else {
	int_ptr_avails--;
	p = int_ptr_avail;
	int_ptr_avail = int_ptr_avail->next;
	}
    p->next = NULL;
    p->i = 0;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_int_ptr */

/*************
 *
 *    free_int_ptr()
 *
 *************/

void free_int_ptr(p)
struct int_ptr *p;
{
    int_ptr_frees++;
    int_ptr_avails++;
    p->next = int_ptr_avail;
    int_ptr_avail = p;
}  /* free_int_ptr */

/*************
 *
 *    int get_ans_lit_node(p1)
 *
 *	struct ans_lit_node * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct ans_lit_node through
 *	the parameter p1.
 *
 *************/

int get_ans_lit_node(p1)
struct ans_lit_node **p1;
{
    struct ans_lit_node *p;
    char *c;
    
    ans_lit_node_gets++;
    if (ans_lit_node_avail == NULL)
	if (tp_alloc(sizeof(struct ans_lit_node), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct ans_lit_node *) c;
    else {
	ans_lit_node_avails--;
	p = ans_lit_node_avail;
	ans_lit_node_avail = ans_lit_node_avail->next;
	}

    p->next = NULL;
    p->parent = NULL;
    p->lit = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_ans_lit_node */

/*************
 *
 *    void free_ans_lit_node()
 *
 *************/

void free_ans_lit_node(p)
struct ans_lit_node *p;
{
    ans_lit_node_frees++;
    ans_lit_node_avails++;
    p->next = ans_lit_node_avail;
    ans_lit_node_avail = p;
}  /* free_ans_lit_node */

/*************
 *
 *    int get_formula_box(p1)
 *
 *	struct formula_box * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula_box through the
 *	parameter p1.
 *
 *************/

int get_formula_box(p1)
struct formula_box **p1;
{
    struct formula_box *p;
    char *c;
    
    formula_box_gets++;
    if (formula_box_avail == NULL)
	if (tp_alloc(sizeof(struct formula_box), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct formula_box *) c;
    else {
	formula_box_avails--;
	p = formula_box_avail;
	formula_box_avail = formula_box_avail->next;
	}

    p->first_child = p->next = p->parent = NULL;
    p->f = NULL;
    p->str[0] = '\0';
    p->type = p->subtype = p->length = p->height = p->x_off = p->y_off = 0;
    p->abs_x_loc = p->abs_y_loc = 0;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_formula_box */

/*************
 *
 *    void free_formula_box()
 *
 *************/

void free_formula_box(p)
struct formula_box *p;
{
    formula_box_frees++;
    formula_box_avails++;
    p->next = formula_box_avail;
    formula_box_avail = p;
}  /* free_formula_box */

/*************
 *
 *    int get_formula(p1)
 *
 *	struct formula * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula through the
 *	parameter p1.
 *
 *************/

int get_formula(p1)
struct formula **p1;
{
    struct formula *p;
    char *c;
    
    formula_gets++;
    if (formula_avail == NULL)
	if (tp_alloc(sizeof(struct formula), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct formula *) c;
    else {
	formula_avails--;
	p = formula_avail;
	formula_avail = formula_avail->next;
	}

    p->type = 0;
    p->quant_type = 0;
    p->parent = p->first_child = p->next = NULL;
    p->t = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_formula */

/*************
 *
 *    void free_formula()
 *
 *************/

void free_formula(p)
struct formula *p;
{
    formula_frees++;
    formula_avails++;
    p->next = formula_avail;
    formula_avail = p;
}  /* free_formula */

/*************
 *
 *    int get_formula_ptr(p1)
 *
 *	struct formula_ptr * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct formula_ptr through the
 *	parameter p1.
 *
 *************/

int get_formula_ptr(p1)
struct formula_ptr **p1;
{
    struct formula_ptr *p;
    char *c;
    
    formula_ptr_gets++;
    if (formula_ptr_avail == NULL)
	if (tp_alloc(sizeof(struct formula_ptr), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct formula_ptr *) c;
    else {
	formula_ptr_avails--;
	p = formula_ptr_avail;
	formula_ptr_avail = formula_ptr_avail->next;
	}

    p->f = NULL;
    p->next = NULL;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_formula_ptr */

/*************
 *
 *    void free_formula_ptr()
 *
 *************/

void free_formula_ptr(p)
struct formula_ptr *p;
{
    formula_ptr_frees++;
    formula_ptr_avails++;
    p->next = formula_ptr_avail;
    formula_ptr_avail = p;
}  /* free_formula_ptr */

/*************
 *
 *    int get_link_node(p1)
 *
 *	struct link_node * in Otter, int in penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct link_node through the
 *	parameter p1.
 *
 *************/

int get_link_node(p1)
struct link_node **p1;
{
    struct link_node *p;
    char *c;
    
    link_node_gets++;
    if (link_node_avail == NULL)
	if (tp_alloc(sizeof(struct link_node), &c) == TROUBLE)
		return(TROUBLE);
	else p = (struct link_node *) c;
    else {
	link_node_avails--;
	p = link_node_avail;
	link_node_avail = link_node_avail->next_sibling;
	}

    p->parent = NULL;
    p->first_child = NULL;
    p->child_first_ans = NULL;
    p->child_last_ans = NULL;
    p->next_sibling = NULL;
    p->prev_sibling = NULL;
    p->first = TRUE;
    p->unit_deleted = FALSE;  /* Initially literal has not been unit deleted */
    p->goal = NULL;
    p->goal_to_resolve = NULL;
    p->current_clause = NULL;
    p->subst = NULL;
    p->unif_position = NULL;
    p->tr = NULL;
    p->near_poss_nuc = UNDEFINED;
    p->farthest_sat = 0;
    p->target_dist = 0;
    p->back_up = UNDEFINED;
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_link_node */

/*************
 *
 *    void free_link_node()
 *
 *************/

void free_link_node(p)
struct link_node *p;
{
    link_node_frees++;
    link_node_avails++;
    p->next_sibling = link_node_avail;
    link_node_avail = p;
}  /* free_link_node */

/*************
 *
 *    free_imd_pos_list(imd_pos) -- free a list of imd_pos nodes.
 *
 *************/

void free_imd_pos_list(p)
struct imd_pos *p;
{
    struct imd_pos *q;

    if (p != NULL) {
	q = p;
	imd_pos_frees++;
	imd_pos_avails++;
	while (q->next != NULL) {
	    imd_pos_frees++;
	    imd_pos_avails++;
	    q = q->next;
	    }
	q->next = imd_pos_avail;
	imd_pos_avail = p;
	}
}  /* free_imd_pos_list */

/*************
 *
 *    free_is_pos_list(is_pos) -- free a list of is_pos nodes.
 *
 *************/

void free_is_pos_list(p)
struct is_pos *p;
{
    struct is_pos *q;

    if (p != NULL) {
	q = p;
	is_pos_frees++;
	is_pos_avails++;
	while (q->next != NULL) {
	    is_pos_frees++;
	    is_pos_avails++;
	    q = q->next;
	    }
	q->next = is_pos_avail;
	is_pos_avail = p;
	}
}  /* free_is_pos_list */

/*************
 *
 *    print_mem()
 *
 *************/

void print_mem(fp)
FILE *fp;
{
    fprintf(fp, "\n------------- memory usage ------------\n");

    fprintf(fp, "%d mallocs of %d bytes each, %.1f K.\n",
	  Malloc_calls, TP_ALLOC_SIZE, (Malloc_calls * (TP_ALLOC_SIZE / 1024.)));

    fprintf(fp, "  type (bytes each)        gets      frees     in use      avail      bytes\n");
    fprintf(fp, "sym_ent (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct sym_ent), sym_ent_gets, sym_ent_frees, sym_ent_gets - sym_ent_frees, sym_ent_avails, (((sym_ent_gets - sym_ent_frees) + sym_ent_avails) * sizeof(struct sym_ent)) / 1024.);
    fprintf(fp, "term (%4d)         %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct term), term_gets, term_frees, term_gets - term_frees, term_avails, (((term_gets - term_frees) + term_avails) * sizeof(struct term)) / 1024.);
    fprintf(fp, "rel (%4d)          %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct rel), rel_gets, rel_frees, rel_gets - rel_frees, rel_avails, (((rel_gets - rel_frees) + rel_avails) * sizeof(struct rel)) / 1024.);
    fprintf(fp, "term_ptr (%4d)     %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct term_ptr), term_ptr_gets, term_ptr_frees, term_ptr_gets - term_ptr_frees, term_ptr_avails, (((term_ptr_gets - term_ptr_frees) + term_ptr_avails) * sizeof(struct term_ptr)) / 1024.);
    fprintf(fp, "formula_ptr_2 (%4d)%11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct formula_ptr_2), formula_ptr_2_gets, formula_ptr_2_frees, formula_ptr_2_gets - formula_ptr_2_frees, formula_ptr_2_avails, (((formula_ptr_2_gets - formula_ptr_2_frees) + formula_ptr_2_avails) * sizeof(struct formula_ptr_2)) / 1024.);
    fprintf(fp, "fpa_head (%4d)     %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct fpa_head), fpa_head_gets, fpa_head_frees, fpa_head_gets - fpa_head_frees, fpa_head_avails, (((fpa_head_gets - fpa_head_frees) + fpa_head_avails) * sizeof(struct fpa_head)) / 1024.);
    fprintf(fp, "fpa_tree (%4d)     %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct fpa_tree), fpa_tree_gets, fpa_tree_frees, fpa_tree_gets - fpa_tree_frees, fpa_tree_avails, (((fpa_tree_gets - fpa_tree_frees) + fpa_tree_avails) * sizeof(struct fpa_tree)) / 1024.);
    fprintf(fp, "context (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct context), context_gets, context_frees, context_gets - context_frees, context_avails, (((context_gets - context_frees) + context_avails) * sizeof(struct context)) / 1024.);
    fprintf(fp, "trail (%4d)        %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct trail), trail_gets, trail_frees, trail_gets - trail_frees, trail_avails, (((trail_gets - trail_frees) + trail_avails) * sizeof(struct trail)) / 1024.);
    fprintf(fp, "imd_tree (%4d)     %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct imd_tree), imd_tree_gets, imd_tree_frees, imd_tree_gets - imd_tree_frees, imd_tree_avails, (((imd_tree_gets - imd_tree_frees) + imd_tree_avails) * sizeof(struct imd_tree)) / 1024.);
    fprintf(fp, "imd_pos (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct imd_pos), imd_pos_gets, imd_pos_frees, imd_pos_gets - imd_pos_frees, imd_pos_avails, (((imd_pos_gets - imd_pos_frees) + imd_pos_avails) * sizeof(struct imd_pos)) / 1024.);
    fprintf(fp, "is_tree (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct is_tree), is_tree_gets, is_tree_frees, is_tree_gets - is_tree_frees, is_tree_avails, (((is_tree_gets - is_tree_frees) + is_tree_avails) * sizeof(struct is_tree)) / 1024.);
    fprintf(fp, "is_pos (%4d)       %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct is_pos), is_pos_gets, is_pos_frees, is_pos_gets - is_pos_frees, is_pos_avails, (((is_pos_gets - is_pos_frees) + is_pos_avails) * sizeof(struct is_pos)) / 1024.);
    fprintf(fp, "fsub_pos (%4d)     %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct fsub_pos), fsub_pos_gets, fsub_pos_frees, fsub_pos_gets - fsub_pos_frees, fsub_pos_avails, (((fsub_pos_gets - fsub_pos_frees) + fsub_pos_avails) * sizeof(struct fsub_pos)) / 1024.);
    fprintf(fp, "literal (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct literal), literal_gets, literal_frees, literal_gets - literal_frees, literal_avails, (((literal_gets - literal_frees) + literal_avails) * sizeof(struct literal)) / 1024.);
    fprintf(fp, "clause (%4d)       %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct clause), clause_gets, clause_frees, clause_gets - clause_frees, clause_avails, (((clause_gets - clause_frees) + clause_avails) * sizeof(struct clause)) / 1024.);
    fprintf(fp, "list (%4d)         %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct list), list_gets, list_frees, list_gets - list_frees, list_avails, (((list_gets - list_frees) + list_avails) * sizeof(struct list)) / 1024.);
    fprintf(fp, "clash_nd (%4d)     %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct clash_nd), clash_nd_gets, clash_nd_frees, clash_nd_gets - clash_nd_frees, clash_nd_avails, (((clash_nd_gets - clash_nd_frees) + clash_nd_avails) * sizeof(struct clash_nd)) / 1024.);
    fprintf(fp, "clause_ptr (%4d)   %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct clause_ptr), clause_ptr_gets, clause_ptr_frees, clause_ptr_gets - clause_ptr_frees, clause_ptr_avails, (((clause_ptr_gets - clause_ptr_frees) + clause_ptr_avails) * sizeof(struct clause_ptr)) / 1024.);
    fprintf(fp, "int_ptr (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct int_ptr), int_ptr_gets, int_ptr_frees, int_ptr_gets - int_ptr_frees, int_ptr_avails, (((int_ptr_gets - int_ptr_frees) + int_ptr_avails) * sizeof(struct int_ptr)) / 1024.);
    fprintf(fp, "link_node (%4d)    %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct link_node), link_node_gets, link_node_frees, link_node_gets - link_node_frees, link_node_avails, (((link_node_gets - link_node_frees) + link_node_avails) * sizeof(struct link_node)) / 1024.);
    fprintf(fp, "ans_lit_node(%4d)  %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct ans_lit_node), ans_lit_node_gets, ans_lit_node_frees, ans_lit_node_gets - ans_lit_node_frees, ans_lit_node_avails, (((ans_lit_node_gets - ans_lit_node_frees) + ans_lit_node_avails) * sizeof(struct ans_lit_node)) / 1024.);
    fprintf(fp, "formula_box(%4d)   %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct formula_box), formula_box_gets, formula_box_frees, formula_box_gets - formula_box_frees, formula_box_avails, (((formula_box_gets - formula_box_frees) + formula_box_avails) * sizeof(struct formula_box)) / 1024.);
    fprintf(fp, "formula(%4d)       %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct formula), formula_gets, formula_frees, formula_gets - formula_frees, formula_avails, (((formula_gets - formula_frees) + formula_avails) * sizeof(struct formula)) / 1024.);
    fprintf(fp, "formula_ptr(%4d)   %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct formula_ptr), formula_ptr_gets, formula_ptr_frees, formula_ptr_gets - formula_ptr_frees, formula_ptr_avails, (((formula_ptr_gets - formula_ptr_frees) + formula_ptr_avails) * sizeof(struct formula_ptr)) / 1024.);

#ifdef ROO
    fprintf(fp, "time_node  (%4d)   %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct time_node), time_node_gets, time_node_frees, time_node_gets - time_node_frees, time_node_avails, (((time_node_gets - time_node_frees) + time_node_avails) * sizeof(struct time_node)) / 1024.);
#endif

}  /* print_mem */

/*************
 *
 *    print_mem_brief()
 *
 *************/

void print_mem_brief(fp)
FILE *fp;
{
    fprintf(fp, "\n------------- memory usage ------------\n");

    fprintf(fp, "%d mallocs of %d bytes each, %.1f K.\n",
	  Malloc_calls, TP_ALLOC_SIZE, (Malloc_calls * (TP_ALLOC_SIZE / 1024.)));

    fprintf(fp, "  type (bytes each)     gets      frees     in use      avail      bytes\n");
    fprintf(fp, "term (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct term), term_gets, term_frees, term_gets - term_frees, term_avails, (((term_gets - term_frees) + term_avails) * sizeof(struct term)) / 1024.);
    fprintf(fp, "rel (%4d)       %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct rel), rel_gets, rel_frees, rel_gets - rel_frees, rel_avails, (((rel_gets - rel_frees) + rel_avails) * sizeof(struct rel)) / 1024.);
    fprintf(fp, "term_ptr (%4d)  %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct term_ptr), term_ptr_gets, term_ptr_frees, term_ptr_gets - term_ptr_frees, term_ptr_avails, (((term_ptr_gets - term_ptr_frees) + term_ptr_avails) * sizeof(struct term_ptr)) / 1024.);
    fprintf(fp, "is_tree (%4d)   %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct is_tree), is_tree_gets, is_tree_frees, is_tree_gets - is_tree_frees, is_tree_avails, (((is_tree_gets - is_tree_frees) + is_tree_avails) * sizeof(struct is_tree)) / 1024.);
}  /* print_mem_brief */

/*************
 *
 *    int total_mem() -- How many K have been dynamically allocated?
 *
 *************/

int total_mem()
{
    return( (int) (Malloc_calls * (TP_ALLOC_SIZE / 1024.)));
}  /* total_mem */

/*************
 *
 *    void print_linked_ur_mem_stats()
 *
 *************/

void print_linked_ur_mem_stats()
{

    printf("context gets=%d frees=%d inuse=%d\n",context_gets, context_frees, context_gets-context_frees);
    printf("trail gets=%d frees=%d inuse=%d\n",trail_gets, trail_frees, trail_gets-trail_frees);
    printf("fpa_tree gets=%d frees=%d inuse=%d\n",fpa_tree_gets, fpa_tree_frees, fpa_tree_gets-fpa_tree_frees);
    printf("term gets=%d frees=%d inuse=%d\n",term_gets, term_frees, term_gets-term_frees);
    printf("link_node gets=%d frees=%d inuse=%d\n",link_node_gets,link_node_frees, link_node_gets-link_node_frees);

}  /* end print_linked_ur_mem_stats */

/*************
 *
 *    int term_ptr_get_size() - return size in Bytes
 *
 *************/

int term_ptr_get_size()
{
    return(term_ptr_gets * sizeof(struct term_ptr));
}  /* term_ptr_get_size */

/*************
 *
 *    int is_tree_get_size() - return size in Bytes
 *
 *************/

int is_tree_get_size()
{
    return(is_tree_gets * sizeof(struct is_tree));
}  /* is_tree_get_size */

#ifdef ROO

/*************
 *
 *    int get_time_node(p1)
 *
 *	struct time_node * in Otter, int in Penguin as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct time_node through the
 *	parameter p1.
 *
 *************/

int get_time_node(p1)
struct time_node **p1;
{
    struct time_node *p;
    char *c;

    time_node_gets++;
    if (time_node_avail == NULL)
	if (tp_alloc(sizeof(struct time_node), &c) == TROUBLE)
		return(TROUBLE);
        else p = (struct time_node *) c;
    else {
        time_node_avails--;
        p = time_node_avail;
        time_node_avail = time_node_avail->next;
        }
	*p1 = p;
    return(NO_TROUBLE);
}  /* get_time_node */

/*************
 *
 *    void free_time_node()
 *
 *************/

void free_time_node(p)
struct time_node *p;
{
    time_node_frees++;
    time_node_avails++;
    p->next = time_node_avail;
    time_node_avail = p;
}  /* free_time_node */

/*****************
 *
 *   void re_initialize_avail()
 *
 ****************/

void re_initialize_avail()
{

    Alloc_block = NULL;
    Alloc_pos = NULL;


    term_avail = NULL;
    rel_avail = NULL;
    sym_ent_avail = NULL;
    term_ptr_avail = NULL;
    fpa_tree_avail = NULL;
    fpa_head_avail = NULL;
    context_avail = NULL;
    trail_avail = NULL;
    imd_tree_avail = NULL;
    imd_pos_avail = NULL;
    is_tree_avail = NULL;
    is_pos_avail = NULL;
    fsub_pos_avail = NULL;
    literal_avail = NULL;
    clause_avail = NULL;
    list_avail = NULL;
    clash_nd_avail = NULL;
    clause_ptr_avail = NULL;
    int_ptr_avail = NULL;

    link_node_avail = NULL;
    ans_lit_node_avail = NULL;
    time_node_avail = NULL;

    Malloc_calls = 0;

    term_gets = 0; term_frees = 0; term_avails = 0;
    rel_gets = 0; rel_frees = 0; rel_avails = 0;
    sym_ent_gets = 0; sym_ent_frees = 0; sym_ent_avails = 0;
    term_ptr_gets = 0; term_ptr_frees = 0; term_ptr_avails = 0;
    fpa_tree_gets = 0; fpa_tree_frees = 0; fpa_tree_avails = 0;
    fpa_head_gets = 0; fpa_head_frees = 0; fpa_head_avails = 0;
    context_gets = 0; context_frees = 0; context_avails = 0;
    trail_gets = 0; trail_frees = 0; trail_avails = 0;
    imd_tree_gets = 0; imd_tree_frees = 0; imd_tree_avails = 0;
    imd_pos_gets = 0; imd_pos_frees = 0; imd_pos_avails = 0;
    is_tree_gets = 0; is_tree_frees = 0; is_tree_avails = 0;
    is_pos_gets = 0; is_pos_frees = 0; is_pos_avails = 0;
    fsub_pos_gets = 0; fsub_pos_frees = 0; fsub_pos_avails = 0;
    literal_gets = 0; literal_frees = 0; literal_avails = 0;
    clause_gets = 0; clause_frees = 0; clause_avails = 0;
    list_gets = 0; list_frees = 0; list_avails = 0;
    clash_nd_gets = 0; clash_nd_frees = 0; clash_nd_avails = 0;
    clause_ptr_gets = 0; clause_ptr_frees = 0; clause_ptr_avails = 0;
    int_ptr_gets = 0; int_ptr_frees = 0; int_ptr_avails = 0;

    link_node_gets = 0; link_node_frees = 0; link_node_avails = 0;
    ans_lit_node_gets = 0; ans_lit_node_frees = 0; ans_lit_node_avails = 0;

    formula_box_gets = 0; formula_box_frees = 0; formula_box_avails = 0;
    formula_gets = 0; formula_frees = 0; formula_avails = 0;
    formula_ptr_gets = 0; formula_ptr_frees = 0; formula_ptr_avails = 0;

    time_node_gets = 0; time_node_frees = 0; time_node_avails = 0;

}  /* re_initialize_avail */

/*************
 *
 *    void init_mem_stats(p)
 *
 ************/

void init_mem_stats(p)
struct mem_stats *p;
{
    p->Malloc_calls = 0;

    p->term_gets = 0; p->term_frees = 0; p->term_avails = 0;
    p->rel_gets = 0; p->rel_frees = 0; p->rel_avails = 0;
    p->sym_ent_gets = 0; p->sym_ent_frees = 0; p->sym_ent_avails = 0;
    p->term_ptr_gets = 0; p->term_ptr_frees = 0; p->term_ptr_avails = 0;
    p->fpa_tree_gets = 0; p->fpa_tree_frees = 0; p->fpa_tree_avails = 0;
    p->fpa_head_gets = 0; p->fpa_head_frees = 0; p->fpa_head_avails = 0;
    p->context_gets = 0; p->context_frees = 0; p->context_avails = 0;
    p->trail_gets = 0; p->trail_frees = 0; p->trail_avails = 0;
    p->imd_tree_gets = 0; p->imd_tree_frees = 0; p->imd_tree_avails = 0;
    p->imd_pos_gets = 0; p->imd_pos_frees = 0; p->imd_pos_avails = 0;
    p->is_tree_gets = 0; p->is_tree_frees = 0; p->is_tree_avails = 0;
    p->is_pos_gets = 0; p->is_pos_frees = 0; p->is_pos_avails = 0;
    p->fsub_pos_gets = 0; p->fsub_pos_frees = 0; p->fsub_pos_avails = 0;
    p->literal_gets = 0; p->literal_frees = 0; p->literal_avails = 0;
    p->clause_gets = 0; p->clause_frees = 0; p->clause_avails = 0;
    p->list_gets = 0; p->list_frees = 0; p->list_avails = 0;
    p->clash_nd_gets = 0; p->clash_nd_frees = 0; p->clash_nd_avails = 0;
    p->clause_ptr_gets = 0; p->clause_ptr_frees = 0; p->clause_ptr_avails = 0;
    p->int_ptr_gets = 0; p->int_ptr_frees = 0; p->int_ptr_avails = 0;

    p->link_node_gets = 0; p->link_node_frees = 0; p->link_node_avails = 0;
    p->ans_lit_node_gets = 0; p->ans_lit_node_frees = 0; p->ans_lit_node_avails = 0;

    p->formula_box_gets = 0; p->formula_box_frees = 0; p->formula_box_avails = 0;
    p->formula_gets = 0; p->formula_frees = 0; p->formula_avails = 0;
    p->formula_ptr_gets = 0; p->formula_ptr_frees = 0; p->formula_ptr_avails = 0;

    p->time_node_gets = 0; p->time_node_frees = 0; p->time_node_avails = 0;

}  /* init_mem_stats */
    
/*************
 *
 *    void add_your_mem_stats(p)
 *
 *************/

void add_your_mem_stats(p)
struct mem_stats *p;
{
    p->Malloc_calls += Malloc_calls;

    p->term_gets += term_gets; p->term_frees += term_frees; p->term_avails += term_avails;
    p->rel_gets += rel_gets; p->rel_frees += rel_frees; p->rel_avails += rel_avails;
    p->sym_ent_gets += sym_ent_gets; p->sym_ent_frees += sym_ent_frees; p->sym_ent_avails += sym_ent_avails;
    p->term_ptr_gets += term_ptr_gets; p->term_ptr_frees += term_ptr_frees; p->term_ptr_avails += term_ptr_avails;
    p->fpa_tree_gets += fpa_tree_gets; p->fpa_tree_frees += fpa_tree_frees; p->fpa_tree_avails += fpa_tree_avails;
    p->fpa_head_gets += fpa_head_gets; p->fpa_head_frees += fpa_head_frees; p->fpa_head_avails += fpa_head_avails;
    p->context_gets += context_gets; p->context_frees += context_frees; p->context_avails += context_avails;
    p->trail_gets += trail_gets; p->trail_frees += trail_frees; p->trail_avails += trail_avails;
    p->imd_tree_gets += imd_tree_gets; p->imd_tree_frees += imd_tree_frees; p->imd_tree_avails += imd_tree_avails;
    p->imd_pos_gets += imd_pos_gets; p->imd_pos_frees += imd_pos_frees; p->imd_pos_avails += imd_pos_avails;
    p->is_tree_gets += is_tree_gets; p->is_tree_frees += is_tree_frees; p->is_tree_avails += is_tree_avails;
    p->is_pos_gets += is_pos_gets; p->is_pos_frees += is_pos_frees; p->is_pos_avails += is_pos_avails;
    p->fsub_pos_gets += fsub_pos_gets; p->fsub_pos_frees += fsub_pos_frees; p->fsub_pos_avails += fsub_pos_avails;
    p->literal_gets += literal_gets; p->literal_frees += literal_frees; p->literal_avails += literal_avails;
    p->clause_gets += clause_gets; p->clause_frees += clause_frees; p->clause_avails += clause_avails;
    p->list_gets += list_gets; p->list_frees += list_frees; p->list_avails += list_avails;
    p->clash_nd_gets += clash_nd_gets; p->clash_nd_frees += clash_nd_frees; p->clash_nd_avails += clash_nd_avails;
    p->clause_ptr_gets += clause_ptr_gets; p->clause_ptr_frees += clause_ptr_frees; p->clause_ptr_avails += clause_ptr_avails;
    p->int_ptr_gets += int_ptr_gets; p->int_ptr_frees += int_ptr_frees; p->int_ptr_avails += int_ptr_avails;

    p->link_node_gets += link_node_gets; p->link_node_frees += link_node_frees; p->link_node_avails += link_node_avails;
    p->ans_lit_node_gets += ans_lit_node_gets; p->ans_lit_node_frees += ans_lit_node_frees; p->ans_lit_node_avails += ans_lit_node_avails;

    p->formula_box_gets += formula_box_gets; p->formula_box_frees += formula_box_frees; p->formula_box_avails += formula_box_avails;
    p->formula_gets += formula_gets; p->formula_frees += formula_frees; p->formula_avails += formula_avails;
    p->formula_ptr_gets += formula_ptr_gets; p->formula_ptr_frees += formula_ptr_frees; p->formula_ptr_avails += formula_ptr_avails;

    p->time_node_gets += time_node_gets; p->time_node_frees += time_node_frees; p->time_node_avails += time_node_avails;

}  /* add_your_mem_stats */

/*************
 *
 *    void print_mem_from_struct(fp, p)
 *
 *************/

void print_mem_from_struct(fp, p)
FILE *fp;
struct mem_stats *p;
{
    fprintf(fp, "\n------------- memory usage (from struct) ------------\n");

    fprintf(fp, "%d Mallocs of %d bytes each, %.1f K.\n",
	  p->Malloc_calls, TP_ALLOC_SIZE, (p->Malloc_calls * (TP_ALLOC_SIZE / 1024.)));

    fprintf(fp, "  type (bytes each)     gets      frees     in use      avail      bytes\n");

    fprintf(fp, "sym_ent (%4d)   %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct sym_ent), p->sym_ent_gets, p->sym_ent_frees, p->sym_ent_gets - p->sym_ent_frees, p->sym_ent_avails, (((p->sym_ent_gets - p->sym_ent_frees) + p->sym_ent_avails) * sizeof(struct sym_ent)) / 1024.);
    fprintf(fp, "term (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct term), p->term_gets, p->term_frees, p->term_gets - p->term_frees, p->term_avails, (((p->term_gets - p->term_frees) + p->term_avails) * sizeof(struct term)) / 1024.);
    fprintf(fp, "rel (%4d)       %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct rel), p->rel_gets, p->rel_frees, p->rel_gets - p->rel_frees, p->rel_avails, (((p->rel_gets - p->rel_frees) + p->rel_avails) * sizeof(struct rel)) / 1024.);
    fprintf(fp, "term_ptr (%4d)  %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct term_ptr), p->term_ptr_gets, p->term_ptr_frees, p->term_ptr_gets - p->term_ptr_frees, p->term_ptr_avails, (((p->term_ptr_gets - p->term_ptr_frees) + p->term_ptr_avails) * sizeof(struct term_ptr)) / 1024.);
    fprintf(fp, "fpa_head (%4d)  %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct fpa_head), p->fpa_head_gets, p->fpa_head_frees, p->fpa_head_gets - p->fpa_head_frees, p->fpa_head_avails, (((p->fpa_head_gets - p->fpa_head_frees) + p->fpa_head_avails) * sizeof(struct fpa_head)) / 1024.);
    fprintf(fp, "fpa_tree (%4d)  %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct fpa_tree), p->fpa_tree_gets, p->fpa_tree_frees, p->fpa_tree_gets - p->fpa_tree_frees, p->fpa_tree_avails, (((p->fpa_tree_gets - p->fpa_tree_frees) + p->fpa_tree_avails) * sizeof(struct fpa_tree)) / 1024.);
    fprintf(fp, "context (%4d)   %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct context), p->context_gets, p->context_frees, p->context_gets - p->context_frees, p->context_avails, (((p->context_gets - p->context_frees) + p->context_avails) * sizeof(struct context)) / 1024.);
    fprintf(fp, "trail (%4d)     %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct trail), p->trail_gets, p->trail_frees, p->trail_gets - p->trail_frees, p->trail_avails, (((p->trail_gets - p->trail_frees) + p->trail_avails) * sizeof(struct trail)) / 1024.);
    fprintf(fp, "imd_tree (%4d)  %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct imd_tree), p->imd_tree_gets, p->imd_tree_frees, p->imd_tree_gets - p->imd_tree_frees, p->imd_tree_avails, (((p->imd_tree_gets - p->imd_tree_frees) + p->imd_tree_avails) * sizeof(struct imd_tree)) / 1024.);
    fprintf(fp, "imd_pos (%4d)   %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct imd_pos), p->imd_pos_gets, p->imd_pos_frees, p->imd_pos_gets - p->imd_pos_frees, p->imd_pos_avails, (((p->imd_pos_gets - p->imd_pos_frees) + p->imd_pos_avails) * sizeof(struct imd_pos)) / 1024.);
    fprintf(fp, "is_tree (%4d)   %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct is_tree), p->is_tree_gets, p->is_tree_frees, p->is_tree_gets - p->is_tree_frees, p->is_tree_avails, (((p->is_tree_gets - p->is_tree_frees) + p->is_tree_avails) * sizeof(struct is_tree)) / 1024.);
    fprintf(fp, "is_pos (%4d)    %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct is_pos), p->is_pos_gets, p->is_pos_frees, p->is_pos_gets - p->is_pos_frees, p->is_pos_avails, (((p->is_pos_gets - p->is_pos_frees) + p->is_pos_avails) * sizeof(struct is_pos)) / 1024.);
    fprintf(fp, "fsub_pos (%4d)  %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct fsub_pos), p->fsub_pos_gets, p->fsub_pos_frees, p->fsub_pos_gets - p->fsub_pos_frees, p->fsub_pos_avails, (((p->fsub_pos_gets - p->fsub_pos_frees) + p->fsub_pos_avails) * sizeof(struct fsub_pos)) / 1024.);
    fprintf(fp, "literal (%4d)   %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct literal), p->literal_gets, p->literal_frees, p->literal_gets - p->literal_frees, p->literal_avails, (((p->literal_gets - p->literal_frees) + p->literal_avails) * sizeof(struct literal)) / 1024.);
    fprintf(fp, "clause (%4d)    %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct clause), p->clause_gets, p->clause_frees, p->clause_gets - p->clause_frees, p->clause_avails, (((p->clause_gets - p->clause_frees) + p->clause_avails) * sizeof(struct clause)) / 1024.);
    fprintf(fp, "list (%4d)      %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct list), p->list_gets, p->list_frees, p->list_gets - p->list_frees, p->list_avails, (((p->list_gets - p->list_frees) + p->list_avails) * sizeof(struct list)) / 1024.);
    fprintf(fp, "clash_nd (%4d)  %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct clash_nd), p->clash_nd_gets, p->clash_nd_frees, p->clash_nd_gets - p->clash_nd_frees, p->clash_nd_avails, (((p->clash_nd_gets - p->clash_nd_frees) + p->clash_nd_avails) * sizeof(struct clash_nd)) / 1024.);
    fprintf(fp, "clause_ptr (%4d)%11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct clause_ptr), p->clause_ptr_gets, p->clause_ptr_frees, p->clause_ptr_gets - p->clause_ptr_frees, p->clause_ptr_avails, (((p->clause_ptr_gets - p->clause_ptr_frees) + p->clause_ptr_avails) * sizeof(struct clause_ptr)) / 1024.);
    fprintf(fp, "int_ptr (%4d)   %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct int_ptr), p->int_ptr_gets, p->int_ptr_frees, p->int_ptr_gets - p->int_ptr_frees, p->int_ptr_avails, (((p->int_ptr_gets - p->int_ptr_frees) + p->int_ptr_avails) * sizeof(struct int_ptr)) / 1024.);
    fprintf(fp, "link_node (%4d) %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct link_node), p->link_node_gets, p->link_node_frees, p->link_node_gets - p->link_node_frees, p->link_node_avails, (((p->link_node_gets - p->link_node_frees) + p->link_node_avails) * sizeof(struct link_node)) / 1024.);
    fprintf(fp, "ans_node  (%4d) %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct ans_lit_node), p->ans_lit_node_gets, p->ans_lit_node_frees, p->ans_lit_node_gets - p->ans_lit_node_frees, p->ans_lit_node_avails, (((p->ans_lit_node_gets - p->ans_lit_node_frees) + p->ans_lit_node_avails) * sizeof(struct ans_lit_node)) / 1024.);

    fprintf(fp, "formula_box (%4d)%11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct formula_box), p->formula_box_gets, p->formula_box_frees, p->formula_box_gets - p->formula_box_frees, p->formula_box_avails, (((p->formula_box_gets - p->formula_box_frees) + p->formula_box_avails) * sizeof(struct formula_box)) / 1024.);
    fprintf(fp, "formula (%4d)    %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct formula), p->formula_gets, p->formula_frees, p->formula_gets - p->formula_frees, p->formula_avails, (((p->formula_gets - p->formula_frees) + p->formula_avails) * sizeof(struct formula)) / 1024.);
    fprintf(fp, "formula_ptr (%4d)%11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct formula_ptr), p->formula_ptr_gets, p->formula_ptr_frees, p->formula_ptr_gets - p->formula_ptr_frees, p->formula_ptr_avails, (((p->formula_ptr_gets - p->formula_ptr_frees) + p->formula_ptr_avails) * sizeof(struct formula_ptr)) / 1024.);

    fprintf(fp, "time_node (%4d)  %11ld%11ld%11ld%11ld%9.1f K\n", sizeof(struct time_node), p->time_node_gets, p->time_node_frees, p->time_node_gets - p->time_node_frees, p->time_node_avails, (((p->time_node_gets - p->time_node_frees) + p->time_node_avails) * sizeof(struct time_node)) / 1024.);
}  /* print_mem_from_struct */


#endif  /* ROO */
