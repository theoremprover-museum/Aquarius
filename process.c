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
 *  process.c -- Routines to handle the processing of generated clauses.
 *
 */

#include "header.h"

#ifndef ROO

/*************
 *
 *    int post_process(c,ct,lst) -- finish processing a clause
 *
 *    The clause has already been integrated, indexed, appended to
 *    Sos.  This routine looks for unit conflict, does back subsumption,
 *    and possibly generates more clauses (factoring, back demod, hot
 *    lists, etc.).  Any newly generated and kept clauses will be
 *    appended to lst and will wait their turn to be post_processed.
 *
 *	Penguin also adds the parameter ct, where ct may be IR for Input
 *	Read clause, i.e. input clauses read by the Penguin which is 
 *	reading the input, IM for Input Message clause, i.e. input
 *	clause received at a Penguin which did not read the input,
 *	or 0 in any other case.
 *	The parameter ct is passed to back_subsume()
 *	since distributed back-subsumption of variants is not needed on input
 *	clauses (ct == IR), to back_demod(), which in turn passes it to
 *	pre_process() invoked by back_demod() on the reduced form of clause c
 *	and to all_factors(), which in turn passes it to
 *	pre_process() invoked by all_factors() on the factors of clause c.
 *	See back_demod() in demod.c and all_factors() in resolve.c.
 *    It is void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

static int post_process(c,ct,lst)
struct clause *c;
int ct;
struct list *lst;
{
    struct clause *d, *e, *v;
    struct clause_ptr *cp1, *cp2;
	int bd, af, input;

	bd = NO_PROOF;				/* default */
	af = NO_PROOF;				/* default */
/* Penguin adds the variable v to be passed to back_subsume().		*/
/* back_subsume() will set it to point to a clause which is a variant	*/
/* of c and which is smaller than c in the distributed subsumption	*/
/* ordering if any such clause is found.				*/

	v = NULL;			 /* default is NULL  */

if (ct == IR || ct == IM)
	input = 1;
else input = 0;

    if (Flags[BACK_DEMOD].val)
	{
        /* c was made into a new demodulator */
	if (c->first_lit != NULL &&
            TP_BIT(c->first_lit->atom->bits, NEW_DEMOD_BIT))
		{
	    CLEAR_BIT(c->first_lit->atom->bits, NEW_DEMOD_BIT);
	    d = cl_find(c->id + 1);  /* demod id is 1 more than clause id */
	    if (Flags[PRINT_BACK_DEMOD].val)
		{
	fprintf(Fdout,">>>> Starting back demodulation with ");
	print_ids(Fdout,d);
	fprintf(Fdout,".\n");
		}
	    CLOCK_START(BACK_DEMOD_TIME)
	    bd = back_demod(d,c,ct,lst);
	    if (bd == PROOF || bd == TROUBLE)
			return(bd);
	    CLOCK_STOP(BACK_DEMOD_TIME)
	    }
	}	/* end of back-demodulation phase */

    if (Flags[BACK_SUB].val)
	{
	CLOCK_START(BACK_SUB_TIME)
	if (back_subsume(c,input,&v,&cp1) == TROUBLE)
		return(TROUBLE);
	CLOCK_STOP(BACK_SUB_TIME)
	while (cp1 != NULL)
		{
	    e = cp1->c;
	    if (e->container != Passive) {
		Stats[CL_BACK_SUB]++;
		if (Flags[PRINT_BACK_SUB].val)
			{			/* Penguin */
			print_ids(Fdout,c);
		    fprintf(Fdout," back subsumes ");
			print_ids(Fdout,e);
			fprintf(Fdout,".\n");
			}
		if (un_index_rem_hide_deld(e) == TROUBLE)
			return(TROUBLE);
		}
	    cp2 = cp1;
	    cp1 = cp1->next;
	    free_clause_ptr(cp2);
	    }	/* end of while */

		if (v != NULL)			/* Penguin */
		{
/* Penguin: if back_subsume() has set v to point to a clause, it means	*/
/* v points to a clause which is a variant of c and which is smaller than*/
/* c in the distributed subsumption ordering. Then c has to be deleted.	*/
/* Clause v has not been inserted in the list returned by back_subsume() */
/* and thus it has not been deleted.					*/

/* The following code is adapted from the code following forward_subsume()*/
/* in proc_gen().  							*/

	    if (Flags[VERY_VERBOSE].val)
		{
		print_ids(Fdout,c);			/* Penguin */
		fprintf(Fdout," subsumed as a variant by ");
		print_ids(Fdout,v);
		fprintf(Fdout,".\n");
		}
	    Stats[CL_FOR_SUB]++;
	    if (v->container == Sos)
		Stats[FOR_SUB_SOS]++;
	    if (v->id < 100)
		Subsume_count[v->id]++;
	    else
		Subsume_count[0]++;
/* The following code is copied from previous part in post_process().	*/
/* We don't need to check that c is not in Passive, because c had been */
/* selected as a back-subsumer, whereas clauses in Passive are used only*/
/* for forward subsumption and unit conflict.				*/
		if (un_index_rem_hide_deld(c) == TROUBLE)
			return(TROUBLE);
		}	/* end of if v != NULL */

	}	/* end of back-subsumption phase */

