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
 *  resolve.c -- Resolution inference rules.
 *
 */

#include "header.h"

/*************
 *
 *    int build_hyper(clash,nuc_subst,nuc_lits,nuc,
 *                             giv_subst,giv_lits,giv_sat,nuc_pos,bh)
 *
 *    This routine constructs a hyperresolvent or UR-resolvent.
 *
 *	struct clause * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct clause through the
 *	parameter bh.
 *
 *************/

static build_hyper(cla,nuc_subst,nuc_lits,nuc,giv_subst,giv_lits,giv_sat,nuc_pos,bh)
struct clash_nd *cla;
struct context *nuc_subst;
struct literal *nuc_lits;
struct clause *nuc;
struct context *giv_subst;
struct literal *giv_lits;
struct clause *giv_sat;
int nuc_pos;
struct clause **bh;
{
    struct clause *res, *sat;
    struct literal *lit, *new, *prev;
    struct clash_nd *c;
    struct int_ptr *ip1, *ip2, *ip3;
    int i;
	struct term *temp;

	*bh = NULL;				/* default */
    if (get_clause(&res) == TROUBLE)
	return(TROUBLE);

    if (get_int_ptr(&ip1) == TROUBLE)
	return(TROUBLE);
/* to be filled in by caller with name of inference rule */
    res->parents = ip1;
    /* If given clause is satellite, add number to parent list. */
    if (giv_sat != NULL) {
	if (get_int_ptr(&ip2) == TROUBLE)
		return(TROUBLE);
	ip2->i = giv_sat->id;
	if (Flags[ORDER_HISTORY].val && nuc_pos != 0)
	    /* insert later in correct position */
	    ip3 = ip2;
	else {
	    ip3 = NULL;
	    ip1->next = ip2;
	    ip1 = ip2;
	    }
	}

    if (get_int_ptr(&ip2) == TROUBLE)
	return(TROUBLE);
    ip2->i = nuc->id;
    ip1->next = ip2;

    lit = giv_lits;
    prev = NULL;
    while (lit != NULL) {
	if (get_literal(&new) == TROUBLE)
		return(TROUBLE);
	new->container = res;
	if (prev == NULL)
	    res->first_lit = new;
	else
	    prev->next_lit = new;
	prev = new;
	new->sign = lit->sign;
	if (apply(lit->atom, giv_subst, &temp) == TROUBLE)
		return(TROUBLE);
		new->atom = temp;
	new->atom->occ.lit = new;
	new->atom->varnum = lit->atom->varnum;  /* copy type of atom */
	lit = lit->next_lit;
	}

    lit = nuc_lits;
    while (lit != NULL) {
	if (get_literal(&new) == TROUBLE)
		return(TROUBLE);
	new->container = res;
	if (res->first_lit == NULL)
	    res->first_lit = new;
	else
	    prev->next_lit = new;
	prev = new;
	new->sign = lit->sign;
	if (apply(lit->atom, nuc_subst, &temp) == TROUBLE)
		return(TROUBLE);
	new->atom = temp;
	new->atom->occ.lit = new;
	new->atom->varnum = lit->atom->varnum;  /* copy type of atom */
	lit = lit->next_lit;
	}

    c = cla;
    i = 1;
    while (c != NULL) {
	if (ip3 != NULL && i == nuc_pos) {
	    /* insert given clause (which is satellite) number here */
	    ip2->next = ip3;
	    ip2 = ip3;
	    }
	if (get_int_ptr(&ip1) == TROUBLE)
		return(TROUBLE);
	ip2->next = ip1;
	ip2 = ip1;
	if (c->evaluable)
	    ip1->i = EVAL_RULE;
	else {
	    sat = c->found_atom->occ.lit->container;
	    ip1->i = sat->id;
	    lit = sat->first_lit;
	    while (lit != NULL) {
		if (lit->atom != c->found_atom) {
		    if (get_literal(&new) == TROUBLE)
			return(TROUBLE);
		    new->container = res;
		    if (res->first_lit == NULL)
			res->first_lit = new;
		    else
			prev->next_lit = new;
		    prev = new;
		    new->sign = lit->sign;
		    if (apply(lit->atom, c->subst, &temp) == TROUBLE)
			return(TROUBLE);
			new->atom = temp;
		    new->atom->occ.lit = new;
		    new->atom->varnum = lit->atom->varnum;  /* type of atom */
		    }
		lit = lit->next_lit;
		}
	    }
	i++;
	c = c->next;
	}
    
    if (ip3 != NULL && i == nuc_pos) {
	/* insert given clause (which is satellite) number here */
	ip2->next = ip3;
	ip2 = ip3;
	}
	*bh = res;
    return(NO_TROUBLE);

}  /* build_hyper */

