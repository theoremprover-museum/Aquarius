/* Penguin */

/* interface.c */
/* Penguin: this file replaces main.c of Otter */

/*
 *
 *                        Otter 2.2
 *
 *                Automated Deduction System
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
 *   Conditional compilation.  Several places in the code assume that
 *   exactly one of the following 3 symbols is defined:
 *      
 *      TP_UNIX -- UNIX C compiler (usually cc or gcc)
 *      THINK_C -- Think-C compiler for Macintosh
 *      TURBO_C -- Turbo C compiler for PCs
 *
 *   If necessary, modify code for one of those to fit your compiler.
 *
 *   (ROO is defined for the parallel, shared memory version of Otter.) 
 */

#define IN_MAIN  /* so that global vars in header.h will not be external */
#include "header.h"

#ifdef TP_UNIX  /* for call to signal() */
#include <signal.h>
#endif

#ifdef THINK_C  /* Macintosh */
#include <console.h>  /* for call to ccommand */
#endif

#ifdef TURBO_C  /* PC */
extern unsigned _stklen = 16000;  /* stack size for Turbo C. */
#endif

/************************
*
*	void input_phase(given_name,exitcode,nodes,who)
*
*************************/

void input_phase(given_name,exitcode,nodes,who)
	char given_name[MAX_FNL];
	int *exitcode, *nodes, *who;
/* given_name is the string to make up the names for the files replacing */
/* standard input, output and error of Otter.				*/
/* *nodes is the number of nodes; *who is the id of the node.		*/
{
    int errors;
	char infile[MAX_FNL], outfile[MAX_FNL], errfile[MAX_FNL];
	char tempinfile[MAX_FNL], tempoutfile[MAX_FNL], temperrfile[MAX_FNL];
	char *keepin, *keepout, *keeperr;
	char penguin_name[MAX_FNL];
	int rai, i, found;

#ifdef THINK_C  /* macintosh */
    argc = ccommand(&argv);
#endif

/* Penguin: interrupt_interact is disabled.
#ifdef TP_UNIX
	signal(SIGINT, interrupt_interact);
#endif
*/

if (init(*nodes,*who) == TROUBLE)
	*exitcode = TROUBLE;
else {	/* else successful init() */
/* Penguin: preparing file names for files replacing standard input, output */
/* and error of Otter. */

keepin = ".in.";
keepout = ".out.";
keeperr = ".err.";
int_str(Whoami,penguin_name);
/* It turns the int id of the Penguin into a string.			*/
cat_str(given_name,keepin,tempinfile,MAX_FNL);
cat_str(tempinfile,penguin_name,infile,MAX_FNL);
cat_str(given_name,keepout,tempoutfile,MAX_FNL);
cat_str(tempoutfile,penguin_name,outfile,MAX_FNL);
cat_str(given_name,keeperr,temperrfile,MAX_FNL);
cat_str(temperrfile,penguin_name,errfile,MAX_FNL);

*exitcode = INPUT_READ;
/* Default value: changed later to TROUBLE, if errors occur, to PROOF, if */
/* a proof is found, to INPUT_READ_ALL if this is the Penguin which read  */
/* the input clauses, to INPUT_READ_GO if Flags[STAND_ALONE] is on or if */
/* there is only one Penguin.						*/

/* Penguin: opening files replacing standard input, output and error of Otter.*/
Fdout = fopen(outfile,"w");
if (Fdout == NULL)
	{
	printf("Penguin%d: error opening \"%s\" for writing.\n",Whoami,outfile);
	*exitcode = TROUBLE;
	}
else { /* output file open */
	Fderr = fopen(errfile,"w");
	if (Fderr == NULL)
	{
	printf("Penguin%d: error opening \"%s\" for writing.\n",Whoami,errfile);
	*exitcode = TROUBLE;
	}
	else /* output files open correctly */
	{
	Fdin = fopen(infile,"r");
	if (Fdin == NULL)
	{
	fprintf(Fdout,"No existing input file given to Penguin%d.\n",Whoami);
	fprintf(Fderr,"No existing input file given to Penguin%d.\n",Whoami);
	*exitcode = TROUBLE;
	}
	else /* Fdin != NULL */
	{
	print_banner();
	rai = read_all_input();
	if (rai == PROOF || rai == TROUBLE)
		*exitcode = rai;
	else {	/* rai == NO_PROOF */
	errors = Stats[INPUT_ERRORS];
    	if (errors != 0)
	{
	fprintf(Fderr, "\n%d input errors were found.\007\n\n", errors);
	fprintf(Fdout,"%d input errors were found.\n", errors);
	print_options(Fdout);
	*exitcode = TROUBLE;
	} /* end if input errors */
	else {	/* errors == 0 */
		if (No_of_nodes == 1 || Flags[STAND_ALONE].val)
			*exitcode = INPUT_READ_GO;
		else { /* not alone */
		found = 0;
		i = 0;
		while (i < No_of_nodes && found == 0)
			{
			if (i != Whoami && Penguins[i] > 1)
				found = 1;
			i++;
			}
/* There are two ways to determine whether this Penguin is the one which */
/* read the input clauses:						*/
/* either for some Penguin i different from itself it has Penguins[i] > 1, */
/* i.e. input clauses to send to Penguin i,				*/
		if (i == No_of_nodes && found == 0 && Penguins[Whoami] > 1)
			found = 1;
/* or it is the only Penguin with allocated clauses.			*/
		if (found)
			*exitcode = INPUT_READ_ALL;
			} /* end of else not alone */
		}	/* end of else errors == 0 */
	} /* end else NO_PROOF */
	} /* end else Fdin not null */
	} /* end else output files open correctly */
	} /* end else output file open */
	} /* end else successful init() */
}  /* input_phase() */