if (v == NULL && Flags[FACTOR].val == 1 && (c->pid == Whoami || Flags[PART_FACTORS].val==0))
/* Penguin: v == NULL guarantees that c has not been deleted.		*/
/* Penguin: it applies all_factor(c) only if c is a resident, unless the */
/* Flags[PART_FACTORS], which is on by default, as been turned off.	*/
	{
	CLOCK_START(FACTOR_TIME)
	af = all_factors(c, ct, lst);
	if (af == PROOF || af == TROUBLE)
		return(af);
	CLOCK_STOP(FACTOR_TIME)
	}

return(NO_PROOF);
}	/* post_process() */


/*************
 *
 *    int post_proc_all(lst_pos,ct,lst)
 *
 *	Penguin adds the parameter ct, to be passed to post_process().
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

int post_proc_all(lst_pos,ct,lst)
struct clause *lst_pos;
int ct;
struct list *lst;
{
    struct clause *c;
	int pp;

	pp = NO_PROOF;				/* default */

    CLOCK_START(POST_PROC_TIME)
    if (lst_pos == NULL)
	c = lst->first_cl;
    else
	c = lst_pos->next_cl;

/* This is why we can call post_proc_all() to postprocess a clause which */
/* has just been pre_processed, without checking that the clause was not */
/* deleted by pre_process(). If this is the case, c gets NULL and the	*/
/* function returns immediately.					*/

    while (c != NULL) {
        pp = post_process(c,ct,lst);
        if (pp == PROOF || pp == TROUBLE)
		return(pp);
	/* this may alter c->next_cl */
	c = c->next_cl;
        /* following moved from end of infer_and_process 19 Jan 90 */
	if (Flags[REALLY_DELETE_CLAUSES].val)
	    /* clauses hidden by back demod, back subsumption */
	    /* also empty clauses are hidden */
	    if (del_hidden_clauses() == TROUBLE)
		return(TROUBLE);
	if (Flags[REALLY_DELETE_MSGS].val)
	    /* clauses hidden when sent */
	    if (del_hidden_msgs() == TROUBLE)
		return(TROUBLE);
	}

    CLOCK_STOP(POST_PROC_TIME)
return(NO_PROOF);
}  /* post_proc_all() */

#endif  /* not ROO */

/*************
 *
 *    int infer_and_process(giv_cl)		
 *
 *	void in Otter, int in Penguin.
 *	It returns PROOF/NO_PROOF/TROUBLE.
 *    The inference rules append kept clauses to Sos.  After the
 *    inference rule is finished, the newly kept clauses are
 *    `post_process'ed (unit_conflict, back subsump, etc.).
 *
 *************/

int infer_and_process(giv_cl)
struct clause *giv_cl;
{
    struct clause *c, *sos_pos;
    struct int_ptr *ip;
	int ppa, pp, pif;

	ppa = NO_PROOF;				/* default */
	pp = NO_PROOF;				/* default */
	pif = NO_PROOF;				/* default */

    if (Flags[CONTROL_MEMORY].val)
	control_memory();

    if (Flags[BINARY_RES].val) {
	sos_pos = Sos->last_cl; 
/* Save position of last clauses in Sos. */
	pif = bin_res(giv_cl);
	if (pif == PROOF || pif == TROUBLE)
		return(pif);
/* Inf rule appends newly kept clauses to Sos. */
#ifndef ROO
	/* Now post_process new clauses in Sos. */
	/* (Post_process may append even more clauses to Sos. Do them all.) */
        /* (ROO does not do this.) */
	ppa = post_proc_all(sos_pos,0,Sos);
	if (ppa == PROOF || ppa == TROUBLE)
		return(ppa);
#endif
	}

    /* For subsequent inference rules, check that the given clause  */
    /*  is still in Usable (not back demodulated or back subsumed). */

    if (giv_cl->container == Usable && Flags[HYPER_RES].val) {
	sos_pos = Sos->last_cl;
	pif = hyper_res(giv_cl);
	if (pif == PROOF || pif == TROUBLE)
		return(pif);
#ifndef ROO
	ppa = post_proc_all(sos_pos,0,Sos);
	if (ppa == PROOF || ppa == TROUBLE)
		return(ppa);
#endif
	}

    if (giv_cl->container == Usable && Flags[NEG_HYPER_RES].val) {
	sos_pos = Sos->last_cl;
	pif = neg_hyper_res(giv_cl);
	if (pif == PROOF || pif == TROUBLE)
		return(pif);
#ifndef ROO
	ppa = post_proc_all(sos_pos,0,Sos);
	if (ppa == PROOF || ppa == TROUBLE)
		return(ppa);
#endif
	}

    if (giv_cl->container == Usable && Flags[UR_RES].val) {
	sos_pos = Sos->last_cl;
	pif = ur_res(giv_cl);
	if (pif == PROOF || pif == TROUBLE)
		return(pif);
#ifndef ROO
	ppa = post_proc_all(sos_pos,0,Sos);
	if (ppa == PROOF || ppa == TROUBLE)
		return(ppa);
#endif
	}

if (giv_cl->container==Usable && giv_cl->pid == Whoami && Flags[PARA_INTO].val)
/* Penguin: para_into is invoked only if giv_cl is a resident, because	*/
/* inference messages are not paramodulated into.			*/
	{
	sos_pos = Sos->last_cl;
	pif = para_into(giv_cl);
	if (pif == PROOF || pif == TROUBLE)
		return(pif);
#ifndef ROO
	ppa = post_proc_all(sos_pos,0,Sos);
	if (ppa == PROOF || ppa == TROUBLE)
		return(ppa);
#endif
	}

