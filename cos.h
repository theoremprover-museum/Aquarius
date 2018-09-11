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
 *  cos.h -- preprocessor definitions of indices for arrays of
 *  flags, parameters, statistics, clocks, and internal flags.
 *
 */

/*************
 *
 *    Flags are boolean valued options.  To install a new flag, append
 *    a new name and index to the end of this list, then insert code to
 *    initialize it in the routine `init_options'.
 *    Example access:  if (Flags[PARA_FROM_LEFT].val) {
 *    See routine `init_options' for defaults.
 *
 *************/

#define MAX_FLAGS           100  /* increase if necessary */

#define INPUT_SOS_FIRST 0 /* use input sos before generated sos */
#define SOS_QUEUE 1 /* first clause on sos is given clause */
#define SOS_STACK 2 /* pick last sos clause as given clause */
#define PRINT_GIVEN 3 /* print given clauses */

#define BINARY_RES 4 /* binary resolution */
#define HYPER_RES 5 /* hyperresolution */
#define UR_RES 6 /* UR-resolution */
#define PARA_INTO 7 /* `into' paramodulation inference rule */
#define PARA_FROM 8 /* `from' paramodulation inference rule */
#define DEMOD_INF 9 /* apply demodulation as an inference rule */

#define PARA_FROM_LEFT 10 /* allow paramodulation from left sides */
#define PARA_FROM_RIGHT 11 /* allow paramodulation from right sides */
#define PARA_INTO_LEFT 12 /* allow paramodulation into left args of = */
#define PARA_INTO_RIGHT 13 /* allow paramodulation into right args of = */
#define PARA_FROM_VARS 14 /* allow paramodulation from variables */
#define PARA_INTO_VARS 15 /* allow paramodulation into variables */
#define PARA_FROM_UNITS_ONLY 16 /* from clause must be unit */
#define PARA_INTO_UNITS_ONLY 17 /* into clause must be unit */
#define PARA_SKIP_SKOLEM 18 /* Skolem function restriction strategy */
#define PARA_ONES_RULE 19 /* paramod only into first args of terms */
#define PARA_ALL 20 /* paramodulate all occurrences of into term */

#define VERY_VERBOSE 21 /* print generated clauses */
#define ORDER_EQ 22 /* flip equalities (+ and -) if right arg heavier */
#define SORT_LITERALS 23 /* sort literals in pre_process */
#define DELETE_IDENTICAL_NESTED_SKOLEM 24 /* delete clauses containing */
#define FOR_SUB 25 /* forward subsumption */
#define UNIT_DELETION 26 /* unit deletion processing */
#define PRINT_KEPT 27 /* print kept clauses */
#define PRINT_PROOFS 28 /* print all proofs found */
#define BACK_SUB 29 /* back subsumption */
#define PRINT_BACK_SUB 30 /* print back subsumed clauses */
#define FACTOR 31 /* factor during post_process */

#define DEMOD_HISTORY 32 /* build history in demodulation */
#define DEMOD_LINEAR 33 /* use linear search instead of index tree */
#define DEMOD_OUT_IN 34 /* demodulate outside-in, (leftmost) */
#define DYNAMIC_DEMOD 36 /* dynamic addition of demodulators */
#define DYNAMIC_DEMOD_ALL 37 /* try to make all equalities into demodulators */
#define PRINT_NEW_DEMOD 38 /* print new demodulators */
#define BACK_DEMOD 39 /* back demodulation */
#define PRINT_BACK_DEMOD 40 /* print back demodulated clauses */
#define SYMBOL_ELIM 41 /* orient equalities to eliminate symbols */
#define KNUTH_BENDIX 42 /* Penguin: Strategy is Knuth-Bendix completion */
/* This option is already in Otter, Penguin has just changed the comment. */
#define LEX_RPO 43 /* lexicographic recursive path ordering */
#define DYNAMIC_DEMOD_LEX_DEP 44 /* allow lex-dep dynamic demodulators */
#define LEX_ORDER_VARS 45 /* consider variables when lex_checking terms */

#define FOR_SUB_FPA 46 /* forward subsump with FPA, not index tree */
#define NO_FAPL 47 /* don't FPA index all positive literals */
#define NO_FANL 48 /* don't FPA index all negative literals */

#define CHECK_ARITY 49 /* require symbols to have fixed arities */
#define PROLOG_STYLE_VARIABLES 50 /* vars start with A-Z */
#define PROCESS_INPUT 51 /* process input usable and sos */
#define SIMPLIFY_FOL 52 /* attempt to simplify during cnf translation */
#define BIRD_PRINT 53 /* print terms a(_,_) in combinatory logic notation */