/*************
 *
 *    int clash(c_start, nuc_subst, nuc_lits, nuc, giv_subst, giv_lits, giv_sat,
 *                  sat_proc, inf_clock, nuc_pos)
 *
 *    This routine is called by both hyper and UR to clash away the
 *    marked literals of the given nucleus, and append kept resolvents
 *    to Sos.
 *
 *    c_start:    Start of the clash_structure list.  There is one node
 *                for each literal that is to be clashed away.
 *    nuc_subst:  Substitution for the nucleus.
 *    nuc_lits:   Non-clashed literals of the nucleus.
 *    nuc:        The nucleus.
 *    giv_subst:  If the given clause is a satellite, then this is its
 *                substitution; else NULL.
 *    giv_lits:   If the given clause is a satellite, then these are its
 *                non-clashed literals; else NULL.
 *    giv_sat:    If the given clause is a satellite, then this is it;
 *                else NULL.
 *    sat_proc:   procedure to identify (other) satellites:  `pos_clause'
 *                for hyper, `unit_clause' for UR.
 *    inf_clock:  Clock (HYPER_TIME or UR_TIME) to be turned off during
 *                call to `pre_process'.
 *    nuc_pos:    If not 0, giv cl is sat, and nuc_pos gives position
 *                of "missing" clash node.  To construct history.
 *
 * It is void in Otter, int in Penguin, as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

static int clash(c_start,nuc_subst,nuc_lits,nuc,giv_subst,giv_lits,giv_sat,sat_proc,inf_clock,nuc_pos)
     struct clash_nd *c_start;
struct context *nuc_subst;
struct literal *nuc_lits;
struct clause *nuc;
struct context *giv_subst;
struct literal *giv_lits;
struct clause *giv_sat;
int (*sat_proc)();
int inf_clock;
int nuc_pos;
{
    struct clash_nd *c, *c_end;
    int found, backup, fpa_depth, sign;
    struct term *f_atom, *nuc_atom_instance;
    struct trail *tr;
    struct clause *res;
    char *s;
	int uok;
	int pp;
	struct fpa_tree *temp;

	pp = NO_PROOF;				/* default */
    fpa_depth = Parms[FPA_LITERALS].val;
    c = NULL;
    backup = 0;

    while (1) {  /* return from within loop */
	if (backup == 0) {
	    if (c_start == NULL || (c != NULL && c->next == NULL)) {
		/* clash is complete */
		if (build_hyper(c_start, nuc_subst, nuc_lits, nuc,
			giv_subst, giv_lits, giv_sat, nuc_pos, &res) == TROUBLE)
			return(TROUBLE);
		if (inf_clock == HYPER_TIME)
		    res->parents->i = HYPER_RES_RULE;
		else if (inf_clock == NEG_HYPER_TIME)
		    res->parents->i = NEG_HYPER_RES_RULE;
		else
		    res->parents->i = UR_RES_RULE;
		Stats[CL_GENERATED]++;
		CLOCK_STOP(inf_clock)
		pp = pre_process(res, 0, Sos);
		if (pp == PROOF || pp == TROUBLE)
			return(pp);
		CLOCK_START(inf_clock)
		backup = 1;
		c_end = c;
		c = NULL;
		}	/* end of if clash is complete */
	    else {	/* else clash is not complete */
		if (c == NULL)   /* just starting */
		    c = c_start;
		else
		    c = c->next;
		
	if (apply(c->nuc_atom, nuc_subst, &nuc_atom_instance) == TROUBLE)
		return(TROUBLE);
		if (c->evaluable) {
		    /* evaluate, but don't take any action yet */
if (convenient_demod(nuc_atom_instance,&nuc_atom_instance) == TROUBLE)
			return(TROUBLE);
		    s = sn_to_str(nuc_atom_instance->sym_num);
		    sign = c->nuc_atom->occ.lit->sign;
		    if (sign)
			c->evaluation = str_ident(s, "$F");
		    else
			c->evaluation = str_ident(s,"$T");
		    c->already_evaluated = 0;
		    }	/* end of if evaluable */
		else {  /* not evaluable */
	if (build_tree(nuc_atom_instance,UNIFY,fpa_depth,c->db,&temp)==TROUBLE)
			return(TROUBLE);
			c->u_tree = temp;
		    }	/* end of else not evaluable */
		
		zap_term(nuc_atom_instance);
		}	/* end of else clash is not complete */
	    }	/* end of if backup == 0 */
	else {  /* backup */
	    if (c_start == NULL ||
		(c != NULL && c->prev == NULL))   /* done with this nucleus */
		return(NO_PROOF);
	    else {
		if (c == NULL)
		    c = c_end;
		else
		    c = c->prev;
		if (!c->evaluable)
		    clear_subst_1(c->tr);
		backup = 0;
		}
	    }	/* end of else backup */

	if (backup == 0) {
	    found = 0;
	    if (c->evaluable) {
		if (c->already_evaluated || !c->evaluation)
		    backup = 1;
		else
		    /* Set flag and proceed. */
		    c->already_evaluated = 1;
		}	/* end of if evaluable */
	    else {	/* else not evaluable */
		f_atom = next_term(c->u_tree, 0);
		tr = NULL;
		while (f_atom != NULL && found == 0) {
#ifdef ROO
		    if ((!giv_sat || giv_sat->giv_cl_seq_no >=
			 f_atom->occ.lit->container->giv_cl_seq_no) &&
			(*sat_proc)(f_atom->occ.lit->container) &&
			
			unify(c->nuc_atom,nuc_subst,f_atom,c->subst,&tr)) {
			found = 1;
			}
#else
		    if ((*sat_proc)(f_atom->occ.lit->container))
			{	/* if pre-conditions to try unify() */
	if (unify(c->nuc_atom,nuc_subst,f_atom,c->subst,&tr,&uok) == TROUBLE)
		return(TROUBLE);
			if (uok == 1)
		       		 found = 1;
			else f_atom = next_term(c->u_tree,0);
		        
			}	/* end of if pre-conditions */
#endif	    
		    else
		        f_atom = next_term(c->u_tree, 0);
		}	/* end of while looping on f_atom or until found */
		if (found) {
		    c->found_atom = f_atom;
		    c->tr = tr;
		    }
		else {
		    backup = 1;
		    }
		}	/* end of else not evaluable */
	    }	/* end of if backup == 0 */
	}  /* end of while(1) */
}  /* clash */