    if (giv_cl->container == Usable && Flags[PARA_FROM].val) {
	sos_pos = Sos->last_cl;
	pif = para_from(giv_cl);
	if (pif == PROOF || pif == TROUBLE)
		return(pif);
#ifndef ROO
	ppa = post_proc_all(sos_pos,0,Sos);
	if (ppa == PROOF || ppa == TROUBLE)
		return(ppa);
#endif
	}

/* Penguin

    if (giv_cl->container == Usable && Flags[LINKED_UR_RES].val) {
	sos_pos = Sos->last_cl;
	linked_ur_res(giv_cl);
#ifndef ROO
	ppa = post_proc_all(sos_pos,0,Sos);
	if (ppa == PROOF || ppa == TROUBLE)
		return(ppa);
#endif
	}

    if (giv_cl->container == Usable && Flags[LINKED_HYPER_RES].val) {
	sos_pos = Sos->last_cl;
	linked_hyper_res(giv_cl);
#ifndef ROO
	ppa = post_proc_all(sos_pos,0,Sos);
	if (ppa == PROOF || ppa == TROUBLE)
		return(ppa);
#endif
	}
Penguin */

    if (giv_cl->container == Usable && Flags[DEMOD_INF].val) {
	sos_pos = Sos->last_cl;
	if (cl_copy(giv_cl,&c) == TROUBLE)
		return(TROUBLE);
	if (get_int_ptr(&ip) == TROUBLE)
		return(TROUBLE);
	ip->i = giv_cl->id;
	c->parents = ip;
	Stats[CL_GENERATED]++;
	if (c->bt == 0)
/* giv_cl is an input clause. c is just its copy: then we call pre_process() */
/* with the second parameter set to InputRead.				*/
		pp = pre_process(c,IR,Sos);
	else pp = pre_process(c,0,Sos);
	if (pp == PROOF || pp == TROUBLE)
		return(pp);
#ifndef ROO
	ppa = post_proc_all(sos_pos,0,Sos);
	if (ppa == PROOF || ppa == TROUBLE)
		return(ppa);
#endif
	}
return(NO_PROOF);
}  /* infer_and_process() */

/***************
*
*	int handle_for_sub(c,input,msg_go_to_usable)		Penguin only
*
*	It is called by proc_gen() to perform forward subsumption.
*	Just like proc_gen() itself, it returns 0 to mean the clause c
*	should be deleted, 1 if the clause should not be deleted,
*	TROUBLE in case of error.
*
***************/

int handle_for_sub(c,input,msg_go_to_usable)
struct clause *c;
int input, *msg_go_to_usable;
{
struct clause *e;
int msg_del;

e = NULL;					/* default */
msg_del = 0;					/* default */
*msg_go_to_usable = 0;				/* default */

/* The variable msg_del is passed to forward_subsume(). 		*/
/* If forward_subsume() returns the clause e and sets msg_del to 0, 	*/
/* e subsumes c as in the sequential case.				*/
/* If forward_subsume() returns the clause e and sets msg_del to 1,	*/
/* c subsumes e, because of distributed subsumption of variants.	*/
/* Accordingly, *msg_go_to_usable will be set to 1 if c subsumes e and	*/
/* e belongs to	Usable: in such a case c should not be added to Sos,	*/
/* but rather to Usable, since it just subsumed as a variant a clause	*/
/* which was already in Usable.						*/
/* The value of *msg_go_to_usable will be passed back to proc_gen()	*/
/* and hence to pre_process().						*/

	CLOCK_START(FOR_SUB_TIME)
	if (forward_subsume(c,input,&msg_del,&e) == TROUBLE)
		return(TROUBLE);
	CLOCK_STOP(FOR_SUB_TIME)

	if (e != NULL) {
		if (msg_del == 0)
		{
/* Penguin: just like in Otter, e subsumes c.				*/
	    if (Flags[VERY_VERBOSE].val)
		{
		fprintf(Fdout,"  subsumed by ");
		print_ids(Fdout,e);
		fprintf(Fdout,".\n");
		}
	    Stats[CL_FOR_SUB]++;
	    if (e->container == Sos)
		Stats[FOR_SUB_SOS]++;
	    if (e->id < 100)
		Subsume_count[e->id]++;
	    else
		Subsume_count[0]++;
	    return(0);
		}	/* end of msg_del == 0 */
		else if (msg_del == 1)
		{
/* Penguin: c subsumes e, because they are variants and c is smaller	*/
/* in the distributed subsumption ordering.				*/
/* It follows a piece of code taken from post_process(), because here	*/
/* c is back-subsuming e.						*/
	    if (e->container != Passive) {
		Stats[CL_BACK_SUB]++;
		if (Flags[PRINT_BACK_SUB].val)
			{			/* Penguin */
			print_ids(Fdout,c);
		    fprintf(Fdout," back subsumes as variant ");
			print_ids(Fdout,e);
			fprintf(Fdout,".\n");
			}
		if (e->container == Usable)
			*msg_go_to_usable = 1;
		if (un_index_rem_hide_deld(e) == TROUBLE)
			return(TROUBLE);
		}	/* end of not in Passive */
		}	/* end of msg_del == 1 */
		}	/* end of e != NULL */
return(1);
}	/* handle_for_sub() */

