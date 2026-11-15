/* $Id$ -*- c -*- 
 * scan.c - HTML scanner for WebSnap.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef FREEBSD
#include <stdlib.h>
#else
#include <alloca.h>
#include <malloc.h>
#endif

#include <tcl.h>

static int Snap_SubstCmd(dummy, interp, argc, argv)
ClientData   dummy;
Tcl_Interp  *interp;
int          argc;
char       **argv;
{
  int c;
  char *input = NULL;
  short done = 0;
  enum {
    text=0,
    inTag,
    inTagBody,
    inTagParamSeparation,
    inTagParamBody,
    inTagParamValueSep,
    inQuotedTagValue,
    inQuotedTagValueMaybeTcl,
    inQuotedTagValueMaybeTclEnd,
    inQuotedTagValueTCLCode,
    inQuotedTagValueTCLVariable,
    inQuotedTagAfterValue,
    inTCLCode,
    inTCLVarReference,
    inMaybeTclEnd
  } state;
  enum {
    afterNewline=0,
    afterRandom
  } tclState;
  char tclAppend[2];
  Tcl_DString cmdBuf;
  Tcl_DString result;
  int dousage = 0;
  int argno;
  int nested_TCL_delimiters = 0;

  for (argno = 1; argno < argc; argno++) {
    if (argv[argno][0] != '-')
      input = &(argv[argno][0]);
    else {
#if 0
      if (strcmp(argv[argno], "-set") == 0) {
	char *varname = argv[++argno];
	char *value   = argv[++argno];

	if (!varname || !value || argno >= argc) {
	  dousage = 1;
	}
	else {
	  Tcl_SetVar(interp, varname, value, 0);
	}
      }
#endif
    }
  }

  if (dousage || !input) {
    Tcl_SetResult(interp, "USAGE: snap_subst string", TCL_STATIC);
    return TCL_ERROR;
  }

  tclAppend[0] = '\0';
  tclAppend[1] = '\0';
  Tcl_DStringInit(&cmdBuf);
  Tcl_DStringInit(&result);

  state = text;
  tclState = afterNewline;

  if (input[0] == '#' && input[1] == '!') {
    while (input[0] != '\n')
      input++;
    if (input[0] == '\n')
      input++;
  }

  for (c = (input++)[0]; !done ; c = (input++)[0]) {
    if (c == '\0') {
      done = 1;
      continue;
    }

    /*
     * Special case: Backslash handling that's going into 'result'
     * This section is transporting the HTML into 'result'.  Whether
     * we do any translation is dependant on specifics of HTML.  From
     * what I remember there isn't need for translation.
     */
    if (1 == 0) {
      char bf[5];
      unsigned long val;
    handle_backslash_result:

#if 0
      /*
       * This version does translation.  That is the following is recognized:
       *
       *  \xXX		Two HEX digits
       *  \###		Three OCTAL digits
       *  \c		One character
       *	\a    Audible alert (bell) (0x7).
       *        \b    Backspace (0x8).
       *        \f    Form feed (0xc).
       *        \n    Newline (0xa).
       *        \r    Carriage-return (0xd).
       *        \t    Tab (0x9).
       *        \v    Vertical tab (0xb).
       *        \<newline>whiteSpace
       */

      c = (input++)[0];
      switch (c) {
      case 'x':  /* hex */
	memset(bf, 0, sizeof bf);
	c = (input++)[0];   /* dig1 */
	if (c == '\0') { done = 1; continue; }
	bf[0] = c;
	c = (input++)[0];   /* dig2 */
	if (c == '\0') { done = 1; continue; }
	bf[1] = c;
	val = strtoul(bf, (char **)NULL, 16);
	c = val & 0xff;
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
	
      case '0':  /* octal */
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
	bf[0] = c;  /* dig1 */
	c = (input++)[0];
	if (c == '\0') { done = 1; continue; }
	bf[1] = c;  /* dig2 */
	c = (input++)[0];
	if (c == '\0') { done = 1; continue; }
	bf[2] = c;  /* dig3 */
	val = strtoul(bf, (char **)NULL, 8);
	c = val & 0xff;
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;

      default:
	switch (c) {
	case 'a': c = 0x07; break;
	case 'b': c = 0x08; break;
	case 'f': c = 0x0c; break;
	case 'n': c = ' ';  break;
	case 'r': c = 0x0d; break;
	case 't': c = 0x09; break;
	case 'v': c = 0x0b; break;
	}
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1); /* char */
      }
      continue;
