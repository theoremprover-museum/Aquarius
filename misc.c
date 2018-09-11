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
 *  misc.c -- Miscellaneous routines.
 *
 */

#include "header.h"

/*************
 *
 *    int init(nodes,who) -- initialize global variables
 *	Penguin adds the two parameters nodes and who representing
 *	the number of nodes and the id of the node respectively.
 *	It returns TROUBLE/NO_TROUBLE.
 *
 *************/

int init(nodes,who)
int nodes, who;
{

int p;

    if (get_is_tree(&Is_pos_lits) == TROUBLE)
	return(TROUBLE); /* index for forward subsumption */

    if (get_is_tree(&Is_neg_lits) == TROUBLE)
	/* index for forward subsumption */
	return(TROUBLE);

    if (get_imd_tree(&Demod_imd) == TROUBLE)  /* index for demodulation */
	return(TROUBLE);

/* Demod_imd, Usable, Sos, Demodulators and Passive are initialized in */
/* read_all_input() in Otter, here in Penguin.				*/

    if (get_list(&Usable) == TROUBLE)
	return(TROUBLE);

    if (get_list(&Sos) == TROUBLE)
	return(TROUBLE);

    if (get_list(&Demodulators) == TROUBLE)
	return(TROUBLE);

    if (get_list(&Passive) == TROUBLE)
	return(TROUBLE);

/* Inbound_msgs and Outbound_msgs for Penguin only. */

    if (get_list(&Inbound_msgs) == TROUBLE)
	return(TROUBLE);

    if (get_list(&Outbound_msgs) == TROUBLE)
	return(TROUBLE);

    Whoami = who;		/* Penguin: the id of the node. */
    No_of_nodes = nodes;	/* Penguin: the number of nodes. */
    Halting = 0;	
/* Penguin: initialized to 0 and set to 1 if HALT. is received.		*/
    Last_choice = MAX_INT;
    Before_last_choice = MAX_INT;
/* Last_choice is the Penguin which the last new settler was assigned to. */
/* Before_last_choice is the Penguin which the new settler before the	*/
/* last one was assigned to.						*/
/* At the beginning they are initialized to MAX_INT.			*/
/* If Last_choice == Whoami, it means that the last new settler was kept */
/* rather than sent and similarly with Before_last_choice and the previous */
/* new settler.								*/

	for (p = 0; p < No_of_nodes; p++)
		Penguins[p] = 1;
/* No_of_nodes Penguins, from 0 to No_of_nodes - 1, are active.		*/
	for (p = No_of_nodes; p < MAX_NO_OF_NODES; p++)
		Penguins[p] = 0;

    if (str_to_sn("$eq_infix", 2, &Eq_sym_num) == TROUBLE)
	return(TROUBLE);

    set_to_predicate(Eq_sym_num);
/* Penguin: establishes that built-in equality is a predicate. */

    if (str_to_sn("$cons", 2, &Cons_sym_num) == TROUBLE)
	return(TROUBLE);

    if (str_to_sn("$nil", 0, &Nil_sym_num) == TROUBLE)
	return(TROUBLE);

    if (str_to_sn("$IGNORE", 1, &Ignore_sym_num) == TROUBLE)
	return(TROUBLE);

    if (str_to_sn("$CONDITIONAL", 3, &Conditional_demodulator_sym_num)==TROUBLE)
	return(TROUBLE);

    if (str_to_sn("$CHR", 1, &Chr_sym_num) == TROUBLE)
	return(TROUBLE);

    clock_init();
    init_options();
return(NO_TROUBLE);
}  /* init() */

/*************
 *
 *    int read_all_input()
 *
 *	void in Otter, int in Penguin as it returns PROOF/NO_PROOF/TROUBLE.
 *
 *************/