/*************
 *
 *    int hyper_res(c) -- hyperresolution
 *
 *    Append kept resolvents to Sos.  Each kept
 *    clause has already passed the pre_process filter (forward
 *    subsumption, etc.), been integrated, and inserted into
 *    appropriate indexes.
 *
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

int hyper_res(giv_cl)
struct clause *giv_cl;
{
    struct literal *lit, *nuc_lits, *giv_lits, *l1, *l2, *l3;
    struct context *nuc_subst, *giv_subst;
    struct clash_nd *clash_list, *c1, *c2;
    struct clause *nuc;
    int m, i, nuc_pos;
    struct term *f_atom;
    struct fpa_tree *ut;
    struct trail *tr;
	int uok;
	int ch;
	struct context *temp;

	ch = NO_PROOF;				/* default */
    CLOCK_START(HYPER_TIME)
    if (num_literals(giv_cl) == 0) {
	CLOCK_STOP(HYPER_TIME)
	return(NO_PROOF);
	}
    else if (!pos_clause(giv_cl)) { /* given clause is nucleus */
	if (giv_cl->pid == Whoami)
/* Penguin: only residents can be nuclei.		*/
	{
	clash_list = NULL;
	nuc_lits = NULL;
	if (get_context(&nuc_subst) == TROUBLE)
		return(TROUBLE);
	nuc_subst->multiplier = 0;
	m = 1;
	lit = giv_cl->first_lit;
	l2 = NULL; c2 = NULL;  /* to quiet lint */
	while (lit != NULL) {
	    /* positive literal || answer literal */
	    if (lit->sign || lit->atom->varnum == ANSWER) {
		if (get_literal(&l1) == TROUBLE)
			return(TROUBLE);
		if (nuc_lits == NULL)
		    nuc_lits = l1;
		else
		    l2->next_lit = l1;
		l2 = l1;
		l1->sign = lit->sign;
		l1->atom = lit->atom;
		}	/* end of positive literal or answer literal */
	    else {            /* put negative literal into clash structure */
		if (get_clash_nd(&c1) == TROUBLE)
			return(TROUBLE);
		if (clash_list == NULL)
		    clash_list = c1;
		else {
		    c2->next = c1;
		    c1->prev = c2;
		    }
		c2 = c1;
		c2->db = Fpa_clash_pos_lits;
		if (get_context(&temp) == TROUBLE)
			return(TROUBLE);
		c2->subst = temp;
		c2->subst->multiplier = m++;
		c2->nuc_atom = lit->atom;
		c2->evaluable = (lit->atom->varnum == EVALUABLE);
		} /* end of else put negative literal into clash structure */
	    lit = lit->next_lit;
	    }	/* end of while on literals of giv_cl */
	ch = clash(clash_list, nuc_subst, nuc_lits, giv_cl,
		  (struct context *) NULL, (struct literal *) NULL,
		  (struct clause *) NULL,
		  pos_clause, HYPER_TIME, 0);
		if (ch == PROOF || ch == TROUBLE)
			return(ch);
	c1 = clash_list;
	while (c1 != NULL) {
	    free_context(c1->subst);
	    c2 = c1;
	    c1 = c1->next;
	    free_clash_nd(c2);
	    }
	l1 = nuc_lits;
	while (l1 != NULL) {
	    l2 = l1;
	    l1 = l1->next_lit;
	    free_literal(l2);
	    }
	free_context(nuc_subst);
	CLOCK_STOP(HYPER_TIME)
	return(NO_PROOF);
	}	/* end of if added by Penguin */
	}	/* end of if given clause is nucleus */
    else {  /* given clause is satellite (positive) */
	if (get_context(&giv_subst) == TROUBLE)
		return(TROUBLE);
	/* substitution for given satellite */
	giv_subst->multiplier = 0;
	if (get_context(&nuc_subst) == TROUBLE)
		return(TROUBLE);
	/* substitution for nucleus */
	nuc_subst->multiplier = 1;
	l3 = giv_cl->first_lit;
	while (l3 != NULL) {  /* for each literal in given satellite */
	    /* collect non-clashed lits (including answers) of given sat*/
	    giv_lits = NULL;
	    lit = giv_cl->first_lit;
	    while (lit != NULL) {
		if (lit != l3) {
		    if (get_literal(&l1) == TROUBLE)
			return(TROUBLE);
		    if (giv_lits == NULL)
			giv_lits = l1;
		    else
			l2->next_lit = l1;
		    l2 = l1;
		    l1->sign = lit->sign;
		    l1->atom = lit->atom;
		    }
		lit = lit->next_lit;
		}	/* end of while looping on literals of giv_cl */
	    if (build_tree(l3->atom,UNIFY,
		Parms[FPA_LITERALS].val,Fpa_clash_neg_lits,&ut) == TROUBLE)
			return(TROUBLE);
	    f_atom = next_term(ut, 0);
	    while (f_atom != NULL) {  /* for each potential nucleus */
		tr = NULL;
		nuc = f_atom->occ.lit->container;
		if (nuc->pid == Whoami)
/* Penguin: only a resident can be a nucleus.				*/
		{
		if (!pos_clause(nuc))
		{	/* if pre-condition to try unify */
	if (unify(l3->atom, giv_subst, f_atom, nuc_subst, &tr, &uok) == TROUBLE)
		return(TROUBLE);
		if (uok)
		{ /* we have a nucleus */

		    /* there are three kinds of literal in the nucleus:    */
		    /*    1. the clashed literal -> do nothing             */
		    /*    2. positive or answer literals -> collect them   */
		    /*    3. negative literals -> put into clash structure */

		    nuc_lits = NULL;
		    clash_list = NULL;
		    m = 2;  /* multipliers for found sats start with 2 */
		    lit = nuc->first_lit;
		    i = 1;
	/* Find index of clausable lit that sat clahes with. */
		    while (lit != NULL) {
			if (lit->atom == f_atom)  /* save position */
			    nuc_pos = i;
			    /* positive || answer */
			else if (lit->sign || lit->atom->varnum == ANSWER) {
			    if (get_literal(&l1) == TROUBLE)
				return(TROUBLE);
			    if (nuc_lits == NULL)
				nuc_lits = l1;
			    else
				l2->next_lit = l1;
			    l2 = l1;
			    l1->sign = lit->sign;
			    l1->atom = lit->atom;
			    }
			else {  /* put literal into clash structure */
			    i++;
			    if (get_clash_nd(&c1) == TROUBLE)
				return(TROUBLE);
			    if (clash_list == NULL)
				clash_list = c1;
			    else {
				c2->next = c1;
				c1->prev = c2;
				}
			    c2 = c1;
			    c2->db = Fpa_clash_pos_lits;
			    if (get_context(&temp) == TROUBLE)
				return(TROUBLE);
				c2->subst = temp;
			    c2->subst->multiplier = m++;
			    c2->nuc_atom = lit->atom;
			    c2->evaluable = (lit->atom->varnum == EVALUABLE);
			    } /* end of else put literal into clash structure */
			lit = lit->next_lit;
			}	/* end of while */

		    ch = clash(clash_list, nuc_subst, nuc_lits, nuc,
			      giv_subst, giv_lits, giv_cl,
			      pos_clause, HYPER_TIME, nuc_pos);
				if (ch == PROOF || ch == TROUBLE)
					return(ch);

		    /* now deallocate the clash structure and literal nodes */
		    c1 = clash_list;
		    while (c1 != NULL) {
			free_context(c1->subst);
			c2 = c1;
			c1 = c1->next;
			free_clash_nd(c2);
			}
		    l1 = nuc_lits;
		    while (l1 != NULL) {
			l2 = l1;
			l1 = l1->next_lit;
			free_literal(l2);
			}
		    clear_subst_1(tr);
		    }	/* end of we have a nucleus */
		}	/* end of if pre-condition to try unify */
	}	/* end of if added by Penguin */
		f_atom = next_term(ut, 0);
		}	/* end of while looping on f_atom */
	    l1 = giv_lits;
	    while (l1 != NULL) {
		l2 = l1;
		l1 = l1->next_lit;
		free_literal(l2);
		}
	    l3 = l3->next_lit;
	    }	/* end of while looping on literals of satellite */
	free_context(giv_subst);
	free_context(nuc_subst);
	CLOCK_STOP(HYPER_TIME)
	return(NO_PROOF);
	}	/* end of else given clause is satellite */
CLOCK_STOP(HYPER_TIME)
return(NO_PROOF);
}  /* hyper_res() */