#else
      /*
       * This version does no translation and is useful for skipping
       * over backslashed text.
       */
      tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);  /* Append '\' */
      c = (input++)[0];
      switch (c) {
      case 'x':  /* hex */
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1); /* x */
	c = (input++)[0];
	if (c == '\0') { done = 1; continue; }
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1); /* dig1 */
	c = (input++)[0];
	if (c == '\0') { done = 1; continue; }
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1); /* dig2 */
	break;
	
      case '0':  /* octal */
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1); /* dig1 */
	c = (input++)[0];
	if (c == '\0') { done = 1; continue; }
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1); /* dig2 */
	c = (input++)[0];
	if (c == '\0') { done = 1; continue; }
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1); /* dig3 */
	break;

      default:
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1); /* char */
      }
      continue;
#endif
    }

    /*
     * Special case: Backslash handling that's going into 'cmdBuf'
     * This is for TCL code.
     */
    if (1 == 0) {
      char bf[5];
      unsigned long val;
    handle_backslash_cmdbuf:

#if 0
      /*
       * This version does translation.  That is the following is recognized:
       *
       *  \xXX		Two HEX digits
       *  \###		Three OCTAL digits
       *  \c		One character
       *	\a    Audible alert (bell) (0x7).
       *        \b    Backspace (0x8).
       *        \f    Form feed (0xc).
       *        \n    Newline (0xa).
       *        \r    Carriage-return (0xd).
       *        \t    Tab (0x9).
       *        \v    Vertical tab (0xb).
       *        \<newline>whiteSpace
       */

      c = (input++)[0];
      switch (c) {
      case 'x':  /* hex */
	memset(bf, 0, sizeof bf);
	c = (input++)[0];   /* dig1 */
	if (c == '\0') { done = 1; continue; }
	bf[0] = c;
	c = (input++)[0];   /* dig2 */
	if (c == '\0') { done = 1; continue; }
	bf[1] = c;
	val = strtoul(bf, (char **)NULL, 16);
	c = val & 0xff;
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1);
	break;
	
      case '0':  /* octal */
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
	bf[0] = c;  /* dig1 */
	c = (input++)[0];
	if (c == '\0') { done = 1; continue; }
	bf[1] = c;  /* dig2 */
	c = (input++)[0];
	if (c == '\0') { done = 1; continue; }
	bf[2] = c;  /* dig3 */
	val = strtoul(bf, (char **)NULL, 8);
	c = val & 0xff;
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1);
	break;

      default:
	switch (c) {
	case 'a': c = 0x07; break;
	case 'b': c = 0x08; break;
	case 'f': c = 0x0c; break;
	case 'n': c = ' ';  break;
	case 'r': c = 0x0d; break;
	case 't': c = 0x09; break;
	case 'v': c = 0x0b; break;
	}
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1); /* char */
      }
      continue;
#else
      /*
       * This version does no translation and is useful for skipping
       * over backslashed text.
       */
      tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1);  /* Append '\' */
      c = (input++)[0];
      switch (c) {
      case 'x':  /* hex */
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1); /* x */
	c = (input++)[0];
	if (c == '\0') { done = 1; continue; }
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1); /* dig1 */
	c = (input++)[0];
	if (c == '\0') { done = 1; continue; }
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1); /* dig2 */
	break;
	
      case '0':  /* octal */
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1); /* dig1 */
	c = (input++)[0];
	if (c == '\0') { done = 1; continue; }
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1); /* dig2 */
	c = (input++)[0];
	if (c == '\0') { done = 1; continue; }
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1); /* dig3 */
	break;

      default:
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1); /* char */
      }
      continue;
