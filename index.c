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
 *  index.c -- Routines for indexing and unindexing clauses.
 *
 */

#include "header.h"

/*************
 *
 *    int index_mark_clash(r) -- recursive routine to mark and index
 *    clashable terms (terms that can be used by paramodulation).
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

static int index_mark_clash(r)
struct rel *r;
{
    struct term *t;
    struct rel *r1;
	int i;					/* Penguin */

	i = 0;					/* default */
    t = r->argval;

    if (t->type == VARIABLE && Flags[PARA_INTO_VARS].val == 0)
	return(NO_TROUBLE);
    else {
	r1 = t->occ.rel;
	while (r1 != NULL && r1->clashable == 0)
	    r1 = r1->nocc;
	r->clashable = 1;
	if (r1 != NULL)
	    return(NO_TROUBLE);  /* because t is already clashable */
	else {
	    if (Flags[PARA_FROM].val)
		{
	if (fpa_insert(t, Parms[FPA_TERMS].val, Fpa_clash_terms) == TROUBLE)
		return(TROUBLE);
		}
	    if (t->type == COMPLEX) {
	if (Flags[PARA_SKIP_SKOLEM].val)
		if (is_skolem(t->sym_num,&i) == TROUBLE)
			return(TROUBLE);
	if (Flags[PARA_SKIP_SKOLEM].val == 0 || i == 0) {
		    r = t->farg;
		    while (r != NULL) {
			if (index_mark_clash(r) == TROUBLE)
				return(TROUBLE);
			if (Flags[PARA_ONES_RULE].val)
			    r = NULL;
			else
			    r = r->narg;
			}	/* end of while */
		    }	/* end of not PARA_SKIP_SKOLEM or not is_skolem() */
		}	/* end of COMPLEX */
	    }	/* end of else */
	}
return(NO_TROUBLE);
}  /* index_mark_clash */

/*************
 *
 *    int un_index_mark_clash(r)
 *
 *    See index_mark_clash.
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

static int un_index_mark_clash(r)
struct rel *r;
{
    struct term *t;
    struct rel *r1;
	int i;						/* Penguin */

	i = 0;						/* default */
    t = r->argval;

    if (t->type == VARIABLE && Flags[PARA_INTO_VARS].val == 0)
	return(NO_TROUBLE);
    else {
	r->clashable = 0;
	r1 = t->occ.rel;
	while (r1 != NULL && r1->clashable == 0)
	    r1 = r1->nocc;
	if (r1 != NULL)
	    return(NO_TROUBLE); 
/* because t is clashable from another containing term */
	else {
	    if (Flags[PARA_FROM].val)
		{
	if (fpa_delete(t, Parms[FPA_TERMS].val, Fpa_clash_terms) == TROUBLE)
			return(TROUBLE);
		}
	    if (t->type == COMPLEX) {
	if (Flags[PARA_SKIP_SKOLEM].val)
		if (is_skolem(t->sym_num,&i) == TROUBLE)
			return(TROUBLE);
	if (Flags[PARA_SKIP_SKOLEM].val == 0 || i == 0) {
		    r = t->farg;
		    while (r != NULL) {
			if (un_index_mark_clash(r) == TROUBLE)
				return(TROUBLE);
			if (Flags[PARA_ONES_RULE].val)
			    r = NULL;
			else
			    r = r->narg;
			}	/* end of while */
		    }	/* end of not PARA_SKIP_SKOLEM or not is_skolem() */
		}	/* end of if COMPLEX */
	    }	/* end of else */
	}	/* end of else */
return(NO_TROUBLE);
}  /* un_index_mark_clash() */

/*************
 *
 *    int index_paramod(atom) -- index for paramodulation inference rules
 *
 *    Index clashable terms for `from' paramodulation, and
 *    index clashable args of equality for `into' paramodulation.
 *
 *    Also mark clashable terms for the paramodulation routines.
 *
 *    Penguin uses the global variable Whoami, i.e. the id of the node, in order
 *    to prevent insertion of the terms of giv_cl in Fpa_clash_terms, the
 *    list of the terms to be paramodulated into, if giv_cl is not a resident.
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

static int index_paramod(atom)
struct term *atom;
{
    struct rel *r;

    /* First index clashable `into' terms for `from' paramodulation. */