/*************
 *
 *    int neg_hyper_res(c) -- negative hyperresolution
 *
 *    Append kept resolvents to Sos.  Each kept
 *    clause has already passed the pre_process filter (forward
 *    subsumption, etc.), been integrated, and inserted into
 *    appropriate indexes.
 *
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

int neg_hyper_res(giv_cl)
struct clause *giv_cl;
{
    struct literal *lit, *nuc_lits, *giv_lits, *l1, *l2, *l3;
    struct context *nuc_subst, *giv_subst;
    struct clash_nd *clash_list, *c1, *c2;
    struct clause *nuc;
    int m, i, nuc_pos;
    struct term *f_atom;
    struct fpa_tree *ut;
    struct trail *tr;
	int uok;
	int ch;
	struct context *temp;

	ch = NO_PROOF;				/* default */

    CLOCK_START(NEG_HYPER_TIME)
    if (num_literals(giv_cl) == 0) {
	CLOCK_STOP(NEG_HYPER_TIME)
	return(NO_PROOF);
	}
    else if (!neg_clause(giv_cl)) {  /* given clause is nucleus */
	if (giv_cl->pid == Whoami)
/* Penguin: only residents can be nuclei.			*/
	{
	clash_list = NULL;
	nuc_lits = NULL;
	if (get_context(&nuc_subst) == TROUBLE)
		return(TROUBLE);
	nuc_subst->multiplier = 0;
	m = 1;
	lit = giv_cl->first_lit;
	l2 = NULL; c2 = NULL;  /* to quiet lint */
	while (lit != NULL) {
	    /* negative literal || answer literal */
	    if (!lit->sign || lit->atom->varnum == ANSWER) {
		if (get_literal(&l1) == TROUBLE)
			return(TROUBLE);
		if (nuc_lits == NULL)
		    nuc_lits = l1;
		else
		    l2->next_lit = l1;
		l2 = l1;
		l1->sign = lit->sign;
		l1->atom = lit->atom;
		}
	    else {            /* put positive literal into clash structure */
		if (get_clash_nd(&c1) == TROUBLE)
			return(TROUBLE);
		if (clash_list == NULL)
		    clash_list = c1;
		else {
		    c2->next = c1;
		    c1->prev = c2;
		    }
		c2 = c1;
		c2->db = Fpa_clash_neg_lits;
		if (get_context(&temp) == TROUBLE)
			return(TROUBLE);
		c2->subst = temp;
		c2->subst->multiplier = m++;
		c2->nuc_atom = lit->atom;
		c2->evaluable = (lit->atom->varnum == EVALUABLE);
		}
	    lit = lit->next_lit;
	    }
	ch = clash(clash_list, nuc_subst, nuc_lits, giv_cl,
		  (struct context *) NULL, (struct literal *) NULL,
		  (struct clause *) NULL,
		  neg_clause, NEG_HYPER_TIME, 0);
		if (ch == PROOF || ch == TROUBLE)
			return(ch);
	c1 = clash_list;
	while (c1 != NULL) {
	    free_context(c1->subst);
	    c2 = c1;
	    c1 = c1->next;
	    free_clash_nd(c2);
	    }
	l1 = nuc_lits;
	while (l1 != NULL) {
	    l2 = l1;
	    l1 = l1->next_lit;
	    free_literal(l2);
	    }
	free_context(nuc_subst);
	CLOCK_STOP(NEG_HYPER_TIME)
	return(NO_PROOF);
	}	/* end of if added by Penguin */
	}	/* end of given clause is nucleus */
    else { /* given clause is satellite (negative) */
	if (get_context(&giv_subst) == TROUBLE)
		return(TROUBLE);
	/* substitution for given satellite */
	giv_subst->multiplier = 0;
	if (get_context(&nuc_subst) == TROUBLE)
		return(TROUBLE);
	/* substitution for nucleus */
	nuc_subst->multiplier = 1;
	l3 = giv_cl->first_lit;
	while (l3 != NULL) {  /* for each literal in given satellite */
	    /* collect non-clashed lits (including answers) of given sat*/
	    giv_lits = NULL;
	    lit = giv_cl->first_lit;
	    while (lit != NULL) {
		if (lit != l3) {
		    if (get_literal(&l1) == TROUBLE)
			return(TROUBLE);
		    if (giv_lits == NULL)
			giv_lits = l1;
		    else
			l2->next_lit = l1;
		    l2 = l1;
		    l1->sign = lit->sign;
		    l1->atom = lit->atom;
		    }
		lit = lit->next_lit;
		}
	    if (build_tree(l3->atom,UNIFY,
		Parms[FPA_LITERALS].val,Fpa_clash_pos_lits,&ut)==TROUBLE)
		return(TROUBLE);
	    f_atom = next_term(ut, 0);
	    while (f_atom != NULL) {  /* for each potential nucleus */
		tr = NULL;
		nuc = f_atom->occ.lit->container;
		if (nuc->pid == Whoami)
/* Penguin: only residents can be nuclei.	*/
		{
		if (!neg_clause(nuc))
		{	/* if pre-condition to try unify */
	if (unify(l3->atom, giv_subst, f_atom, nuc_subst, &tr, &uok) == TROUBLE)
		return(TROUBLE);
		if (uok)
		{	/* we have a nucleus */

		    /* there are three kinds of literal in the nucleus:    */
		    /*    1. the clashed literal -> do nothing             */
		    /*    2. negative or answer literals -> collect them   */
		    /*    3. positive literals -> put into clash structure */

		    nuc_lits = NULL;
		    clash_list = NULL;
		    m = 2;  /* multipliers for found sats start with 2 */
		    lit = nuc->first_lit;
		    i = 1;
	/* Find index of clahsable lit that sat clashes with. */
		    while (lit != NULL) {
			if (lit->atom == f_atom)  /* save position */
			    nuc_pos = i;
			    /* negative || answer */
			else if (!lit->sign || lit->atom->varnum == ANSWER) {
			    if (get_literal(&l1) == TROUBLE)
				return(TROUBLE);
			    if (nuc_lits == NULL)
				nuc_lits = l1;
			    else
				l2->next_lit = l1;
			    l2 = l1;
			    l1->sign = lit->sign;
			    l1->atom = lit->atom;
			    }
			else {  /* put literal into clash structure */
			    i++;
			    if (get_clash_nd(&c1) == TROUBLE)
				return(TROUBLE);
			    if (clash_list == NULL)
				clash_list = c1;
			    else {
				c2->next = c1;
				c1->prev = c2;
				}
			    c2 = c1;
			    c2->db = Fpa_clash_neg_lits;
			    if (get_context(&temp) == TROUBLE)
				return(TROUBLE);
				c2->subst = temp;
			    c2->subst->multiplier = m++;
			    c2->nuc_atom = lit->atom;
			    c2->evaluable = (lit->atom->varnum == EVALUABLE);
			    } /* end of else put literal into clash structure */
			lit = lit->next_lit;
			}	/* end of while */

		    ch = clash(clash_list, nuc_subst, nuc_lits, nuc,
			      giv_subst, giv_lits, giv_cl,
			      neg_clause, NEG_HYPER_TIME, nuc_pos);
				if (ch == PROOF || ch == TROUBLE)
					return(ch);
		    /* now deallocate the clash structure and literal nodes */
		    c1 = clash_list;
		    while (c1 != NULL) {
			free_context(c1->subst);
			c2 = c1;
			c1 = c1->next;
			free_clash_nd(c2);
			}
		    l1 = nuc_lits;
		    while (l1 != NULL) {
			l2 = l1;
			l1 = l1->next_lit;
			free_literal(l2);
			}
		    clear_subst_1(tr);
		    }	/* end of we have a nucleus */
		}	/* end of if pre-condition to try unify */
		}	/* end of if added by Penguin */
		f_atom = next_term(ut, 0);
		}	/* end of while */
	    l1 = giv_lits;
	    while (l1 != NULL) {
		l2 = l1;
		l1 = l1->next_lit;
		free_literal(l2);
		}
	    l3 = l3->next_lit;
	    }
	free_context(giv_subst);
	free_context(nuc_subst);
	CLOCK_STOP(NEG_HYPER_TIME)
	return(NO_PROOF);
	}	/* end of else given clause is satellite */