#endif
    }

    switch (state) {
    case text:
      switch (c) {
      case '<':
	state = inTag;
	continue;
      case '\\':
	goto handle_backslash_result;
      default:
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      }
      break;
      
    case inTag:
      switch (c) {
      case '{':
	state = inTCLCode;
	tclState = afterRandom;
	Tcl_DStringInit(&cmdBuf);
	break;
      case '$':
	state = inTCLVarReference;
	Tcl_DStringInit(&cmdBuf);
	break;
      case '\\':
	goto handle_backslash_result;
      default:
	state = inTagBody;
	tclAppend[0] = '<';  Tcl_DStringAppend(&result, tclAppend, -1);
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      }
      break;

    case inTCLVarReference:
      switch (c) {
      case '>': {
	char *val;
	state = text;
	val = Tcl_GetVar(interp, Tcl_DStringValue(&cmdBuf),
			 TCL_LEAVE_ERR_MSG);
	if (!val) {
	  Tcl_DStringAppend(&result,
			    "<P><B><PRE>An error occurred in processing.  "
			    "Could not access value for variable \"",
			    -1);
	  Tcl_DStringAppend(&result, Tcl_DStringValue(&cmdBuf), -1);
	  Tcl_DStringAppend(&result, "\".</PRE></b>\n", -1);
	}
	else {
	  Tcl_DStringAppend(&result, val, -1);
	}
	break;
      }
	
      case '\\':
	goto handle_backslash_cmdbuf;
      default:
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1);
	break;
      }
      break;
      
    case inTagBody:
      switch (c) {
      case '>':
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	state = text;
	break;
      case '\\':
	goto handle_backslash_result;
      case ' ':
      case '\t':
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	state = inTagParamSeparation;
	break;
      default:
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      }
      break;


    case inTagParamSeparation:
      switch (c) {
      case ' ':
      case '\t':
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      case '>':
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	state = text;
	break;
      case '\\':
	goto handle_backslash_result;
      default:
	state = inTagParamBody;
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      }
      break;

    case inTagParamBody:
      switch (c) {
      case ' ':
      case '\t':
	state = inTagParamSeparation;
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      case '>':
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	state = text;
	break;
      case '=':
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	state = inTagParamValueSep;
	break;
      case '\\':
	goto handle_backslash_result;
      default:
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      }
      break;

    case inTagParamValueSep:
      switch (c) {
      case ' ':
      case '\t':   /* Actually an error */
	state = inTagParamSeparation;
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      case '>':   /* Actually an error */
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	state = text;
	break;
      case '"':
	state = inQuotedTagValue;
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      case '\\':
	goto handle_backslash_result;
      default:
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      }
      break;

    case inQuotedTagValue:
      switch (c) {
      case '<':
	state = inQuotedTagValueMaybeTcl;
	break;
      case '\\':
	goto handle_backslash_result;
      case '"':
	state = inQuotedTagAfterValue;
	/* fall through */
      default:
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
      }
      break;

    case inQuotedTagValueMaybeTcl:
      switch (c) {
      case '{':
	state = inQuotedTagValueTCLCode;
	Tcl_DStringInit(&cmdBuf);
	break;
      case '$':
	state = inQuotedTagValueTCLVariable;
	Tcl_DStringInit(&cmdBuf);
	break;
      case '\\':
	goto handle_backslash_result;
      default:
	state = inQuotedTagValue;
	tclAppend[0] = '{';  Tcl_DStringAppend(&result, tclAppend, -1);
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      }
      break;

    case inQuotedTagValueTCLVariable:
      switch (c) {
      case '>': {
	char *val;
	state = inQuotedTagValue;
	val = Tcl_GetVar(interp, Tcl_DStringValue(&cmdBuf),
			 TCL_LEAVE_ERR_MSG);
	if (!val) {
	  Tcl_DStringAppend(&result,
			    "ERROR:  "
			    "Could not access value for variable \"",
			    -1);
	  Tcl_DStringAppend(&result, Tcl_DStringValue(&cmdBuf), -1);
	  Tcl_DStringAppend(&result, "\".", -1);
	}
	else {
	  Tcl_DStringAppend(&result, val, -1);
	}
	break;
      }
	
      case '\\':
	goto handle_backslash_cmdbuf;
      default:
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1);
	break;
      }
      break;

    case inQuotedTagValueTCLCode:
      switch (c) {
      case '}':
	state = inQuotedTagValueMaybeTclEnd;
	break;
      case '\\':
	goto handle_backslash_cmdbuf;
      case '<':
	if (input[0] == '{') {
	  nested_TCL_delimiters++;
	}
	/* fall through */
      default:
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1);
	break;
      }
      break;

    case inQuotedTagValueMaybeTclEnd:
      switch (c) {
      case '>': {

	nested_TCL_delimiters--;
	if (nested_TCL_delimiters >= 0) {
	  state = inTCLCode;
	  Tcl_DStringAppend(&cmdBuf, "}>", -1);
	  continue;
	}

	state = inQuotedTagValue;
	/*printf("subst eval %s\n", Tcl_DStringValue(&cmdBuf));*/
	switch (Tcl_Eval(interp, Tcl_DStringValue(&cmdBuf))) {
	case TCL_OK:
	case TCL_RETURN:
	  Tcl_DStringAppend(&result, interp->result, -1);
	  break;
	default:
	  /*printf("<P><B><PRE>An error occurred in processing.  TCL Code is %s\n",
	   * Tcl_GetVar(interp, "errorCode", TCL_GLOBAL_ONLY));
	   *printf("%s\n", Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY));*/
	  Tcl_DStringAppend(&result, "ERROR: ", -1);
	  Tcl_DStringAppend(&result, Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY), -1);
	  break;
	}
	Tcl_DStringInit(&cmdBuf);
	break;
      }
      case '\\':
	Tcl_DStringAppend(&cmdBuf, "}", -1);
	state = inQuotedTagValueTCLCode;
	goto handle_backslash_cmdbuf;
      default:
	Tcl_DStringAppend(&cmdBuf, "}", -1);
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1);
	state = inQuotedTagValueTCLCode;
	break;
      }
      break;

    case inQuotedTagAfterValue:
      switch (c) {
      case ' ':
      case '\t':
	state = inTagParamSeparation;
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      case '>':
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	state = text;
	break;
      case '\\':
	goto handle_backslash_result;
      default:
	state = inTagParamBody;
	tclAppend[0] = c;    Tcl_DStringAppend(&result, tclAppend, -1);
	break;
      }
      break;

    case inTCLCode:
      switch (c) {
      case '}':
	state = inMaybeTclEnd;
	break;
      case '\\':
	goto handle_backslash_cmdbuf;
      case '<':
	if (input[0] == '{') {
	  nested_TCL_delimiters++;
	}
	/* fall through */
      default:
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1);
	break;
      }
      break;

    case inMaybeTclEnd:
      switch (c) {
      case '>': {

	nested_TCL_delimiters--;
	if (nested_TCL_delimiters >= 0) {
	  state = inTCLCode;
	  Tcl_DStringAppend(&cmdBuf, "}>", -1);
	  continue;
	}

	state = text;
	/*printf("subst eval %s\n", Tcl_DStringValue(&cmdBuf));*/
	switch (Tcl_Eval(interp, Tcl_DStringValue(&cmdBuf))) {
	case TCL_OK:
	case TCL_RETURN:
	  Tcl_DStringAppend(&result, interp->result, -1);
	  break;
	default:
	  Tcl_DStringAppend(&result,
			    "<P><B><PRE>An error occurred in processing.  TCL Code is ",
			    -1);
	  Tcl_DStringAppend(&result,
			    Tcl_GetVar(interp, "errorCode", TCL_GLOBAL_ONLY),
			    -1);
	  Tcl_DStringAppend(&result, "\n<BR>", -1);
	  Tcl_DStringAppend(&result,
			    Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY),
			    -1);
	  Tcl_DStringAppend(&result, "\n</PRE></b>\n", -1);

	  /*printf("<P><B><PRE>An error occurred in processing.  TCL Code is %s\n",
	   * Tcl_GetVar(interp, "errorCode", TCL_GLOBAL_ONLY));
	   * printf("%s\n", Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY));*/
	  break;
	}
	Tcl_DStringInit(&cmdBuf);
	break;
      }
      case '\\':
	Tcl_DStringAppend(&cmdBuf, "}", -1);
	state = inTCLCode;
	goto handle_backslash_cmdbuf;
      default:
	Tcl_DStringAppend(&cmdBuf, "}", -1);
	tclAppend[0] = c;    Tcl_DStringAppend(&cmdBuf, tclAppend, -1);
	state = inTCLCode;
	break;
      }
      break;

    default:
      break;
    }
  }

  Tcl_SetResult(interp, Tcl_DStringValue(&result), TCL_VOLATILE);

  Tcl_DStringFree(&cmdBuf);
  Tcl_DStringFree(&result);

  return TCL_OK;
}

