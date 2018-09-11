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
 *  options.c -- Routines to manage flags and parameters.
 *
 */

#include "header.h"

/*************
 *
 *    init_options()
 *
 *************/

void init_options()
{
    int i;

    for (i = 0; i < MAX_FLAGS; i++)
	Flags[i].name = "";
    for (i = 0; i < MAX_PARMS; i++)
	Parms[i].name = "";

    /* flags are boolean valued options */

    Flags[BINARY_RES].name = "binary_res";
    Flags[BINARY_RES].val = 0;

    Flags[HYPER_RES].name = "hyper_res";
    Flags[HYPER_RES].val = 0;

    Flags[NEG_HYPER_RES].name = "neg_hyper_res";
    Flags[NEG_HYPER_RES].val = 0;

    Flags[UR_RES].name = "ur_res";
    Flags[UR_RES].val = 0;

    Flags[PARA_FROM].name = "para_from";
    Flags[PARA_FROM].val = 0;

    Flags[PARA_INTO].name = "para_into";
    Flags[PARA_INTO].val = 0;

    Flags[PARA_FROM_LEFT].name = "para_from_left";
    Flags[PARA_FROM_LEFT].val = 1;

    Flags[PARA_FROM_RIGHT].name = "para_from_right";
    Flags[PARA_FROM_RIGHT].val = 1;

    Flags[PARA_FROM_VARS].name = "para_from_vars";
    Flags[PARA_FROM_VARS].val = 0;

    Flags[PARA_INTO_VARS].name = "para_into_vars";
    Flags[PARA_INTO_VARS].val = 0;

    Flags[PARA_INTO_LEFT].name = "para_into_left";
    Flags[PARA_INTO_LEFT].val = 1;

    Flags[PARA_INTO_RIGHT].name = "para_into_right";
    Flags[PARA_INTO_RIGHT].val = 1;

    Flags[PARA_ONES_RULE].name = "para_ones_rule";
    Flags[PARA_ONES_RULE].val = 0;

    Flags[PARA_ALL].name = "para_all";
    Flags[PARA_ALL].val = 0;

    Flags[DEMOD_INF].name = "demod_inf";
    Flags[DEMOD_INF].val = 0;

    Flags[DEMOD_LINEAR].name = "demod_linear";
    Flags[DEMOD_LINEAR].val = 0;

    Flags[VERY_VERBOSE].name = "very_verbose";
    Flags[VERY_VERBOSE].val = 0;

    Flags[FOR_SUB_FPA].name = "for_sub_fpa";
    Flags[FOR_SUB_FPA].val = 0;

    Flags[FOR_SUB].name = "for_sub";
    Flags[FOR_SUB].val = 1;

    Flags[BACK_SUB].name = "back_sub";
    Flags[BACK_SUB].val = 1;

    Flags[FREE_ALL_MEM].name = "free_all_mem";
    Flags[FREE_ALL_MEM].val = 0;

    Flags[NO_FAPL].name = "no_fapl";
    Flags[NO_FAPL].val = 0;

    Flags[NO_FANL].name = "no_fanl";
    Flags[NO_FANL].val = 0;

    Flags[FACTOR].name = "factor";
    Flags[FACTOR].val = 0;

    Flags[PRINT_KEPT].name = "print_kept";
    Flags[PRINT_KEPT].val = 1;

    Flags[DEMOD_HISTORY].name = "demod_history";
    Flags[DEMOD_HISTORY].val = 1;

    Flags[UNIT_DELETION].name = "unit_deletion";
    Flags[UNIT_DELETION].val = 0;

    Flags[SORT_LITERALS].name = "sort_literals";
    Flags[SORT_LITERALS].val = 0;

    Flags[PRINT_GIVEN].name = "print_given";
    Flags[PRINT_GIVEN].val = 1;

    Flags[PRINT_BACK_SUB].name = "print_back_sub";
    Flags[PRINT_BACK_SUB].val = 1;

    Flags[CHECK_ARITY].name = "check_arity";
    Flags[CHECK_ARITY].val = 1;

    Flags[SOS_QUEUE].name = "sos_queue";
    Flags[SOS_QUEUE].val = 0;

    Flags[BIRD_PRINT].name = "bird_print";
    Flags[BIRD_PRINT].val = 0;

    Flags[ATOM_WT_MAX_ARGS].name = "atom_wt_max_args";
    Flags[ATOM_WT_MAX_ARGS].val = 0;

    Flags[TERM_WT_MAX_ARGS].name = "term_wt_max_args";
    Flags[TERM_WT_MAX_ARGS].val = 0;

    Flags[PRINT_LISTS_AT_END].name = "print_lists_at_end";
    Flags[PRINT_LISTS_AT_END].val = 0;

    Flags[ORDER_EQ].name = "order_eq";
    Flags[ORDER_EQ].val = 0;

    Flags[DYNAMIC_DEMOD].name = "dynamic_demod";
    Flags[DYNAMIC_DEMOD].val = 0;

    Flags[BACK_DEMOD].name = "back_demod";
    Flags[BACK_DEMOD].val = 0;

    Flags[PRINT_NEW_DEMOD].name = "print_new_demod";
    Flags[PRINT_NEW_DEMOD].val = 1;

    Flags[PRINT_BACK_DEMOD].name = "print_back_demod";
    Flags[PRINT_BACK_DEMOD].val = 1;

    Flags[DEMOD_OUT_IN].name = "demod_out_in";
    Flags[DEMOD_OUT_IN].val = 0;

    Flags[PROCESS_INPUT].name = "process_input";
    Flags[PROCESS_INPUT].val = 0;

    Flags[SIMPLIFY_FOL].name = "simplify_fol";
    Flags[SIMPLIFY_FOL].val = 0;

    Flags[KNUTH_BENDIX].name = "knuth_bendix";
    Flags[KNUTH_BENDIX].val = 0;

    Flags[NEW_FUNCTIONS].name = "new_functions";
    Flags[NEW_FUNCTIONS].val = 0;

    Flags[PRINT_PROOFS].name = "print_proofs";
    Flags[PRINT_PROOFS].val = 1;

    Flags[SYMBOL_ELIM].name = "symbol_elim";
#ifdef ROO
    Flags[SYMBOL_ELIM].val = 0;
#else
    Flags[SYMBOL_ELIM].val = 1;
#endif

    Flags[LEX_ORDER_VARS].name = "lex_order_vars";
    Flags[LEX_ORDER_VARS].val = 0;

    Flags[DYNAMIC_DEMOD_ALL].name = "dynamic_demod_all";
    Flags[DYNAMIC_DEMOD_ALL].val = 0;

  Flags[DELETE_IDENTICAL_NESTED_SKOLEM].name = "delete_identical_nested_skolem";
    Flags[DELETE_IDENTICAL_NESTED_SKOLEM].val = 0;

    Flags[PARA_FROM_UNITS_ONLY].name = "para_from_units_only";
    Flags[PARA_FROM_UNITS_ONLY].val = 0;

    Flags[PARA_INTO_UNITS_ONLY].name = "para_into_units_only";
    Flags[PARA_INTO_UNITS_ONLY].val = 0;

    Flags[REALLY_DELETE_CLAUSES].name = "really_delete_clauses";
    Flags[REALLY_DELETE_CLAUSES].val = 0;

    Flags[REALLY_DELETE_MSGS].name = "really_delete_msgs";	/* Penguin */
    Flags[REALLY_DELETE_MSGS].val = 1;

    Flags[LEX_RPO].name = "lex_rpo";
    Flags[LEX_RPO].val = 0;

    Flags[PROLOG_STYLE_VARIABLES].name = "prolog_style_variables";
    Flags[PROLOG_STYLE_VARIABLES].val = 0;

    Flags[SOS_STACK].name = "sos_stack";
    Flags[SOS_STACK].val = 0;

    Flags[DYNAMIC_DEMOD_LEX_DEP].name = "dynamic_demod_lex_dep";
    Flags[DYNAMIC_DEMOD_LEX_DEP].val = 0;

    Flags[PROG_SYNTHESIS].name = "prog_synthesis";
    Flags[PROG_SYNTHESIS].val = 0;

    Flags[ANCESTOR_SUBSUME].name = "ancestor_subsume";
    Flags[ANCESTOR_SUBSUME].val = 0;

    Flags[INPUT_SOS_FIRST].name = "input_sos_first";
    Flags[INPUT_SOS_FIRST].val = 0;

    Flags[LINKED_UR_RES].name = "linked_ur_res";
    Flags[LINKED_UR_RES].val = 0;

    Flags[LINKED_UR_TRACE].name = "linked_ur_trace";
    Flags[LINKED_UR_TRACE].val = 0;

    Flags[PARA_SKIP_SKOLEM].name = "para_skip_skolem";
    Flags[PARA_SKIP_SKOLEM].val = 0;

    Flags[INDEX_FOR_BACK_DEMOD].name = "index_for_back_demod";
    Flags[INDEX_FOR_BACK_DEMOD].val = 0;

    Flags[LINKED_SUB_UNIT_USABLE].name = "linked_sub_unit_usable";
    Flags[LINKED_SUB_UNIT_USABLE].val = 0;

    Flags[LINKED_SUB_UNIT_SOS].name = "linked_sub_unit_sos";
    Flags[LINKED_SUB_UNIT_SOS].val = 0;

    Flags[LINKED_UNIT_DEL].name = "linked_unit_del";
    Flags[LINKED_UNIT_DEL].val = 0;

    Flags[LINKED_TARGET_ALL].name = "linked_target_all";
    Flags[LINKED_TARGET_ALL].val = 0;

    Flags[LINKED_HYPER_RES].name = "linked_hyper_res";
    Flags[LINKED_HYPER_RES].val = 0;

    Flags[CONTROL_MEMORY].name = "control_memory";
    Flags[CONTROL_MEMORY].val = 0;

    Flags[N_RESOLUTION].name = "n_resolution";
    Flags[N_RESOLUTION].val = 0;

    Flags[ORDER_HISTORY].name = "order_history";
    Flags[ORDER_HISTORY].val = 0;

    Flags[SUPPRESS_WEIGHT_WARNING].name = "suppress_weight_warning";
    Flags[SUPPRESS_WEIGHT_WARNING].val = 0;

/* Penguin: see file cos.h for comments on these flags.			*/

	Flags[SATURATION].name = "saturation";
	Flags[SATURATION].val = 0;

	Flags[ALTERNATE_FIT].name = "alternate_fit";
	Flags[ALTERNATE_FIT].val = 1;

	Flags[FIRST_FIT].name = "first_fit";
	Flags[FIRST_FIT].val = 0;

	Flags[ALT_FIRST_FIT].name = "alt_first_fit";
	Flags[ALT_FIRST_FIT].val = 0;

	Flags[HALF_ALT_FIT].name = "half_alt_fit";
	Flags[HALF_ALT_FIT].val = 0;

	Flags[OWN_FACTORS].name = "own_factors";
	Flags[OWN_FACTORS].val = 1;

	Flags[OWN_NFR].name = "own_nfr";
	Flags[OWN_NFR].val = 1;

	Flags[OWN_IN_USABLE].name = "own_in_usable";
	Flags[OWN_IN_USABLE].val = 0;

	Flags[OWN_IN_SOS].name = "own_in_sos";
	Flags[OWN_IN_SOS].val = 0;

	Flags[IN_MSG_QUEUE].name = "in_msg_queue";
	Flags[IN_MSG_QUEUE].val = 0;

	Flags[IN_MSG_STACK].name = "in_msg_stack";
	Flags[IN_MSG_STACK].val = 0;

	Flags[OUT_MSG_QUEUE].name = "out_msg_queue";
	Flags[OUT_MSG_QUEUE].val = 0;

	Flags[OUT_MSG_STACK].name = "out_msg_stack";
	Flags[OUT_MSG_STACK].val = 0;

	Flags[PRIORITY_MSGS].name = "priority_msgs";
	Flags[PRIORITY_MSGS].val = 0;

	Flags[SOS_QUEUE_MOD].name = "sos_queue_mod";
	Flags[SOS_QUEUE_MOD].val = 0;

	Flags[SOS_D_LIGHT].name = "sos_d_light";
	Flags[SOS_D_LIGHT].val = 0;

	Flags[PRINT_RECEIVED].name = "print_received";
	Flags[PRINT_RECEIVED].val = 1;

	Flags[PRINT_SENT].name = "print_sent";
	Flags[PRINT_SENT].val = 1;

	Flags[PRINT_ALLOC].name = "print_alloc";
	Flags[PRINT_ALLOC].val = 1;

	Flags[PRINT_UPDATES].name = "print_updates";
	Flags[PRINT_UPDATES].val = 1;

	Flags[PART_FACTORS].name = "part_factors";
	Flags[PART_FACTORS].val = 1;

	Flags[POST_PROC_NS_BEFORE_SEND].name = "post_proc_ns_before_send";
	Flags[POST_PROC_NS_BEFORE_SEND].val = 0;

	Flags[EAGER_BD_INF_MSGS].name = "eager_bd_inf_msgs";
	Flags[EAGER_BD_INF_MSGS].val = 1;

	Flags[STAND_ALONE].name = "stand_alone";
	Flags[STAND_ALONE].val = 0;

/* Penguin */

    /* parms are integer valued options */

    Parms[FPA_LITERALS].name = "fpa_literals";
#ifdef TURBO_C  /* to save memory */
    Parms[FPA_LITERALS].val = 3;
#else
    Parms[FPA_LITERALS].val = 8;
#endif
    Parms[FPA_LITERALS].min = 0;
    Parms[FPA_LITERALS].max = 100;  /* check MAX_PATH before increasing */

    Parms[FPA_TERMS].name = "fpa_terms";
#ifdef TURBO_C  /* to save memory */
    Parms[FPA_TERMS].val = 3;
#else
    Parms[FPA_TERMS].val = 8;
#endif
    Parms[FPA_TERMS].min = 0;
    Parms[FPA_TERMS].max = 100;  /* check MAX_PATH before increasing */

    Parms[DEMOD_LIMIT].name = "demod_limit";
    Parms[DEMOD_LIMIT].val = 100;
    Parms[DEMOD_LIMIT].min = 0;
    Parms[DEMOD_LIMIT].max = MAX_INT;

    Parms[MAX_WEIGHT].name = "max_weight";
    Parms[MAX_WEIGHT].val = 0;
    Parms[MAX_WEIGHT].min = -MAX_INT;
    Parms[MAX_WEIGHT].max =  MAX_INT;

    Parms[MAX_GIVEN].name = "max_given";
    Parms[MAX_GIVEN].val = 0;
    Parms[MAX_GIVEN].min = 0;
    Parms[MAX_GIVEN].max = MAX_INT;

    Parms[MAX_SECONDS].name = "max_seconds";
    Parms[MAX_SECONDS].val = 0;
    Parms[MAX_SECONDS].min = 0;
    Parms[MAX_SECONDS].max = MAX_INT;

    Parms[NEG_WEIGHT].name = "neg_weight";
    Parms[NEG_WEIGHT].val = 0;
    Parms[NEG_WEIGHT].min = -MAX_INT;
    Parms[NEG_WEIGHT].max =  MAX_INT;

    Parms[MAX_KEPT].name = "max_kept";
    Parms[MAX_KEPT].val = 0;
    Parms[MAX_KEPT].min = 0;
    Parms[MAX_KEPT].max = MAX_INT;

    Parms[MAX_GEN].name = "max_gen";
    Parms[MAX_GEN].val = 0;
    Parms[MAX_GEN].min = 0;
    Parms[MAX_GEN].max = MAX_INT;

    Parms[MAX_MEM].name = "max_mem";
    Parms[MAX_MEM].val = 0;
    Parms[MAX_MEM].min = 0;
    Parms[MAX_MEM].max = MAX_INT;

    Parms[MAX_LITERALS].name = "max_literals";
    Parms[MAX_LITERALS].val = 0;
    Parms[MAX_LITERALS].min = 0;
    Parms[MAX_LITERALS].max = MAX_INT;

    Parms[REPORT].name = "report";
    Parms[REPORT].val = 0;
    Parms[REPORT].min = 0;
    Parms[REPORT].max = MAX_INT;

    Parms[MAX_PROOFS].name = "max_proofs";
    Parms[MAX_PROOFS].val = 1;
    Parms[MAX_PROOFS].min = 0;
    Parms[MAX_PROOFS].max = MAX_INT;

    Parms[STATS_LEVEL].name = "stats_level";
    Parms[STATS_LEVEL].val = 2;
    Parms[STATS_LEVEL].min = 0;
    Parms[STATS_LEVEL].max = 3;

    Parms[REDUCE_WEIGHT_LIMIT].name = "reduce_weight_limit";
    Parms[REDUCE_WEIGHT_LIMIT].val = 0;
    Parms[REDUCE_WEIGHT_LIMIT].min = 0;
    Parms[REDUCE_WEIGHT_LIMIT].max = MAX_INT;

    Parms[MAX_UR_DEPTH].name = "max_ur_depth";
    Parms[MAX_UR_DEPTH].val = 5;
    Parms[MAX_UR_DEPTH].min = 0;
    Parms[MAX_UR_DEPTH].max = 100;

    Parms[MAX_UR_DED_SIZE].name = "max_ur_deduction_size";
    Parms[MAX_UR_DED_SIZE].val = 20;
    Parms[MAX_UR_DED_SIZE].min = 0;
    Parms[MAX_UR_DED_SIZE].max = 100;

    Parms[MAX_DISTINCT_VARS].name = "max_distinct_vars";
    Parms[MAX_DISTINCT_VARS].val = -1;
    Parms[MAX_DISTINCT_VARS].min = -1;
    Parms[MAX_DISTINCT_VARS].max = MAX_INT;

    Parms[PICK_GIVEN_RATIO].name = "pick_given_ratio";
    Parms[PICK_GIVEN_RATIO].val = -1;
    Parms[PICK_GIVEN_RATIO].min = -1;
    Parms[PICK_GIVEN_RATIO].max = MAX_INT;

    Parms[RANDOM_RATIO].name = "random_ratio";
    Parms[RANDOM_RATIO].val = -1;
    Parms[RANDOM_RATIO].min = -1;
    Parms[RANDOM_RATIO].max = MAX_INT;

    Parms[RANDOM_SEED].name = "random_seed";
    Parms[RANDOM_SEED].val = 1;
    Parms[RANDOM_SEED].min = 1;
    Parms[RANDOM_SEED].max = MAX_INT;

    Parms[MAX_SC_W_ALT].name = "max_sc_w_alt";
    Parms[MAX_SC_W_ALT].val = MAX_INT;	/* It is not used for now. */
    Parms[MAX_SC_W_ALT].min = 1;
    Parms[MAX_SC_W_ALT].max = MAX_INT;

/* Penguin */

}  /* init_options */