CLOCK_STOP(NEG_HYPER_TIME)
return(NO_PROOF);
}  /* neg_hyper_res */

/*************
 *
 *    int ur_res(c) -- unit resulting (UR) resolution
 *
 *    Append kept resolvents to Sos.  Each kept
 *    clause has already passed the pre_process filter (forward
 *    subsumption, etc.), been integrated, and inserted into
 *    appropriate indexes.
 *
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

int ur_res(giv_cl)
struct clause *giv_cl;
{
    struct literal *lit, *nuc_lits, *giv_lits;
    struct literal *l1, *l2, *l3, *box, *f_lit;
    struct context *nuc_subst, *giv_subst;
    struct clash_nd *clash_list, *c1, *c2;
    struct clause *nuc;
    int m, i, nlits, j, nuc_pos;
    struct term *f_atom;
    struct fpa_tree *ut;
    struct trail *tr;
	int uok;
	int ch;
	struct context *temp;

	ch = NO_PROOF;				/* default */
    CLOCK_START(UR_TIME)
    nlits = num_literals(giv_cl);
    if (nlits == 0) {
	CLOCK_STOP(UR_TIME)
	return(NO_PROOF);
	}
    if (nlits > 1) {  /* given clause is nucleus (non-unit) */
if (giv_cl->pid == Whoami)
	{
/* Each Penguin uses as nuclei only its residents, whereas satellites may be */
/* either residents or clauses brought in by inference messages.	*/
	clash_list = NULL;
	if (get_context(&nuc_subst) == TROUBLE)
		return(TROUBLE);
	nuc_subst->multiplier = 0;
	m = 1;
	if (get_literal(&nuc_lits) == TROUBLE)
		return(TROUBLE);
	 /* for boxed literal */
	l2 = nuc_lits;
	lit = giv_cl->first_lit;
	while (lit != NULL) {
	    if (lit->atom->varnum == ANSWER) {  /* if answer literal */
		if (get_literal(&l1) == TROUBLE)
			return(TROUBLE);
		l2->next_lit = l1;
		l2 = l1;
		l1->sign = lit->sign;
		l1->atom = lit->atom;
		}	/* end of if answer literal */
	    lit = lit->next_lit;
	    }	/* end of while looping on lit */
	c2 = NULL;  /* to quiet lint */
	for (i = 1; i < nlits; i++) {  /* set up nlits-1 empty clash nodes */
	    if (get_clash_nd(&c1) == TROUBLE)
		return(TROUBLE);
	    if (clash_list == NULL)
		clash_list = c1;
	    else {
		c2->next = c1;
		c1->prev = c2;
		}
	    c2 = c1;
	    if (get_context(&temp) == TROUBLE)
		return(TROUBLE);
		c2->subst = temp;
	    c2->subst->multiplier = m++;
	    }	/* end of for */
	box = giv_cl->first_lit;
	while (box != NULL) {
	    if (box->atom->varnum != ANSWER) {  /* if not answer literal */
		c1 = clash_list;
		nuc_lits->sign = box->sign;
		nuc_lits->atom = box->atom;
		lit = giv_cl->first_lit;
		while (lit != NULL) {
		    /* if not boxed or answer literal */
		if (lit != box && lit->atom->varnum != ANSWER)
			{
			c1->nuc_atom = lit->atom;
			c1->db = (lit->sign ? Fpa_clash_neg_lits : 
					      Fpa_clash_pos_lits);
			c1 = c1->next;
			}	/* end of if not boxed or answer literal */
		    lit = lit->next_lit;
		    }	/* end of while looping on lit */
		if (c1 != NULL) {
		    output_stats(Fdout, 4);
	fprintf(Fderr, "ABEND, ur_res: too many clash nodes (nuc).\007\n");
		 fprintf(Fdout, "ABEND, ur_res: too many clash nodes (nuc).\n");
		    return(TROUBLE);
		    }
		ch = clash(clash_list, nuc_subst, nuc_lits, giv_cl,
		      (struct context *) NULL, (struct literal *) NULL,
		      (struct clause *) NULL,
		      unit_clause, UR_TIME, 0);
			if (ch == PROOF || ch == TROUBLE)
				return(ch);
		} /* end of if not answer literal */
	    box = box->next_lit;
	    } /* end of while looping on box */
	c1 = clash_list;
	while (c1 != NULL) {
	    free_context(c1->subst);
	    c2 = c1;
	    c1 = c1->next;
	    free_clash_nd(c2);
	    }
	l1 = nuc_lits;
	while (l1 != NULL) {
	    l2 = l1;
	    l1 = l1->next_lit;
	    free_literal(l2);
	    }
	free_context(nuc_subst);
	CLOCK_STOP(UR_TIME)
	return(NO_PROOF);
	}	/* end of if added for Penguin */
	}	/* end of nlits > 1: given clause is nucleus: non-unit */
    else { /* given clause is satellite (unit) */
	if (get_context(&giv_subst) == TROUBLE)
		return(TROUBLE);
	/* substitution for given satellite */
	giv_subst->multiplier = 0;
	if (get_context(&nuc_subst) == TROUBLE)
		return(TROUBLE);
	/* substitution for nucleus */
	nuc_subst->multiplier = 1;
	/* collect any answer literals from given satellite */
	/* and get clashable literal (l3) */
	giv_lits = NULL;
	lit = giv_cl->first_lit;
	while (lit != NULL) {
	    if (lit->atom->varnum != ANSWER)  /* if not answer lit */
		l3 = lit;  /* the only non-answer literal */
	    else {	/* else answer lit */
		if (get_literal(&l1) == TROUBLE)
			return(TROUBLE);
		if (giv_lits == NULL)
		    giv_lits = l1;
		else
		    l2->next_lit = l1;
		l2 = l1;
		l1->sign = lit->sign;
		l1->atom = lit->atom;
		}	/* end of else answer lit */
	    lit = lit->next_lit;
	    }	/* end of while looping on lit */
if (build_tree(l3->atom,UNIFY,Parms[FPA_LITERALS].val,
	l3->sign ? Fpa_clash_neg_lits : Fpa_clash_pos_lits,&ut)==TROUBLE)
		return(TROUBLE);
	f_atom = next_term(ut, 0);
	while (f_atom != NULL) {  /* for each potential nucleus */
if (f_atom->occ.lit->container->pid == Whoami)
	{
/* Each Penguin considers as nuclei its residents only, while it considers */
/* as satellites both residents and clauses brought in by inference messages. */
	    tr = NULL;
	    f_lit = f_atom->occ.lit;
	    nuc = f_lit->container;
	    nlits = num_literals(nuc);
	    if (nlits > 1)
		{	/* if pre-condition to try unify() */
	if (unify(l3->atom, giv_subst, f_atom, nuc_subst, &tr, &uok) == TROUBLE)
		return(TROUBLE);
		if (uok)
		{ /* unify() succeeds: we have a nucleus */
		m = 2;
		if (get_literal(&nuc_lits) == TROUBLE)
			return(TROUBLE);
		/* for boxed literal */
		/* now append any answer literals to nuc_lits */
		l2 = nuc_lits;
		lit = nuc->first_lit;
		while (lit != NULL) {
		    if (lit->atom->varnum == ANSWER) {  /* if answer literal */
			if (get_literal(&l1) == TROUBLE)
				return(TROUBLE);
			l2->next_lit = l1;
			l2 = l1;
			l1->sign = lit->sign;
			l1->atom = lit->atom;
			}	/* end of if answer literal */
		    lit = lit->next_lit;
		    }	/* end of while looping on lit */
		/* build clash structure for this nucleus */
		clash_list = NULL;
		for (i = 2; i < nlits; i++) {  /* nlits-2 empty clash nodes */
		    if (get_clash_nd(&c1) == TROUBLE)
			return(TROUBLE);
		    if (clash_list == NULL)
			clash_list = c1;
		    else {
			c2->next = c1;
			c1->prev = c2;
			}
		    c2 = c1;
		    if (get_context(&temp) == TROUBLE)
			return(TROUBLE);
			c2->subst = temp;
		    c2->subst->multiplier = m++;
		    }	/* end of for */
		box = nuc->first_lit;
		while (box != NULL) {
		    /* if not clashed or answer literal */
		    if (box != f_lit && box->atom->varnum != ANSWER) {
			c1 = clash_list;
			nuc_lits->sign = box->sign;
			nuc_lits->atom = box->atom;
			lit = nuc->first_lit;
			j = 1;
			while (lit != NULL) {
			    /* if not boxed or clashed or answer literal */
			    if (lit != box && lit != f_lit &&
					      lit->atom->varnum != ANSWER) {
				c1->nuc_atom = lit->atom;
				c1->db = (lit->sign ? Fpa_clash_neg_lits : 
						      Fpa_clash_pos_lits);
				c1 = c1->next;
				j++;
			} /* end of if not boxed or clashed or answer literal */
			    if (lit == f_lit)
				nuc_pos = j;  /* For ordered history option */
			    lit = lit->next_lit;
			    }	/* end of while looping on lit */
			if ( c1 != NULL)  {
			    output_stats(Fdout, 4);
	fprintf(Fderr, "ABEND, ur_res: too many clash nodes (sat).\007\n");
	fprintf(Fdout, "ABEND, ur_res: too many clash nodes (sat).\n");
			    return(TROUBLE);
			    }
			ch = clash(clash_list, nuc_subst, nuc_lits, nuc,
			      giv_subst, giv_lits, giv_cl,
			      unit_clause, UR_TIME, nuc_pos);
				if (ch == PROOF || ch == TROUBLE)
					return(ch);
			}	/* end of if not clash or answer literal */
		    box = box->next_lit;
		    }	/* end of while looping on box */
		c1 = clash_list;
		while (c1 != NULL) {
		    free_context(c1->subst);
		    c2 = c1;
		    c1 = c1->next;
		    free_clash_nd(c2);
		    }
		l1 = nuc_lits;
		while (l1 != NULL) {
		    l2 = l1;
		    l1 = l1->next_lit;
		    free_literal(l2);
		    }

		clear_subst_1(tr);
		}	/* end of if unify() succeeds: we have a nucleus */
		}	/* end of if pre-condition to try unify() */
	}	/* end of if added for Penguin */
	    f_atom = next_term(ut, 0);
	    }	/* end of while looping on f_atom */
	/* free answer literals from given satellite */
	l1 = giv_lits;
	while (l1 != NULL) {
	    l2 = l1;
	    l1 = l1->next_lit;
	    free_literal(l2);
	    }

	free_context(giv_subst);
	free_context(nuc_subst);
	CLOCK_STOP(UR_TIME)
	return(NO_PROOF);
	}	/* end of else given clause is satellite (unit) */
