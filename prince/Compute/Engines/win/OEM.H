/* Maple V Release 2 C-Callable OEM Kernel Header File
   Copyright (c) 1992, 1993 by Waterloo Maple Software

   This header file may be used in the compilation of a program that
   incorporates the Maple V Release 2 C-Callable OEM Kernel. This
   header file may NOT be distributed in source form. */

#ifndef __ALL_H__
typedef long MPL_INT;
typedef MPL_INT ***ALGEB;
typedef char MPL_CHAR;
#endif

/* The following are definitions common to the text string and DAG
   interfaces. */

extern MPL_INT bytesUsed;           /* Number of bytes Maple has requested
				   from its own internal storage
				   allocator. */

extern MPL_INT bytesAllocated;      /* Number of bytes Maple's storage
				   allocator has requested from the
				   operating system. */

extern MPL_CHAR *stackBaseAddress;  /* The approximate value of the stack
				   pointer at the time the MAPLE or
				   Evaluate functions are invoked. This
				   is set automatically by these
				   functions, and should not usually be
				   accessed by the OEM application. */

extern MPL_INT maxMapleStackSize;   /* The amount of stack space available at
				   the time that MAPLE or Evaluate are
				   invoked. This should be set by the
				   OEM application to about 1000 less
				   than the actual number of bytes of
				   stack space available at the time of
				   the call. */

/* The following are the definitions for the text string interface. Although
   there are some new definitions here since earlier releases, C code written
   to interface with earlier releases should still work as-is. */

/* Line type tags. */
#define M_OUTPUT        0       /* Maple output line */
#define M_TRUNC         1       /* Truncated Maple output line */
#define M_ERROR         2       /* Computation error message */
#define M_SYNTAX        3       /* Syntax error message */
#define M_STATUS        4       /* Status information */
#define M_QUIT          5       /* End of session */
#define M_DEBUG         6       /* Debugging information */
#define M_BLANK         7       /* Blank line in prettyprinted output */
#define M_HELP          8       /* Help system output */

/* Range of valid line type tags. */
#define M_MINSTATUS     0
#define M_MAXSTATUS     8

extern int max_output_line;     /* The longest allowable output line, set
				   to 10000 bytes by default. */

/* The structure returned by a call to the MAPLE() function. */
typedef struct M_Result {
    MPL_INT Number_Lines;           /* Number of lines in the output structure. */
    MPL_INT *Output_Status;         /* Array of line type tags. */
    MPL_CHAR **Output_Lines;        /* Array of pointers to output lines. The
				   actual output of line N starts at address
				   Output_Lines[N]+8, since the first 8 bytes
				   contain Maple internal information. */
} *Maple_Result;

/* The two functions comprising the text string interface. */
extern int maple_initialize();  /* Initialize Maple. */
extern Maple_Result MAPLE();    /* Execute a Maple source statement. */

/* The following are the defintions for the DAG interface. This is new for
   Release 2 on most platforms. Please refer to "Using the C-Callable Maple
   V Kernel" for details. */

#define INSANE (-123456789)     /* Value returned when the sanity check in
				   a DAG interface routine fails. */

extern ALGEB CreateNode();      /* Create a Maple DAG node. */
extern ALGEB ReadNode();        /* Read one word from a node. */
extern MPL_INT WriteNode();         /* Write one word to a node. */
extern ALGEB Evaluate();        /* Evaluate a node. */
extern ALGEB CreateName();      /* Create a name. */

/* These macros extract the type and length of a DAG. Do NOT modify the
   headers of a DAG. Use the functions listed above instead. */
#define M_ID(x) ((int)(*x)&037400000)
#define M_LENGTH(x) ((int)(*x)&0377777)

/* These are the DAG type tags, as used by CreateNode and returned by ID. */
#define M_AND 2359296
#define M_ASSIGN 3801088
#define M_CATENATE 917504
#define M_EQUATION 1835008
#define M_EXPSEQ 2752512
#define M_FLOAT 524288
#define M_FOR 3932160
#define M_FUNCTION 1572864
#define M_HASH 4718592
#define M_HASHTAB 4849664
#define M_IF 4063232
#define M_INEQUAT 1966080
#define M_INTNEG 131072
#define M_INTPOS 262144
#define M_LESSEQ 2097152
#define M_LESSTHAN 2228224
#define M_LIST 2883584
#define M_LOCAL 3014656
#define M_NAME 655360
#define M_NOT 2490368
#define M_OR 2621440
#define M_PARAM 3145728
#define M_POWER 1048576
#define M_PROC 3276800
#define M_PROD 1179648
#define M_RANGE 3407872
#define M_RATIONAL 393216
#define M_READ 4194304
#define M_SAVE 4325376
#define M_SERIES 1310720
#define M_SET 3538944
#define M_STATSEQ 4456448
#define M_STOP 4587520
#define M_SUM 1441792
#define M_TABLE 3670016
#define M_TABLEREF 786432
#define M_UNEVAL 1703936