/*************
 *
 *    int proc_gen(c, ct, demod_flag_ptr, msg_go_ptr)
 *
 *    (ct == IR || ct == IM) -> input -> c is an input clause, and some tests
 *	should not be performed.
 *	else ct == 0.
 *
 *	Also, if ct == IM, even more checks are turned off, because they
 *	have been already done by the Penguin which read the input.
 *
 *    This routine takes a generated clause and:
 *        -.  (optional) print the clause
 *        -.  (optional) n_resolution check
 *        -.  (if order_eq || lex_order_vars) renumber vars
 *        -.  demodulates it (if any demoulators are present)
 *	  -.  if there is a smaller clause with same pid and lid, delete c
 *        -.  handle evaluable literals
 *        -.  (optional) flip equalities
 *        -.  merges identical literals
 *        -.  (optional) sorts literals
 *        -.  (optional & !input) max literals test
 *        -.  if tautology, delete c
 *        -.  (optional & !input) max weight test
 *        -.  (optional & !input) delete_identical_nested_skolems
 *        -.  (optional) forward subsumption
 *        -.  (optional) unit_deletion
 *        -.  renumber vars if not already done
 *        -.  (optional & !input) max_distinct_vars check
 *
 *    Return 0 if clause should be deleted, 1 otherwise,
 *	TROUBLE in case of error.
 *
 *
 *************/