/***************
*
*	void get_type_of_co(type)
*
***************/

void get_type_of_co(type)
int *type;
{
	*type = Flags[PRIORITY_MSGS].val;

}	/* get_type_of_co() */

/*************************
*
*	small_infer(exitcode)
*
*	small_infer() corresponds to a single iteration of the MAIN LOOP
*	in infer(). small_infer() is invoked if set(priority_msgs).
*	Otherwise, infer() is invoked.
*
**************************/

void small_infer(exitcode)
	int *exitcode;
{
    struct clause *giv_cl;
    int early_stop, stop_proof, extracted;
    char *str;

/* It returns because:							*/
/* either it found a proof: stop_proof == PROOF;			*/
/* or it received HALT. from another Penguin: Halting == 1; 		*/
/* (Halting will be set to 1 by store_special() in this file)		*/
/* or there has been an error: stop_proof == TROUBLE;			*/
/* or the search ended because it has hit a threshold value on some	*/
/* parameter and then it prints the reason why: early_stop != 0;	*/
/* or it needs to send: Stats[OUTBOUND_MSGS_SIZE]> 0; it returns 	*/
/* Stats[OUTBOUND_MSGS_SIZE], i.e. the number of pending outbound messages; */
/* or Sos is empty: giv_cl == NULL; then either it waits to receive 	*/
/* messages or a saturated set has been generated.			*/

stop_proof = NO_PROOF;
giv_cl = NULL;
early_stop = 0;
extracted = 0;

if (Halting == 0 && Inbound_msgs->first_cl != NULL)
	stop_proof = move_messages();
/* If there are any messages in Inbound_msgs, it pre_process() them, 	*/
/* thereby appending them to Sos, and post_process() them.		*/

if (stop_proof == PROOF || stop_proof == TROUBLE)
	{
	*exitcode = stop_proof;
	return;
	}

if (Halting == 0 && Stats[OUTBOUND_MSGS_SIZE] > 0)
	{
	*exitcode = Stats[OUTBOUND_MSGS_SIZE];
	return;
	}

if (Halting == 0)
{	/* Halting == 0, then extract given clause */
stop_proof = extract_given_clause(Sos,&giv_cl);
/* extract_given_clause() decrements Stats[_SIZE] and rem_from_list(giv_cl) */
extracted = 1;
}

if (stop_proof == TROUBLE)
	{
	*exitcode = TROUBLE;
	return;
	}

if (giv_cl != NULL)
{
if (Flags[POST_PROC_NS_BEFORE_SEND].val && giv_cl->pid!=Whoami && giv_cl->dest==giv_cl->pid)
/* Send new settler. New settlers have been appended to Sos and need to be */
/* sent here, only if the flag POST_PROC_NS_BEFORE_SEND is on. If it is off */
/* they have not been appended to Sos and they were sent by pre_process().*/
	{
	stop_proof = send_clause(giv_cl);
	if (stop_proof == TROUBLE)
			{
			*exitcode = TROUBLE;
			return;
			}
	}
else {
/* Not a new settler to be sent, but a given clause to be processed. */

	    if (Parms[REDUCE_WEIGHT_LIMIT].val != 0)
		reduce_weight_limit();

	    Stats[CL_GIVEN]++;
	    if (Flags[PRINT_GIVEN].val) {
		fprintf(Fdout,"\ngiven clause #%ld: ", Stats[CL_GIVEN]);
		fprintf(Fdout,"(wt=%d) ", giv_cl->weight);
		print_clause(Fdout, giv_cl);
		fflush(Fdout);
		}

#ifndef TP_UNIX   /* if PC or Macintosh, so user knows something is happening */
		fprintf(Fderr, "\ngiven clause #%ld: ", Stats[CL_GIVEN]);
	fprintf(Fderr, "(wt=%d) ", giv_cl->weight);
		print_clause(Fderr, giv_cl);
		fflush(Fderr);
#endif

if (Flags[EAGER_BD_INF_MSGS].val == 1)
if (giv_cl->pid == Whoami && giv_cl->dest == NONE && !eq_target(giv_cl))
/* Broadcast resident as inference message.				*/
/* It is done here only if the flag EAGER_BD_INF_MSGS is on. Otherwise, */
/* it will be done after expansion.					*/
/* It is done only if giv_cl is a resident, because if giv_cl is a clause */
/* brought in by an inference message, it has been already broadcasted. */
/* Broadcasting is done at the PCN level with no need for forwarding.	*/
/* If, instead, the broadcasting algorithm requires forwarding, this	*/
/* statement needs to be executed also for giv_cl->pid != Whoami,	*/
/* provided No_of_nodes > 2.						*/
/* giv_cl is NOT sent if						*/
/* it has been broadcasted already (giv_cl->dest==IN_ALL_PENGUINS_S/U/D/P || */
/* giv_cl->dest == ALL_PENGUINS), which also includes x = x,		*/
/* or if it is an equational target. Non-input equational targets are not */
/* sent, they stay at the Penguins, waiting to be simplified and para_into./
/* Input equational targets are broadcasted, but this has been already */
/* taken care of by either read_all_input() or pre_process().		*/
		{
		giv_cl->dest = ALL_PENGUINS;
		stop_proof = send_clause(giv_cl);
		if (stop_proof == TROUBLE)
			{
			*exitcode = TROUBLE;
			return;
			}
		}

	    stop_proof = index_lits_clash(giv_cl);
		if (stop_proof == TROUBLE)
			{
			*exitcode = TROUBLE;
			return;
			}

	    append_cl(Usable, giv_cl);

/* giv_cl is appended to Usable regardless of whether it is a resident or */
/* it came in as an inference message, in order to form the localized global */
/* data_base. 								*/
/* If it came as an inference message, its usage for expansion steps will */
/* be restricted: see files process.c, paramod.c and resolve.c.		*/

		Stats[USABLE_SIZE]++;

		stop_proof = infer_and_process(giv_cl);

		if (stop_proof == PROOF || stop_proof == TROUBLE)
			{
			*exitcode = stop_proof;
			return;
			}

if (Flags[EAGER_BD_INF_MSGS].val == 0)
if (giv_cl->pid == Whoami && giv_cl->dest == NONE && !eq_target(giv_cl))
/* Broadcast resident as inference message.				*/
/* It is done here only if the flag EAGER_BD_INF_MSGS is off. Otherwise, */
/* it has been done at the beginning of the treatment of the given_clause. */
		{
		giv_cl->dest = ALL_PENGUINS;
		stop_proof = send_clause(giv_cl);
		if (stop_proof == TROUBLE)
			{
			*exitcode = TROUBLE;
			return;
			}
		}

	if (Parms[REPORT].val > 0)
		report();
}	/* end of else not new settler */
}  /* end of if giv_cl != NULL */

early_stop = check_stop();

if (early_stop != 0)
		{
		if (early_stop == 1)
			str = "max_given";
		else if (early_stop == 2)
			str = "max_seconds";
		else if (early_stop == 3)
			str = "max_gen";
		else if (early_stop == 4)
			str = "max_kept";
	fprintf(Fderr, "\nsearch stopped by %s option.\007\n\n", str);
	fprintf(Fdout,"\nsearch stopped by %s option.\n", str);
		*exitcode = EARLY_STOP;
		return;
		}

if (extracted == 1 && giv_cl == NULL && Halting == 0)
	{
	    fprintf(Fderr,"\nSos empty at Penguin%d.\n\n",Whoami);
	    fprintf(Fdout,"\nSos empty at Penguin%d.\n\n",Whoami);
		if (No_of_nodes == 1)
/* If there is just one node, Penguin behaves like Otter, i.e. given_clause= */
/* = NULL means termination upon generation of a consistent set of clauses. */
/* It returns PROOF even if no refutation has been found, because at the */
/* upper level it needs to do the same operations as in case of PROOF.	*/
/* Also, terminating in this way means that a saturated set has been */
/* generated, which is also a successful way to halt, and/or that the given */
/* theorem has been disproved, because no refutation was found by negating */
/* it, so it is also a successful result.				*/
			*exitcode = PROOF;
		else if (No_of_nodes > 1)
			{
			*exitcode = WAIT_TO_RECEIVE;
		if (Flags[SATURATION].val)
			cleanup(PERIODICAL);
			}
/* Recall that in Penguin, unlike in Otter, cleanup() does not call	*/
/* free_all_mem(), which is called by new_clean_up() from the pcn	*/
/* function penguin(). Thus cleanup() here is just printing out informations. */
		return;
	}

if (Halting == 1)
	{
	*exitcode = HALT_RECEIVED;
	return;
	}

*exitcode = CONTINUATION;
/* If neither PROOF, nor TROUBLE, nor HALT_RECEIVED, nor Sos empty nor a */
/* reason for early stop has been found, it simply returns that it needs to */
/* continue.								*/

}  /* small_infer() */