typedef struct {
  Tcl_DString name;
  int         hasDefault;
  Tcl_DString defVal;
} _snap_arg;

typedef struct {
  Tcl_Interp  *interp;
  Tcl_DString  name;
  _snap_arg    *arglist;  /* Array of 'numargs' elements */
  int          numargs;
  Tcl_DString  body;
} _snap_template;

static int Snap_TemplateDoCmd(dummy, interp, argc, argv)
ClientData   dummy;
Tcl_Interp  *interp;
int          argc;
char       **argv;
{
  _snap_template *tmpl = (_snap_template *)dummy;
  char           *body = Tcl_DStringValue(&(tmpl->body));
  int             paramNum;
  int             argNum;
  int             passed_argc = 2;
  char           *passed_argv[3];
  Tcl_CallFrame   frame;
  int             tcl_result = TCL_OK;
  Tcl_CmdInfo     cmdInfo;

  memset(&cmdInfo, 0, sizeof cmdInfo);
  Tcl_GetCommandInfo(interp, argv[0], &cmdInfo);

  memset(&frame, 0, sizeof frame);
  Tcl_PushCallFrame(interp, &frame, cmdInfo.namespacePtr, 1);

  argNum = 1;  /* skip first arg - function name */
  for (paramNum = 0; paramNum < tmpl->numargs; paramNum++) {
    _snap_arg *argdesc = &(tmpl->arglist[paramNum]);
    if (argNum < argc) {
      if (Tcl_SetVar(interp, Tcl_DStringValue(&(argdesc->name)), argv[argNum++], 0) == NULL) {
	Tcl_AppendResult(interp, "Could not set parameter '",
			 Tcl_DStringValue(&(argdesc->name)),
			 "' to value '", argv[argNum++],
			 "' in template '",
			 argv[0], "'",  0);
	tcl_result = TCL_ERROR;
	goto leave;
      }
    }
    else {
      if (argdesc->hasDefault) {
	if (Tcl_SetVar(interp,
		       Tcl_DStringValue(&(argdesc->name)),
		       Tcl_DStringValue(&(argdesc->defVal)),
		       0) == NULL) {
	  Tcl_AppendResult(interp,
			   "Could not set parameter '",
			   Tcl_DStringValue(&(argdesc->name)),
			   "' to default value '",
			   Tcl_DStringValue(&(argdesc->defVal)),
			   "' in template '",
			   argv[0], "'", 
			   0);
	  tcl_result = TCL_ERROR;
	  goto leave;
	}
      }
      else {
	Tcl_AppendResult(interp, "No value provided for parameter '",
			 Tcl_DStringValue(&(argdesc->name)),
			 "' in template '",
			 argv[0], "'", 0);
	tcl_result = TCL_ERROR;
	goto leave;
      }
    }
  }

  if (argNum < argc) {
    Tcl_AppendResult(interp, "Too many arguments provided for template ", argv[0], 0);
    tcl_result = TCL_ERROR;
    goto leave;
  }

  passed_argv[0] = "snap_subst";
  passed_argv[1] = body;

  tcl_result = Snap_SubstCmd(dummy, interp, passed_argc, passed_argv);

leave:
  Tcl_PopCallFrame(interp);

  return tcl_result;
}