#define FREE_ALL_MEM 54 /* free all memory to avail lists at end of run */
#define ATOM_WT_MAX_ARGS 55
/* Default weight of atom is max of weights of arguments */
#define TERM_WT_MAX_ARGS 56
/* Default weight of term is max of weights of arguments */
#define PRINT_LISTS_AT_END 57
/* Print Usable, Sos, Demodulators at end of run */
#define REALLY_DELETE_CLAUSES 58 /* delete back demod and back_subed cls */

#define PROG_SYNTHESIS 59 /* program synthesis mode */
#define ANCESTOR_SUBSUME 60 /* ancestor subsumption */
#define NEW_FUNCTIONS 61 /* Try to introduce new function symbols in demod */
#define LINKED_UR_RES 62 /* linked UR resolution inference rule */
#define LINKED_UR_TRACE 63 /* trace linked UR res inference rule */
#define LINKED_SUB_UNIT_USABLE 64 /* use Usable list to subsume subsumable */
                            /* intermediate unit clauses or target   */
                            /* during linked UR resolution.          */
#define LINKED_SUB_UNIT_SOS 65 /* use Sos list to subsume subsumable */
                            /* intermediate unit clauses or target   */
                            /* during linked UR resolution.          */
#define INDEX_FOR_BACK_DEMOD 66 /* index (FPA) all terms for back demod */
#define LINKED_UNIT_DEL 67 /* use Unit Deletion during linked UR resolution. */
                           /* Any unit cl in Usable or Sos list that resolves*/
                           /* a non-target literal without instantiating it */
                           /* will be the only resolver against that literal.*/
#define LINKED_TARGET_ALL 68 /* If set, all literals are targets. */

#define LINKED_HYPER_RES  69  /* Linked hyper inference rule */

#define CONTROL_MEMORY    70
#define N_RESOLUTION      71  /* restrict binary resolution */
#define ORDER_HISTORY     72  /* Nucleus number first for hyper, UR. */
#define NEG_HYPER_RES     73  /* negative hyperresolution inf rule */
#define SUPPRESS_WEIGHT_WARNING 74

/* Penguin */

#define SATURATION		75
/* Using the knuth_bendix strategy to try to generate a saturated set	*/
/* of equations. If set, it also turns knuth_bendix on (see dependent_	*/
/* options()) in options.c.						*/

#define	ALTERNATE_FIT		76
/* Allocation of new settlers: if Penguin p last sent a new settler to	*/
/* Penguin q, it sends the next to Penguin (q+1) mod No_of_nodes.	*/
/* ALTERNATE_FIT is the default.					*/
#define FIRST_FIT		77
/* Allocation of new settlers: critical pairs become residents at the	*/
/* Penguin where they have been generated.				*/
/* It may be used if there is a target theorem (so that every node */
/* has at least the target theorem as resident): if SATURATION is on,	*/
/* FIRST_FIT is automatically replaced by ALT_FIRST_FIT (see dependent_ */
/* options() in options.c.						*/
#define ALT_FIRST_FIT		78
/* Allocation of new settlers: use ALTERNATE_FIT for input new settlers,*/
/* FIRST_FIT for non-input new settlers.				*/
#define	HALF_ALT_FIT		79
/* Allocation of new settlers: if Penguin p sent its last two new settlers */
/* to Penguins q and r in the order, then: if r != p, it keeps the next	*/
/* new settler for itself, else, i.e. r == p, it sends the next new settler */
/* to Penguin (q+1) mod No_of_nodes.					*/
#define	OWN_FACTORS		80
/* If it is on, a Penguin keeps as residents the factors of its residents */
/* regardless of the allocation policy. Default is on.			*/
#define	OWN_NFR			81
/* If it is on, a Penguin keeps as residents the equations generated by the */
/* new_function_rule, i.e. KB-type of ``splitting'', from its residents */
/* regardless of the allocation policy. Default is on.			*/
#define OWN_IN_USABLE		82
/* If it is on, each Penguin keeps as resident the input clauses in Usable */
/* regardless of the allocation policy. Default is off.			*/
#define OWN_IN_SOS		83
/* If it is on, each Penguin keeps as resident the input clauses in Sos	*/
/* regardless of the allocation policy. Default is off.			*/

#define REALLY_DELETE_MSGS	84
/* Deletion of sent messages: default is 1. Recall that messages being */
/* sent, i.e. clauses in Outbound_msgs are copies of clauses in Usable. */