CLOCK_STOP(UR_TIME)
return(NO_PROOF);
}  /* ur_res() */

/*************
 *
 *    int one_unary_answer(c)
 *
 *************/

int one_unary_answer(c)
struct clause *c;
{
    struct literal *l;

    for (l = c->first_lit; l != NULL && l->atom->varnum != ANSWER; l = l->next_lit);  /* empty body */
    if (l == NULL)
	return(0);
    else if (sn_to_arity(l->atom->sym_num) != 1)
	return(0);
    else {
	for (l = l->next_lit; l != NULL && l->atom->varnum != ANSWER; l = l->next_lit);  /* empty body */
	return(l == NULL);
	}
}  /* one_unary_answer */

/*************
 *
 *    int build_term(sn,arg1,arg2,arg3,result)
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter result.
 *
 *************/

build_term(sn,arg1,arg2,arg3,result)
int sn;
struct term *arg1, *arg2, *arg3;
struct term **result;
{
    int arity;
    struct rel *r1, *r2, *r3;
    struct term *t;

    arity = sn_to_arity(sn);
    if (arity != 3) {
	output_stats(Fdout, 4);
	fprintf(Fderr, "ABEND, build_term, bad arity.\007\n");
	fprintf(Fdout, "ABEND, build_term, bad arity.");
	return(TROUBLE);
	}
    if (get_term(&t) == TROUBLE)
	return(TROUBLE);
    t->sym_num = sn;
    t->type = COMPLEX;
    if (get_rel(&r1) == TROUBLE)
	return(TROUBLE);
    if (get_rel(&r2) == TROUBLE)
	return(TROUBLE);
    if (get_rel(&r3) == TROUBLE)
	return(TROUBLE);
    t->farg = r1;
    r1->narg = r2;
    r2->narg = r3;
    r1->argval = arg1;
    r2->argval = arg2;
    r3->argval = arg3;
	*result = t;
    return(NO_TROUBLE);
}  /* build_term */