/*************
 *
 *    print_options(fp)
 *
 *************/

void print_options(fp)
FILE *fp;
{
    int i, j;

    fprintf(fp, "\n--------------- options ---------------\n");

    j = 0;
    for (i = 0; i < MAX_FLAGS; i++)  /* print set flags */
	if (Flags[i].name[0] != '\0') {
            fprintf(fp, "%s", Flags[i].val ? "set(" : "clear(");
	    fprintf(fp, "%s). ", Flags[i].name);
	    j++;
	    if (j % 3 == 0)
	        fprintf(fp, "\n");
            }

    fprintf(fp, "\n\n");

    j = 0;
    for (i = 0; i < MAX_PARMS; i++)  /* print parms */
	if (Parms[i].name[0] != '\0') {
	    fprintf(fp, "assign(");
	    fprintf(fp, "%s, %d). ", Parms[i].name, Parms[i].val);
	    j++;
	    if (j % 3 == 0)
		fprintf(fp, "\n");
	    }
    fprintf(fp, "\n");

}  /* print_options */

/*************
 *
 *    p_options()
 *
 *************/

void p_options()
{
    print_options(Fdout);
}  /* p_options */

/*************
 *
 *    int change_flag(fp, term, set)
 *
 *    Assume term is COMPLEX, with either `set' or `clear' as functor.
 *
 *    Warning and error messages go to file fp.
 *
 *************/

