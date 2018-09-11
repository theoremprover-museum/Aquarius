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
 *  types.h -- type declarations
 *
 */

struct term {
    struct rel *farg;           /* subterm list; used for complex only */
    union {               /* term is atom iff (NAME or COMPLEX) && varnum > 0 */
        struct rel *rel;        /* superterm list; used for all except atoms */
	struct literal *lit;    /* containing literal; used for atoms */
	} occ;
    int fpa_id;                 /* used to order fpa lists */
    short sym_num;              /* used for names, complex, and sometimes vars */
    VAR_TYPE varnum;            /* used for variables */
    unsigned char type;         /* NAME, VARIABLE, or COMPLEX */
    unsigned char bits;         /* bit flags (see macros.h) */
    };

struct rel {  /* relations between terms */
    struct term *argval;     /* subterm */
    struct term *argof;      /* superterm */
    struct rel *narg;        /* rest of subterm list */
    struct rel *nocc;        /* rest of superterm list */
    unsigned char path;      /* used in paramod to mark path to into term */
    unsigned char clashable; /* paramodclashability flag */
    };

struct sym_ent {  /* symbol table entry */
    struct sym_ent *next;
    int sym_num;           /* unique identifier */
    int arity;             /* arity 0 for constants, variables */
    int lex_val;           /* can be used to assign a lexical value */
  int eval_code; /* identifies evaluable functions and predicates ($ symbols) */
    int skolem;            /* identifies Skolem constants and functions */
    int special_unary;     /* identifies special unary symbol for lex check */
    int lex_rpo_status;    /* status for lexicographic RPO */
    int predicate;	
/* Penguin: identifies whether predicate or function. It is done in order */
/* to be able to give to equality the smallest lexical value among all the */
/* predicate symbols. See at the end of read_all_input() in misc.c	*/
    char name[MAX_NAME];   /* the print symbol */
    };

struct term_ptr {     /* for constructing a list of pointers to terms */
    struct term *term;
    struct term_ptr *next;
    };

struct formula_ptr_2 {     /* for many-linked list of pointers to formulas */
    struct formula *f;
    struct formula_ptr_2 *prev, *next, *left, *right, *up, *down;
    };

struct fpa_tree {     /* for constructing fpa path lookup tree */
    struct term_ptr *terms;   /* for leaves only */
    struct fpa_tree *left;    /* for AND and OR nodes */
    struct fpa_tree *right;   /* for AND and OR nodes */
    struct term *left_term;   /* for OR nodes only */
    struct term *right_term;  /* for OR nodes only */
    int type;                 /* 1 AND,  2 OR,  3 LEAF */
    int *path;       /* for debugging only */
    };

struct fpa_head {            /* head of an FPA list */
    struct term_ptr *terms;       /* list of terms with path */
    struct fpa_head *next;        /* next FPA list */
    int *path;
    };

struct context {          /* substitution table */
    struct term *terms[MAX_VARS];
    struct context *contexts[MAX_VARS];
    int status[MAX_VARS];  /* for batch occur check */
    int multiplier;  /* needed for apply, not for unify or match */
    int built_in_multiplier;  /* the use of this is optional */
    };

struct trail {                /* to record an entry that has been made in a    */
    struct context *context;  /* substitution table, so that it can be undone  */
    struct trail *next;
    int varnum;
    };

struct imd_tree {  /* index/match/demodulate tree */
    struct imd_tree *next, *kids;
    struct term_ptr *atoms;
    unsigned short lab;    /* variable number or symbol number */
    unsigned char type;    /* VARIABLE, NAME, or COMPLEX */
    
	                  /* the following are used for leaves only */
    VAR_TYPE max_vnum;  /* maximum variable number, for clearing substitution */
    };

struct imd_pos {  /* save a stack of states for backtrack in imd indexing */
    struct imd_pos *next;
    struct imd_tree *imd;
    struct rel *rel_stack[MAX_AL_TERM_DEPTH];  /* save position in given term */
    int reset;    /* flag for clearing instantiation on backtracking */
    int stack_pos;                          /* for backtracking            */
    };

struct is_tree {  /* index-subsume tree */
    struct is_tree *next;  /* sibling */
    union {
	struct is_tree *kids;    /* for internal nodes */
	struct term_ptr *terms;  /* for leaves */
	} u;
    unsigned short lab;    /* variable number or symbol number */
    unsigned char type;    /* VARIABLE, NAME, or COMPLEX */
    };