/*************************
*
*	infer(exitcode)
*
**************************/

void infer(exitcode)
	int *exitcode;
{
    struct clause *giv_cl;
    int early_stop, stop_proof;
    char *str;

early_stop = 0;
/* early_stop != 0 : the search is interrupted because a threshold value */
/* on some parameter has been hit.					*/
stop_proof = NO_PROOF;
/* stop_proof == PROOF : the search is interrupted because of refutation. */
/* stop_proof == TROUBLE : the search is interrupted on error.	*/
/* Halting == 1: the search is interrupted because of HALT. */
/* received by another Penguin.						*/
/* (Halting will be set to 1 by store_special() in this file)		*/
giv_cl = NULL;
/* giv_cl == NULL : the search is interrupted because the Sos is empty:	*/
/* either we need to wait for messages from the outside			*/
/* or the given set of clauses is consistent and a saturated set of	*/
/* clauses has been generated.						*/

if (!Halting && Inbound_msgs->first_cl != NULL)
	stop_proof = move_messages();

if (!Halting && stop_proof == NO_PROOF && Stats[OUTBOUND_MSGS_SIZE] == 0)
/* If there are outbound messages pending, it will return to send them and */
/* therefore there is no point in selecting a given_clause.		*/
	stop_proof = extract_given_clause(Sos,&giv_cl);

/* --------------------- MAIN LOOP STARTS HERE --------------------- */

while (!Halting && giv_cl!=NULL && early_stop==0 && Stats[OUTBOUND_MSGS_SIZE]==0 && (stop_proof==NO_PROOF || stop_proof==NO_TROUBLE))
{
if (Flags[POST_PROC_NS_BEFORE_SEND].val && giv_cl->pid!=Whoami && giv_cl->dest==giv_cl->pid)
/* Send new settler. New settlers have been appended to Sos and need to be */
/* sent here, only if the flag post_proc_ns_before_send is on. If it is off */
/* they have not been appended to Sos and they were sent by pre_process().*/
	{
	stop_proof = send_clause(giv_cl);
		if (stop_proof == TROUBLE)
		break;
	}
else {	/* Not a new settler to be sent, but a given_clause to be processed. */

	    if (Parms[REDUCE_WEIGHT_LIMIT].val != 0)
		reduce_weight_limit();

	    Stats[CL_GIVEN]++;
	    if (Flags[PRINT_GIVEN].val) {
		fprintf(Fdout,"\ngiven clause #%ld: ", Stats[CL_GIVEN]);
		fprintf(Fdout,"(wt=%d) ", giv_cl->weight);
		print_clause(Fdout, giv_cl);
		fflush(Fdout);
		}

#ifndef TP_UNIX   /* if PC or Macintosh, so user knows something is happening */
		fprintf(Fderr, "\ngiven clause #%ld: ", Stats[CL_GIVEN]);
	fprintf(Fderr, "(wt=%d) ", giv_cl->weight);
		print_clause(Fderr, giv_cl);
		fflush(Fderr);
#endif

if (Flags[EAGER_BD_INF_MSGS].val == 1)
if (giv_cl->pid == Whoami && giv_cl->dest == NONE && !eq_target(giv_cl))
/* Broadcast inference messages.					*/
/* It is done here only if the flag EAGER_BD_INF_MSGS is on. Otherwise, */
/* it will be done after expansion.					*/
/* It is done only if giv_cl is a resident, because if giv_cl is a clause */
/* brought in by an inference message, it has been already broadcasted. */
/* Broadcasting is done at the PCN level with no need for forwarding.	*/
/* If, instead, the broadcasting algorithm requires forwarding, this	*/
/* statement needs to be executed also for giv_cl->pid != Whoami,	*/
/* provided No_of_nodes > 2.						*/
/* giv_cl is NOT sent if						*/
/* it has been broadcasted already (giv_cl->dest==IN_ALL_PENGUINS_S/U/D/P || */
/* giv_cl->dest == ALL_PENGUINS), which also includes x=x,		*/
/* or if it is an equational target. Non-input equational targets are not */
/* sent, they stay at the Penguins, waiting to be simplified and para_into./
/* Input equational targets are broadcasted, but this has been already */
/* taken care of by either read_all_input() or pre_process().		*/
		{
		giv_cl->dest = ALL_PENGUINS;
		stop_proof = send_clause(giv_cl);
		if (stop_proof == TROUBLE)
			break;
		}

	    stop_proof = index_lits_clash(giv_cl);

		if (stop_proof == TROUBLE)
			break;

	    append_cl(Usable, giv_cl);

/* giv_cl is appended to Usable regardless of whether it is a resident or */
/* it came in as an inference message in order to form the global data_base. */
/* If it came as an inference message, its usage for expansion steps will */
/* be restricted: see files process.c, paramod.c and resolve.c.		*/

		Stats[USABLE_SIZE]++;

		stop_proof = infer_and_process(giv_cl);
	    if (stop_proof == PROOF || stop_proof == TROUBLE)
			break;

if (Flags[EAGER_BD_INF_MSGS].val == 0)
if (giv_cl->pid == Whoami && giv_cl->dest == NONE && !eq_target(giv_cl))
/* Broadcast inference messages.					*/
/* It is done here only if the flag EAGER_BD_INF_MSGS is off. Otherwise, */
/* it has been done at the beginning of the treatment of the given_clause. */
		{
		giv_cl->dest = ALL_PENGUINS;
		stop_proof = send_clause(giv_cl);
		if (stop_proof == TROUBLE)
			break;
		}

	    early_stop = check_stop();

		if (early_stop != 0)
			break;

	}	/* end of else not a new settler */

if (Inbound_msgs->first_cl != NULL)
		stop_proof = move_messages();
/* If there are any messages in Inbound_msgs, it pre_process() them, 	*/
/* thereby appending them to Sos and post_process() them.		*/

if (stop_proof == PROOF || stop_proof == TROUBLE)
			break;

if (Halting == 0 && Stats[OUTBOUND_MSGS_SIZE] == 0)
		{
		stop_proof = extract_given_clause(Sos,&giv_cl);
/* If it needs to exit to send, there is no need to extract the next given */
/* clause.								*/
/* extract_given_clause() decrements Stats[_SIZE] and rem_from_list(giv_cl) */

	if (stop_proof != TROUBLE && giv_cl != NULL && Parms[REPORT].val > 0)
		report();
		}

	    }  /* end of main loop */

/* --------------------- MAIN LOOP ENDS HERE --------------------- */

/* It stopped because:							*/
/* either it found a proof: stop_proof == PROOF;			*/
/* or it received and HALT. message from another Penguin: Halting == 1;	*/
/* or there has been an error: stop_proof == TROUBLE;			*/
/* or the search ended because it has hit a threshold value on some	*/
/* parameter and then it prints the reason why: early_stop != 0;	*/
/* or it needs to send: return Stats[OUTBOUND_MSGS_SIZE], i.e. the number of */
/* pending outbound messages;						*/
/* or Sos is empty and then either it waits to receive messages		*/
/* or a saturated set has been generated.				*/

if (Halting)
	*exitcode = HALT_RECEIVED;
else if (stop_proof == PROOF || stop_proof == TROUBLE)
	*exitcode = stop_proof;
else if (early_stop != 0)
	{
	if (early_stop == 1)
		str = "max_given";
	else if (early_stop == 2)
		str = "max_seconds";
	else if (early_stop == 3)
		str = "max_gen";
	else if (early_stop == 4)
		str = "max_kept";
	fprintf(Fderr, "\nsearch stopped by %s option.\007\n\n", str);
	fprintf(Fdout,"\nsearch stopped by %s option.\n", str);
	*exitcode = EARLY_STOP;
	}
else if (Stats[OUTBOUND_MSGS_SIZE] > 0)
	*exitcode = Stats[OUTBOUND_MSGS_SIZE];
else { /* giv_cl == NULL */
	    fprintf(Fderr,"\nSos empty at Penguin%d.\n\n",Whoami);
	    fprintf(Fdout,"\nSos empty at Penguin%d.\n\n",Whoami);
		if (No_of_nodes == 1)
/* If there is just one node, Penguin behaves like Otter, i.e. given_clause= */
/* = NULL means termination upon generation of a consistent set of clauses. */
/* It returns PROOF even if no refutation has been found, because at the */
/* upper level it needs to do the same operations as in case of PROOF.	*/
/* Also, terminating in this way means that a saturated set has been */
/* generated, which is also a successful way to halt, and/or that the given */
/* theorem has been disproved, because no refutation was found by negating */
/* it, so it is also a successful result.				*/
			*exitcode = PROOF;
		else if (No_of_nodes > 1)
			{
			*exitcode = WAIT_TO_RECEIVE;
			if (Flags[SATURATION].val)
				cleanup(PERIODICAL);
			}
/* Recall that in Penguin, unlike in Otter, cleanup() does not call	*/
/* free_all_mem(), which is called by new_clean_up() from the pcn	*/
/* function penguin(). Thus cleanup() here is just printing out informations. */
	}

}  /* infer() */