int change_flag(fp, t, set)
FILE *fp;
struct term *t;
int set;
{
    char *flag_name;
    int i, found;

    if (t->farg == NULL || t->farg->narg != NULL ||
		           t->farg->argval->type == COMPLEX) {
	fprintf(fp, "ERROR: ");
	print_term(fp, t);
	fprintf(fp, " must have one simple argument.\n");
	Stats[INPUT_ERRORS]++;
	return(0);
	}
    else {
	flag_name = sn_to_str(t->farg->argval->sym_num);
	found = 0;
	i = 0;
	while (i < MAX_FLAGS && found == 0)
	    if (str_ident(flag_name, Flags[i].name))
		found = 1;
	    else
		i++;
	if (found == 0) {
	    fprintf(fp, "ERROR: ");
	    print_term(fp, t);
	    fprintf(fp, " flag name not found.\n");
	    Stats[INPUT_ERRORS]++;
	    return(0);
	    }
	else if (Flags[i].val == set) {
	    fprintf(fp, "WARNING: ");
	    print_term(fp, t);
	    if (set)
		fprintf(fp, " flag already set.\n");
	    else
		fprintf(fp, " flag already clear.\n");
	    return(1);
	    }
	else {
	    Flags[i].val = set;
	    return(1);
	    }
	}
}  /* change_flag */