int read_all_input()
{
    struct list *l;
    struct term *t, *t1;
    struct clause *c, *c2;
    int rc, error, list_errors, i;
    struct formula_ptr *fp;
	struct sym_ent *ug;
	int pp, ppa;
	int tempint;

	pp = NO_PROOF;				/* default */
	ppa = NO_PROOF;				/* default */
    CLOCK_START(INPUT_TIME)

    if (read_term(Fdin, &rc, &t) == TROUBLE)
	return(TROUBLE);
    while (t != NULL || rc == 0) {
	error = 0;
	if (t == NULL)
	    error = 1;
	else if (t->type != COMPLEX)
	    error = 1;
	else if (str_ident("set", sn_to_str(t->sym_num))) {
	    if (change_flag(Fdout, t, 1)) {
		print_term(Fdout, t); fprintf(Fdout,".\n");
		}
	    }
	else if (str_ident("clear", sn_to_str(t->sym_num))) {
	    if (change_flag(Fdout, t, 0)) {
		print_term(Fdout, t); fprintf(Fdout,".\n");
		}
	    }
	else if (str_ident("assign", sn_to_str(t->sym_num))) {
	    if (change_parm(Fdout, t)) {
		print_term(Fdout, t); fprintf(Fdout,".\n");
		}
	    }
	else if (str_ident("list", sn_to_str(t->sym_num))) {
	    t1 = t->farg->argval;
	    if (t1->type == COMPLEX || t->farg->narg != NULL) {
		fprintf(Fdout,"ERROR, bad argument to list: ");
		print_term(Fdout, t); fprintf(Fdout,".\n");
		Stats[INPUT_ERRORS]++;
		}
	    else if (str_ident("usable", sn_to_str(t1->sym_num)) ||
		     str_ident("axioms", sn_to_str(t1->sym_num))) {
		if (str_ident("axioms", sn_to_str(t1->sym_num)))
		fprintf(Fderr, "NOTICE: Please change 'axioms' to 'usable'.\n");
		fprintf(Fdout,"\n");
		print_term(Fdout, t);
		fprintf(Fdout,".\n");
		if (read_cl_list(Fdin, &list_errors, &l) == TROUBLE)
			return(TROUBLE);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		else if (Flags[PROCESS_INPUT].val == 0) {
/* Remark that cl_integrate() and send_clause() are called here because */
/* pre_process() won't be invoked.					*/
		    c = l->first_cl;
		    while (c != NULL) {
			/* Stats[INPUT_ERRORS] += process_linked_tags(c); */
		if (cl_integrate(c,IR,IN_ALL_PENGUINS_U) == TROUBLE)
			return(TROUBLE);
/* Penguin: the second parameter is Input Read clause.			*/
			if (Flags[STAND_ALONE].val == 0)
			if (send_clause(c) == TROUBLE)
				return(TROUBLE);
			c = c->next_cl;
			}
		    }
		print_cl_list(Fdout, l);
		append_lists(Usable,l);
		}
	    else if (str_ident("sos", sn_to_str(t1->sym_num))) {
		fprintf(Fdout,"\n");
		print_term(Fdout, t);
		fprintf(Fdout,".\n");
		if (read_cl_list(Fdin, &list_errors, &l) == TROUBLE)
			return(TROUBLE);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		else if (Flags[PROCESS_INPUT].val == 0) {
/* Remark that cl_integrate() and send_clause() are called here because */
/* pre_process() won't be invoked.					*/
		    c = l->first_cl;
		    while (c != NULL) {
		if (cl_integrate(c,IR,IN_ALL_PENGUINS_S) == TROUBLE)
			return(TROUBLE);
/* Penguin: the second parameter is Input Read clause.			*/
			if (Flags[STAND_ALONE].val == 0)
			if (send_clause(c) == TROUBLE)
				return(TROUBLE);
			c = c->next_cl;
			}
		    }
		print_cl_list(Fdout, l);
		append_lists(Sos,l);
		}
	    else if (str_ident("demodulators", sn_to_str(t1->sym_num))) {
		fprintf(Fdout,"\n");
		print_term(Fdout, t);
		fprintf(Fdout,".\n");
		if (read_cl_list(Fdin, &list_errors, &l) == TROUBLE)
			return(TROUBLE);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		c = l->first_cl;
		while (c != NULL) {
                    if (check_input_demod(c,&tempint) == TROUBLE)
			return(TROUBLE);
			if (tempint)
			{
/* Remark that cl_integrate() and send_clause() are called here because */
/* pre_process() won't be invoked on input clauses in Demodulators.	*/
		     if (cl_integrate(c,IR,IN_ALL_PENGUINS_D) == TROUBLE)
				return(TROUBLE);
/* Penguin: the second parameter is Input Read clause.			*/
			if (Flags[STAND_ALONE].val == 0)
			if (send_clause(c) == TROUBLE)
				return(TROUBLE);
			}
		    else {
			Stats[INPUT_ERRORS]++;
			fprintf(Fdout,"ERROR, bad demodulator: ");
			print_clause(Fdout, c);
			}
		    c = c->next_cl;
		    }
		print_cl_list(Fdout, l);
		append_lists(Demodulators,l);
		}
	    else if (str_ident("passive", sn_to_str(t1->sym_num))) {
		fprintf(Fdout,"\n");
		print_term(Fdout, t);
		fprintf(Fdout,".\n");
		if (read_cl_list(Fdin, &list_errors, &l) == TROUBLE)
			return(TROUBLE);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		c = l->first_cl;
/* Remark that cl_integrate() and send_clause() are called here because */
/* pre_process() won't be invoked on input clauses in Passive.		*/
		while (c != NULL) {
	if (cl_integrate(c,IR,IN_ALL_PENGUINS_P) == TROUBLE)
		return(TROUBLE);
/* Penguin: the second parameter is Input Read clause.			*/
			if (Flags[STAND_ALONE].val == 0)
			if (send_clause(c) == TROUBLE)
				return(TROUBLE);
		    c = c->next_cl;
		    }

		print_cl_list(Fdout, l);
		append_lists(Passive,l);
		}
	    else {
		if (str_ident("axioms", sn_to_str(t1->sym_num)))
		    fprintf(Fderr, "Name of axioms list is now 'usable'.\n");
		fprintf(Fdout,"ERROR, unknown list: ");
		print_term(Fdout, t); fprintf(Fdout,".\n");
		if (read_cl_list(Fdin, &list_errors, &l) == TROUBLE)
			return(TROUBLE);
		print_cl_list(Fdout, l);
		Stats[INPUT_ERRORS]++;
		}
	    }
	else if (str_ident("formula_list", sn_to_str(t->sym_num))) {
	    t1 = t->farg->argval;
	    if (t1->type == COMPLEX || t->farg->narg != NULL) {
		fprintf(Fdout,"ERROR, bad argument to list: ");
		print_term(Fdout, t); fprintf(Fdout,".\n");
		Stats[INPUT_ERRORS]++;
		}
	    else if (str_ident("usable", sn_to_str(t1->sym_num)) ||
		     str_ident("axioms", sn_to_str(t1->sym_num))) {
		if (str_ident("axioms", sn_to_str(t1->sym_num)))
		fprintf(Fderr, "NOTICE: Please change 'axioms' to 'usable'.\n");
		fprintf(Fdout,"\n");
		print_term(Fdout, t);
		fprintf(Fdout,".\n");
		if (read_formula_list(Fdin, &list_errors, &fp) == TROUBLE)
			return(TROUBLE);
		print_formula_list(Fdout,fp);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		else {
		    CLOCK_START(CLAUSIFY_TIME)
		    if (clausify_formula_list(fp,&l) == TROUBLE)
			return(TROUBLE);
		    CLOCK_STOP(CLAUSIFY_TIME)
		    if (Flags[PROCESS_INPUT].val == 0) {
/* Remark that cl_integrate() and send_clause() are called here because */
/* pre_process() won't be invoked.					*/
			c = l->first_cl;
			while (c != NULL) {
		if (cl_integrate(c,IR,IN_ALL_PENGUINS_U) == TROUBLE)
			return(TROUBLE);
/* Penguin: the second parameter is Input Read clause.			*/
			if (Flags[STAND_ALONE].val == 0)
				if (send_clause(c) == TROUBLE)
					return(TROUBLE);
			    c = c->next_cl;
			    }
			}
	fprintf(Fdout,"\n-------> usable clausifies to:\n\nlist(usable).\n");
		    print_cl_list(Fdout, l);
		    append_lists(Usable,l);
		    }
		}
	    else if (str_ident("sos", sn_to_str(t1->sym_num))) {
		fprintf(Fdout,"\n");
		print_term(Fdout, t);
		fprintf(Fdout,".\n");
		if (read_formula_list(Fdin, &list_errors, &fp) == TROUBLE)
			return(TROUBLE);
		print_formula_list(Fdout,fp);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		else {
		    CLOCK_START(CLAUSIFY_TIME)
		    if (clausify_formula_list(fp,&l) == TROUBLE)
			return(TROUBLE);
		    CLOCK_STOP(CLAUSIFY_TIME)
		    if (Flags[PROCESS_INPUT].val == 0) {
/* Remark that cl_integrate() and send_clause() are called here because */
/* pre_process() won't be invoked.					*/
			c = l->first_cl;
			while (c != NULL) {
		if (cl_integrate(c,IR,IN_ALL_PENGUINS_S) == TROUBLE)
			return(TROUBLE);
/* Penguin: the second parameter is Input Read clause.			*/
			if (Flags[STAND_ALONE].val == 0)
				if (send_clause(c) == TROUBLE)
					return(TROUBLE);
			    c = c->next_cl;
			    }
			}
		 fprintf(Fdout,"\n-------> sos clausifies to:\n\nlist(sos).\n");
		    print_cl_list(Fdout, l);
		    append_lists(Sos,l);
		    }
		}
	    else if (str_ident("passive", sn_to_str(t1->sym_num))) {
		fprintf(Fdout,"\n");
		print_term(Fdout, t);
		fprintf(Fdout,".\n");
		if (read_formula_list(Fdin, &list_errors, &fp) == TROUBLE)
			return(TROUBLE);
		print_formula_list(Fdout,fp);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		else {
		    CLOCK_START(CLAUSIFY_TIME)
		    if (clausify_formula_list(fp,&l) == TROUBLE)
			return(TROUBLE);
		    CLOCK_STOP(CLAUSIFY_TIME)
		    c = l->first_cl;
/* Remark that cl_integrate() and send_clause() are called here because */
/* pre_process() won't be invoked on clauses in the passive list.	*/
		    while (c != NULL) {
		if (cl_integrate(c,IR,IN_ALL_PENGUINS_P) == TROUBLE)
			return(TROUBLE);
/* Penguin: the second parameter is Input Read clause.			*/
			if (Flags[STAND_ALONE].val == 0)
			if (send_clause(c) == TROUBLE)
				return(TROUBLE);
			c = c->next_cl;
			}
		    
	fprintf(Fdout,"\n-------> passive clausifies to:\n\nlist(passive).\n");
		    print_cl_list(Fdout, l);
		    append_lists(Passive,l);
		    }
		}
	    else {
		if (str_ident("axioms", sn_to_str(t1->sym_num)))
		    fprintf(Fderr, "Name of axioms list is now 'usable'.\n");
		fprintf(Fdout,"ERROR, unknown formula_list: ");
		print_term(Fdout, t); fprintf(Fdout,".\n");
		if (read_cl_list(Fdin, &list_errors, &l) == TROUBLE)
			return(TROUBLE);
		print_cl_list(Fdout, l);
		Stats[INPUT_ERRORS]++;
		}
	    }
	else if (str_ident("weight_list", sn_to_str(t->sym_num))) {
	    t1 = t->farg->argval;
	    if (t1->type != NAME || t->farg->narg != NULL) {
		fprintf(Fdout,"ERROR, bad argument to Weight_list: ");
		print_term(Fdout, t); fprintf(Fdout,".\n");
		Stats[INPUT_ERRORS]++;
		}
	    else if (str_ident("purge_gen", sn_to_str(t1->sym_num))) {
		fprintf(Fdout,"\n");
		print_term(Fdout, t);
		fprintf(Fdout,".\n");
		if (Weight_purge_gen != NULL) {
		fprintf(Fdout,"----> ERROR, already have purge weight list.\n");
		    Stats[INPUT_ERRORS]++;
		    }
	if (read_list(Fdin, &list_errors, 0, &Weight_purge_gen) == TROUBLE)
		return(TROUBLE);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		if (get_is_tree(&Weight_purge_gen_index) == TROUBLE)
			return(TROUBLE);
if (set_wt_list(Weight_purge_gen,Weight_purge_gen_index,&list_errors)==TROUBLE)
		return(TROUBLE);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		print_list(Fdout, Weight_purge_gen);
		}
	    else if (str_ident("pick_given", sn_to_str(t1->sym_num))) {
		fprintf(Fdout,"\n");
		print_term(Fdout, t);
		fprintf(Fdout,".\n");
		if (Weight_pick_given != NULL) {
		 fprintf(Fdout,"----> ERROR, already have pick weight list.\n");
		    Stats[INPUT_ERRORS] ++;
		    }
	if (read_list(Fdin, &list_errors, 0, &Weight_pick_given) == TROUBLE)
		return(TROUBLE);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		if (get_is_tree(&Weight_pick_given_index) == TROUBLE)
			return(TROUBLE);
if (set_wt_list(Weight_pick_given,Weight_pick_given_index,&list_errors)==TROUBLE)
		return(TROUBLE);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		print_list(Fdout, Weight_pick_given);
		}
	    else if (str_ident("pick_and_purge", sn_to_str(t1->sym_num))) {
		fprintf(Fdout,"\n");
		print_term(Fdout, t);
		fprintf(Fdout,".\n");
		if (Weight_pick_given != NULL || Weight_purge_gen != NULL) {
fprintf(Fdout,"----> ERROR, already have pick weight list or purge weight list.\n");
		    Stats[INPUT_ERRORS] ++;
		    }
if (read_list(Fdin, &list_errors, 0, &Weight_purge_gen) == TROUBLE)
	return(TROUBLE);
Weight_pick_given = Weight_purge_gen;
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
	if (get_is_tree(&Weight_purge_gen_index) == TROUBLE)
		return(TROUBLE);
	Weight_pick_given_index = Weight_purge_gen_index;
if (set_wt_list(Weight_pick_given,Weight_pick_given_index,&list_errors)==TROUBLE)
		return(TROUBLE);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		print_list(Fdout, Weight_pick_given);
		}
	    else if (str_ident("terms", sn_to_str(t1->sym_num))) {
		fprintf(Fdout,"\n");
		print_term(Fdout, t);
		fprintf(Fdout,".\n");
		if (Weight_terms != NULL) {
		 fprintf(Fdout,"----> ERROR, already have term weight list.\n");
		    Stats[INPUT_ERRORS] ++;
		    }
		if (read_list(Fdin, &list_errors, 0, &Weight_terms) == TROUBLE)
			return(TROUBLE);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		if (get_is_tree(&Weight_terms_index) == TROUBLE)
			return(TROUBLE);
if (set_wt_list(Weight_terms, Weight_terms_index, &list_errors) == TROUBLE)
		return(TROUBLE);
		if (list_errors != 0)
		    Stats[INPUT_ERRORS] += list_errors;
		print_list(Fdout, Weight_terms);
		}
	    else {
		fprintf(Fdout,"ERROR, unknown Weight_list: ");
		print_term(Fdout, t); fprintf(Fdout,".\n");
	if (read_list(Fdin, &list_errors, 0, &Weight_pick_given) == TROUBLE)
			return(TROUBLE);
		print_list(Fdout, Weight_pick_given);
		Stats[INPUT_ERRORS]++;
		}
	    }
	else if (str_ident("lex", sn_to_str(t->sym_num))) {
if (t->farg==NULL || t->farg->narg != NULL || proper_list(t->farg->argval) == 0)
		{
		fprintf(Fdout,"ERROR, argument of lex term is not a list: ");
		print_term_nl(Fdout, t);
		Stats[INPUT_ERRORS]++;
		}
	    else {
		fprintf(Fdout,"\n");
	        print_term(Fdout, t);
		fprintf(Fdout,".\n");
	        if (set_lex_vals(t) == TROUBLE)
			return(TROUBLE);
		}
	    }
	else if (str_ident("lrpo_lr_status", sn_to_str(t->sym_num)) ||
	         str_ident("lrpo_rl_status", sn_to_str(t->sym_num))) {
if (t->farg==NULL || t->farg->narg != NULL || proper_list(t->farg->argval) == 0)
	{
	fprintf(Fdout,"ERROR, argument of lrpo_status term is not a list: ");
		print_term_nl(Fdout, t);
		Stats[INPUT_ERRORS]++;
		}
	    else {
		fprintf(Fdout,"\n");
	        print_term(Fdout, t);
		fprintf(Fdout,".\n");
	        if (str_ident("lrpo_lr_status", sn_to_str(t->sym_num)))
			{
		    if (set_lrpo_status(t, LRPO_LR_STATUS) == TROUBLE)
			return(TROUBLE);
			}
		else
			{
		    if (set_lrpo_status(t, LRPO_RL_STATUS) == TROUBLE)
			return(TROUBLE);
			}
		}
	    }
	else if (str_ident("skolem", sn_to_str(t->sym_num))) {
if (t->farg==NULL || t->farg->narg != NULL || proper_list(t->farg->argval) == 0)
		{
		fprintf(Fdout,"ERROR, argument of skolem term is not a list: ");
		print_term_nl(Fdout, t);
		Stats[INPUT_ERRORS]++;
		}
	    else {
		fprintf(Fdout,"\n");
	        print_term(Fdout, t);
		fprintf(Fdout,".\n");
	        if (set_skolem(t) == TROUBLE)
			return(TROUBLE);
		}
	    }
	else if (str_ident("special_unary", sn_to_str(t->sym_num))) {
if (t->farg==NULL || t->farg->narg != NULL || proper_list(t->farg->argval) == 0)
	{
	fprintf(Fdout,"ERROR, argument of special_unary term is not a list: ");
		print_term_nl(Fdout, t);
		Stats[INPUT_ERRORS]++;
		}
	    else {
		fprintf(Fdout,"\n");
	        print_term(Fdout, t);
		fprintf(Fdout,".\n");
	        if (set_special_unary(t) == TROUBLE)
			return(TROUBLE);
		Internal_flags[SPECIAL_UNARY_PRESENT] = 1;
		}
	    }
	    
	else
	    error = 1;

	if (error) {
	    Stats[INPUT_ERRORS]++;
	    if (t != NULL) {
		fprintf(Fdout,"ERROR, command not found: ");
		print_term(Fdout, t); fprintf(Fdout,".\n");
		}
	    }
	if (t != NULL)
	    zap_term(t);
	if (read_term(Fdin, &rc, &t) == TROUBLE)
		return(TROUBLE);
	}

    CLOCK_STOP(INPUT_TIME)

    if (Stats[INPUT_ERRORS] == 0) {

	fprintf(Fdout,"\n");
	dependent_options();
	check_options();
	fprintf(Fdout,"\n");

	mark_evaluable_symbols(1);
/* Penguin: the parameter set to 1 says that we are reading the input rather */
/* receiving input messages.						*/

	/* index demodulators */

	if (Demodulators->first_cl != NULL && Flags[DEMOD_LINEAR].val == 0) {
/* Otter had get_imd_tree() here, but Penguin has it in init().		*/
	    c = Demodulators->first_cl;
	    while (c != NULL) {
		if (imd_insert(c, Demod_imd) == TROUBLE)
			return(TROUBLE);
		c = c->next_cl;
		}
	    }
	
	/* index <passive list, don't pre_process, even if flag is set */
	
	for (c = Passive->first_cl; c != NULL; c = c->next_cl)
	    if (index_lits_all(c) == TROUBLE)
		return(TROUBLE);
	
	if (Flags[PROCESS_INPUT].val) {
	    CLOCK_START(PROCESS_INPUT_TIME)
	    fprintf(Fdout,"\n------------> process usable:\n");
	    l = Usable;
	    if (get_list(&Usable) == TROUBLE)
		return(TROUBLE);
	    c = l->first_cl;
	    while (c != NULL) {
		c2 = c;
		c = c->next_cl;
		cl_clear_vars(c2);  /* destroy input variable names */
		pp = pre_process(c2,IR,Usable);
		if (pp == PROOF || pp == TROUBLE)
			return(pp);
		}
	    free_list(l);
	    c2 = NULL;
#ifndef ROO
	    ppa = post_proc_all((struct clause *) NULL,IR,Usable);
	    if (ppa == PROOF || ppa == TROUBLE)
			return(ppa);
		
if (Flags[STAND_ALONE].val == 0 && Flags[POST_PROC_NS_BEFORE_SEND].val)
		{
/* If this flag is on, the clauses were not sent by pre_process()	*/
/* and therefore we need to send them here.				*/
		c = Usable->first_cl;
		while (c != NULL)
			{
			if (send_clause(c) == TROUBLE)
				return(TROUBLE);
			c = c->next_cl;
			}
		}
#endif
	    fprintf(Fdout,"\n------------> process sos:\n");
	    l = Sos;
	    if (get_list(&Sos) == TROUBLE)
		return(TROUBLE);
	    c = l->first_cl;
	    while (c != NULL) {
		c2 = c;
		c = c->next_cl;
		cl_clear_vars(c2);  /* destroy input variable names */
		pp = pre_process(c2,IR,Sos);
		if (pp == PROOF || pp == TROUBLE)
			return(pp);
		}
	    free_list(l);
	    c2 = NULL;
#ifndef ROO	    
	    ppa = post_proc_all((struct clause *) NULL,IR,Sos);
		if (ppa == PROOF || ppa == TROUBLE)
			return(ppa);

if (Flags[STAND_ALONE].val == 0 && Flags[POST_PROC_NS_BEFORE_SEND].val)
		{
/* If this flag is on, the clauses have not been sent by pre_process()	*/
/* and therefore we need to send them here.				*/
		c = Sos->first_cl;
		while (c != NULL)
			{
			if (send_clause(c) == TROUBLE)
				return(TROUBLE);
			c = c->next_cl;
			}
		}
#endif	    
	    fprintf(Fdout,"\n------------> done processing input.\n\n");
	    CLOCK_STOP(PROCESS_INPUT_TIME)
	    }
	else {  /* index usable and sos (not passive) */
	    for (c = Usable->first_cl; c != NULL; c = c->next_cl) {
		if (index_lits_clash(c) == TROUBLE)
			return(TROUBLE);
		if (index_lits_all(c) == TROUBLE)
			return(TROUBLE);
		}
	    for (c = Sos->first_cl; c != NULL; c = c->next_cl)
		if (index_lits_all(c) == TROUBLE)
			return(TROUBLE);
	    }
	c = Sos->first_cl;
	for (c = Sos->first_cl, i = 0; c != NULL; c = c->next_cl, i++)
	    if (Flags[INPUT_SOS_FIRST].val)
		c->weight = -MAX_INT;
	    else
		c->weight = weight_cl(c, Weight_pick_given_index);
	Stats[SOS_SIZE] = i;
/* Penguin */
	for (c = Usable->first_cl, i = 0; c != NULL; c = c->next_cl, i++)
		;
	Stats[USABLE_SIZE] = i;
	for (c = Demodulators->first_cl, i = 0; c != NULL; c = c->next_cl, i++)
		;
	Stats[DEMODULATORS_SIZE] = i;
	for (c = Passive->first_cl, i = 0; c != NULL; c = c->next_cl, i++)
		;
	Stats[PASSIVE_SIZE] = i;
	for (c = Outbound_msgs->first_cl, i = 0; c != NULL; c = c->next_cl, i++)
		;
	Stats[OUTBOUND_MSGS_SIZE] = i;
	for (c = Inbound_msgs->first_cl, i = 0; c != NULL; c = c->next_cl, i++)
		;
	Stats[INBOUND_MSGS_SIZE] = i;
	
	ug = sn_to_node(Eq_sym_num);
	if (ug->lex_val == MAX_INT)
/* If built-in equality has still MAX_INT, the default value, as	*/
/* lex_val, meaning the user has not explicitly listed "=" in		*/
/* the lex list, i.e. the precedence.			      		*/
	check_beq_lex_val();
/* Then, if any predicate symbol has been listed in the lex list	*/
/* built-in equality gets the smallest lex_val among the predicates.	*/
/* This is done by Penguin because Penguin compares also literals and */
/* clauses, while Otter compares just terms (regarding also atoms as	*/
/* terms).								*/
	}

    fflush(Fdout);
return(NO_PROOF);
}  /* read_all_input */