static void Snap_TemplateDeleteCmd(dummy)
ClientData   dummy;
{
  _snap_template *tmpl = (_snap_template *)dummy;
  int i;

  Tcl_DStringFree(&(tmpl->name));

  for (i = 0; i < tmpl->numargs; i++) {
    Tcl_DStringFree(&(tmpl->arglist[i].name));
    if (tmpl->arglist[i].hasDefault)
      Tcl_DStringFree(&(tmpl->arglist[i].defVal));
  }
  free(tmpl->arglist);

  Tcl_DStringFree(&(tmpl->body));
  free(tmpl);

  return;
}

static int Snap_TemplateCmd(dummy, interp, argc, argv)
ClientData   dummy;
Tcl_Interp  *interp;
int          argc;
char       **argv;
{
  _snap_template *tmpl = NULL;
  int             i;
  int             nParams;
  char          **lParams;
  int             res;

  if (argc != 4) {
    Tcl_SetResult(interp, "USAGE: snap_template name arg-list body", TCL_STATIC);
    return TCL_ERROR;
  }

  tmpl = calloc(1, sizeof (_snap_template));
  if (!tmpl) {
    Tcl_AppendResult(interp, "Out of memory", 0);
    return TCL_ERROR;
  }

  tmpl->interp = interp;
  Tcl_DStringInit(&(tmpl->name));
  Tcl_DStringAppend(&(tmpl->name), argv[1], -1);

  /*  Tcl_DStringInit(&(tmpl->arglist));
   *Tcl_DStringAppend(&(tmpl->arglist), argv[2], -1);*/
  res = Tcl_SplitList(interp, argv[2], &nParams, &lParams);
  if (res != TCL_OK) {
    Tcl_AppendResult(interp,
		     "Bad argument specification '",
		     argv[2], "' list in template '", argv[1], "'.", 0);

  procError:
    if (lParams) Tcl_Free((char *)lParams);
    Tcl_DStringFree(&(tmpl->name));
    Tcl_DStringFree(&(tmpl->body));
    free(tmpl);
    return TCL_ERROR;
  }

  tmpl->arglist = calloc(nParams+1, sizeof(_snap_arg));
  if (!tmpl->arglist) {
    Tcl_AppendResult(interp, "Out of memory", 0);
    goto procError;
  }
    
  tmpl->numargs = nParams;

  for (i = 0; i < nParams; i++) {

    int    fieldCount;
    char **fieldValues;
    int    res;

    res = Tcl_SplitList(interp, lParams[i], &fieldCount, &fieldValues);

    Tcl_DStringInit(&(tmpl->arglist[i].name));
    Tcl_DStringAppend(&(tmpl->arglist[i].name), fieldValues[0], -1);
    switch (fieldCount) {
    case 2:
      tmpl->arglist[i].hasDefault = 1;
      Tcl_DStringInit(&(tmpl->arglist[i].defVal));
      Tcl_DStringAppend(&(tmpl->arglist[i].defVal), fieldValues[1], -1);
      break;
    case 1:
      tmpl->arglist[i].hasDefault = 0;
      break;
    default:
      Tcl_AppendResult(interp, "Badly formed argument specification '",
		       lParams[i], "' in template '", argv[1], "'.",
		       "  Argument list '", argv[2], "'.", 0);
      goto procError;
    }

    Tcl_Free((char *)fieldValues);
  }
  Tcl_Free((char *)lParams);

  Tcl_DStringInit(&(tmpl->body));
  Tcl_DStringAppend(&(tmpl->body), argv[3], -1);

  (void)Tcl_CreateCommand(interp, argv[1], Snap_TemplateDoCmd, tmpl, Snap_TemplateDeleteCmd);

  return TCL_OK;
}