if (atom->occ.lit->container->pid == Whoami)
{ /* Penguin: if it's a resident */
    if (Flags[PARA_INTO_UNITS_ONLY].val == 0 || 
	         num_literals(atom->occ.lit->container) == 1) {
	if (atom->varnum == POS_EQ || atom->varnum == NEG_EQ) {
	    if (Flags[PARA_INTO_LEFT].val)
		{
		if (index_mark_clash(atom->farg) == TROUBLE)
			return(TROUBLE);
		}
	    if (Flags[PARA_INTO_RIGHT].val)
		{
		if (index_mark_clash(atom->farg->narg) == TROUBLE)
			return(TROUBLE);
		}
	    }	/* end of if POS_EQ or NEG_EQ */
	else {
	    r = atom->farg;
	    while (r != NULL) {
		if (index_mark_clash(r) == TROUBLE)
			return(TROUBLE);
		/*  should the ones rule apply to atoms? I guess not */
		/*
		if (Flags[PARA_ONES_RULE].val)
		    r = NULL;
		else
		*/
		    r = r->narg;
		}	/* end of while */
	    }	/* end of else */
	}	/* end of if not PARA_INTO_UNITS_ONLY or not unit */
} /* Penguin: end of it's a resident */

    /* Now index clashable `from' terms for `into' paramodulation. */

    if (Flags[PARA_FROM_UNITS_ONLY].val == 0 || 
		 num_literals(atom->occ.lit->container) == 1) {
	if (atom->varnum == POS_EQ && Flags[PARA_INTO].val) {
	    if (Flags[PARA_FROM_LEFT].val) {
	if (Flags[PARA_FROM_VARS].val || atom->farg->argval->type != VARIABLE)
	{
if (fpa_insert(atom->farg->argval,Parms[FPA_TERMS].val,Fpa_alphas)==TROUBLE)
			return(TROUBLE);
	}
		}	/* end of if para_from_left */

	    if (Flags[PARA_FROM_RIGHT].val) {
if (Flags[PARA_FROM_VARS].val || atom->farg->narg->argval->type != VARIABLE)
	{
if (fpa_insert(atom->farg->narg->argval,Parms[FPA_TERMS].val,Fpa_alphas)==TROUBLE)
			return(TROUBLE);
	}
		}	/* end of if para_from_right */
	    }	/* end of if POS_EQ and para_into */
	}	/* end of if not para_from_units_only or unit */
return(NO_TROUBLE);
}  /* index_paramod() */

/*************
 *
 *    int un_index_paramod(atom)
 *
 *    See index_paramod.
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

static int un_index_paramod(atom)
struct term *atom;
{
    struct rel *r;

    if (Flags[PARA_INTO_UNITS_ONLY].val == 0 || 
	         num_literals(atom->occ.lit->container) == 1) {
	if (atom->varnum == POS_EQ || atom->varnum == NEG_EQ) {
	    if (Flags[PARA_INTO_LEFT].val)
		{
		if (un_index_mark_clash(atom->farg) == TROUBLE)
			return(TROUBLE);
		}
	    if (Flags[PARA_INTO_RIGHT].val)
		{
		if (un_index_mark_clash(atom->farg->narg) == TROUBLE)
			return(NO_TROUBLE);
		}
	    }
	else {
	    r = atom->farg;
	    while (r != NULL) {
		if (un_index_mark_clash(r) == TROUBLE)
			return(NO_TROUBLE);
		/*
		if (Flags[PARA_ONES_RULE].val)
		    r = NULL;
		else
		*/
		    r = r->narg;
		}
	    }
	}

    if (Flags[PARA_FROM_UNITS_ONLY].val == 0 || 
		 num_literals(atom->occ.lit->container) == 1) {
	if (atom->varnum == POS_EQ && Flags[PARA_INTO].val) {
	    if (Flags[PARA_FROM_LEFT].val)
	if (Flags[PARA_FROM_VARS].val || atom->farg->argval->type != VARIABLE)
	{
if (fpa_delete(atom->farg->argval, Parms[FPA_TERMS].val, Fpa_alphas)==TROUBLE)
			return(TROUBLE);
	}

	    if (Flags[PARA_FROM_RIGHT].val)
if (Flags[PARA_FROM_VARS].val || atom->farg->narg->argval->type != VARIABLE)
	{
if (fpa_delete(atom->farg->narg->argval,Parms[FPA_TERMS].val,Fpa_alphas)==TROUBLE)
			return(TROUBLE);
	}
	    }
	}