#define IN_MSG_QUEUE		85
/* Treat Inbound_msgs as a fifo queue.					*/

#define IN_MSG_STACK		86
/* Treat Inbound_msgs as a lifo stack.					*/

#define OUT_MSG_QUEUE		87
/* Treat Outbound_msgs as a fifo queue.					*/

#define OUT_MSG_STACK		88
/* Treat Outbound_msgs as a lifo stack.					*/

#define PRIORITY_MSGS		89
/* If it is on, main_infer() of PCN invokes small_infer(), otherwise it */
/* invokes infer(). If small_infer() is running, messages are served	*/
/* with higher priority wrt. the inner working of the Penguin than under */
/* infer().								*/

#define	SOS_QUEUE_MOD		90
/* Sos is treated as a queue, sorted by the lexicographic combination of */
/* the ordering on integers applied to lids and the ordering on integers */
/* applied to ids. For clauses which do not belong to this Penguin, the id */
/* is divided by No_of_nodes, in order to compensate that those clauses get */
/* at any Penguin which receives them as foreigners a much higher id than */
/* the one they got at the Penguin where they were generated, when they were */
/* generated, because of the communication delay.			*/

#define SOS_D_LIGHT		91
/* Select the lightest resident in Sos, the lightest non-resident in Sos, */
/* compare them by the lexicographic combination defined above and pick	*/
/* the smallest one.							*/

#define PRINT_SENT		92	/* print sent clauses */
#define PRINT_RECEIVED		93	/* print received clauses */
#define PRINT_ALLOC		94	/* print allocation decisions */
#define PRINT_UPDATES		95	/* print updates to global db */
/* All on by default, also implied by very_verbose.			*/

#define PART_FACTORS		96
/* It is on by default. If on, each Penguin generates only the factors of */
/* its residents.							*/

#define POST_PROC_NS_BEFORE_SEND	97
/* If it is off, a new settler is sent after having been pre_processed(). */
/* If it is on, a new settler is kept temporarily for post_processing() */
/* and it is sent only when it is selected as given_clause.		*/
/* Default is off. If KNUTH_BENDIX is on, it is turned on automatically. */

#define EAGER_BD_INF_MSGS		98
/* If it is on, a resident is broadcasted as inference message before the */
/* expansion phase in the main loop. If it is off, it is broadcasted */
/* after the expansion phase in the main loop. Default: it is on.	*/

#define STAND_ALONE			99
/* If it is on, each Penguin ignore the others, no messages are sent and */
/* received. Default is off.						*/

/* Penguin */

/* end of Flags */

/*************
 *
 *    Parms are integer valued options.  To install a new parm, append
 *    a new name and index to the end of this list, then insert code to
 *    initialize it in the routine `init_options'.
 *    Example access:  if (Parms[FPA_LITERALS].val == 4) {
 *    See routine `init_options' for defaults.
 *
 *************/

#define MAX_PARMS        30  /* increase if necessary */

#define REPORT            0  /* output stats and times every n seconds */
#define MAX_SECONDS       1  /* stop search after this many seconds */
#define MAX_GEN           2  /* stop search after this many generated clauses */
#define MAX_KEPT          3  /* stop search after this many kept clauses */
#define MAX_GIVEN         4  /* stop search after this many given clauses */
#define MAX_MEM           5  /* stop search after this many K bytes allocated */

#define MAX_LITERALS      6  /* max # of lits in kept clause (0 -> no limit) */
#define MAX_WEIGHT        7  /* maximum weight of kept clauses */
/* The default weight function (used if no weight templates are input) is */
/* symbol count. Thus MAX_WEIGHT can be used to say to discard all clauses */
/* with more than n symbols.						*/

#define FPA_LITERALS      8  /* FPA indexing depth for literals */
#define FPA_TERMS         9  /* FPA indexing depth for terms */

#define DEMOD_LIMIT      10  /* Limit on number of rewrites per clause */
#define MAX_PROOFS       11  /* stop search after this many empty clauses */
#define NEG_WEIGHT       12  /* add this value to weight of negative literals */
#define STATS_LEVEL      13  /* higher stats_level -> output more statistics */

#define REDUCE_WEIGHT_LIMIT 14  /* lower the weight limit on the fly */

#define MAX_UR_DEPTH     15  /* max depth for linked UR (normal depth = 0) */
#define MAX_UR_DED_SIZE  16  /* max resolutions in a single linked UR */

#define MAX_DISTINCT_VARS 17