/*************
 *
 *    int combine_answers(res, a1, s1, a2, s2)
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int combine_answers(res, a1, s1, a2, s2)
struct clause *res;
struct term *a1;
struct context *s1;
struct term *a2;
struct context *s2;
{
    struct clause *par1, *par2;
    int condition_par1;
    struct term *condition, *then_part, *else_part, *temp;
    struct literal *lit1, *lit2, *prev_lit;
	int tempint;

    par1 = a1->occ.lit->container;
    par2 = a2->occ.lit->container;

    if (one_unary_answer(par1) && one_unary_answer(par2)) {

	condition_par1 = a2->occ.lit->sign;

        if (condition_par1)
	{
	    if (apply(a1, s1, &condition) == TROUBLE)
		return(TROUBLE);
	}
	else
	{
	    if (apply(a2, s2, &condition) == TROUBLE)
		return(TROUBLE);
	}

	for (lit1 = res->first_lit, prev_lit = NULL;
	     lit1->atom->varnum != ANSWER;
	     prev_lit = lit1, lit1 = lit1->next_lit);
	     /* empty body */
for (lit2 = lit1->next_lit; lit2->atom->varnum != ANSWER; lit2 = lit2->next_lit)
			;
	    /* empty body */

	if (condition_par1) {
	    then_part = lit1->atom->farg->argval;
	    else_part = lit2->atom->farg->argval;
	    }
	else {
	    then_part = lit2->atom->farg->argval;
	    else_part = lit1->atom->farg->argval;
	    }

	if (prev_lit == NULL)
	    res->first_lit = lit1->next_lit;
	else
	    prev_lit->next_lit = lit1->next_lit;

	free_rel(lit1->atom->farg);
	free_term(lit1->atom);
	free_literal(lit1);

if (str_to_sn("if", 3, &tempint) == TROUBLE)
	return(TROUBLE);

if (build_term(tempint, condition, then_part, else_part, &temp) == TROUBLE)
	return(TROUBLE);

lit2->atom->farg->argval = temp;
	 }
return(NO_TROUBLE);
}  /* combine_answers */

/*************
 *
 *    int build_bin_res(a1,s1,a2,s2,bbr)
 *
 *    Build a binary resolvent.  a1 and a2 are the clashed literals,
 *    and s1 and s2 are the respective unifying substitutions.
 *	struct clause * in Otter, int in Penguin, as its returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct clause through the
 *	parameter bbr.
 *
 *************/

int build_bin_res(a1,s1,a2,s2,bbr)
struct term *a1;
struct context *s1;
struct term *a2;
struct context *s2;
struct clause **bbr;
{
    struct clause *res;
    struct literal *lit, *new, *prev;
    struct int_ptr *ip0, *ip1, *ip2;
	struct term *temp;

	*bbr = NULL; 				/* default */
    if (get_clause(&res) == NULL)
	return(TROUBLE);
    prev = NULL;
    lit = a1->occ.lit->container->first_lit;
    while (lit != NULL) {
	if (lit->atom != a1) {
	    if (get_literal(&new) == TROUBLE)
		return(TROUBLE);
	    new->container = res;
	    if (prev == NULL)
		res->first_lit = new;
	    else
		prev->next_lit = new;
	    prev = new;
	    new->sign = lit->sign;
	    if (apply(lit->atom, s1, &temp) == TROUBLE)
		return(TROUBLE);
		new->atom = temp;
	    new->atom->occ.lit = new;
	    new->atom->varnum = lit->atom->varnum;  /* copy type of atom */
	    }
	lit = lit->next_lit;
	}

    lit = a2->occ.lit->container->first_lit;
    while (lit != NULL) {
	if (lit->atom != a2) {
	    if (get_literal(&new) == TROUBLE)
		return(TROUBLE);
	    new->container = res;
	    if (res->first_lit == NULL)
		res->first_lit = new;
	    else
		prev->next_lit = new;
	    prev = new;
	    new->sign = lit->sign;
	    if (apply(lit->atom, s2, &temp) == TROUBLE)
		return(TROUBLE);
		new->atom = temp;
	    new->atom->occ.lit = new;
	    new->atom->varnum = lit->atom->varnum;  /* copy type of atom */
	    }
	lit = lit->next_lit;
	}
    
    if (get_int_ptr(&ip0) == TROUBLE)
	return(TROUBLE);
    if (get_int_ptr(&ip1) == TROUBLE)
	return(TROUBLE);
    if (get_int_ptr(&ip2) == TROUBLE)
	return(TROUBLE);
    ip0->i = BINARY_RES_RULE;
    ip1->i = a1->occ.lit->container->id;
    ip2->i = a2->occ.lit->container->id;
    ip0->next = ip1;
    ip1->next = ip2;
    res->parents = ip0;
    if (Flags[PROG_SYNTHESIS].val)
	if (combine_answers(res, a1, s1, a2, s2) == TROUBLE)
		return(TROUBLE);

	*bbr = res;
    	return(NO_TROUBLE);
}  /* build_bin_res */