static int Snap_IncludeCmd(dummy, interp, argc, argv)
ClientData   dummy;
Tcl_Interp  *interp;
int          argc;
char       **argv;
{
  int passed_argc = 2;
  char *passed_argv[3];
  int fd;
  struct stat stbuf;
  int stres;
  char *file;

  if (argc != 2) {
    Tcl_SetResult(interp, "USAGE: snap_include file", TCL_STATIC);
    return TCL_ERROR;
  }

  file = argv[1];

  stres = stat(file, &stbuf);
  if (stres < 0) {
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, "File ", file, " does not exist.", 0);
    return TCL_ERROR;
  }
  passed_argv[0] = "snap_subst";
  passed_argv[1] = alloca(stbuf.st_size + 10);
  memset(passed_argv[1], 0, stbuf.st_size + 10);
  fd = open(file, O_RDONLY);
  if (fd < 0) {
    Tcl_AppendResult(interp, "File ", file, " cannot be opened for reading.", 0);
    return TCL_ERROR;
  }
  if (read(fd, passed_argv[1], stbuf.st_size) != stbuf.st_size) {
    Tcl_AppendResult(interp, "File ", file, " cannot be read.", 0);
    return TCL_ERROR;
  }
  close(fd);
  
  return Snap_SubstCmd(dummy, interp, passed_argc, passed_argv);
}

static Tcl_DString header;
static int         header_init = 0;
static int         saw_content_type = 0;

