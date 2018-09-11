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
 *  header.h -- This is the main "include" file for Otter.
 *  All of the .c files include this file.
 *
 */

/************ BASIC INCLUDES ************/

#include <stdio.h>
#ifndef TP_NO_STDLIB
#include <stdlib.h>  /* doesn't exist on some UNIXes */
#endif

/*********** INCLUDES FOR TIMES AND DATES ************/

/*********** TURBO_C for PC ***********/

#ifdef TURBO_C
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>

/*********** THINK_C for Macintosh ***********/

#else
#ifdef THINK_C
#include <time.h>

/*********** ELSE ASSUME UNIX ***********/

#else
#include <sys/time.h>
#ifndef ROO
#include <sys/resource.h>  /* p4 includes this */
#endif
#endif
#endif


/*********** SIZES OF INTEGERS ***************/

#define MAX_LONG_INT 2147483647  /* a big integer (must fit into long) */

#ifdef TURBO_C        /* PC -> 16 bit integers */
#define MAX_INT      32767

#else
#ifdef THINK_C   /* Macintosh -> 16 bit integers */
#define MAX_INT      32767

#else            /* assume 32 bit (or bigger) integers */
#define MAX_INT      2147483647
#endif
#endif

/******** MISCELLANEOUS LIMITS *********/

#define MAX_NAME 51   /* maximum # of chars in any symbol (including '\0') */

#define MAX_BUF 5000  /* maximum # of chars in input string (including '\0') */

#define MAX_VARS 64    /* maximum # of distinct variables in clause */
/* #define MAX_VARS 250   needed for the problem f3b2 */
#define VAR_TYPE unsigned short  /* must be able to hold MAX_VARS * (max-multiplier + 1) */

#define FPA_SIZE 500  /* size of FPA hash tables */

/******** TYPES *********/

#define NAME 1        /* basic types of term */
#define VARIABLE 2
#define COMPLEX 3

                      /* types of non-VARIABLE term -- varnum field is used */

#define TERM 0              /* not an atom */
#define NORM_ATOM 1         /* normal atom */
#define POS_EQ 2            /* positive equality atom */
#define NEG_EQ 3            /* negative equality atom */
#define ANSWER 4            /* answer literal atom */
#define LEX_DEP_DEMOD 5     /* lex-dependent demodulator atom */
#define EVALUABLE 6         /* $ID, $LT, etc */
#define CONDITIONAL_DEMOD 7 /* conditional demodulator */

                        /* types of unification property tree */
#define UNIFY 1
#define INSTANCE 2
#define MORE_GEN 3

                 /* integer codes for inference rules--used in parent lists */

#define BINARY_RES_RULE     -2
#define HYPER_RES_RULE      -3
#define UR_RES_RULE         -4
#define PARA_INTO_RULE      -5
#define PARA_FROM_RULE      -6
#define FACTOR_RULE         -7
#define NEW_DEMOD_RULE      -8
#define BACK_DEMOD_RULE     -9
#define DEMOD_RULE         -10
#define UNIT_DEL_RULE      -11
#define NEW_FUNCTION_RULE  -12
#define FLIP_EQ_RULE       -13
#define LINKED_UR_RES_RULE -14
#define NEG_HYPER_RES_RULE -15
#define EVAL_RULE          -16  /* not really an inference rule */

                /* integer codes evaluable functions and predicates */
                /* When adding more, update mark_evaluable_symbols in io.c. */

#define SUM_SYM           1
#define PROD_SYM          2
#define DIFF_SYM          3
#define DIV_SYM           4
#define MOD_SYM           5

#define EQ_SYM            6
#define NE_SYM            7
#define LT_SYM            8
#define LE_SYM            9
#define GT_SYM           10
#define GE_SYM           11

#define AND_SYM          12
#define OR_SYM           13
#define NOT_SYM          14

#define IF_SYM           15

#define ID_SYM           16
#define LNE_SYM          17
#define LLT_SYM          18
#define LLE_SYM          19
#define LGT_SYM          20
#define LGE_SYM          21
#define T_SYM            22
#define F_SYM            23
#define NEXT_CL_NUM_SYM  24
#define ATOMIC_SYM       25
#define NUMBER_SYM       26
#define VAR_SYM          27
#define TRUE_SYM         28
#define OUT_SYM          29

#define BIT_AND_SYM      30
#define BIT_OR_SYM       31
#define BIT_XOR_SYM      32
#define BIT_NOT_SYM      33
#define SHIFT_LEFT_SYM   34
#define SHIFT_RIGHT_SYM  35
#define GROUND_SYM       36

#define MAX_FS_TERM_DEPTH 300  /* max depth of terms in IS-tree */
#define MAX_AL_TERM_DEPTH 500  /* max depth of alphas in IMD-tree */

		   /* comparing symbols and terms */

#define LESS_THAN        1
#define GREATER_THAN     2
#define SAME_AS          3
#define NOT_COMPARABLE   4
#define NOT_GREATER_THAN 5
#define NOT_LESS_THAN    6