/*************
 *
 *    int set_lex_vals(t) 
 *    t is a lex term with a list as its one and only argument.
 *    Set lexical values of the members to 1, 2, 3, ... .
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int set_lex_vals(t)
struct term *t;
{
    struct rel *r;
    int i;
    struct sym_ent *p;

for (r=t->farg, i=1; r->argval->sym_num!=Nil_sym_num; r=r->argval->farg->narg, i++) {
	p = sn_to_node(r->argval->farg->argval->sym_num);
	if (p == NULL) {
	    output_stats(Fdout, 4);
	    fprintf(Fderr, "ABEND, error processing lex_term.\007\n");
	    fprintf(Fdout, "ABEND, error processing lex_term: ");
	    print_term_nl(Fdout, t);
	    return(TROUBLE);
	    }
	else
	    p->lex_val = i;
	}
return(NO_TROUBLE);
}  /* set_lex_vals */

/*************
 *
 *    set_lrpo_status(t, val) 
 *	 t is a lex term with a list as its one and only argument.
 *    Set lrpo_status values of the members to val.
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int set_lrpo_status(t, val)
struct term *t;
int val;
{
    struct rel *r;
    struct sym_ent *p;

for (r = t->farg; r->argval->sym_num != Nil_sym_num; r = r->argval->farg->narg) {
	p = sn_to_node(r->argval->farg->argval->sym_num);
	if (p == NULL) {
	    output_stats(Fdout, 4);
	    fprintf(Fderr, "ABEND, error processing lrpo_status.\007\n");
	    fprintf(Fdout, "ABEND, error processing lrpo_status: ");
	    print_term_nl(Fdout, t);
	    return(TROUBLE);
	    }
	else
	    p->lex_rpo_status = val;
	}
return(NO_TROUBLE);
}  /* set_lrpo_status */