/*************************
*
*	infer_alone(exitcode)
*
**************************/

void infer_alone(exitcode)
	int *exitcode;
{
    struct clause *giv_cl;
    int early_stop, stop_proof;
    char *str;

early_stop = 0;
/* early_stop != 0 : the search is interrupted because a threshold value */
/* on some parameter has been hit.					*/
stop_proof = NO_PROOF;
/* stop_proof == PROOF : the search is interrupted because of refutation. */
/* stop_proof == TROUBLE : the search is interrupted on error.	*/
giv_cl = NULL;
/* giv_cl == NULL : the search is interrupted because the Sos is empty:	*/
/* the given set of clauses is consistent and a saturated set of	*/
/* clauses has been generated.						*/

	stop_proof = extract_given_clause(Sos,&giv_cl);

/* --------------------- MAIN LOOP STARTS HERE --------------------- */

while (giv_cl!=NULL && early_stop==0 && (stop_proof==NO_PROOF || stop_proof==NO_TROUBLE))
{

	    if (Parms[REDUCE_WEIGHT_LIMIT].val != 0)
		reduce_weight_limit();

	    Stats[CL_GIVEN]++;
	    if (Flags[PRINT_GIVEN].val) {
		fprintf(Fdout,"\ngiven clause #%ld: ", Stats[CL_GIVEN]);
		fprintf(Fdout,"(wt=%d) ", giv_cl->weight);
		print_clause(Fdout, giv_cl);
		fflush(Fdout);
		}

#ifndef TP_UNIX   /* if PC or Macintosh, so user knows something is happening */
		fprintf(Fderr, "\ngiven clause #%ld: ", Stats[CL_GIVEN]);
	fprintf(Fderr, "(wt=%d) ", giv_cl->weight);
		print_clause(Fderr, giv_cl);
		fflush(Fderr);
#endif

	    stop_proof = index_lits_clash(giv_cl);

		if (stop_proof == TROUBLE)
			break;

	    append_cl(Usable, giv_cl);

		Stats[USABLE_SIZE]++;

		stop_proof = infer_and_process(giv_cl);
	    if (stop_proof == PROOF || stop_proof == TROUBLE)
			break;

	    early_stop = check_stop();

		if (early_stop != 0)
			break;

		stop_proof = extract_given_clause(Sos,&giv_cl);

	if (stop_proof != TROUBLE && giv_cl != NULL && Parms[REPORT].val > 0)
		report();

	    }  /* end of main loop */

/* --------------------- MAIN LOOP ENDS HERE --------------------- */

/* It stopped because:							*/
/* either it found a proof: stop_proof == PROOF;			*/
/* or there has been an error: stop_proof == TROUBLE;			*/
/* or the search ended because it has hit a threshold value on some	*/
/* parameter and then it prints the reason why: early_stop != 0;	*/
/* or Sos is empty and or a saturated set has been generated.		*/

if (stop_proof == PROOF || stop_proof == TROUBLE)
	*exitcode = stop_proof;
else if (early_stop != 0)
	{
	if (early_stop == 1)
		str = "max_given";
	else if (early_stop == 2)
		str = "max_seconds";
	else if (early_stop == 3)
		str = "max_gen";
	else if (early_stop == 4)
		str = "max_kept";
	fprintf(Fderr, "\nsearch stopped by %s option.\007\n\n", str);
	fprintf(Fdout,"\nsearch stopped by %s option.\n", str);
	*exitcode = EARLY_STOP;
	}
else { /* giv_cl == NULL */
	    fprintf(Fderr,"\nSos empty at Penguin%d.\n\n",Whoami);
	    fprintf(Fdout,"\nSos empty at Penguin%d.\n\n",Whoami);
			*exitcode = PROOF;
	}

}  /* infer_alone() */