/*************
 *
 *    int bin_res(giv_cl) -- binary resolution
 *
 *    Append kept resolvents to Sos.  Each kept
 *    clause has already passed the pre_process filter (forward
 *    subsumption, etc.), been integrated, and inserted into
 *    appropriate indexes.
 *
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

int bin_res(giv_cl)
struct clause *giv_cl;
{
    struct literal *g_lit;
    struct term *g_atom, *f_atom;
    struct context *gs, *fs;
    struct trail *tr;
    struct fpa_tree *ut;
    struct fpa_head **db;
    struct clause *resolvent;
	int uok;
	int pp;

	pp = NO_PROOF;				/* default */
    CLOCK_START(BINARY_TIME)
    if (get_context(&gs) == TROUBLE)
	return(TROUBLE);
    gs->multiplier = 0;
    if (get_context(&fs) == TROUBLE)
	return(TROUBLE);
    fs->multiplier = 1;
    g_lit = giv_cl->first_lit;
    while (g_lit != NULL) {
	g_atom = g_lit->atom;
	if (g_atom->varnum != ANSWER && (g_lit->sign || giv_cl->pid==Whoami))
	{
/* It proceeds only if it is not an answer literal 			*/
/* Penguin: if the literal is negative, it proceeds only if the clause is */
/* a resident.								 */
	    if (g_lit->sign)
		db = Fpa_clash_neg_lits;
	    else 
		db = Fpa_clash_pos_lits;
	    if (build_tree(g_lit->atom,UNIFY,
			    Parms[FPA_LITERALS].val,db,&ut) == TROUBLE)
		return(TROUBLE);
	    f_atom = next_term(ut, 0); 
	    while (f_atom != NULL) {
if (f_atom->occ.lit->sign || f_atom->occ.lit->container->pid == Whoami) 
/* Penguin: if the literal is negative, it proceeds only if the clause is */
/* a resident.								 */
		{
		tr = NULL;
#ifdef ROO
                if (giv_cl->giv_cl_seq_no >=
                    f_atom->occ.lit->container->giv_cl_seq_no &&
                    unify(g_atom, gs, f_atom, fs, &tr)) {

#else		
		if (unify(g_atom, gs, f_atom, fs, &tr, &uok) == TROUBLE)
			return(TROUBLE);
		if (uok)
		 {	/* if unify succeeds */
#endif
	 if (build_bin_res(g_atom,gs,f_atom,fs,&resolvent) == TROUBLE)
			return(TROUBLE);
		    clear_subst_1(tr);
		    Stats[CL_GENERATED]++;
		    CLOCK_STOP(BINARY_TIME)
		    pp = pre_process(resolvent, 0, Sos);
			if (pp == PROOF || pp ==TROUBLE)
				return(pp);
		    CLOCK_START(BINARY_TIME)
		    }	/* end of if unify succeeds */
		 }	/* end of if f_atom positive or resident */
		f_atom = next_term(ut, 0);
		}	/* end of while on f_atom */
	    }	/* end of if g_lit not answer and either positive or resident */
	g_lit = g_lit->next_lit;
	}	/* end of while on the literals of the given clause */
    free_context(gs);
    free_context(fs);
    CLOCK_STOP(BINARY_TIME)
return(NO_PROOF);
}  /* bin_res */

/*************
 *
 *    all_factors(c, ct, lst) -- generate and pre_process all binary factors c.
 *
 *    Indirect recursive calls will get factors of factors, etc.
 *
 *	Penguin adds the parameter ct passed from post_process() and to
 *	passed to pre_process(): see comment at the call to pre_process().
 *
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

int all_factors(c, ct, lst)
struct clause *c;
int ct;
struct list *lst;
{
    struct literal *l1, *l2, *l3, *l4, *l5;
    struct context *s;
    struct clause *d;
    struct trail *tr;
    struct int_ptr *ip0, *ip1;
	int uok;
	int pp;
	struct term *temp;

	pp = NO_PROOF;				/* default */
    if (get_context(&s) == TROUBLE)
	return(TROUBLE);
    s->multiplier = 0;
    l1 = c->first_lit;
    while (l1 != NULL) {
	l2 = l1->next_lit;
	while (l2 != NULL) {
	    tr = NULL;
	    if (l1->sign == l2->sign)
		{	/* pre-condition to try unify */
	if (unify(l1->atom, s, l2->atom, s, &tr, &uok) == TROUBLE)
		return(TROUBLE);
		if (uok)
		{	/* unify succeeds */
		if (get_clause(&d) == TROUBLE)
			return(TROUBLE);
		if (get_int_ptr(&ip0) == TROUBLE)
			return(TROUBLE);
		ip0->i = FACTOR_RULE;
		if (get_int_ptr(&ip1) == TROUBLE)
			return(TROUBLE);
		ip1->i = c->id;
		d->parents = ip0;
		ip0->next = ip1;

		l3 = NULL;
		l5 = c->first_lit;
		while (l5 != NULL) {  /* l2 is the literal to exclude */
		    if (l5 != l2) {
			if (get_literal(&l4) == TROUBLE)
				return(TROUBLE);
			l4->sign = l5->sign;
			l4->container = d;
			if (l3 == NULL)
			    d->first_lit = l4;
			else
			    l3->next_lit = l4;
			if (apply(l5->atom, s, &temp) == TROUBLE)
				return(TROUBLE);
			l4->atom = temp;
			l4->atom->occ.lit = l4;
	                l4->atom->varnum = l5->atom->varnum;  /* copy type */
			l3 = l4;
			}	/* end of if */
		    l5 = l5->next_lit;
		    }	/* end of while looping on l5 */
		clear_subst_1(tr);
		Stats[CL_GENERATED]++;
		Stats[FACTORS]++;
		CLOCK_STOP(FACTOR_TIME)
#ifdef ROO
		pre_process(d, 0, lst);
#else
		CLOCK_STOP(POST_PROC_TIME)
if (Flags[PROCESS_INPUT].val && Flags[POST_PROC_NS_BEFORE_SEND].val && ct==IR)
		pp = pre_process(d, IR, lst);
		else
		pp = pre_process(d, 0, lst);
/* The factors of a clause are newly generated clauses, or raw critical pairs */
/* and therefore pre_process() should be called with the second parameter ct */
/* set to 0. However, if we are generating the factors of an input clause, */
/* ct == IR, BEFORE sending the input clause to the other Penguins, its	*/
/* factors should also be regarded as input clauses.			*/
/* We check for POST_PROC_NS_BEFORE_SEND to be on, because factoring is	*/
/* done during post-processing and post-processing of input clauses, treated */
/* in this respect similar to new_settlers, is done before sending 	*/
/* only if POST_PROC_NS_BEFORE_SEND is on. If it is off, post_processing is */
/* done after sending.							*/
		if (pp == PROOF || pp == TROUBLE)
			return(pp);
		CLOCK_START(POST_PROC_TIME)
#endif
		CLOCK_START(FACTOR_TIME)
		}	/* end of if unify succeeds */
		}	/* end of if pre-condition to try unify */
	    l2 = l2->next_lit;
	    }	/* end of while looping on l2 */
	l1 = l1->next_lit;
	}	/* end of while looping on l1 */
    free_context(s);
return(NO_PROOF);
}  /* all_factors */