/*************
 *
 *    int change_parm(fp, term, set)
 *
 *    Assume term is COMPLEX, with `assign' as functor.
 *
 *    Warning and error messages go to file fp.
 *
 *************/

int change_parm(fp, t)
FILE *fp;
struct term *t;
{
    char *parm_name, *int_name;
    int i, found, new_val, rc;

    if (t->farg == NULL || t->farg->narg == NULL ||
		           t->farg->narg->narg != NULL ||
		           t->farg->argval->type == COMPLEX ||
		           t->farg->narg->argval->type == COMPLEX) {
	fprintf(fp, "ERROR: ");
	print_term(fp, t);
	fprintf(fp, " must have two simple arguments.\n");
	Stats[INPUT_ERRORS]++;
	return(0);
	}
    else {
	parm_name = sn_to_str(t->farg->argval->sym_num);
	found = 0;
	i = 0;
	while (i < MAX_PARMS && found == 0)
	    if (str_ident(parm_name, Parms[i].name))
		found = 1;
	    else
		i++;
	if (found == 0) {
	    fprintf(fp, "ERROR: ");
	    print_term(fp, t);
	    fprintf(fp, " parm name not found.\n");
	    Stats[INPUT_ERRORS]++;
	    return(0);
	    }
	else {
	    int_name = sn_to_str(t->farg->narg->argval->sym_num);
	    rc = str_int(int_name, &new_val);
	    if (rc == 0) {
		fprintf(fp, "ERROR: ");
		print_term(fp, t);
		fprintf(fp, " second argument must be integer.\n");
		Stats[INPUT_ERRORS]++;
		return(0);
		}
	    else if (new_val < Parms[i].min || new_val > Parms[i].max) {
		fprintf(fp, "ERROR: ");
		print_term(fp, t);
		fprintf(fp, " integer must be in range [%d,%d].\n",
				Parms[i].min, Parms[i].max);
		Stats[INPUT_ERRORS]++;
		return(0);
		}
	    else if (new_val == Parms[i].val) {
		fprintf(fp, "WARNING: ");
		print_term(fp, t);
		fprintf(fp, " already has that value.\n");
		return(1);
		}
	    else {
		Parms[i].val = new_val;
		return(1);
		}
	    }
	}
}  /* change_parm */