#define PICK_GIVEN_RATIO 18  /* pick lightest n times, then pick first */

#define RANDOM_RATIO     20  /* pick lightest n times, then pick random */
#define RANDOM_SEED      21  /* for random_ratio */

/* Penguin */

#define	MAX_SC_W_ALT		22
/* Maximum number of settled clauses without alternating the allocation. */
/* If FIRST_FIT or ALT_FIRST_FIT is on, when Penguins[Whoami] > Parms[MAX_ */
/* SC_W_ALT].val, replace them by ALTERNATE_FIT.			*/

    /* end of Parms */

/*************
 *
 *    Statistics.  To install a new statistic, append a new name and index
 *    to the end of this list, then insert the code to output it in the
 *    routine `print_stats'.
 *    Example access:  Stats[INPUT_ERRORS]++;
 *
 *************/

#define MAX_STATS			36

#define INPUT_ERRORS			0
#define CL_INPUT			1
#define CL_GENERATED			2
#define CL_KEPT				3
#define CL_FOR_SUB			4
#define CL_BACK_SUB			5
#define CL_TAUTOLOGY			6
#define CL_GIVEN			7
#define CL_WT_DELETE			8
#define REWRITES			9
#define FACTORS				10
#define UNIT_DELETES			11
#define EMPTY_CLAUSES			12
#define FPA_OVERLOADS			13  /* not output if 0 */
#define FPA_UNDERLOADS			14  /* not output if 0 */
#define CL_VAR_DELETES			15  /* not output if 0 */
#define FOR_SUB_SOS			16
#define NEW_DEMODS			17
#define CL_BACK_DEMOD			18
#define LINKED_UR_DEPTH_HITS		19
#define LINKED_UR_DED_HITS		20
#define SOS_SIZE			21
#define USABLE_SIZE			22	/* Penguin */
#define PASSIVE_SIZE			23	/* Penguin */
#define DEMODULATORS_SIZE		24	/* Penguin */
#define INBOUND_MSGS_SIZE		25	/* Penguin */
#define OUTBOUND_MSGS_SIZE		26	/* Penguin */
#define K_MALLOCED			27
#define CL_NOT_ANC_SUBSUMED		28
#define CL_SENT				29	/* Penguin */

#ifdef ROO
#define ROO_ALMOST_KEPT       (MAX_STATS-1)
#endif

    /* end of Stats */

/*************
 *
 *    Clocks.  To install a new clock, append a new name and index
 *    to the end of this list, then insert the code to output it in the
 *    routine `print_times'.  Example of use: CLOCK_START(INPUT_TIME),
 *    CLOCK_STOP(INPUT_TIME),  micro_sec = clock_val(INPUT_TIME);.
 *    See files macros.h and clocks.c.
 *
 *************/

#define MAX_CLOCKS          40
 
#define INPUT_TIME           0
#define CLAUSIFY_TIME        1
#define PROCESS_INPUT_TIME   2

#define BINARY_TIME          3
#define HYPER_TIME           4
#define UR_TIME              5
#define PARA_INTO_TIME       6
#define PARA_FROM_TIME       7

#define PRE_PROC_TIME        8
#define DEMOD_TIME           9
#define FOR_SUB_TIME        10
#define UNIT_DEL_TIME       11
#define KEEP_CL_TIME        12
#define PRINT_CL_TIME       13

#define POST_PROC_TIME      14
#define CONFLICT_TIME       15
#define BACK_DEMOD_TIME     16
#define BACK_SUB_TIME       17
#define FACTOR_TIME         18

#define WEIGH_CL_TIME       19
#define WINDOW_TIME         20
#define RENUMBER_TIME       21
#define LRPO_TIME           22
#define LINKED_UR_TIME      23
#define NEG_HYPER_TIME      24

#ifdef ROO
#define PROCESS_TIME_NODE_LIST       (MAX_CLOCKS-6)
#define MASTER_WORK_TIME             (MAX_CLOCKS-5)
#define TASK_A_TIME                  (MAX_CLOCKS-4)
#define TASK_B_TIME                  (MAX_CLOCKS-3)
#define TASK_C_TIME                  (MAX_CLOCKS-2)
#define TASK_D_TIME                  (MAX_CLOCKS-1)
#endif

    /* end of clocks */


/*************
 *
 *    internal flags--invisible to users
 *
 *************/

#define MAX_INTERNAL_FLAGS 10

#define SPECIAL_UNARY_PRESENT 0
#define DOLLAR_PRESENT 1