/***************
*
*	void store_special(msg_text,msg_pid)			Penguin only
*
***************/

void store_special(msg_text,msg_pid)
char *msg_text;
int *msg_pid;
{
if (str_ident(msg_text,"HALT"))
	Halting = 1;

else if (str_ident(msg_text,"TROUBLE") || str_ident(msg_text,"E_STOP"))
	Penguins[*msg_pid] = 0;

/* It the received control message says that another Penguin has halted in */
/* trouble or in early_stop, it updates the array of Penguins accordingly. */

fprintf(Fdout,"** RECEIVED: %s from Penguin%d.\n",msg_text,*msg_pid);

}	/* store_special() */

/******************
*
*	void store_mesg(msg_text,msg_pid,msg_lid,msg_bt,msg_dest,exitcode)
*
*	Penguin only
*
******************/

void store_mesg(msg_text,msg_pid,msg_lid,msg_bt,msg_dest,exitcode)
char *msg_text;
int *msg_pid, *msg_lid, *msg_bt, *msg_dest;
int *exitcode;

{
struct clause *c;
int r;

if (read_clause_msg(msg_text,&r,&c) == TROUBLE)
	*exitcode = TROUBLE;
else if (c == NULL || r == 0)
	{
	output_stats(Fdout,4);
fprintf(Fderr,"ABEND: store_mesg; received a message carrying an ill formed clause.\n");
fprintf(Fdout,"ABEND: store_mesg; received a message carrying an ill formed clause.\n");
	*exitcode = TROUBLE;
	}
else {
	c->pid = *msg_pid;
	c->lid = *msg_lid;
	c->bt = *msg_bt;
	c->dest = *msg_dest;
	append_cl(Inbound_msgs,c);
	Stats[INBOUND_MSGS_SIZE]++;
	if (Flags[PRINT_RECEIVED].val)
		{
		fprintf(Fdout,"** RECEIVED: ");
		print_clause(Fdout,c);
		fflush(Fdout);
		}
	*exitcode = NO_TROUBLE;
	}
} /* store_mesg() */