/*************
 *
 *    int set_special_unary(t) 
 *	 t is a lex term with a list as its one and only argument.
 *    Set special_unary values of the members to 1.
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int set_special_unary(t)
struct term *t;
{
    struct rel *r;
    struct sym_ent *p;

for (r = t->farg; r->argval->sym_num != Nil_sym_num; r = r->argval->farg->narg) {
	p = sn_to_node(r->argval->farg->argval->sym_num);
	if (p == NULL) {
	    output_stats(Fdout, 4);
	    fprintf(Fderr, "ABEND, error processing set_special_unary.\007\n");
	    fprintf(Fdout, "ABEND, error processing set_special_unary: ");
	    print_term_nl(Fdout, t);
	    return(TROUBLE);
	    }
	else
	    p->special_unary = 1;
	}
return(NO_TROUBLE);
}  /* set_special_unary */

/*************
 *
 *    int set_skolem(t) 
 *	 t is a lex term with a list as its one and only argument.
 *
 *    Set the major function symbol (including constants) of each member of the
 *    list to be a skolem symbol.  (This is called only when skolem symbols
 *    are not created by skolemization by OTTER.)
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int set_skolem(t)
struct term *t;
{
    struct rel *r;
    struct sym_ent *p;

for (r = t->farg; r->argval->sym_num != Nil_sym_num; r = r->argval->farg->narg) {
	p = sn_to_node(r->argval->farg->argval->sym_num);
	if (p == NULL) {
	    output_stats(Fdout, 4);
	    fprintf(Fderr, "ABEND, error processing set_skolem.\007\n");
	    fprintf(Fdout, "ABEND, error processing set_skolem: ");
	    print_term_nl(Fdout, t);
	    return(TROUBLE);
	    }
	else
	    p->skolem = 1;
	}
return(NO_TROUBLE);
}  /* set_skolem */

/*************
 *
 *    int free_all_mem()
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE
 *
 *************/

int free_all_mem()
{
    struct clause *c;

    c = find_last_cl(Usable);
    while (c != NULL) {
	Stats[USABLE_SIZE]--;		/* Penguin */
        rem_from_list(c);
	if (un_index_lits_clash(c) == TROUBLE)
		return(TROUBLE);
	if (un_index_lits_all(c) == TROUBLE)
		return(TROUBLE);
	if (cl_del_int(c) == TROUBLE)
		return(TROUBLE);
	c = find_last_cl(Usable);
	}
    free_list(Usable);
    Usable = NULL;

    c = find_last_cl(Sos);
    while (c != NULL) {
	Stats[SOS_SIZE]--;
        rem_from_list(c);
	if (un_index_lits_all(c) == TROUBLE)
		return(TROUBLE);
	if (cl_del_int(c) == TROUBLE)
		return(TROUBLE);
	c = find_last_cl(Sos);
	}
    free_list(Sos);
    Sos = NULL;

    c = find_last_cl(Passive);
    while (c != NULL) {
	Stats[PASSIVE_SIZE]--;		/* Penguin */
        rem_from_list(c);
	if (un_index_lits_all(c) == TROUBLE)
		return(TROUBLE);
	if (cl_del_int(c) == TROUBLE)
		return(TROUBLE);
	c = find_last_cl(Passive);
	}
    free_list(Passive);
    Passive = NULL;
    
    c = find_last_cl(Demodulators);
    while (c != NULL) {
	Stats[DEMODULATORS_SIZE]--;	/* Penguin */
        rem_from_list(c);
	if (Flags[DEMOD_LINEAR].val == 0)  /* if imd indexing */
	    if (imd_delete(c, Demod_imd) == TROUBLE)
		return(TROUBLE);
	if (cl_del_int(c) == TROUBLE)
		return(TROUBLE);
	c = find_last_cl(Demodulators);
	}
    free_list(Demodulators);
    Demodulators = NULL;
    
/* Penguin */

    c = find_last_cl(Inbound_msgs);
    while (c != NULL) {
	Stats[INBOUND_MSGS_SIZE]--;
        rem_from_list(c);
	if (c->id > 0)
	{
	if (cl_del_int(c) == TROUBLE)
		return(TROUBLE);
	}
	else cl_del_non(c);
	c = find_last_cl(Inbound_msgs);
	}
    free_list(Inbound_msgs);
    Inbound_msgs = NULL;

    c = find_last_cl(Outbound_msgs);
    while (c != NULL) {
	Stats[OUTBOUND_MSGS_SIZE]--;
        rem_from_list(c);
	if (c->id > 0)
	{
	if (cl_del_int(c) == TROUBLE)
		return(TROUBLE);
	}
	else cl_del_non(c);
	c = find_last_cl(Outbound_msgs);
	}
    free_list(Outbound_msgs);
    Outbound_msgs = NULL;

/* Penguin */

    if (Demod_imd != NULL) {
	free_imd_tree(Demod_imd);
	Demod_imd = NULL;
	}

    /* Weight_purge_gen and Weight_pick_given might point to the same list */

    if (Weight_purge_gen != NULL) {
	weight_index_delete(Weight_purge_gen_index);
	zap_list(Weight_purge_gen);
	if (Weight_purge_gen == Weight_pick_given) {
	    Weight_pick_given = NULL;
	    Weight_pick_given_index = NULL;
	    }
	Weight_purge_gen = NULL;
	Weight_purge_gen_index = NULL;
	}

    if (Weight_pick_given != NULL) {
	weight_index_delete(Weight_pick_given_index);
	zap_list(Weight_pick_given);
	Weight_pick_given = NULL;
	Weight_pick_given_index = NULL;
	}

    if (Weight_terms != NULL) {
	weight_index_delete(Weight_terms_index);
	zap_list(Weight_terms);
	Weight_terms = NULL;
	Weight_terms_index = NULL;
	}
    free_is_tree(Is_pos_lits);
    free_is_tree(Is_neg_lits);
    Is_pos_lits = Is_neg_lits = NULL;

    if (del_hidden_clauses() == TROUBLE)
	return(TROUBLE);
    if (del_hidden_msgs() == TROUBLE)
	return(TROUBLE);
    free_sym_tab();
return(NO_TROUBLE);
}  /* free_all_mem */