int proc_gen(c, ct, demod_flag_ptr,msg_go_ptr)
struct clause *c;
int ct;
int *demod_flag_ptr, *msg_go_ptr;
{
    struct clause *e;
    int wt, i, already_renumbered;
	int input, tempint, modified, msg_go1, msg_go2, hfs;
	/* Variables added by Penguin.	*/

/* The variables msg_go1 and msg_go2 are passed to the two calls to	*/
/* handle_for_sub(). If either one is set to 1 by handle_for_sub(),	*/
/* *msg_go_ptr will be set to 1 when proc_gen() returns 1. The meaning	*/
/* of *msg_go_ptr set to 1 is that c should be appended to Usable,	*/
/* because it has subsumed as a variant a clause in Usable. This may	*/
/* apply only if proc_gen() returns 1, i.e. c is not to be deleted.	*/
/* If proc_gen() returns 0 and c should be deleted, *msg_go_ptr remains	*/
/* set to its default value of 0.					*/
/* The variable modified is to be passed to demod_cl()			*/
/* to know whether demod_cl() reduces the clause or not.		*/

	*msg_go_ptr = 0;		/* default */
	modified = 0;			/* default */
	msg_go1 = 0;			/* default */
	msg_go2 = 0; 			/* default */

	if (ct == IR || ct == IM)	/* Penguin */
		input = 1;
	else input = 0;

    if (Flags[VERY_VERBOSE].val) {
	fprintf(Fdout,"\n  ");
	CLOCK_START(PRINT_CL_TIME)
	print_clause(Fdout, c);
	CLOCK_STOP(PRINT_CL_TIME)
	}

    if (Flags[N_RESOLUTION].val && !n_resolution_check(c)) {
/* n_resolution() is defined in clause.c				*/
	Stats[CL_WT_DELETE]++;
	return(0);
	}

    if (Flags[ORDER_EQ].val || Flags[LEX_ORDER_VARS].val) {
	/* renumber variables now, else renumber after forward subsumption */
	if (renumber_vars(c) == 0) {
	    Stats[CL_VAR_DELETES]++;
	    print_clause(Fdout, c);
	    return(0);
	    }
	already_renumbered = 1;
	}
    else
	already_renumbered = 0;

if (!input && c->pid != Whoami && c->pid != MAX_INT && c->dest==ALL_PENGUINS)
/* Penguin: c is neither an input clause,				*/
/* nor a resident at this Penguin,					*/
/* nor a newly generated clause,					*/
/* nor a copy of an input clause read at another Penguin and brought	*/
/* here by an inference message;					*/
/* c is a non-input clause brought in by an inference message.		*/
/* update_db(c,lst) returns 1 if c has to be deleted because lst	*/
/* contains a smaller clause with same pid, same lid and more recent	*/
/* birth-time than c.							*/
/* On the other hand, if lst contains a clause d greater than c,	*/
/* with same pid, same lid, less recent than c, d is deleted by 	*/
/* update_db(c,lst).							*/
	{
	if (update_db(c,Usable,&e) == TROUBLE)
		return(TROUBLE);
	if (e == NULL)
		{
	if (update_db(c,Sos,&e) == TROUBLE)
		return(TROUBLE);
	if (e == NULL)
		{
	if (update_db(c,Demodulators,&e) == TROUBLE)
		return(TROUBLE);
	if (e == NULL)
		{
		if (update_db(c,Inbound_msgs,&e) == TROUBLE)
			return(TROUBLE);
		if (e == NULL)
			if (update_db(c,Outbound_msgs,&e) == TROUBLE)
				return(TROUBLE);
		}
		}
		}
	if (e != NULL)
		{
		if (Flags[PRINT_UPDATES].val)
			{
		fprintf(Fdout," deleted by ");
		print_clause(Fdout,e);
		fprintf(Fdout," smaller clause with same global id.\n");
			}
		return(0);
		}
	}	/* calls to update_db() */

/* We apply update_db() before the other contraction rules, especially before */
/* demodulation to prevent the following phenomenon: c is an equation l=r' */
/* from another Penguin. Also, c is the reduced form of an equation l=r, */
/* which is already at this Penguin. If we apply update first, l=r' eliminates*/
/* l=r. If we apply demodulation first, l=r demodulates l=r' to r=r' and then */
/* l=r is deleted by r=r' in update_db(). The result is unsound, as both l=r */
/* and its reduced form l=r' are lost. It is like simplifying the reduced form*/
/* of an equation by the equation itself! This could never happen in a 	*/
/* sequential computation, but in a distributed one it needs to be controlled.*/

if (Flags[FOR_SUB].val)
if (c->first_lit->atom->varnum==POS_EQ && !input && c->pid!=Whoami && c->pid!=MAX_INT)
	{	/* for_sub if c is an equation belonging to another Penguin */
	hfs = handle_for_sub(c,input,&msg_go1);
	if (hfs == 0 || hfs == TROUBLE)
		return(hfs);
	}
/* If c is an equation from another Penguin, forward subsumption needs to be */
/* applied before demodulation, because if c is a variant of a resident, we  */
/* need to eliminate the older variant based on birth times. If we apply     */
/* demodulation first, since demodulation in this implementation applies also*/
/* to variants, c would be demodulated by the resident even if c is older than*/
/* the resident. Elimination of variants based on birth times is implemented */
/* in forward and backward subsumption, not in demodulation.		     */

if (ct!=IM && Demodulators->first_cl != NULL || Internal_flags[DOLLAR_PRESENT])
/* ct != IM: the Penguin which read the input did not send a clause which */
/* can be simplified by another input clause.				*/
	{
	CLOCK_START(DEMOD_TIME)
	if (demod_cl(c,&modified) == TROUBLE)
		return(TROUBLE);
/* It sets modified to 1 if the clause is reduced, 0 otherwise.		*/
	CLOCK_STOP(DEMOD_TIME)
	if (Flags[VERY_VERBOSE].val) {
	    fprintf(Fdout,"  after demodulation: ");
	    CLOCK_START(PRINT_CL_TIME)
            print_clause(Fdout, c);
            CLOCK_STOP(PRINT_CL_TIME)
	    }
	/* (variables might no longer start x,y,z,... .) */
	}

    /* False lits of c may be deleted even if test fails. */
    if (Internal_flags[DOLLAR_PRESENT] && lit_t_f_reduce(c)) {
	Stats[CL_TAUTOLOGY]++;
	return(0);
	}

    if (Flags[ORDER_EQ].val) {
	if (Flags[LEX_RPO].val)
	{
	    if (order_equalities_lrpo(c, &tempint) == TROUBLE)
		return(TROUBLE);
		*demod_flag_ptr = tempint;
	}
	else
	    *demod_flag_ptr = order_equalities(c);
	/* demod_flag is set if c is an eq unit that should be a demodulator */
	/* 0->no, 1->yes, 2->lex_dep */
	/* (variables might no longer start x,y,z,... .) */
	}
    else
	*demod_flag_ptr = 0;

    /* Unit deletion could be placed here, but it seems to work better */
    /* when delayed until after forward subsumption. */

	if (ct != IM)
/* The Penguin which read the input did not send a clause with identical */
/* literals.								*/
    cl_merge(c);        /* merge identical literals */

    if (Flags[SORT_LITERALS].val)
	sort_lits(c);

if (!input && Parms[MAX_LITERALS].val != 0) {
	if (num_literals(c) > Parms[MAX_LITERALS].val) {
	    Stats[CL_WT_DELETE]++;
	    return(0);
	    }
	}

	if (ct != IM)
/* The Penguin which read the input did not send a tautology. */
    if (tautology(c)) {
	Stats[CL_TAUTOLOGY]++;
	return(0);
	}

if (!input && Parms[MAX_WEIGHT].val != 0) {
	CLOCK_START(WEIGH_CL_TIME)
	wt = weight_cl(c, Weight_purge_gen_index);
	CLOCK_STOP(WEIGH_CL_TIME)
	if (wt > Parms[MAX_WEIGHT].val) {
            if (Flags[VERY_VERBOSE].val)
		fprintf(Fdout,"  deleted because weight=%d.\n", wt);
	    Stats[CL_WT_DELETE]++;
	    return(0);
	    }
	}

if (!input && Flags[DELETE_IDENTICAL_NESTED_SKOLEM].val) {
	if (ident_nested_skolems(c,&tempint) == TROUBLE)
		return(TROUBLE);
	if (tempint == 1)
		{
	    Stats[CL_WT_DELETE]++;
	    return(0);
	    }
	}

if (Flags[FOR_SUB].val && ct != IM)
if (c->first_lit->atom->varnum!=POS_EQ || ct==IR || c->pid==Whoami || c->pid==MAX_INT || modified)
{ /* for_sub if c is not an equation or it is an input equation read here or */
/* a resident equation or a raw critical pair or none of these hold, i.e. */
/* c is an equation belonging to another Penguin, but it has been modified by */
/* demodulation (modified == 1) and thus the for_sub test done before	*/
/* demodulation need to be repeated.					*/
/* We exclude ct == IM, since the Penguin which read the input did 	*/
/* not send a clause which can be subsumed by another input clause.	*/

	hfs = handle_for_sub(c,input,&msg_go2);
	if (hfs == 0 || hfs == TROUBLE)
		return(hfs);
}

    if (ct != IM && Flags[UNIT_DELETION].val && num_literals(c) > 1) {
/* ct != IM: unit_del() already done by the Penguin which read the input. */
	CLOCK_START(UNIT_DEL_TIME)
        if (unit_del(c, &i) == TROUBLE)
		return(TROUBLE);
	CLOCK_STOP(UNIT_DEL_TIME)
	/* If any unit deletion occurred, and it's now an equality unit,  */
	/* and dynamic_demod is set, call order_equalities (again) to     */
	/* see if it should be a demodulator. */
	if (i && Flags[DYNAMIC_DEMOD].val && num_literals(c) == 1 &&
	                       c->first_lit->atom->varnum == POS_EQ) {
	    if (Flags[LEX_RPO].val)
		{
		if (order_equalities_lrpo(c,&tempint) == TROUBLE)
			return(TROUBLE);	/* it sets demod_flag_ptr */
		*demod_flag_ptr = tempint;
		}
	    else
		*demod_flag_ptr = order_equalities(c);
	    }
	
	}	/* end of UNIT DELETION */

    if (already_renumbered == 0 && renumber_vars(c) == 0) {
	Stats[CL_VAR_DELETES]++;
	print_clause(Fdout, c);
	return(0);
	}

if (!input && Parms[MAX_DISTINCT_VARS].val != -1)
	{
	/* this is here because vars must be renumbered */
	if (distinct_vars(c) > Parms[MAX_DISTINCT_VARS].val) {
	    Stats[CL_WT_DELETE]++;
	    return(0);
	    }
	}

if (msg_go1 == 1 || msg_go2 == 1)
	*msg_go_ptr = 1;

    return(1);
}  /* proc_gen() */