/*****************
*
*	get_mesg(msg_text,msg_pid,msg_lid,msg_bt,msg_dest,r)	Penguin only
*
*****************/

void get_mesg(msg_text,msg_pid,msg_lid,msg_bt,msg_dest,r)
char *msg_text;
int *msg_pid, *msg_lid, *msg_bt, *msg_dest;
int *r;

{
struct clause *next_cl;

if ((next_cl = extract_given_msg(Outbound_msgs)) != NULL)
{
/* extract_given_msg() decrements Stats[_SIZE] and rem_from_list next_cl */
Stats[CL_SENT]++;
sprint_clause(msg_text,next_cl);
*msg_pid = next_cl->pid;
*msg_lid = next_cl->lid;
*msg_bt = next_cl->bt;
*msg_dest = next_cl->dest;
hide_msg(next_cl);
/* The clause will be deleted if Flags[REALLY_DELETE_MSGS].val, whose	*/
/* default value is 1.							*/
/* Recall that in case of input clauses and inference messages, deleting */
/* the clause extracted from Outbound_msgs does not conflict with saving */
/* the clause for the localized global data base, because the clause in */
/* Outbound_msgs is a copy of a clause in Usable. 			*/
/* We do not call un_indexing functions, 				*/
/* because when the copies were made to be appended to Outbound_msgs,	*/
/* indexing functions were not called and the literals of the copies 	*/
/* in Usable need to remain in the Fpa-lists. 				*/
/* On the other hand, if the clause being sent is a new settler, all the */
/* necessary un_indexing and deletion activities before hide_msg() */
/* have been already taken care of by send_clause() at the moment of	*/
/* appending the clause to Outbound_msgs, see send_clause() in clause.c. */
*r = 1;
}
else *r = 0;
} /* get_mesg() */

/************
*
*	static int move_messages()			Penguin only
*
*	It moves incoming messages from Inbound_msgs to Sos.
*	It returns PROOF/NO_PROOF/TROUBLE.
*
************/

static int move_messages()
{
int msts;

	while (Inbound_msgs->first_cl != NULL)
		{
		msts = mv_msg();
/* mv_msg() calls extract_given_msg(Inbound_msgs), 			*/
/* which removes a clause from Inbound_msgs.				*/
/* As long as move_messages() moves all the clauses in Inbound_msgs to Sos */
/* in one shot, the policy used in extract_given_msg() to extract elements */
/* from Inbound_msgs is not relevant. They go to Sos all together and then */
/* it is the policy of extraction from Sos which matters.		*/
		if (msts == PROOF || msts == TROUBLE)
			return(msts);
		}
	if (Stats[CL_GIVEN] == 0)
		mark_evaluable_symbols(0);
/* If moving in the input messages, call mark_evaluable_symbols(), that the */
/* Penguin which read the input called in read_all_input.		*/
/* The parameter set to 0 says that we are not reading the input, but moving */
/* in input messages.							*/

return(NO_PROOF);
}	/* move_messages() */

/************
*
*	static int mv_msg()			Penguin only
*	It moves one message from Inbound_msgs to its destination list.
*	It returns PROOF/NO_PROOF/TROUBLE.
*
*************/