return(NO_TROUBLE);
}  /* un_index_paramod */

/*************
 *
 *    int index_lits_all(c)
 *
 *    Index literals for forward subsumption, back subsumption, and
 *    unit conflict.
 *    Positive and negative literals go into different indexes.
 *    The NO_FAPL, NO_FANL and FOR_SUB_FPA flags are checked to determine
 *    what and how to index.
 *
 *    NO_FAPL can be set if you are generating only positive clauses
 *    and back subsumption is off.  It surpresses indexing of positive
 *    literals in the non-clashable index.  (The index for subsumption
 *     and unit conflict.)
 *    Similarly for negative literals with NO_FANL.
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int index_lits_all(c)
struct clause *c;
{
    struct literal *lit;

    lit = c->first_lit;
    while (lit != NULL) {
	if (lit->atom->varnum == ANSWER)
	    ;  /* skip answer literal */
	else if (lit->sign) {
	    if (Flags[NO_FAPL].val == 0)
		{
if (fpa_insert(lit->atom, Parms[FPA_LITERALS].val, Fpa_pos_lits) == TROUBLE)
			return(TROUBLE);
		}
	    if (Flags[FOR_SUB_FPA].val == 0 && Flags[FOR_SUB].val)
		{
		if (is_insert(lit->atom, Is_pos_lits) == TROUBLE)
			return(TROUBLE);
		}
	    }
	else {
	    if (Flags[NO_FANL].val == 0)
		{
if (fpa_insert(lit->atom, Parms[FPA_LITERALS].val, Fpa_neg_lits) == TROUBLE)
			return(TROUBLE);
		}
	    if (Flags[FOR_SUB_FPA].val == 0 && Flags[FOR_SUB].val)
		{
		if (is_insert(lit->atom, Is_neg_lits) == TROUBLE)
			return(TROUBLE);
		}
	    }
	lit = lit->next_lit;
	}
return(NO_TROUBLE);
}  /* index_lits_all */

/*************
 *
 *    int un_index_lits_all(c)
 *
 *    See index_lits_all.
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int un_index_lits_all(c)
struct clause *c;
{
    struct literal *lit;

    lit = c->first_lit;
    while (lit != NULL) {
	if (lit->atom->varnum == ANSWER)
	    ;  /* skip answer literal */
	else if (lit->sign) {
	    if (Flags[NO_FAPL].val == 0)
		{
if (fpa_delete(lit->atom, Parms[FPA_LITERALS].val, Fpa_pos_lits) == TROUBLE)
			return(TROUBLE);
		}
	    if (Flags[FOR_SUB_FPA].val == 0 && Flags[FOR_SUB].val)
		{
		if (is_delete(lit->atom, Is_pos_lits) == TROUBLE)
			return(TROUBLE);
		}
	    }
	else {
	    if (Flags[NO_FANL].val == 0)
		{
if (fpa_delete(lit->atom, Parms[FPA_LITERALS].val, Fpa_neg_lits) == TROUBLE)
			return(TROUBLE);
		}
	    if (Flags[FOR_SUB_FPA].val == 0 && Flags[FOR_SUB].val)
		{
		if (is_delete(lit->atom, Is_neg_lits) == TROUBLE)
			return(TROUBLE);
		}
	    }
	lit = lit->next_lit;
	}
return(NO_TROUBLE);
}  /* un_index_lits_all */