#define LRPO_MULTISET_STATUS  0  /* lex RPO multiset status   */
#define LRPO_LR_STATUS        1  /* lex RPO left-right status */
#define LRPO_RL_STATUS        2  /* lex RPO right-left status */

		   /* linked-UR resolution inference rule. */

#define BOOLEAN char
#define FALSE 0
#define TRUE 1
#define UNDEFINED -1
#define NOT_SPECIFIED 0
#define NUCLEUS      1
#define LINK         2
#define BOTH         3
#define SATELLITE    4

                   /* first-order formulae */

#define ATOM_FORM 1
#define NOT_FORM 2
#define AND_FORM 3
#define OR_FORM 4
#define IMP_FORM 5
#define IFF_FORM 6
#define QUANT_FORM 7

#define ALL_QUANT 1
#define EXISTS_QUANT 2


#include "cos.h"        /* flag, parameter, statistic, and clock names */

/************* END OF ALL GLOBAL CONSTANT DEFINITIONS ****************/

#include "pheader.h" /* additional constants of Penguin */

#include "macros.h"  /* preprocessor (#define) macros */

#include "proto.h"   /* function prototypes */

#include "types.h"   /* all of the type declarations */

/*********** GLOBAL VARIABLES ***********/

#ifdef IN_MAIN
#define CLASS         /* empty string if included by main program */
#else
#define CLASS extern  /* extern if included by anything else */
#endif

/* Penguin */

/* Files replacing stdin, stdout, stderr */
CLASS FILE *Fdin, *Fdout, *Fderr;

CLASS int Whoami;				/* id of the node */
CLASS int No_of_nodes;				/* number of nodes */

CLASS int Last_choice;
/* The last choice in allocation: it is used to determine the destination */
/* of new settlers under the ALTERNATE_FIT policy.			*/
CLASS int Before_last_choice;
/* The choice before the last: it is used to determine the destination */
/* of new settlers under the HALF_ALT_FIT policy.			*/

CLASS int Penguins[MAX_NO_OF_NODES];
/* For each node i, whether it is up (Penguins[i] > 0)	 		*/
/* or down (Penguins[i] == 0)						*/ 
/* For all k, 0 <= k <= No_of_nodes - 1, 				*/
/* if k == Whoami, Penguins[k] == (# of clauses settled down at Whoami)+1, */
/* so that Penguins[k] is the value to give to the lid of the next clause */
/* which settles down at Whoami,					*/
/* if k != Whoami, Penguins[k] == (# of clauses sent by Whoami to settle */
/* at Penguin k) + 1.							*/
/* Let i be the Penguin which reads the input:				*/
/* after reading the input, for all k, 0 <= k != i <= No_of_nodes - 1,  */
/* Penguin i sends the value of its Penguins[k], i.e. the number of input*/
/* clauses assigned as residents to Penguin k, to Penguin k, so that it can */
/* set accordingly Penguin[k] at k.					*/
CLASS int Halting;
/* Set to 1 by store_special() in interface.c to inform the main inference */
/* loop that an HALT. message has  been received.			*/

/* lists of clauses */

CLASS struct list *Usable;
CLASS struct list *Sos;
CLASS struct list *Demodulators;
CLASS struct list *Passive;
CLASS struct list *Inbound_msgs;		/* Penguin */
CLASS struct list *Outbound_msgs;		/* Penguin */

/* FPA (indexing) lists for resolution inference rules */

CLASS struct fpa_head *Fpa_clash_pos_lits[FPA_SIZE];
CLASS struct fpa_head *Fpa_clash_neg_lits[FPA_SIZE];

/* FPA lists for unit conflict and back subsumption */

CLASS struct fpa_head *Fpa_pos_lits[FPA_SIZE];
CLASS struct fpa_head *Fpa_neg_lits[FPA_SIZE];

/* FPA lists for paramodulation inference rules */

CLASS struct fpa_head *Fpa_clash_terms[FPA_SIZE]; /* clashable terms */
CLASS struct fpa_head *Fpa_alphas[FPA_SIZE];      /* alphas (left and right) */
CLASS struct fpa_head *Fpa_back_demod[FPA_SIZE];  /* back demod candidates */

/* discrimination tree forward subsumption index */

CLASS struct is_tree *Is_pos_lits;  /* positive literals */
CLASS struct is_tree *Is_neg_lits;  /* negative literals */

/* discrimination tree index for demodulators */

CLASS struct imd_tree *Demod_imd;

/* Lists of weight templates */

CLASS struct term_ptr *Weight_purge_gen;    /* screen generated clauses */
CLASS struct term_ptr *Weight_pick_given;   /* pick given clause */
CLASS struct term_ptr *Weight_terms;        /* order terms */

/* Simple indexes (one level only) for weight templates */

CLASS struct is_tree *Weight_purge_gen_index;
CLASS struct is_tree *Weight_pick_given_index;
CLASS struct is_tree *Weight_terms_index;

/* options (Flags and Parms) */