static int mv_msg()
{
struct clause *giv_msg, *sos_pos, *us_pos;
struct term *t;
int pp, good_demod;

giv_msg = extract_given_msg(Inbound_msgs);
/* extract_given_msg() removes the clause from the list and	*/
/* decrements Stats[_SIZE].					 */

if (giv_msg != NULL)
{
if (giv_msg->dest==IN_ALL_PENGUINS_S) /* Message carrying an input Sos clause */
{
if (Flags[PROCESS_INPUT].val)
	{
	sos_pos = Sos->last_cl;

	pp = pre_process(giv_msg,IM,Sos);

	if (pp == PROOF || pp == TROUBLE)
		return(pp);

	pp = post_proc_all(sos_pos,IM,Sos);

	if (pp == PROOF || pp == TROUBLE)
		return(pp);
	}	/* end of if PROCESS_INPUT */
else {
	if (cl_integrate(giv_msg,IM,NONE) == TROUBLE)
		return(TROUBLE);
/* IM means Input Message: see cl_integrate() in clause.c for details.	*/
/* Remark that cl_integrate() is called by mv_msg() only if mv_msg() does */
/* not call pre_process(). If mv_msg() calls pre_process(), it's the latter */
/* which calls cl_integrate().						*/

	if (index_lits_all(giv_msg) == TROUBLE)
		return(TROUBLE);

	append_cl(Sos,giv_msg);
	Stats[SOS_SIZE]++;

    if (Flags[PRINT_KEPT].val) {
	fprintf(Fdout,"** KEPT: ");
	CLOCK_START(PRINT_CL_TIME)
	print_clause(Fdout,giv_msg);
	CLOCK_STOP(PRINT_CL_TIME)
	}
	}	/* end of not PROCESS_INPUT */
}	/* end of if IN_ALL_PENGUINS_S */
else if (giv_msg->dest==IN_ALL_PENGUINS_U)
/* Message carrying an input Usable clause */
{
if (Flags[PROCESS_INPUT].val)
{
	us_pos = Usable->last_cl;

	pp = pre_process(giv_msg,IM,Usable);

	if (pp == PROOF || pp == TROUBLE)
		return(pp);

	pp = post_proc_all(us_pos,IM,Usable);

	if (pp == PROOF || pp == TROUBLE)
		return(pp);
}	/* end of if PROCESS_INPUT */
else {
if (cl_integrate(giv_msg,IM,NONE) == TROUBLE)
	return(TROUBLE);
if (index_lits_all(giv_msg) == TROUBLE)
	return(TROUBLE);
if (index_lits_clash(giv_msg) == TROUBLE)
	return(TROUBLE);

append_cl(Usable,giv_msg);
Stats[USABLE_SIZE]++;

    if (Flags[PRINT_KEPT].val) {
	fprintf(Fdout,"** KEPT: ");
	CLOCK_START(PRINT_CL_TIME)
	print_clause(Fdout,giv_msg);
	CLOCK_STOP(PRINT_CL_TIME)
	}
}	/* end of not PROCESS_INPUT */
}	/* end of if IN_ALL_PENGUINS_U */
else if (giv_msg->dest == IN_ALL_PENGUINS_D)
/* Message carrying an input Demodulator clause.		*/
{
if (check_input_demod(giv_msg,&good_demod) == TROUBLE)
	return(TROUBLE);
if (good_demod)
{
if (cl_integrate(giv_msg,IM,NONE) == TROUBLE)
	return(TROUBLE);

append_cl(Demodulators,giv_msg);
Stats[DEMODULATORS_SIZE]++;

if (Flags[DEMOD_LINEAR].val == 0)
	imd_insert(giv_msg,Demod_imd);

    if (Flags[PRINT_KEPT].val) {
	fprintf(Fdout,"** KEPT: ");
	CLOCK_START(PRINT_CL_TIME)
	print_clause(Fdout,giv_msg);
	CLOCK_STOP(PRINT_CL_TIME)
	}
}	/* end of if good_demod */
else {
fprintf(Fdout,"ERROR, bad demodulator: ");
print_clause(Fdout,giv_msg);
}
}	/* end of if IN_ALL_PENGUINS_D */
else if (giv_msg->dest == IN_ALL_PENGUINS_P)
/* Message carrying an input Passive clause.				*/
{
if (cl_integrate(giv_msg,IM,NONE) == TROUBLE)
	return(TROUBLE);
if (index_lits_all(giv_msg) == TROUBLE)
	return(TROUBLE);

append_cl(Passive,giv_msg);
Stats[PASSIVE_SIZE]++;

    if (Flags[PRINT_KEPT].val) {
	fprintf(Fdout,"** KEPT: ");
	CLOCK_START(PRINT_CL_TIME)
	print_clause(Fdout,giv_msg);
	CLOCK_STOP(PRINT_CL_TIME)
	}
}	/* end of if IN_ALL_PENGUINS_P */
else if (giv_msg->dest == ALL_PENGUINS || giv_msg->dest == giv_msg->pid)
{
/* If inference message or new settler, then pre_process, post_process 	*/
/* and append to Sos.							*/

sos_pos = Sos->last_cl;

pp = pre_process(giv_msg,0,Sos);

if (pp == PROOF || pp == TROUBLE)
	return(pp);

/* Forward contraction on the selected message, then appended to Sos. */
/* The second parameter is set to 0 to mean the incoming message is not */
/* an input clause.							*/

pp = post_proc_all(sos_pos,0,Sos);

if (pp == PROOF || pp == TROUBLE)
	return(pp);

/* Backward contraction by the selected message: post_proc_all() will	*/
/* select as back-demodulator/back-subsumer the clause pointed to by	*/
/* sos_pos->next_cl, i.e. the selected message.				*/
/* The second parameter is set to 0 to mean the incoming message is not */
/* an input clause.							*/

/* This function does for inference messages the contraction phases	*/
/* that are done for input clauses by read_all_input().			*/
}	/* end of if inference message or new settler			*/
}	/* end of if not NULL						*/

return(NO_PROOF);
}	/* mv_msg() */

/*************
 *
 *	void new_clean_up(lc)			Penguin only
 *
 *	It replaces the call to free_all_mem() that Otter has in
 *	cleanup(). Penguin needs to defer the call to free_all_mem()
 *	until both main_infer()
 *	and receive() have completed.
 *	It returns TROUBLE/NO_TROUBLE through the parameter lc.
 *
 *************/