/*************
 *
 *    check_options()  --  check for inconsistent and odd settings options
 *
 *    If a bad combination of settings is found, either a warning
 *    message is printed, or an ABEND occurs.
 *
 *************/

void check_options()
{
    if (Flags[BINARY_RES].val == 0 &&
        Flags[HYPER_RES].val == 0 &&
        Flags[NEG_HYPER_RES].val == 0 &&
        Flags[UR_RES].val == 0 &&
        Flags[PARA_FROM].val == 0 &&
        Flags[PARA_INTO].val == 0 &&
        Flags[DEMOD_INF].val == 0 &&
        Flags[LINKED_UR_RES].val == 0)
	fprintf(Fderr, "WARNING: no inference rules are set.\n");
    if (Flags[PARA_FROM].val &&
             Flags[PARA_FROM_RIGHT].val==0 && Flags[PARA_FROM_LEFT].val==0)
	{
	fprintf(Fderr,"WARNING: PARA_FROM is set, but PARA_FROM_LEFT and\n");
	fprintf(Fderr,"PARA_FROM_RIGHT are both clear.\n");
	}
    if (Flags[PARA_INTO].val &&
            Flags[PARA_FROM_RIGHT].val == 0 && Flags[PARA_FROM_LEFT].val == 0)
	{
	fprintf(Fderr,"WARNING: PARA_INTO is set, but PARA_FROM_LEFT and\n");
	fprintf(Fderr,"PARA_FROM_RIGHT are both clear.\n");
	}

if (Flags[PARA_FROM].val==0 && Flags[PARA_INTO].val==0 && Flags[PARA_INTO_VARS].val)
fprintf(Fderr,"WARNING: PARA_FROM, PARA_INTO rules are clear, but PARA_INTO_VARS is set.\n");

if (Flags[PARA_FROM].val==0 && Flags[PARA_INTO].val==0 && Flags[PARA_FROM_VARS].val)
fprintf(Fderr,"WARNING: PARA_FROM, PARA_INTO rules are clear, but PARA_FROM_VARS is set.\n");

if (Flags[PARA_FROM].val==0 && Flags[PARA_INTO].val==0 && Flags[PARA_ONES_RULE].val)
fprintf(Fderr,"WARNING: PARA_FROM, PARA_INTO rules are clear, but PARA_ONES_RULE is set.\n");

    if (Flags[NO_FAPL].val && Flags[HYPER_RES].val == 0)
	fprintf(Fderr, "WARNING: NO_FAPL is set, but HYPER_RES is clear.\n");
    if (Flags[NO_FAPL].val && Flags[FOR_SUB_FPA].val)
	fprintf(Fderr, "WARNING: NO_FAPL and FOR_SUB_FPA are both set.\n");
    if (Flags[NO_FAPL].val && Flags[BACK_SUB].val)
	fprintf(Fderr, "WARNING: NO_FAPL and BACK_SUB are both set.\n");
    if (Flags[KNUTH_BENDIX].val && Flags[LEX_RPO].val == 0)
	fprintf(Fderr, "WARNING: KNUTH_BENDIX is set and LEX_RPO is clear.\n");

    /* selecting the given clause */

    if (Parms[PICK_GIVEN_RATIO].val != -1)
	if (Parms[RANDOM_RATIO].val != -1)
fprintf(Fderr,"WARNING: PICK_GIVEN_RATIO has priority over RANDOM_RATIO.\n");
	else if (Flags[SOS_STACK].val)
fprintf(Fderr,"WARNING: PICK_GIVEN_RATIO has priority over SOS_STACK.\n");
	else if (Flags[SOS_QUEUE].val)
fprintf(Fderr,"WARNING: PICK_GIVEN_RATIO has priority over SOS_QUEUE.\n");

    if (Parms[RANDOM_RATIO].val != -1)
	if (Flags[SOS_STACK].val)
	  fprintf(Fderr,"WARNING: RANDOM_RATIO has priority over SOS_STACK.\n");
	else if (Flags[SOS_QUEUE].val)
	  fprintf(Fderr,"WARNING: RANDOM_RATIO has priority over SOS_QUEUE.\n");

    if (Flags[SOS_STACK].val && Flags[SOS_QUEUE].val)
	fprintf(Fderr, "WARNING, SOS_QUEUE has priority over SOS_STACK.\n");

    if (Flags[SOS_STACK].val && Flags[INPUT_SOS_FIRST].val)
fprintf(Fderr, "WARNING, INPUT_S0S_FIRST ignored, because SOS_STACK is set.\n");

    if (Flags[BINARY_RES].val && !Flags[FACTOR].val)
	fprintf(Fderr, "WARNING, BINARY_RES is set, but FACTOR is clear.\n");

}  /* check_options */