struct is_pos {  /* save a stack of states for backtrack in is indexing */
    struct is_pos *next;
    struct is_tree *is;
    struct rel *rel_stack[MAX_FS_TERM_DEPTH];  /* save position in given term */
    int reset;    /* flag for clearing instantiation on backtracking */
    int stack_pos;                          /* for backtracking            */
    };

struct fsub_pos {  /* to save position in set of subsuming literals */
    struct term_ptr *terms;  /* list of identical terms from leaf of is tree */
    struct is_pos *pos;  /* stack of states for backtracking */
    };

struct literal {
    struct clause *container;  /* containing clause */
    struct literal *next_lit;
    struct term *atom;
    char sign;
    BOOLEAN target;
    };

struct clause {
    struct int_ptr *parents;
    struct list *container;
    struct clause *prev_cl, *next_cl;  /* prev and next clause in list */
    struct literal *first_lit;
    int id;
    int pid;			/* process id: Penguin only */
    int lid;			/* local id: Penguin only */
    int bt;			/* birth time: Penguin only */
    int dest;			/* destination: Penguin only */
    int weight;
    char type;  /* for linked inf rules */
#ifdef ROO
    int giv_cl_seq_no;
    unsigned char owner;
#endif
    };

struct list {  /* the primary way to build a list of clauses */
    struct clause *first_cl, *last_cl;
    char name[MAX_NAME];  /* name of list */
    };

struct clause_ptr {  /* an alternate way to build a list of clauses */
    struct clause *c;
    struct clause_ptr *next;
    };

struct int_ptr {  /* for building a list of integers */
    struct int_ptr *next;
    int i;
    };

struct clash_nd {   /* for hyper and UR--one for each clashable lit of nuc */
    struct term *nuc_atom;   /* atom from nucleus */
    struct fpa_head **db;    /* fpa index to use for finding satellites */
    struct fpa_tree *u_tree; /* unification path tree (position in sats) */
    struct context *subst;   /* unifying substitution */
    struct trail *tr;        /* trail to undo substitution */
    struct term *found_atom; /* unifying atom */
    int evaluable;           /* $ evaluation */
    int evaluation;          /* $ evaluation */
    int already_evaluated;   /* $ evaluation */
    struct clash_nd *prev, *next;  /* links */
    };

struct clock {    /* for timing various operations, see cos.h, macros.h, and clocks.c */
    long accum_sec;   /* accumulated time */
    long accum_usec;
    long curr_sec;    /* time since clock has been turned on */
    long curr_usec;
    };

struct ans_lit_node {
    struct ans_lit_node *next;
    struct link_node *parent;
    struct literal *lit;
    };

struct link_node {
    struct link_node *parent, *next_sibling, *prev_sibling, *first_child;
    struct ans_lit_node *child_first_ans, *child_last_ans;
    BOOLEAN first;
    BOOLEAN unit_deleted;  /* TRUE if goal_to_resolve has been unit deleted */
    struct term *goal, *goal_to_resolve;
    struct clause *current_clause;
    struct context *subst;
    struct trail *tr;
    struct fpa_tree *unif_position;
    int near_poss_nuc, farthest_sat, target_dist, back_up;
    };

struct formula_box {
    int type;     /* FORMULA, OPERATOR */
    int subtype;      /* COMPLEX_FORM, ATOM_FORM */
                      /* OR_OP, AND_OP, NOT_OP, EXISTS_OP, ALL_OP */
    struct formula *f;
    char str[100];

    int length, height;   /* size of box */
    int x_off, y_off;    /* offset from parent */
    int abs_x_loc, abs_y_loc; /* absolute location of box in window */

    struct formula_box *first_child;
    struct formula_box *next;
    struct formula_box *parent;
    };

struct formula {
    struct formula *parent, *first_child, *next;
    struct term *t;  /* for atoms and for quantifier variables */
    char type;
    char quant_type;
    };

struct formula_ptr {
    struct formula *f;
    struct formula_ptr *next;
    };

#ifdef ROO

struct time_node {
    /* to keep nodes in limbo until it is safe to delete them */
    unsigned long time;
    struct time_node *next;
    void *node;
    int tag;
    };

#endif