/*************
 *
 *    output_stats(fp, level) -- print memory, clause, and time stats
 *
 *************/

void output_stats(fp, level)
FILE *fp;
int level;
{
    int i, j;

    if (level == 0)
	;  /* do nothing */
    else {
	if (level == 4)
	    print_options(fp);

	if (level == 1) {
	    print_mem_brief(fp);
	    print_stats_brief(fp);
	    print_times_brief(fp);
	    }
	else {
	    print_mem(fp);
	    print_stats(fp);
	    print_times(fp);
	    if (level >= 3) {
fprintf(fp, "\nForward subsumption counts, subsumer:number_subsumed.\n");
		for (i = 0; i < 10; i++) {
		    for (j = 1; j < 10; j++)
			fprintf(fp, "%2d:%-4d ", 10*i+j, Subsume_count[10*i+j]);
		    if (i < 9)  /* don't do 100 */
		fprintf(fp, "%2d:%-4d\n", 10*i+10, Subsume_count[10*i+10]);
		    else
			fprintf(fp, "\n");
		    }
		fprintf(fp, "All others: %d.\n", Subsume_count[0]);
		}
	    }
	}
}  /* output_stats */

/*************
 *
 *    print_stats(fp)
 *
 *************/

void print_stats(fp)
FILE *fp;
{
    fprintf(fp, "\n-------------- statistics -------------\n");
    fprintf(fp, "clauses input            %7ld\n", Stats[CL_INPUT]);
    fprintf(fp, "clauses given            %7ld\n", Stats[CL_GIVEN]);
    fprintf(fp, "clauses generated        %7ld\n", Stats[CL_GENERATED]);
  if (Flags[FACTOR].val)
    fprintf(fp, "  (factors generated)    %7ld\n", Stats[FACTORS]);
    fprintf(fp, "demod & eval rewrites    %7ld\n", Stats[REWRITES]);
  if (Parms[MAX_WEIGHT].val != 0 || Parms[MAX_LITERALS].val != 0 || Flags[N_RESOLUTION].val)
    fprintf(fp, "clauses wt,lit,sk delete %7ld\n", Stats[CL_WT_DELETE]);
    fprintf(fp, "tautologies deleted      %7ld\n", Stats[CL_TAUTOLOGY]);
    fprintf(fp, "clauses forward subsumed %7ld\n", Stats[CL_FOR_SUB]);
  if (Flags[ANCESTOR_SUBSUME].val)    
    fprintf(fp, "cl not subsumed due to ancestor_subsume %7ld\n", Stats[CL_NOT_ANC_SUBSUMED]);
    fprintf(fp, "  (subsumed by sos)      %7ld\n", Stats[FOR_SUB_SOS]);
  if (Flags[UNIT_DELETION].val || Flags[LINKED_UNIT_DEL].val)
    fprintf(fp, "unit deletions           %7ld\n", Stats[UNIT_DELETES]);
    fprintf(fp, "clauses kept             %7ld\n", Stats[CL_KEPT]);

#ifdef ROO
    fprintf(fp, "clauses almost kept      %7ld\n", Stats[ROO_ALMOST_KEPT]);
#endif

  if (Flags[DYNAMIC_DEMOD].val)
    fprintf(fp, "new demodulators         %7ld\n", Stats[NEW_DEMODS]);
    fprintf(fp, "empty clauses            %7ld\n", Stats[EMPTY_CLAUSES]);
  if (Flags[BACK_DEMOD].val)
    fprintf(fp, "clauses back demodulated %7ld\n", Stats[CL_BACK_DEMOD]);
    fprintf(fp, "clauses back subsumed    %7ld\n", Stats[CL_BACK_SUB]);

    fprintf(fp, "sos size             %7ld\n", Stats[SOS_SIZE]);
/* Penguin */
fprintf(fp,"usable size               %7ld\n",Stats[USABLE_SIZE]);
fprintf(fp,"passive size              %7ld\n",Stats[PASSIVE_SIZE]);
fprintf(fp,"demodulators size         %7ld\n",Stats[DEMODULATORS_SIZE]);
fprintf(fp,"outbound messages size    %7ld\n",Stats[OUTBOUND_MSGS_SIZE]);
fprintf(fp,"inbound messages size     %7ld\n",Stats[INBOUND_MSGS_SIZE]);
/* Penguin */
    fprintf(fp, "Kbytes malloced          %7ld\n", Stats[K_MALLOCED]);

  if (Flags[LINKED_UR_RES].val) {
    fprintf(fp, "linked UR depth hits     %7ld\n", Stats[LINKED_UR_DEPTH_HITS]);
    fprintf(fp, "linked UR deduct hits    %7ld\n", Stats[LINKED_UR_DED_HITS]);
/* Penguin */
fprintf(fp,"sent clauses		%7ld\n",Stats[CL_SENT]);
/* Penguin */
    }

    /* The following are output only if not 0. */
    /* They aren't errors, but they are anomalies. */

    if (Stats[CL_VAR_DELETES] != 0)
	fprintf(fp, "cl deletes, too many vars  %7ld\n", Stats[CL_VAR_DELETES]);
    if (Stats[FPA_OVERLOADS] != 0)
	fprintf(fp, "fpa argument overloads     %7ld\n", Stats[FPA_OVERLOADS]);
    if (Stats[FPA_UNDERLOADS] != 0)
	fprintf(fp, "fpa argument underloads    %7ld\n", Stats[FPA_UNDERLOADS]);

	
}  /* print_stats */

/*************
 *
 *    print_stats_brief(fp)
 *
 *************/

void print_stats_brief(fp)
FILE *fp;
{
    fprintf(fp, "\n-------------- statistics -------------\n");
    fprintf(fp, "clauses generated        %7ld\n", Stats[CL_GENERATED]);
    fprintf(fp, "clauses kept             %7ld\n", Stats[CL_KEPT]);
#ifdef ROO
    fprintf(fp, "clauses almost kept      %7ld\n", Stats[ROO_ALMOST_KEPT]);
#endif
    fprintf(fp, "clauses forward subsumed %7ld\n", Stats[CL_FOR_SUB]);
    fprintf(fp, "clauses back subsumed    %7ld\n", Stats[CL_BACK_SUB]);
}  /* print_stats_brief */

/*************
 *
 *    p_stats()
 *
 *************/

void p_stats()
{
    print_stats(Fdout);
}  /* p_stats */

/*************
 *
 *    print_times(fp)
 *
 *************/

void print_times(fp)
FILE *fp;
{
    long t, min, hr;

    fprintf(fp, "\n----------- times (seconds) -----------\n");
    t = run_time();
    fprintf(fp, "run time         %8.2f  ", t / 1000.);
    t = t / 1000; hr = t / 3600; t = t % 3600; min = t / 60; t = t % 60;
    fprintf(fp, "                 (run time  %ld hr, %ld min, %ld sec)\n", hr, min, t); 
    fprintf(fp, "system time      %8.2f\n", system_time() / 1000.);

#ifdef ROO

    fprintf(fp, "task A time      %8.2f\n", clock_val(TASK_A_TIME) /1000.);
    fprintf(fp, "task B time      %8.2f\n", clock_val(TASK_B_TIME) /1000.);
    fprintf(fp, "task C time      %8.2f\n", clock_val(TASK_C_TIME) /1000.);
    fprintf(fp, "task D time      %8.2f\n", clock_val(TASK_D_TIME) /1000.);
    fprintf(fp, "master work time %8.2f\n", clock_val(MASTER_WORK_TIME) /1000.);
    fprintf(fp, "process time nodes%7.2f\n", clock_val(PROCESS_TIME_NODE_LIST) /1000.);

#endif

    fprintf(fp, "input time       %8.2f\n", clock_val(INPUT_TIME) / 1000.);
    fprintf(fp, "  clausify time  %8.2f\n", clock_val(CLAUSIFY_TIME) / 1000.);
  if (Flags[PROCESS_INPUT].val)
    fprintf(fp, "  process input  %8.2f\n", clock_val(PROCESS_INPUT_TIME) / 1000.);
  if (Flags[BINARY_RES].val)
    fprintf(fp, "binary_res time  %8.2f\n", clock_val(BINARY_TIME) / 1000.);
  if (Flags[HYPER_RES].val)
    fprintf(fp, "hyper_res time   %8.2f\n", clock_val(HYPER_TIME) / 1000.);
  if (Flags[NEG_HYPER_RES].val)
    fprintf(fp, "neg_hyper_res time%7.2f\n", clock_val(NEG_HYPER_TIME) / 1000.);
  if (Flags[UR_RES].val)
    fprintf(fp, "UR_res time      %8.2f\n", clock_val(UR_TIME) / 1000.);
  if (Flags[PARA_INTO].val)
    fprintf(fp, "para_into time   %8.2f\n", clock_val(PARA_INTO_TIME) / 1000.);
  if (Flags[PARA_FROM].val)
    fprintf(fp, "para_from time   %8.2f\n", clock_val(PARA_FROM_TIME) / 1000.);
  if (Flags[LINKED_UR_RES].val)
    fprintf(fp, "linked_ur time   %8.2f\n", clock_val(LINKED_UR_TIME) / 1000.);
    fprintf(fp, "pre_process time %8.2f\n", clock_val(PRE_PROC_TIME) / 1000.);
    fprintf(fp, "  demod time     %8.2f\n", clock_val(DEMOD_TIME) / 1000.);
    fprintf(fp, "  weigh cl time  %8.2f\n", clock_val(WEIGH_CL_TIME) / 1000.);
    fprintf(fp, "  for_sub time   %8.2f\n", clock_val(FOR_SUB_TIME) / 1000.);
  if (Flags[UNIT_DELETION].val)
    fprintf(fp, "  unit_del time  %8.2f\n", clock_val(UNIT_DEL_TIME) / 1000.);
    fprintf(fp, "  renumber time  %8.2f\n", clock_val(RENUMBER_TIME) / 1000.);
    fprintf(fp, "  keep cl time   %8.2f\n", clock_val(KEEP_CL_TIME) / 1000.);
    fprintf(fp, "  print_cl time  %8.2f\n", clock_val(PRINT_CL_TIME) / 1000.);
    fprintf(fp, "  conflict time  %8.2f\n", clock_val(CONFLICT_TIME) / 1000.);
    fprintf(fp, "post_process time%8.2f\n", clock_val(POST_PROC_TIME) / 1000.);
  if (Flags[BACK_DEMOD].val)
    fprintf(fp, "  back demod time%8.2f\n", clock_val(BACK_DEMOD_TIME) / 1000.);
    fprintf(fp, "  back_sub time  %8.2f\n", clock_val(BACK_SUB_TIME) / 1000.);
  if (Flags[FACTOR].val)
    fprintf(fp, "  factor time    %8.2f\n", clock_val(FACTOR_TIME) / 1000.);
    fprintf(fp, "lex_rpo time     %8.2f\n", clock_val(LRPO_TIME) / 1000.);
}  /* print_times */