CLASS struct {  /* Flags are boolean valued options */
    char *name;
    int val;
    } Flags[MAX_FLAGS];

CLASS struct {  /* Parms are integer valued options */
    char *name;
    int val;
    int min, max;  /* minimum and maximum permissible values */
    } Parms[MAX_PARMS];

CLASS int Internal_flags[MAX_INTERNAL_FLAGS];  /* invisible to user */

/* statistics */

CLASS long Stats[MAX_STATS];
CLASS int Subsume_count[100];

/* clocks */

CLASS struct clock Clocks[MAX_CLOCKS];

/* Other built-in symbols */

CLASS int Eq_sym_num, Cons_sym_num, Nil_sym_num, Ignore_sym_num, Conditional_demodulator_sym_num, Chr_sym_num;



/************* Rest of file is ROO (shared-memory parallel) stuff **********/

#ifdef ROO

#ifdef SYMMETRY
#include "/Net/anagram/anagram8/lusk/p4/lib/p4.h"
#include "/usr/local/atrace/atrace_log.h"
#else
#include "/Net/encore/efs2/lusk/p4/lib/p4.h"
#include "/Net/encore/efs2/mccune/roo/atrace_log.h"
#endif

#define TRACING 0

ALOG_DEC;  /* macro to declare logging stuff */

#define MAX_PROCS 50          /* maximum number of ROO processes */

#define MAX_CLOCK 4294967295  /* maximum microsecond clock value */

#define TERM_PTR 1            /* special deletion node codes */
#define FPA_HEAD 2
#define IS_TREE  3
#define IMD_TREE 4

#define K_INDEX_SIZE 100

struct mem_stats {  /* To collect total mem_stats from all processes. */

    int Malloc_calls;  /* number of calls to malloc */

    long term_gets, term_frees, term_avails;
    long rel_gets, rel_frees, rel_avails;
    long sym_ent_gets, sym_ent_frees, sym_ent_avails;
    long term_ptr_gets, term_ptr_frees, term_ptr_avails;
    long fpa_tree_gets, fpa_tree_frees, fpa_tree_avails;
    long fpa_head_gets, fpa_head_frees, fpa_head_avails;
    long context_gets, context_frees, context_avails;
    long trail_gets, trail_frees, trail_avails;
    long imd_tree_gets, imd_tree_frees, imd_tree_avails;
    long imd_pos_gets, imd_pos_frees, imd_pos_avails;
    long is_tree_gets, is_tree_frees, is_tree_avails;
    long is_pos_gets, is_pos_frees, is_pos_avails;
    long fsub_pos_gets, fsub_pos_frees, fsub_pos_avails;
    long literal_gets, literal_frees, literal_avails;
    long clause_gets, clause_frees, clause_avails;
    long list_gets, list_frees, list_avails;
    long clash_nd_gets, clash_nd_frees, clash_nd_avails;
    long clause_ptr_gets, clause_ptr_frees, clause_ptr_avails;
    long int_ptr_gets, int_ptr_frees, int_ptr_avails;

    long link_node_gets, link_node_frees, link_node_avails;
    long ans_lit_node_gets, ans_lit_node_frees, ans_lit_node_avails;

    long formula_gets, formula_frees, formula_avails;
    long formula_box_gets, formula_box_frees, formula_box_avails;
    long quantifier_gets, quantifier_frees, quantifier_avails;
    long formula_ptr_gets, formula_ptr_frees, formula_ptr_avails;

    long time_node_gets, time_node_frees, time_node_avails;

};

struct globmem {             /* Common area in shared memory */
    int Int_clause_count;
    int sym_ent_count;
    int atom_count;
    int term_count;
    int giv_cl_count;
    int k_flag;
    int nprocs;
    int b_count;
    int num_a_tasks;
    int num_b_tasks;
    int k_length, m_length, n_length;
    int k_hwm, m_hwm, n_hwm;
    int gpid;
    struct p4_askfor_monitor dispatcher;
    struct p4_barrier_monitor gate;
    p4_lock_t k_lock;
    p4_lock_t pid_lock;
    p4_lock_t all_index_lock;
    p4_lock_t clash_index_lock;
    p4_lock_t imd_index_lock;
    p4_lock_t list_move_lock;
    p4_lock_t mem_stats_lock;
    p4_lock_t print_lock;
    p4_lock_t sym_tab_lock;
    int proofs_found;
    struct mem_stats memory_stats;
    struct list *k_list;
    struct clause *k_index[K_INDEX_SIZE];
    struct time_node *time_node_list;
    long Stats[MAX_STATS];
    struct clock Clocks[MAX_CLOCKS];

    struct {
        int mallocs;
        unsigned long time_finished_given;
        p4_lock_t dc_lock;
        struct list *deleted_clauses;
        } proc_data[MAX_PROCS];

    struct cl_ptr_list {
        struct clause_ptr *first, *last;
        } m_list, n_list;
};

CLASS struct globmem *Glob;
CLASS int Pid;  /* process ID */

#endif  /* ROO */