/*************
 *
 *    int pre_process(c, ct, lst)
 *	lst is either Usable (only in the input phase) or Sos.
 *	ct is either I(nput)R(ead) clause for an input clause read at this
 *	Penguin, or I(nput)M(essage) for an input clause brought in by a 
 *	message, or 0 for any other clause.
 *	It is void in Otter, int in Penguin.
 *	It returns PROOF if check_for_proof() invoked within finds a proof,
 *	TROUBLE if an error occurs,
 *	NO_PROOF otherwise.
 *
 *************/

int pre_process(c, ct, lst)
struct clause *c;
int ct;
struct list *lst;
{
    int i, demod_flag, msg_go;
	int ec, mp;

/* Penguin adds the variable msg_go to be passed to proc_gen().		*/
/* proc_gen() sets it to 1 only if, in case c survives, c has to be added */
/* to Usable, regardless to what lst is.				*/
/* This situation arises only if c is a clause brought in by an inference */
/* message which has subsumed as a variant a clause already in Usable.	*/
/* Penguin also adds the variables mp and ec simply as short hands for */
/* Stats[EMPTY_CLAUSES] and Parms[MAX_PROOFS].val.		*/

    CLOCK_START(PRE_PROC_TIME)
    i = proc_gen(c, ct, &demod_flag, &msg_go);
/* proc_gen() returns 0 if c should be deleted.				*/

    if (i == TROUBLE)
	return(TROUBLE);
    else if (i == 0) {
	cl_del_non(c);
	CLOCK_STOP(PRE_PROC_TIME)
	return(NO_PROOF);
	}
#ifdef ROO

    /* The following kludge is so that Task b knows where to put kept clause. */

    if (lst == Usable)
	c->id = -1;
    else if (lst == Sos)
	c->id = -2;
    else
	c->id = 0;

    store_in_k(c);

#else  /* not ROO */

if (ct == IR && lst == Usable)
	{
    if (cl_integrate(c,IR,IN_ALL_PENGUINS_U) == TROUBLE)
	return(TROUBLE);
	}
else if (ct == IR && lst == Sos)
	{
    if (cl_integrate(c,IR,IN_ALL_PENGUINS_S) == TROUBLE)
	return(TROUBLE);
	}
else if (ct == IM)
	{
    if (cl_integrate(c,IM,NONE) == TROUBLE)
	return(TROUBLE);
	}
else  {
	if (cl_integrate(c,0,NONE) == TROUBLE)
		return(TROUBLE);
	}
/* cl_integrate() sets c->pid by calling decide_allocation(), c->lid	*/
/* and c->dest. 							*/
/* The third parameter is used by cl_integrate() to set c->dest for	*/
/* (IR) input clauses.							*/

if (ct==IR || ct==IM || c->pid==Whoami || c->dest==ALL_PENGUINS || Flags[POST_PROC_NS_BEFORE_SEND].val == 1)
/* ct == IR: c is an input clause read at this Penguin,			*/
/* ct == IM: c is an input clause arrived				*/
/* as message from the Penguin which read the input,			*/
/* c->pid == Whoami: c is either a new settler settling down at this Penguin, */
/* or a resident of this Penguin which has been reduced.		 */
/* c->dest == ALL_PENGUINS: c is an inference message from another Penguin. */
/* If all these conditions fail, c is a new settler destinated to another */
/* Penguin. However, if Flags[POST_PROC_NS_BEFORE_SEND] is on, we keep it */
/* temporarily, to post_process it and it will be sent in the main loop. */
	{	/* keep the clause */
    CLOCK_START(KEEP_CL_TIME)
    if (index_lits_all(c) == TROUBLE)
	return(TROUBLE);
    if (lst == Usable || msg_go == 1)
/* c is appended to Usable if:						*/
/* either lst is Usable							*/
/* or msg_go has been set to 1 by proc_gen(), meaning c is an inference	*/
/* message which has subsumed as a variant a clause in Usable.		*/
	{
	if (index_lits_clash(c) == TROUBLE)
		return(TROUBLE);
	append_cl(Usable,c);
	Stats[USABLE_SIZE]++;			/* Penguin */
	}
    else if (lst == Sos)
	{
	append_cl(Sos,c);
	Stats[SOS_SIZE]++;
	}
/* pre_process() is called with lst == Usable || lst == Sos		*/
    c->weight = weight_cl(c, Weight_pick_given_index);
    Stats[CL_KEPT]++;
    CLOCK_STOP(KEEP_CL_TIME)

    if (ct == IR || ct == IM || Flags[PRINT_KEPT].val) {
	fprintf(Fdout,"** KEPT: ");
	CLOCK_START(PRINT_CL_TIME)
	print_clause(Fdout, c);
	CLOCK_STOP(PRINT_CL_TIME)
	}

    if (num_literals(c) == 1 && c->first_lit->atom->varnum == POS_EQ)
	{	/* if positive equation */
	if ((ct == IR || Flags[PRINT_NEW_DEMOD].val) &&
	       Flags[DYNAMIC_DEMOD_ALL].val && demod_flag == 0) {
	    fprintf(Fdout,"++++ cannot make into demodulator: ");
	    print_clause(Fdout, c);
	    }

	if (Flags[NEW_FUNCTIONS].val && c->pid == Whoami)
/* Each Penguin introduces a new function (i.e. ``splits'' the equation c */
/* into two new equations) only for its residents. This is ``splitting''  */
/* in the Knuth-Bendix sense. Since it is an expansion rule, it is	*/
/* restricted based on ownership.					*/
		{
	    i = new_function(c, lst);
		if (i == PROOF || i == TROUBLE)
			return(i);
		}
	else
	    i = 0;

	/* don't try to make into demod if new function introduced */

	if (demod_flag != 0 && Flags[DYNAMIC_DEMOD].val && i == 0)
	    /* Make sure that there are no calls to cl_integrate() */
	    /* between KEEP and here, because id of dynamic demodulator */
            /* must be 1 more than the id of the KEPT copy.             */
	    if (new_demod(c, demod_flag) == TROUBLE)
		return(TROUBLE);
	}	/* if positive equation */
	}	/* keep the clause */

if (Flags[POST_PROC_NS_BEFORE_SEND].val == 0)
if (ct == IR || (c->pid != Whoami && c->dest == c->pid))
/* Either input clause or new settler destinated to another Penguin.	*/
	if (send_clause(c) == TROUBLE)
		return(TROUBLE);

