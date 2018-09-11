/* Penguin */

#define MAX_NO_OF_NODES		32
#define MAX_FNL			24	/* Max file name length. */
#define MAX_CNL			 4	/* Max channel name length. */
#define MAX_MSG			5000	/* same as MAX_BUF in Otter */
#define MAX_IINNTT	2147483647	/* same as MAX_INT in Otter */

#define INPUT_ERROR		5
#define INPUT_READ		6
#define INPUT_READ_ALL		18
#define INPUT_READ_GO		27
#define PROOF			-10
#define EARLY_STOP		-11
#define HALT_RECEIVED		-20
#define TROUBLE			-9
#define WAIT_TO_RECEIVE		-5
#define CONTINUATION		-22
/* All these codes need to be negative, in order not to be confused with */
/* a positive number representing the number of messages to be sent.	*/

#define NO_TROUBLE		-14
#define	NO_PROOF		-17

#define FINAL			14
#define PERIODICAL		10

#define ALL_PENGUINS		-21
#define IN_ALL_PENGUINS_S	-41
#define IN_ALL_PENGUINS_U	-36
#define IN_ALL_PENGUINS_D	-42
#define IN_ALL_PENGUINS_P	-39
/* If the destination is ALL_PENGUINS, the message is a non-input inference */
/* message to be broadcasted.						*/
/* If the destination is IN_ALL_PENGUINS_*, the message is an input clause */
/* to be broadcasted.							*/
/* IN_ALL_PENGUINS_S: input clause belonging to Sos,			*/
/* IN_ALL_PENGUINS_U: input clause belonging to Usable,			*/
/* IN_ALL_PENGUINS_D: input clause belonging to Demodulators.		*/
/* IN_ALL_PENGUINS_P: input clause belonging to Passive.		*/
#define NONE			-25
/* Blank value for the destination of a clause.				*/
/* These values are negative so that they are not confused with ids of	*/
/* specific Penguins.							*/

/* Clause types to be passed to cl_integrate() in Penguin.		*/
#define IR			1	/* Input Read clause */
#define IM			2	/* Input Message clause */
#define ND			3	/* New Demodulator */
#define EC			4	/* Empty Clause */
/* The default value when cl_integrate() is called on any other clause is 0. */

/* Target types: see target() and eq_target() in clause.c for the definitions.*/
#define	KB_TARGET		23
#define	EXT_EQ_TARGET		33
#define	ANS_TARGET		16

#define SOLVED_TARGET	7
/* Used for the field type in clause. This field is used only by the linked */
/* inference rules in Otter. It is initialized to NOT_SPECIFIED, see header.h.*/
/* The linked inference rules are off in Penguin. Therefore the field type */
/* remains largely unused. The Penguins use it as follows: in those problems */
/* where the Passive list is used to store targets, waiting to generate an */
/* empty clause by unit_conflict(), the field type of such a clause in Passive*/
/* is set to SOLVED_TARGET as soon as it has generated an empty clause	*/
/* by unit_conflict() with a generated clause being pre_processed().	*/
/* In this way we know which of the targets in Passive have been solved.*/
/* This is useful only if more than one target is given, i.e.		*/
/* Parms[MAX_PROOFS].val > 1, as we are seeking to prove more than one	*/
/* target in a single run. This is done when we want to prove first lemmas */
/* to be used on the way to the proof of a main theorem.		*/
/* An example are the proofs in Lukasiewicz's logic.			*/
/* Otter does not need this extra information, because it generates one	*/
/* empty clause per target. But the Penguins do, because they may generate*/
/* more than one empty clause per target, due to the redundancy	*/
/* represented by messages. For instance, a target clause in Passive may */
/* generate an empty clause by unit_conflict() with a leaving new settler, */
/* then receive an inference message from the Penguin where that new settler */
/* has settled, and generate another empty clause. It follows that the number */
/* of generated empty clauses in Stats[EMPTY_CLAUSES] may reach 	*/
/* Parms[MAX_PROOF].val, because one target has generated more than one	*/
/* empty clause, whereas another target has not been proved.		*/
/* Thus we need to know which targets have generated an empty clause and */
/* which have not.							*/