/*************
 *
 *    int index_lits_clash(c)
 *
 *    Index literals for inference rules, and index terms for paramodulation if
 *    any paramodulation inference rules are set.
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int index_lits_clash(c)
struct clause *c;
{
    struct literal *lit;

    lit = c->first_lit;
    while (lit != NULL) {
	if (lit->atom->varnum == ANSWER || lit->atom->varnum == EVALUABLE)
	    ;  /* skip answer literals and evaluable literals */
	else if (lit->sign)
	{
if (fpa_insert(lit->atom,Parms[FPA_LITERALS].val,Fpa_clash_pos_lits)==TROUBLE)
			return(TROUBLE);
	}
	else
	{
if (fpa_insert(lit->atom,Parms[FPA_LITERALS].val,Fpa_clash_neg_lits)==TROUBLE)
			return(TROUBLE);
	}
	if (Flags[PARA_FROM].val || Flags[PARA_INTO].val)
		{
	    if (index_paramod(lit->atom) == TROUBLE)
			return(TROUBLE);
		}
	lit = lit->next_lit;
	}
return(NO_TROUBLE);
}  /* index_lits_clash */

/*************
 *
 *    int un_index_lits_clash(c)
 *
 *    See index_lits_clash.
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int un_index_lits_clash(c)
struct clause *c;
{
    struct literal *lit;

    lit = c->first_lit;
    while (lit != NULL) {
	if (lit->atom->varnum == ANSWER || lit->atom->varnum == EVALUABLE)
	    ;  /* skip answer literals and evaluable literals */
	else if (lit->sign)
	{
if (fpa_delete(lit->atom,Parms[FPA_LITERALS].val,Fpa_clash_pos_lits)==TROUBLE)
			return(TROUBLE);
	}
	else {
if (fpa_delete(lit->atom,Parms[FPA_LITERALS].val,Fpa_clash_neg_lits)==TROUBLE)
			return(TROUBLE);
	}
	if (Flags[PARA_FROM].val || Flags[PARA_INTO].val)
		{
	    if (un_index_paramod(lit->atom) == TROUBLE)
			return(TROUBLE);
		}
	lit = lit->next_lit;
	}
return(NO_TROUBLE);
}  /* un_index_lits_clash */

/***************
*
*	int un_index_rem_and_hide(e)		Penguin only
*
*	It un_index, rem_from_list and hide a clause in either Sos or
*	Usable or Demodulators.
*
***************/

int un_index_rem_and_hide(e)
struct clause *e;
{

if (e->container == Usable)
	{
	if (un_index_lits_clash(e) == TROUBLE)
		return(TROUBLE);
	if (un_index_lits_all(e) == TROUBLE)
		return(TROUBLE);
	rem_from_list(e);
	Stats[USABLE_SIZE]--;
	hide_clause(e);
	}
else if (e->container == Sos)
	{
	if (un_index_lits_all(e) == TROUBLE)
		return(TROUBLE);
	rem_from_list(e);
	Stats[SOS_SIZE]--;
	hide_clause(e);
	}
else if (e->container == Demodulators)
	{
	if (Flags[DEMOD_LINEAR].val == 0)
		imd_delete(e,Demod_imd);
	rem_from_list(e);
	Stats[DEMODULATORS_SIZE]--;
	hide_clause(e);
	}

return(NO_TROUBLE);
}	/* un_index_rem_and_hide() */


/***************
*
*	int un_index_rem_hide_deld(e)		Penguin only
*
*	It un_index, rem_from_list and hide a clause in either Sos or
*	Usable or Demodulators by calling un_index_rem_and_hide().
*	Also, if the clause is in Sos or Usable it checks whether there
*	is a copy in Demodulators and applies un_index_rem_and_hide() to
*	the copy in Demodulators as well.
*
***************/

int un_index_rem_hide_deld(e)
struct clause *e;
{
struct clause *ce;

if ((e->container == Usable || e->container == Sos) && Flags[DYNAMIC_DEMOD].val)
		{
		ce = cl_find(e->id + 1);
if (ce != NULL && ce->pid == e->pid && ce->lid == e->lid && ce->container == Demodulators)
			if (un_index_rem_and_hide(ce) == TROUBLE)
				return(TROUBLE);
		}
	if (un_index_rem_and_hide(e) == TROUBLE)
		return(TROUBLE);

return(NO_TROUBLE);
}	/* un_index_rem_hide_deld() */