    if (check_for_proof(c) == TROUBLE)
	return(TROUBLE);

/* The effect of check_for_proof() is to increase Stats[EMPTY_CLAUSES] if */
/* an empty clause is found.						*/

ec = Stats[EMPTY_CLAUSES];
mp = Parms[MAX_PROOFS].val;

	    if (mp == 1)
		{
		if (ec >= 1)
			/* do not look for more proofs */
			return(PROOF);
		}
	    else if (mp > 1)
		{
		if (ex_ans_target(Passive))
			{
			if (ec >= mp && all_solved_ans_target(Passive))
				return(PROOF);
			/* do not look for more proofs */
			}
		else
			{
			if (ec >= mp)
			/* do not look for more proofs */
				return(PROOF);
			}
		}
/* Otter has cleanup() and exit(0) here. Penguin exits gracefully returning */
/* PROOF and cleanup() will be done at the end of the main loop.	*/
/* The default for Parms[MAX_PROOFS].val is 1, so that we are seeking one */
/* proof for one theorem. If it is set to 0, the prover keeps running	*/
/* even after generating proofs and halts by other parameters, such as it */
/* it hits the maximum number of given clauses. If it is set to n =/= 0, */
/* it seeks n proofs; the latter options is used to prove several theorems */
/* in one run.								*/
/* See pheader.h for explanation of why the case Parms[MAX_PROOFS].val>1 */
/* is treated in Penguin differently than in Otter.			*/

#endif

    CLOCK_STOP(PRE_PROC_TIME)
return(NO_PROOF);
}  /* pre_process() */

/**************
*
*	int update_db(c,lst,ee)		Penguin
*
*	It returns TROUBLE/NO_TROUBLE.
*	If the list lst contains a clause p with the same
*	pid and lid as c, p->bt >= c->bt and p < c, it sets *ee to p.
*	It set *ee to NULL otherwise.	
*	If the ids are the same, but p->bt <= c->bt and p > c, p is deleted.
*
***************/

int update_db(c,lst,ee)
struct clause *c;
struct list *lst;
struct clause **ee;
{
struct clause *p;
int tempint, tempint2;

*ee = NULL; 			/* default */
p = lst->first_cl;

while (p != NULL)
	{
	if (p->pid == c->pid && p->lid == c->lid)
		{
		if (p->bt >= c->bt)
		{	/* p more recent than c */
		if (greater_cl(c,p,&tempint) == TROUBLE)
			return(TROUBLE);
		if (tempint)	/* c > p */
/* It checks only for c > p or p > c. If c and p are variants, one of them */
/* will be deleted by subsumption. If they are uncomparable without being  */
/* variants, we keep them both, at least for now.			   */
			{
			*ee = p;
			return(NO_TROUBLE);
			}
		}
		if (p->bt <= c->bt)
		{	/* c more recent than p */
		if (greater_cl(p,c,&tempint2) == TROUBLE)
			return(TROUBLE);
		if (tempint2)	/* p > c */
			{
		if (Flags[PRINT_UPDATES].val)
			{
		print_clause(Fdout,c);
		fprintf(Fdout," deletes ");
		print_clause(Fdout,p);
		fprintf(Fdout," greater clause with same global id.\n");
			}
			if (p->container == Inbound_msgs)
				{
				hide_msg(p);
				Stats[INBOUND_MSGS_SIZE]--;
				}
			else if (p->container == Outbound_msgs)
				{
				hide_msg(p);
				Stats[OUTBOUND_MSGS_SIZE]--;
				}
			else { /* Usable || Sos || Demodulators */
			if (un_index_rem_hide_deld(p) == TROUBLE)
				return(TROUBLE);
			      } /* end of Usable || Sos || Demodulators */
			} /* end of p > c */
			} /* end of c more recent than p */
		} /* end of if ids are equal */
	p = p->next_cl;
	} /* end of while */
return(NO_TROUBLE);
} /* update_db() */

