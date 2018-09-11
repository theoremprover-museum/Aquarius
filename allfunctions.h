/* allfunctions.h made 
Sat Jul  4 13:27:29 EDT 1992
*/

/* av.c */

int tp_alloc();
int get_term();
void free_term();
int get_rel();
void free_rel();
int get_sym_ent();
void free_sym_ent();
int get_term_ptr();
void free_term_ptr();
int get_formula_ptr_2();
void free_formula_ptr_2();
int get_fpa_tree();
void free_fpa_tree();
int get_fpa_head();
void free_fpa_head();
int get_context();
void free_context();
int get_trail();
void free_trail();
int get_imd_tree();
void free_imd_tree();
int get_imd_pos();
void free_imd_pos();
int get_is_tree();
void free_is_tree();
int get_is_pos();
void free_is_pos();
int get_fsub_pos();
void free_fsub_pos();
int get_literal();
void free_literal();
int get_clause();
void free_clause();
int get_list();
void free_list();
int get_clash_nd();
void free_clash_nd();
int get_clause_ptr();
void free_clause_ptr();
int get_int_ptr();
void free_int_ptr();
int get_ans_lit_node();
void free_ans_lit_node();
int get_formula_box();
void free_formula_box();
int get_formula();
void free_formula();
int get_formula_ptr();
void free_formula_ptr();
int get_link_node();
void free_link_node();
void free_imd_pos_list();
void free_is_pos_list();
void print_mem();
void print_mem_brief();
int total_mem();
void print_linked_ur_mem_stats();
int term_ptr_get_size();
int is_tree_get_size();
int get_time_node();
void free_time_node();
void re_initialize_avail();
void init_mem_stats();
void add_your_mem_stats();
void print_mem_from_struct();

/* io.c */

void print_variable();
void str_print_variable();
void print_term();
void str_print_term();
int sprint_term();
void p_term();
void print_term_nl();
int proper_list();
int str_to_sn();
void mark_evaluable_symbols();
void print_syms();
void p_syms();
void free_sym_tab();
char *sn_to_str();
int sn_to_arity();
int sn_to_ec();
int set_to_predicate();
struct sym_ent *sn_to_node();
int in_sym_tab();
int mark_as_skolem();
int is_skolem();
void str_copy();
int str_ident();
int initial_str();
int read_buf();
void skip_white();
int is_delim();
void get_word();
int str_term();
int str_atom();
int set_vars();
int set_vars_term();
int var_name();
int read_term();
int read_list();
void print_list();
void print_error();
int is_symbol();
void bird_print();
int str_int();
int str_long();
void int_str();
void long_str();
void reverse();
void cat_str();
void check_beq_lex_val();

/* share.c */

void init_term_tab_for_roo();
static int linear_vars();
static void mark_linear_ground();
static int hash_term();
static int term_compare();
int integrate_term();
int disintegrate_term();
void zap_term();
void print_term_tab();
void p_term_tab();
void test_terms();
int all_instances();
int all_instances_fpa();
int bd_kludge_insert();
int bd_kludge_delete();

/* fpa.c */

static void path_mark_end();
static int hash_path();
static int path_comp();
static int path_size();
static int path_copy();
static int insert_fpa_tab();
static void delete_fpa_tab();
int term_fpa_rec();
int fpa_insert();
int fpa_delete();
static int get_leaf_node();
static int all_args_vars();
static int build_tree_local();
int build_tree();
struct term *next_term();
int build_for_all();
void zap_prop_tree();
void print_fpa_tab();
void p_fpa_tab();
void print_prop_tree();
void p_prop_tree();
void print_path();
void p_path();
int new_sym_num();
int new_sym_num();

/* clocks.c */

void clock_init();
long clock_val();
void clock_reset();
char *get_time();
long system_time();
long run_time();

/* unify.c */

int occur_check();
int unify();
int unify_no_occur_check();
int match();
int apply();
int term_ident();
void clear_subst_2();
void clear_subst_1();
void print_subst();
void p_subst();
void print_trail();

/* demod.c */

static int contract_lin();
static void dollar_out_non_list();
static void dollar_out();
static int dollar_contract();
static int demod();
static int left_most_one_step();
static int demod_out_in();
static int un_share_special();
int convenient_demod();
void zap_term_special();
int apply_demod();
int demod_cl();
int back_demod();
int back_demod();
int lit_t_f_reduce();
int check_input_demod();
int new_demod();

/* weight.c */

static struct is_tree *weight_retrieve();
int weight();
int wt_match();
static void set_wt_term();
static int set_wt_template();
static int weight_insert();
int set_wt_list();
void weight_index_delete();
static int lex_compare_sym_nums();
static int lex_order();
static int lex_order_vars();
static int term_order();
int lex_check();
static void get_var_multiset();
int var_subset();
static int sym_occur();
static int sym_elim();
int order_equalities();
int term_ident_x_vars();
int new_function();

/* imd.c */

static int insert_imd_tree();
int imd_insert();
static int end_term_imd();
int imd_delete();
int contract_imd();
void print_imd_tree();
void p_imd_tree();

/* is.c */

static int insert_is_tree();
int is_insert();
static int end_term_is();
int is_delete();
int is_retrieve();
int fs_retrieve();
void canc_fs_pos();
void print_is_tree();
void p_is_tree();

/* clause.c */