/*************
 *
 *    print_times_brief(fp)
 *
 *************/

void print_times_brief(fp)
FILE *fp;
{

    fprintf(fp, "\n----------- times (seconds) -----------\n");
    fprintf(fp, "run time         %8.2f\n", run_time() / 1000.);
  if (Flags[BINARY_RES].val)
    fprintf(fp, "binary_res time  %8.2f\n", clock_val(BINARY_TIME) / 1000.);
  if (Flags[HYPER_RES].val)
    fprintf(fp, "hyper_res time   %8.2f\n", clock_val(HYPER_TIME) / 1000.);
  if (Flags[NEG_HYPER_RES].val)
    fprintf(fp, "neg_hyper   time %8.2f\n", clock_val(NEG_HYPER_TIME) / 1000.);
  if (Flags[UR_RES].val)
    fprintf(fp, "UR_res time      %8.2f\n", clock_val(UR_TIME) / 1000.);
  if (Flags[PARA_INTO].val)
    fprintf(fp, "para_into time   %8.2f\n", clock_val(PARA_INTO_TIME) / 1000.);
  if (Flags[PARA_FROM].val)
    fprintf(fp, "para_from time   %8.2f\n", clock_val(PARA_FROM_TIME) / 1000.);
  if (Flags[LINKED_UR_RES].val)
    fprintf(fp, "linked_ur time   %8.2f\n", clock_val(LINKED_UR_TIME) / 1000.);
    fprintf(fp, "for_sub time     %8.2f\n", clock_val(FOR_SUB_TIME) / 1000.);
    fprintf(fp, "back_sub time    %8.2f\n", clock_val(BACK_SUB_TIME) / 1000.);
    fprintf(fp, "conflict time    %8.2f\n", clock_val(CONFLICT_TIME) / 1000.);
  if (Demodulators->first_cl != NULL || Internal_flags[DOLLAR_PRESENT])
    fprintf(fp, "demod time       %8.2f\n", clock_val(DEMOD_TIME) / 1000.);
}  /* print_times_brief */

/*************
 *
 *    p_times()
 *
 *************/

void p_times()
{
    print_times(Fdout);
}  /* p_times */

/*************
 *
 *    append_lists(l1, l2) -- append l2 to l1 and free the header node l2
 *
 *************/

void append_lists(l1, l2)
struct list *l1;
struct list *l2;
{
    struct clause *c;

    if (l1->last_cl != NULL)  /* if l1 not empty */
        l1->last_cl->next_cl = l2->first_cl;
    else
	l1->first_cl = l2->first_cl;

    if (l2->first_cl != NULL) {  /* if l2 not empty */
	l2->first_cl->prev_cl = l1->last_cl;
	l1->last_cl = l2->last_cl;
	}

    c = l2->first_cl;
    while (c != NULL) {
	c->container = l1;
	c = c->next_cl;
	}

    free_list(l2);
}  /* append_lists */

/*************
 *
 *    int copy_and_append_list(l1, l2) -- Penguin
 *	copy the elements of l1 and append them to l2
 *
 *	It returns TROUBLE/NO_TROUBLE.
 *
 *************/

int copy_and_append_list(l1, l2)
struct list *l1;
struct list *l2;
{
    struct clause *c1, *c2;

	if (l1 == NULL)
		return(NO_TROUBLE);
    if (l1->first_cl != NULL)
	{
    	if (l2 == NULL)
		if (get_list(&l2) == TROUBLE)
			return(TROUBLE);

	c1 = l1->first_cl;
	
	while (c1 != NULL)
	{
	if (cl_copy(c1, &c2) == TROUBLE)
		return(TROUBLE);
	append_cl(l2,c2);
	c1 = c1->next_cl;
	}
	}
return(NO_TROUBLE);

}  /* copy_and_append_list */

/*************
*
*	int half_list(list1,list2,list1_length)		Penguin
*
*	It appends to list2 half the elements of list1.
*	It returns TROUBLE/NO_TROUBLE.
*
**************/

int half_list(list1,list2,list1_length)
struct list *list1, *list2;
int list1_length;
{
int n;
struct clause *c;

if (list2 == NULL)
	if (get_list(&list2) == TROUBLE)
		return(TROUBLE);

for (n=0; n < list1_length / 2; n++)
	{
	if (extract_given_clause(list1,&c) == TROUBLE)
		return(TROUBLE);
/* extract_given_clause() decrements Stats[_SIZE] */
	append_cl(list2,c);
	if (list2 == Sos)
		Stats[SOS_SIZE]++;
	else if (list2 == Usable)
		Stats[USABLE_SIZE]++;
	else if (list2 == Passive)
		Stats[PASSIVE_SIZE]++;
	else if (list2 == Outbound_msgs)
		Stats[OUTBOUND_MSGS_SIZE]++;
	else if (list2 == Inbound_msgs)
		Stats[INBOUND_MSGS_SIZE]++;
	}
return(NO_TROUBLE);
} /* half_list */

/*************
 *
 *    int copy_term(term, ct) -- Return a copy of the term.
 *
 *    The bits field is not copied.
 *
 *	struct term * in Otter, int in Penguin, as it returns TROUBLE/
 *	NO_TROUBLE. It returns the pointer to struct term through the
 *	parameter ct.
 *
 *************/

int copy_term(t, ct)
struct term *t;
struct term **ct;
{
    struct rel *r, *r2, *r3;
    struct term *t2;
	struct term *tempterm;

	*ct = NULL;				/* default */
    if (get_term(&t2) == TROUBLE)
	return(TROUBLE);
    t2->type = t->type;
    t2->sym_num = t->sym_num;
    t2->varnum = t->varnum;
    if (t->type != COMPLEX)
	{
	*ct = t2;
	return(NO_TROUBLE);
	}
    else {
	r3 = NULL;
	r = t->farg;
	while (r != NULL) {
	    if (get_rel(&r2) == TROUBLE)
		return(TROUBLE);
	    if (r3 == NULL)
		t2->farg = r2;
	    else 
		r3->narg = r2;
	    if (copy_term(r->argval, &tempterm) == TROUBLE)
		return(TROUBLE);
		r2->argval = tempterm;
	    r3 = r2;
	    r = r->narg;
	    }
	*ct = t2;
	return(NO_TROUBLE);
	}
}  /* copy_term */

/*************
 *
 *    int biggest_var(term)  --  return largest variable number (-1 if none)
 *
 *************/

int biggest_var(t)
struct term *t;
{
    struct rel *r;
    int i, j;

    if (t->type == VARIABLE)
	return(t->varnum);
    else if (t->type == NAME) 
	return(-1);
    else {
	r = t->farg;
	i = -1;
	while (r != NULL) {
	    j = biggest_var(r->argval);
	    if (j > i)
		i = j;
	    r = r->narg;
	    }
	return(i);
	}
}  /* biggest_var */

/*************
 *
 *    zap_list(term_ptr) -- Free a list of nonintegrated terms.
 *
 *************/

void zap_list(p)
struct term_ptr *p;
{
    struct term_ptr *q;

    while (p != NULL) {
	zap_term(p->term);
	q = p;
	p = p->next;
	free_term_ptr(q);
	}
}  /* zap_list */