void new_clean_up(lc)
int *lc;
{
	cleanup(FINAL);
	if (Flags[FREE_ALL_MEM].val)
		{
		if (free_all_mem() == TROUBLE)
			*lc = TROUBLE;
		}
	*lc = NO_TROUBLE;

}	/* new_clean_up() */ 

/**************
 *
 *	void is_up(p,r)		Penguin only
 *
 *	It sets *r to the contents of Penguins[*p]: *r > 0 means
 *	Penguin *p is up.
 *
 ***************/

void is_up(p,r)
int *p, *r;
{
*r = Penguins[*p];
}	/* is_up() */

/**************
 *
 *	void update_settled_count(n)		Penguin only
 *
 *	It is called by a Penguin which has not read the input clauses.
 *	Such a Penguin receives from the Penguin which has read the input
 *	clauses the number of input clauses assigned to it as residents
 *	and update accordingly its Penguins array, so that Penguins[Whoami]
 *	is the value to give as lid to the next clause which settles down
 *	at this Penguin.
 *
 ***************/

void update_settled_count(n)
int *n;
{
Penguins[Whoami] = *n;
}	/* update_settled_count() */

/*************
 *
 *	int is_other_penguin_up()		Penguin only
 *
 *	It returns 1 if any other Penguin is up, 0 otherwise.
 *
 ************/

int is_other_penguin_up()
{
int p;

	for (p = 0; p < No_of_nodes; p++)
		if (p != Whoami && Penguins[p] > 0)
			return(1);
return(0);
}	/* is_other_penguin_up() */

/*************
 *
 *    void print_banner()
 *
 *************/

void print_banner()
{
    int i;
    char host[64];

#ifdef TP_UNIX
    if (gethostname(host, 64) != 0)
	str_copy("???", host);
#else
#ifdef THINK_C
    str_copy("a Macintosh", host);
#else
#ifdef TURBO_C
    str_copy("a PC", host);
#else
    str_copy("???", host);
#endif
#endif
#endif

printf("---- PENGUIN%d ----\nThe job began on %s, %s\n",Whoami,host,get_time());
fprintf(Fdout,"---- PENGUIN%d ----\nThe job began on %s, %s\n",Whoami,host,get_time());
fprintf(Fderr,"---- PENGUIN%d ----\nThe job began on %s, %s\n",Whoami,host,get_time());

}  /* print_banner() */


/* Penguin does not use interrupt_interact() */
/*
#ifdef TP_UNIX
*/
/*************
 *
 *    void interrupt_interact()
 *
 *    This routine provides some primitive interaction with the user.
 *
 *************/
/*
void interrupt_interact()
{
    FILE *fp;
    struct term *t;
    int rc, go_back;

    fp = fopen("/dev/tty", "r+");

    if (fp == NULL)
	printf("interaction failure: cannot find tty.\n");
    else {

	printf("\n --- BEGIN INTERACTION ---\n\n");
	fprintf(fp, "\n --- BEGIN INTERACTION ---\n");
fprintf(fp,"Commands are help, kill, continue, set(..), and assign(..,..).\n");
	fprintf(fp, "All commands must end with a period.\n> ");
	fflush(fp);
        fseek(fp, (long) 0, 2); 
	t = read_term(fp, &rc);
	fflush(fp);
	go_back = 0;
	while ((t != NULL || rc == 0) && !go_back) {
	    if (t == NULL)
		fprintf(fp, " malformed term; try again.\n> ");
	    else if (t->type == NAME || t->type == VARIABLE) {
		
		if (str_ident("help", sn_to_str(t->sym_num))) {
fprintf(fp,"Commands are help, kill, continue, set(..), and assign(..,..).\n");
		    fprintf(fp, "All commands end with a period.\n> ");
		    }
		else if (str_ident("kill", sn_to_str(t->sym_num))) {
		    printf("\nkilled during interaction.\n");
		    fprintf(stderr, "killed during interaction.\007\n");
		    fclose(fp);
		    cleanup();
		    exit(1);
		    }
		else if (str_ident("continue", sn_to_str(t->sym_num)))
		    go_back = 1;
		else
		    fprintf(fp, " command not understood.\n> ");
		}
	    else if (str_ident("set", sn_to_str(t->sym_num))) {
		if (change_flag(fp, t, 1)) {
		    print_term(stdout, t); printf(".\n");
		    fprintf(fp, " ok.\n> ");
		    }
		}
	    else if (str_ident("clear", sn_to_str(t->sym_num))) {
		if (change_flag(fp, t, 0)) {
		    print_term(stdout, t); printf(".\n");
		    fprintf(fp, " ok.\n> ");
		    }
		}
	    else if (str_ident("assign", sn_to_str(t->sym_num))) {
		if (change_parm(fp, t)) {
		    print_term(stdout, t); printf(".\n");
		    fprintf(fp, " ok.\n> ");
		    }
		}
	    else
		fprintf(fp, " command not understood.\n> ");
	    
	    if (t != NULL)
		zap_term(t);
	    if (go_back == 0) {
		fflush(fp);
		fseek(fp, (long) 0, 2);  
		t = read_term(fp, &rc);
		fflush(fp);
		}
	    }
	
	printf("\n --- end interaction ---\n\n");
	fprintf(fp,"\n --- end interaction ---\n");
	
	fclose(fp);
	}

}
*/
/*
#endif
*/
/* Penguin does not use interrupt_interact() */