/*******************
*
*	void decide_allocation(c,input,imt)			Penguin only
*
********************/

void decide_allocation(c,input,imt)
struct clause *c;
int input, imt;
{
int candidate, par_id, factor_of_resident, nfr_of_resident;
struct clause *p;

if (Flags[STAND_ALONE].val)
	c->pid = Whoami;

else if (is_other_penguin_up() == 0)
	c->pid = Whoami;
/* If no other Penguin is up, the clause stays here.			*/

else if (reflexivity(c))
	c->pid = Whoami;
/* Each Penguin owns its copy of x = x.					*/

else if (eq_target(c))
	c->pid = Whoami;
/* Equational target belong to the Penguin where they are.		*/
/* Equational targets are not sent as messages, except for the input 	*/
/* equational targets, they sit at the Penguins waiting to be simplified */
/* and paramodulated into by positive equations.			*/
/* See eq_target() and target() in clause.c for the notions of target.	*/

else if (Flags[OWN_IN_USABLE].val && input && imt == IN_ALL_PENGUINS_U)
	c->pid = Whoami;
/* If Flags[OWN_IN_USABLE] is on, each Penguin owns the input clauses in */
/* Usable.								*/

else if (Flags[OWN_IN_SOS].val && input && imt == IN_ALL_PENGUINS_S)
	c->pid = Whoami;
/* If Flags[OWN_IN_SOS] is on, each Penguin owns the input clauses in Sos. */

else {	/* non-trivial */

if (Flags[FIRST_FIT].val || Flags[ALT_FIRST_FIT].val)
	if (Penguins[Whoami] >= Parms[MAX_SC_W_ALT].val)
		{	/* change flags */
		Flags[ALTERNATE_FIT].val = 1;

if (Flags[FIRST_FIT].val)
	{
		Flags[FIRST_FIT].val = 0;
fprintf(Fdout,"Penguin clears first_fit and sets alternate_fit,\n");
	}
else if (Flags[ALT_FIRST_FIT].val)
	{
		Flags[ALT_FIRST_FIT].val = 0;
fprintf(Fdout,"Penguin clears alt_first_fit and sets alternate_fit,\n");
	}

fprintf(Fdout,"because the number of clauses settled at this Penguin has\n");
fprintf(Fdout,"hit the threshold MAX_SettledClauses_Without_ALTernate.\n");

/* This mechanism can be kept off by setting Parms[MAX_SC_W_ALT].val to	*/
/* MAX_INT.								*/
		}	/* end of change flags */

factor_of_resident = 0;				/* default */
nfr_of_resident = 0;				/* default */

if (Flags[FACTOR].val && Flags[OWN_FACTORS].val && c->parents!=NULL && c->parents->i==FACTOR_RULE)
	{
	par_id = c->parents->next->i;
	p = cl_find(par_id);
	if (p->pid == Whoami)
		factor_of_resident = 1;
	}	/* end of special treatment of factors */

if (Flags[NEW_FUNCTIONS].val && Flags[OWN_NFR].val && c->parents!=NULL && c->parents->i==NEW_FUNCTION_RULE)
	{
	par_id = c->parents->next->i;
	p = cl_find(par_id);
	if (p->pid == Whoami)
		nfr_of_resident = 1;
	}
/* end of special treatment of equations generated by new_function_rule, */
/* i.e. KB type of ``splitting ''.					 */

if (factor_of_resident || nfr_of_resident)
	c->pid = Whoami;

else if (Flags[FIRST_FIT].val || (Flags[ALT_FIRST_FIT].val && !input))
	c->pid = Whoami;

else if (Flags[ALTERNATE_FIT].val || (Flags[ALT_FIRST_FIT].val && input))
{
if (Last_choice == MAX_INT)
	c->pid = Whoami;
/* The first choice is the Penguin itself.				*/
else
	{
	candidate = (Last_choice + 1) % No_of_nodes;
	while (Penguins[candidate] == 0)
		candidate = (candidate + 1) % No_of_nodes;
	c->pid = candidate;
	}

}	/* end if Flags[ALTERNATE_FIT].val */

else if (Flags[HALF_ALT_FIT].val)
{
if (Last_choice != Whoami)	/* includes the first choice also */
	c->pid = Whoami;
/* It alternates between itself and others.				*/
else
	{
	candidate = (Before_last_choice + 1) % No_of_nodes;
	while (Penguins[candidate] == 0)
		candidate = (candidate + 1) % No_of_nodes;
	c->pid = candidate;
	}

}	/* end of if Flags[HALF_ALT_FIT].val */

if (Last_choice == MAX_INT)
	Before_last_choice = c->pid;
else Before_last_choice = Last_choice;
Last_choice = c->pid;
/* Before_last_choice and Last_choice are updated only if the choice has */
/* not been trivial, i.e. neither c is reflexivity nor c is a target 	*/
/* equation nor no other Penguin is up.			 		*/

}	/* end of else non-trivial */
}	/* decide_allocation() */