/*************
 *
 *     int occurs_in(t1, t2) -- Does t1 occur in t2?
 *
 *     term_ident is used to check identity.
 *
 *************/

int occurs_in(t1, t2)
struct term *t1;
struct term *t2;
{
    struct rel *r;

    if (term_ident(t1, t2))
	return(1);
    else if (t2->type != COMPLEX)
	return(0);
    else {
	r = t2->farg;
	while (r != NULL && occurs_in(t1, r->argval) == 0) 
	    r = r->narg;
	return(r != NULL);
	}
}  /* occurs_in */

/*************
 *
 *    int sn_occur(sn, t)
 *
 *    Is sn the sym_num of t or any subterms of t?
 *
 *************/

int sn_occur(sn, t)
int sn;
struct term *t;
{
    struct rel *r;
    int occurs;

    if (t->type != COMPLEX)
	return(t->sym_num == sn);
    else if (t->sym_num == sn)
	return(1);
    else {
	occurs = 0;
	r = t->farg;
	while (r != NULL && occurs == 0) {
	    occurs = sn_occur(sn, r->argval);
	    r = r->narg;
	    }
	return(occurs);
	}
}  /* sn_occur */

/*************
 *
 *    is is_atom(t) -- Is t an atom?
 *
 *    A term is an atom iff it is not a variable and varnum != 0.
 *    (The varnum field of an atom gives its type---equality, answer, evaluable, etc.)
 *
 *************/

int is_atom(t)
struct term *t;
{
    return(t->type != VARIABLE && t->varnum != 0);
}  /* is_atom */

/*************
 *
 *    int id_nested_skolems(t,ins)
 *
 *    Does t or any of its subterms have the identical_nested_skolems property?
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE and 0/1 through the
 *	parameter ins.
 *
 *************/

static int id_nested_skolems(t,ins)
struct term *t;
int *ins;
{
    struct rel *r;
    int occurs;
	int tempint;

	*ins = 0;				/* default */
    if (t->type != COMPLEX)
	return(NO_TROUBLE);
    else {
	occurs = 0;
	if (is_skolem(t->sym_num,&tempint) == TROUBLE)
		return(TROUBLE);
	if (tempint == 1) {
	    r = t->farg;
	    while (r != NULL && occurs == 0) {
		occurs = sn_occur(t->sym_num, r->argval);
		r = r->narg;
		}
	    }
	if (occurs)
	{
		*ins = 1;
	    return(NO_TROUBLE);
	}
	else {
	    occurs = 0;
	    r = t->farg;
	    while (r != NULL && occurs == 0) {
		if (id_nested_skolems(r->argval,&occurs) == TROUBLE)
			return(TROUBLE);
		r = r->narg;
		}
		*ins = occurs;
	    return(NO_TROUBLE);
	    }
	}
}  /* id_nested_skolems */

/*************
 *
 *    int ident_nested_skolems(c,insk)
 *
 *    Do any of the terms in clause c have the
 *    identical_nested_skolems property?
 *
 *	Penguin: it returns TROUBLE/NO_TROUBLE and 0/1 through the
 *	parameter insk.
 *
 *************/

int ident_nested_skolems(c,insk)
struct clause *c;
int *insk;
{
    struct literal *l;
    int occurs;

    l = c->first_lit;
    occurs = 0;
    while (l != NULL && occurs == 0) {
	if (id_nested_skolems(l->atom,&occurs) == TROUBLE)
		return(TROUBLE);
	l = l->next_lit;
	}
	*insk = occurs;
    return(NO_TROUBLE);
}  /* ident_nested_skolems */

/*************
 *
 *    int ground(t) -- is a term ground?
 *
 *************/

int ground(t)
struct term *t;
{
    struct rel *r;
    int ok;

    if (t->type == NAME)
	return(1);
    else if (t->type == VARIABLE)
	return(0);
    else { /* COMPLEX */
	ok = 1;
	for (r = t->farg; r != NULL && ok; r = r->narg)
	    ok = ground(r->argval);
	return(ok);
	}
}  /* ground */

/*************
 *
 *    void cleanup(casus)
 *
 *	Penguin adds the parameter casus to distinguish whether cleanup()
 *	is being invoked at the end of the computation (casus == FINAL)
 *	or when the Sos is empty and the Penguin is waiting for messages
 *	(casus == PERIODICAL).
 *
 *************/

void cleanup(casus)
int casus;
{
    int i;

if (casus == FINAL)
fprintf(Fdout,"\n------------ END OF SEARCH AT NODE %d ------------\n",Whoami);
else if (casus == PERIODICAL)
fprintf(Fdout,"\n------------ SITUATION AT NODE %d ------------\n",Whoami);

    if (Flags[PRINT_LISTS_AT_END].val) {
	fprintf(Fdout,"\nlist(usable).\n"); print_cl_list(Fdout, Usable);
	fprintf(Fdout,"\nlist(sos).\n"); print_cl_list(Fdout, Sos);
	if (Demodulators != NULL) {
	    fprintf(Fdout,"\nlist(demodulators).\n");
	    print_cl_list(Fdout, Demodulators);
	    }
	fprintf(Fdout,"\n");
	}

/* Otter had a call to free_all_mem() here, if Flags[FREE_ALL_MEM] is set, */
/* but Penguin delays the call to free_all_mem() and has it called by	*/
/* new_clean_up(), called in turn by the pcn function penguin() itself,	*/
/* so that free_all_mem() is invoked only after both main_infer() and	*/
/* receive() have terminated.						*/

    if (Parms[STATS_LEVEL].val >= 2 && casus == FINAL)
	print_options(Fdout);

if (casus == FINAL)
	output_stats(Fdout, Parms[STATS_LEVEL].val);
else if (casus == PERIODICAL)
	output_stats(Fdout,1);

#ifdef ROO

    print_mem_from_struct(Fdout, &(Glob->memory_stats));

    /* print communication lists only if length <= 200 */
    
    fprintf(Fdout,"\nk_list InDeX:\n");
    for (i = 0; i < K_INDEX_SIZE; i++)
	if (Glob->k_index[i] != NULL) {
	    fprintf(Fdout,"first of weight %d: ", i);
	    print_clause(Fdout, Glob->k_index[i]);
	    }
    
    fprintf(Fdout,"\nk_list:\n");
    if (Glob->k_length > 200)
	fprintf(Fdout,"    size=%d.\n", Glob->k_length);
    else
	print_cl_list(Fdout, Glob->k_list);

    fprintf(Fdout,"\nm_list:\n");
    if (Glob->m_length > 200)
	fprintf(Fdout,"    size=%d.\n", Glob->m_length);
    else
        print_cl_ptr_list(Fdout, &Glob->m_list);

    fprintf(Fdout,"\nn_list:\n");
    if (Glob->n_length > 200)
	fprintf(Fdout,"    size=%d.\n", Glob->n_length);
    else
	print_cl_ptr_list(Fdout, &Glob->n_list);
    
    fprintf(Fdout,"\nK-list high-water-mark = %d\n", Glob->k_hwm);
    fprintf(Fdout,"M-list high-water-mark = %d\n", Glob->m_hwm);
    fprintf(Fdout,"N-list high-water-mark = %d\n", Glob->n_hwm);

#endif  /* ROO */

if (casus == FINAL)
	fprintf(Fdout,"The job finished        %s", get_time());
else if (casus == PERIODICAL)
	fprintf(Fdout,"\n");

}  /* cleanup */

/*************
 *
 *    int check_stop()  --  Should the search be terminated?
 *
 *    return:
 *        0 if we should not stop;
 *        1 if we should stop because of max_given option;
 *        2 if we should stop because of max_seconds option;
 *        3 if we should stop because of max_gen option;
 *        4 if we should stop because of max_kept option.
 *
 *************/

int check_stop()
{
    long given, seconds, gen, kept;
    int max_given, max_seconds, max_gen, max_kept;

#ifdef ROO
    given = Glob->giv_cl_count;
#else
    given = Stats[CL_GIVEN];
#endif
    seconds = run_time() / 1000;
    gen = Stats[CL_GENERATED];
    kept = Stats[CL_KEPT];

    max_given = Parms[MAX_GIVEN].val;
    max_seconds = Parms[MAX_SECONDS].val;
    max_gen = Parms[MAX_GEN].val;
    max_kept = Parms[MAX_KEPT].val;

    if (max_given != 0 && given >= max_given)
	return(1);
    else if (max_seconds != 0 && seconds >= max_seconds)
	return(2);
    else if (max_gen != 0 && gen >= max_gen)
	return(3);
    else if (max_kept != 0 && kept >= max_kept)
	return(4);
    else
	return(0);
}  /* check_stop */

/*************
 *
 *    report() -- possibly report statistics and times
 *
 *************/

void report()
{
    static int next_report;
    float runtime;

    if (next_report == 0)
	next_report = Parms[REPORT].val;

    runtime = run_time() / 1000.;
    if (runtime >= next_report) {
fprintf(Fdout,"\n----- report at %9.2f seconds ----- %s", runtime, get_time());
	output_stats(Fdout, Parms[STATS_LEVEL].val);
fprintf(Fderr,"A report (%.2f seconds) has been sent to the output file.\007\n", runtime);
	while (runtime >= next_report)
	    next_report += Parms[REPORT].val;
	}
}  /* report */