/*************
 *
 *    dependent_options() -- set options that depend on others
 *
 *    Seed random number generator if applicable.
 *
 *    saturation -> knuth_bendix		 Penguin 
 *    knuth_bendix -> dynamic_demod_all
 *    knuth_bendix -> clear para_into_right
 *    knuth_bendix -> para_into_left
 *    knuth_bendix -> back_demod
 *    knuth_bendix -> para_from
 *    knuth_bendix -> para_into
 *    knuth_bendix -> para_from_left
 *    knuth_bendix -> clear para_from_right
 *    knuth_bendix -> post_proc_ns_before_send		Penguin
 *    back_demod -> dynamic_demod
 *    dynamic_demod_all -> dynamic_demod
 *    dynamic_demod -> order_eq
 *    new_functions -> clear symbol_elim
 *    very_verbose -> print_kept
 *    very_verbose -> print_sent			Penguin
 *    very_verbose -> print_received			Penguin
 *    very_verbose -> print_alloc			Penguin
 *
 *************/

void dependent_options()
{
    if (Parms[RANDOM_RATIO].val != -1)
	srand(Parms[RANDOM_SEED].val);

/* Penguin */

if (Flags[SATURATION].val && Flags[KNUTH_BENDIX].val == 0) {
	Flags[KNUTH_BENDIX].val = 1;
fprintf(Fdout,"PENGUIN sets knuth_bendix, because saturation is set.\n");
	}

if (Flags[SATURATION].val && Flags[FIRST_FIT].val == 1) {
	Flags[FIRST_FIT].val = 0;
	Flags[ALT_FIRST_FIT].val = 1;
fprintf(Fdout,"PENGUIN clears first_fit and sets alt_first_fit, because saturation is set.\n");
	}

if (Flags[KNUTH_BENDIX].val && Flags[POST_PROC_NS_BEFORE_SEND].val == 0) {
	Flags[POST_PROC_NS_BEFORE_SEND].val = 1;
fprintf(Fdout,"PENGUIN sets post_proc_ns_before_send, because knuth_bendix is set.\n");
	}

/* ALTERNATE_FIT is the default, if the user sets something else, 	*/
/* ALTERNATE_FIT is cleared.						*/

if (Flags[ALT_FIRST_FIT].val && Flags[ALTERNATE_FIT].val) {
	Flags[ALTERNATE_FIT].val = 0;
fprintf(Fdout,"PENGUIN clears alternate_fit, because alt_first_fit is set.\n");
	}

if (Flags[FIRST_FIT].val && Flags[ALTERNATE_FIT].val) {
	Flags[ALTERNATE_FIT].val = 0;
fprintf(Fdout,"PENGUIN clears alternate_fit, because first_fit is set.\n");
	}

if (Flags[HALF_ALT_FIT].val && Flags[ALTERNATE_FIT].val) {
	Flags[ALTERNATE_FIT].val = 0;
fprintf(Fdout,"PENGUIN clears alternate_fit, because half_alt_fit is set.\n");
	}

if (Flags[VERY_VERBOSE].val && Flags[PRINT_RECEIVED].val == 0) {
	Flags[PRINT_RECEIVED].val = 1;
fprintf(Fdout,"PENGUIN sets print_received, because very_verbose is set.\n");
	}

if (Flags[VERY_VERBOSE].val && Flags[PRINT_SENT].val == 0) {
	Flags[PRINT_SENT].val = 1;
fprintf(Fdout,"PENGUIN sets print_sent, because very_verbose is set.\n");
	}

if (Flags[VERY_VERBOSE].val && Flags[PRINT_ALLOC].val == 0) {
	Flags[PRINT_ALLOC].val = 1;
fprintf(Fdout,"PENGUIN sets print_alloc, because very_verbose is set.\n");
	}

if (Flags[VERY_VERBOSE].val && Flags[PRINT_UPDATES].val == 0) {
	Flags[PRINT_UPDATES].val = 1;
fprintf(Fdout,"PENGUIN sets print_updates, because very_verbose is set.\n");
	}

/* Penguin */

    if (Flags[KNUTH_BENDIX].val && Flags[DYNAMIC_DEMOD_ALL].val == 0) {
	Flags[DYNAMIC_DEMOD_ALL].val = 1;
fprintf(Fdout,"PENGUIN sets dynamic_demod_all, because knuth_bendix is set.\n");
	}
    if (Flags[KNUTH_BENDIX].val && Flags[PARA_INTO_RIGHT].val) {
	Flags[PARA_INTO_RIGHT].val = 0;
fprintf(Fdout,"PENGUIN clears para_into_right, because knuth_bendix is set.\n");
	}
    if (Flags[KNUTH_BENDIX].val && Flags[PARA_INTO_LEFT].val == 0) {
	Flags[PARA_INTO_LEFT].val = 1;
fprintf(Fdout,"PENGUIN sets para_into_left, because knuth_bendix is set.\n");
	}
    if (Flags[KNUTH_BENDIX].val && Flags[BACK_DEMOD].val == 0) {
	Flags[BACK_DEMOD].val = 1;
fprintf(Fdout,"PENGUIN sets back_demod, because knuth_bendix is set.\n");
	}
    if (Flags[KNUTH_BENDIX].val && Flags[PARA_FROM].val == 0) {
	Flags[PARA_FROM].val = 1;
fprintf(Fdout,"PENGUIN sets para_from, because knuth_bendix is set.\n");
	}
    if (Flags[KNUTH_BENDIX].val && Flags[PARA_INTO].val == 0) {
	Flags[PARA_INTO].val = 1;
fprintf(Fdout,"PENGUIN sets para_into, because knuth_bendix is set.\n");
	}
    if (Flags[KNUTH_BENDIX].val && Flags[PARA_FROM_LEFT].val == 0) {
	Flags[PARA_FROM_LEFT].val = 1;
fprintf(Fdout,"PENGUIN sets para_from_left, because knuth_bendix is set.\n");
	}
    if (Flags[KNUTH_BENDIX].val && Flags[PARA_FROM_RIGHT].val) {
	Flags[PARA_FROM_RIGHT].val = 0;
fprintf(Fdout,"PENGUIN clears para_from_right, because knuth_bendix is set.\n");
	}
    if (Flags[BACK_DEMOD].val && Flags[DYNAMIC_DEMOD].val == 0) {
	Flags[DYNAMIC_DEMOD].val = 1;
fprintf(Fdout,"PENGUIN sets dynamic_demod, because back_demod is set.\n");
	}
    if (Flags[DYNAMIC_DEMOD_ALL].val && Flags[DYNAMIC_DEMOD].val == 0) {
	Flags[DYNAMIC_DEMOD].val = 1;
fprintf(Fdout,"PENGUIN sets dynamic_demod, because dynamic_demod_all is set.\n");
	}
    if (Flags[DYNAMIC_DEMOD].val && Flags[ORDER_EQ].val == 0) {
	Flags[ORDER_EQ].val = 1;
fprintf(Fdout,"PENGUIN sets order_eq, because dynamic_demod is set.\n");
	}
    if (Flags[NEW_FUNCTIONS].val && Flags[SYMBOL_ELIM].val) {
	Flags[SYMBOL_ELIM].val = 0;
fprintf(Fdout,"PENGUIN clears symbol_elim, because new_functions is set.\n");
	}
    if (Flags[VERY_VERBOSE].val && Flags[PRINT_KEPT].val == 0) {
	Flags[PRINT_KEPT].val = 1;
fprintf(Fdout,"PENGUIN sets print_kept, because very_verbose is set.\n");
	}
}  /* dependent_options */