int next_cl_num();
int cl_integrate();
int cl_del_int();
void cl_del_non();
void cl_int_chk();
int read_cl_list();
int read_clause();
int set_vars_cl();
void print_ids();
void print_clause();
int read_clause_msg();
void str_print_clause();
int sprint_clause();
void p_clause();
void print_cl_list();
void cl_merge();
int tautology();
int size_ancestor_bag();
int size_ancestor_set();
int proof_length();
int subsume();
int map_rest();
int anc_subsume();
int forward_subsume();
int back_subsume();
int unit_conflict();
int pos_clause();
int all_pos_but_lit();
int neg_clause();
int num_literals();
int unit_clause();
int reflexivity();
int target();
int eq_target();
int ex_ans_target();
int all_solved_ans_target();
void append_cl();
void prepend_cl();
void insert_before_cl();
void insert_after_cl();
void rem_from_list();
int insert_clause();
int weight_cl();
void hide_clause();
int del_hidden_clauses();
void hide_msg();
int del_hidden_msgs();
int send_clause();
int cl_copy();
void remove_var_syms();
int cl_insert_tab();
int cl_delete_tab();
struct clause *cl_find();
void sort_lits();
int all_cont_cl();
void zap_cl_list();
void mark_literal();
int get_ancestors();
int proof_level();
int renumber_vars();
int renum_vars_term();
void clear_var_names();
void cl_clear_vars();
void distinct_vars_rec();
int distinct_vars();
struct clause *find_first_cl();
struct clause *find_last_cl();
int find_random_cl();
struct clause *find_lightest_cl();
struct clause *find_lightest_with_p();
struct clause *find_d_light_cl();
struct clause *find_first_cl_mod_nop();
int find_given_clause();
int extract_given_clause();
struct clause *find_given_msg();
struct clause *extract_given_msg();
int unit_del();
int n_resolution_check();

/* options.c */

void init_options();
void print_options();
void p_options();
int change_flag();
int change_parm();
void check_options();
void dependent_options();

/* resolve.c */

static build_hyper();
static int clash();
int hyper_res();
int neg_hyper_res();
int ur_res();
int one_unary_answer();
int combine_answers();
int build_bin_res();
int bin_res();
int all_factors();

/* index.c */

static int index_mark_clash();
static int un_index_mark_clash();
static int index_paramod();
static int un_index_paramod();
int index_lits_all();
int un_index_lits_all();
int index_lits_clash();
int un_index_lits_clash();
int un_index_rem_and_hide();
int un_index_rem_hide_deld();

/* paramod.c */

static int apply_substitute();
static int build_bin_para();
static int para_from_up();
static int para_from_alpha();
int para_from();
static int para_into_terms();
int para_into();

/* formula.c */

void print_formula();
void p_formula();
static void str_print_formula();
int sprint_formula();
static int str_form_term();
static int term_to_formula();
int str_formula();
int read_formula();
int read_formula_list();
void print_formula_list();
int copy_formula();
void zap_formula();
int negate_formula();
int nnf();
static void rename_free_formula();
static int skolem();
int skolemize();
int anti_skolemize();
static int subst_free_term();
int subst_free_formula();
int gen_sk_sym();
int skolem_symbol();
int contains_skolem_symbol();
int new_var_name();
int new_functor_name();
static int uq_all();
int unique_all();
static void mark_free_var_term();
static void mark_free_var_formula();
struct formula *zap_quant();
static void flatten_top_2();
void flatten_top();
static int distribute();
int cnf();
int dnf();
static int rename_syms_term();
int rename_syms_formula();
void subst_sn_term();
void subst_sn_formula();
int gen_subsume_prop();
struct formula *subsume_conj();
struct formula *subsume_disj();
int formula_ident();
void conflict_tautology();
void ts_and_fs();
static int set_vars_term_2();
static int set_vars_cl_2();
static int disj_to_clause();
static int cnf_to_list();
int clausify();
int clausify_formula_list();
int negation_inward();
int expand_imp();
int iff_to_conj();
int iff_to_disj();
int nnf_cnf();
int nnf_dnf();
int nnf_skolemize();
int clausify_formed();
int rms_conflict_tautology();
int rms_subsume_conj();
int rms_subsume_disj();
int free_occurrence();
int rms_distribute_quants();
static int separate_free();
int rms_push_free();
int rms_quantifiers();
static int rms_distribute();
int rms();
static void introduce_var_term();
static int introduce_var();
int renumber_unique();
int gen_subsume_rec();
int gen_subsume();
int gen_conflict();
int gen_tautology();
int rms_cnf();
int rms_dnf();

/* process.c */

static int post_process();
int post_proc_all();
int infer_and_process();
int handle_for_sub();
int proc_gen();
int pre_process();
int update_db();
void decide_allocation();

/* misc.c */

int init();
int read_all_input();
int set_lex_vals();
int set_lrpo_status();
int set_special_unary();
int set_skolem();
int free_all_mem();
void output_stats();
void print_stats();
void print_stats_brief();
void p_stats();
void print_times();
void print_times_brief();
void p_times();
void append_lists();
int copy_and_append_list();
int half_list();
int copy_term();
int biggest_var();
void zap_list();
int occurs_in();
int sn_occur();
int is_atom();
static int id_nested_skolems();
int ident_nested_skolems();
int ground();
void cleanup();
int check_stop();
void report();
void reduce_weight_limit();
void control_memory();
static void proof_message();
int print_proof();
int check_for_proof();

/* lrpo.c */

static int sym_precedence();
static int lrpo_status();
static struct rel *reverse_args();
static void reverse_rl();
static int lrpo_lex();
static int num_occurrences();
static int set_multiset_diff();
static int lrpo_multiset();
int lrpo();
int lrpo_greater();
int order_equalities_lrpo();
static int make_clause_into_term();
int greater_cl();

/* interface.c */

void input_phase();
void get_type_of_co();
void small_infer();
void infer();
void infer_alone();
void store_special();
void store_mesg();
void get_mesg();
static int move_messages();
static int mv_msg();
void new_clean_up();
void is_up();
void update_settled_count();
int is_other_penguin_up();
void print_banner();
void interrupt_interact();