/*************
 *
 *    void reduce_weight_limit()
 *
 *************/

void reduce_weight_limit()
{
    int i, new_limit, g;

    /* example:  3975 means at CL_GIVEN==39, reduce limit to 75 */ 

    i = Parms[REDUCE_WEIGHT_LIMIT].val;
    g = i / 100;
    new_limit = i % 100;

    if (Stats[CL_GIVEN] == g) {
	Parms[MAX_WEIGHT].val = new_limit;
	fprintf(Fdout,"\nreducing weight limit to %d.\n", new_limit);
	}
}  /* reduce_weight_limit */

/*************
 *
 *    void control_memory()
 *
 *************/

void control_memory()
{
    static int next_control_point = 0;
    int sos_distribution[500];
    int i, j, wt, n, control;
    struct clause *c;

    if (Parms[MAX_MEM].val != 0 && total_mem()*3 > Parms[MAX_MEM].val) {
	if (!next_control_point)
	    control = 1;
	else if (next_control_point == Stats[CL_GIVEN])
	    control = 1;
	else
	    control = 0;
	}
    else
	control = 0;

    if (control) {
	next_control_point = Stats[CL_GIVEN] + 10;
	for (i = 0; i < 500; i++)
	    sos_distribution[i] = 0;
	for (c = Sos->first_cl; c; c = c->next_cl) {
	    if (c->weight < 0)
		wt = 0;
	    else if (c->weight >= 500)
		wt = 499;
	    else
		wt = c->weight;
	    sos_distribution[wt]++;
	    }

	i = 0; n = 0;
	while (i < 500 && n*20 <= Stats[SOS_SIZE]) {
	    n += sos_distribution[i];
	    i++;
	    }
	i--;
	
	/* reset weight limit to i */

	if (i < Parms[MAX_WEIGHT].val || Parms[MAX_WEIGHT].val == 0) {
	    Parms[MAX_WEIGHT].val = i;
	    fprintf(Fderr, "\007\n\n");
	    fprintf(Fdout,"\nResetting weight limit to %d.\n\n", i);
	    fprintf(Fderr, "\nResetting weight limit to %d.\n\n", i);
	    fprintf(Fdout,"sos_size=%d\n", Stats[SOS_SIZE]);
	    fprintf(Fdout,"weight: number of sos clauses with that weight\n");
	    for (j = 0; j < 100; j++)
		fprintf(Fdout,"%d:  %d\n", j, sos_distribution[j]);
	    }
	}
	
}  /* control_memory */

/*************
 *
 *    proof_message()
 *
 *************/

static void proof_message()
{

#ifdef TP_SUN

    /* system call cuserid (to get username) seems to be on SUN only */

    long i;
    char s[L_cuserid];

    i = run_time();  /* i is milliseconds */
    
    if (i > 10000) {
	/* If more than 10 seconds, print excitedly. (Now that's real AI!) */
	cuserid(s);   /* get username */
	printf("Penguin%d: We have a proof, %s, WE HAVE A PROOF!!",Whoami,s);
	}
    else
	printf("Penguin%d: ---------------- PROOF ----------------",Whoami);

    printf("\007\n\n");

#else

    printf("Penguin%d: ---------------- PROOF ----------------",Whoami);
    printf("\007\n\n");

#endif

}  /* proof_message */

/*************
 *
 *    int print_proof(fp, c)
 *
 *	void in Otter, int in Penguin, as it returns TROUBLE/NO_TROUBLE.
 *
 *************/

int print_proof(fp, c)
FILE *fp;
struct clause *c;
{
    struct clause_ptr *cp1, *cp2, *cp3;

    cp1 = NULL;
    if (get_ancestors(c, &cp1) == TROUBLE)
	return(TROUBLE);
    while (cp1 != NULL) {
	cp3 = cp1->next;
	if (cp3 != NULL && cp3->c->parents != NULL &&
	                   cp3->c->parents->i == NEW_DEMOD_RULE &&
			   cp3->c->parents->next->i == cp1->c->id) {
	    /* skip over dynamic demodulator copy */
	    fprintf(fp, "%d,", cp3->c->id);
	    print_clause(fp, cp1->c);
	    cp2 = cp1;
	    cp1 = cp3->next;
	    free_clause_ptr(cp2);
	    free_clause_ptr(cp3);
	    }
	else {
	    print_clause(fp, cp1->c);
	    cp2 = cp1;
	    cp1 = cp1->next;
	    free_clause_ptr(cp2);
	    }
	}
return(NO_TROUBLE);
}  /* print_proof */

/*************
 *
 *    check_for_proof(c)
 *
 *	struct clause * in Otter, int in Penguin, as it returns
 *	TROUBLE/NO_TROUBLE. It does not return the empty clause as it is not
 *	used by pre_process(). The reason why check_for_proof() does not
 *	return PROOF, is that it communicates the finding of an empty clause
 *	by increasing Stats[EMPTY_CLAUSES]. It will be up to pre_process()
 *	to return PROOF: see pre_process() in process.c.
 *
 *************/

int check_for_proof(c)
struct clause *c;
{
    struct clause *e;
    struct clause_ptr *cp1, *cp2;
    int number_of_lits;
	int prle;


    e = NULL;					/* default */
    number_of_lits = num_literals(c);
    if (number_of_lits == 0) {
#ifdef ROO
	ALOG_LOG(Pid, 9, c->id, "");
	Glob->proofs_found++;
#endif
        fprintf(Fdout,"\n-----> EMPTY CLAUSE at %6.2f sec ----> ",
                         run_time() / 1000.);

	print_clause(Fdout, c);
	fprintf(Fdout,"\n");
#ifndef ROO
	if (c->container == Sos)
	    Stats[SOS_SIZE]--;
	else if (c->container == Usable)		/* Penguin */
	    Stats[USABLE_SIZE]--;
	else if (c->container == Passive)		/* Penguin */
	    Stats[PASSIVE_SIZE]--;
	else if (c->container == Demodulators)		/* Penguin */
	    Stats[DEMODULATORS_SIZE]--;
	else if (c->container == Outbound_msgs)		/* Penguin */
	    Stats[OUTBOUND_MSGS_SIZE]--;
	else if (c->container == Inbound_msgs)		/* Penguin */
	    Stats[INBOUND_MSGS_SIZE]--;
	Stats[CL_KEPT]--;  /* don't count empty clauses */
	rem_from_list(c);  /* pre_process has already KEPT it */
#endif	
	if (c->container == Outbound_msgs || c->container == Inbound_msgs)
		hide_msg(c);
	else hide_clause(c);
	Stats[EMPTY_CLAUSES]++;
	e = c;
	if (Flags[PRINT_PROOFS].val) {
	    proof_message();
	    fprintf(Fdout,"Level of proof is %d, ", proof_level(e)-1);
	if (proof_length(e,&prle) == TROUBLE)
		return(TROUBLE);
	    fprintf(Fdout,"length is %d.\n", prle-1);
	    fprintf(Fdout,"\n---------------- PROOF ----------------\n\n");
	    if (print_proof(Fdout, e) == TROUBLE)
		return(TROUBLE);
	    fprintf(Fdout,"\n------------ end of proof -------------\n\n");
	    fflush(Fdout);
	    }
	}
    else if (number_of_lits == 1) {
	CLOCK_START(CONFLICT_TIME)
	if (unit_conflict(c,&cp1) == TROUBLE)
		return(TROUBLE);
	CLOCK_STOP(CONFLICT_TIME)
	while (cp1 != NULL) {  /* empty clause from unit conflict */
            e = cp1->c;
#ifdef ROO
	    ALOG_LOG(Pid, 9, e->id, "");
	    Glob->proofs_found++;
#endif
	    cp2 = cp1->next;
	    free_clause_ptr(cp1);
	    cp1 = cp2;

	    if (cl_integrate(e,EC,NONE) == TROUBLE)
		return(TROUBLE);
/* Penguin: the second parameter is Empty Clause.			*/
	    fprintf(Fdout,"\n----> UNIT CONFLICT at %6.2f sec ----> ",
			 run_time() / 1000.);
	    print_clause(Fdout, e);
	    fprintf(Fdout,"\n");
	    hide_clause(e);
	    if (Flags[PRINT_PROOFS].val) {
		proof_message();
		fprintf(Fdout,"Level of proof is %d, ", proof_level(e)-1);
	if (proof_length(e,&prle) == TROUBLE)
		return(TROUBLE);
		fprintf(Fdout,"length is %d.\n", prle-1);
		fprintf(Fdout,"\n---------------- PROOF ----------------\n\n");
		if (print_proof(Fdout, e) == TROUBLE)
			return(TROUBLE);
		fprintf(Fdout,"\n------------ end of proof -------------\n\n");
		fflush(Fdout);
		}	/* end of if Flags[PRINT_PROOFS].val */
	    }	/* end of while */
	}	/* end of number of literals is 1 */
    return(NO_TROUBLE);
}  /* check_for_proof() */