void Snap_HeaderInit()
{
  if (!header_init) {
    Tcl_DStringInit(&header);
    header_init = 1;
  }
}

static int Snap_HeaderCmd(dummy, interp, argc, argv)
ClientData   dummy;
Tcl_Interp  *interp;
int          argc;
char       **argv;
{
  int argno;

  if (argc < 2) {
    Tcl_AppendResult(interp, "USAGE: snap header tag val ?val? ...", 0);
    return TCL_ERROR;
  }

  Snap_HeaderInit();

  Tcl_DStringAppend(&header, argv[1], -1);
  Tcl_DStringAppend(&header, ": ", -1);
  Tcl_DStringAppend(&header, argv[2], -1);
  Tcl_DStringAppend(&header, "\n", -1);
  for (argno = 3; argno < argc; argno++) {
    Tcl_DStringAppend(&header, "        ", -1);
    Tcl_DStringAppend(&header, argv[argno], -1);
    Tcl_DStringAppend(&header, "\n", -1);
  }

  return TCL_OK;
}

void Snap_HeaderMinimum()
{
  Snap_HeaderInit();
  if (!saw_content_type) {
    Tcl_DStringAppend(&header, "Content-Type: text/html\n", -1);
    saw_content_type = 1;
  }
}

char *Snap_HeaderTxt()
{
  Snap_HeaderInit();
  return Tcl_DStringValue(&header);
}

static int Snap_HeaderTextCmd(dummy, interp, argc, argv)
ClientData   dummy;
Tcl_Interp  *interp;
int          argc;
char       **argv;
{
  int argno;

  if (argc < 2) {
    Tcl_AppendResult(interp, "USAGE: snap headertext", 0);
    return TCL_ERROR;
  }

  Tcl_SetResult(interp, Snap_HeaderTxt(), TCL_VOLATILE);
  return TCL_OK;
}

static int Snap_MainCmd(dummy, interp, argc, argv)
ClientData   dummy;
Tcl_Interp  *interp;
int          argc;
char       **argv;
{
  int passed_argc;
  char **passed_argv;
  int argno;
  char *whichCmd = argv[1];

  passed_argv = alloca((argc + 1) * sizeof (char *));
  memset(passed_argv, 0, (argc + 1) * sizeof (char *));

  for (passed_argc = 0, argno = 0; argno < argc; argno++) {
    if (argno != 1) {
      passed_argv[passed_argc] = argv[argno];
      passed_argc++;
    }
  }
  
  if (!argv[1]) {
    Tcl_AppendResult(interp, "USAGE: snap cmd arg ?arg?", 0);
    return TCL_ERROR;
  }
  else if (strcmp(argv[1], "body") == 0) {
    return Snap_SubstCmd(dummy, interp, passed_argc, passed_argv);
  }
  else if (strcmp(argv[1], "include") == 0) {
    return Snap_IncludeCmd(dummy, interp, passed_argc, passed_argv);
  }
  else if (strcmp(argv[1], "template") == 0) {
    return Snap_TemplateCmd(dummy, interp, passed_argc, passed_argv);
  }
  else if (strcmp(argv[1], "header") == 0) {
    return Snap_HeaderCmd(dummy, interp, passed_argc, passed_argv);
  }
  else if (strcmp(argv[1], "headertext") == 0) {
    return Snap_HeaderTextCmd(dummy, interp, passed_argc, passed_argv);
  }
  else {
    Tcl_AppendResult(interp, "Unknown command ", argv[1], 0);
    return TCL_ERROR;
  }
}



int Snap_Init(interp)
Tcl_Interp *interp;
{
  Tcl_CreateCommand(interp, "snap_subst", Snap_SubstCmd, (ClientData) 0,
		    (Tcl_CmdDeleteProc *)NULL);
  Tcl_CreateCommand(interp, "snap_include", Snap_IncludeCmd, (ClientData) 0,
		    (Tcl_CmdDeleteProc *)NULL);
  Tcl_CreateCommand(interp, "snap_template", Snap_TemplateCmd, (ClientData) 0,
		    (Tcl_CmdDeleteProc *)NULL);
  Tcl_CreateCommand(interp, "snap", Snap_MainCmd, (ClientData) 0,
		    (Tcl_CmdDeleteProc *)NULL);
  return TCL_OK;
}
