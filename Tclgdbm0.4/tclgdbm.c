/*
 * $Id: gdbmcmd.c,v 1.8 1994/05/02 06:52:09 lindig Exp $
 */

#ifdef MALLOC_FUNC_CHECK
#include <malloc_dbg.h>
#endif

#include <stdio.h>
#include <tcl.h>
#include <gdbm.h>

/*
 * prototypes for library functions
 */

EXTERN VOID free _ANSI_ARGS_((VOID *));
EXTERN VOID perror _ANSI_ARGS_((const char *));

#ifndef MALLOC_FUNC_CHECK
EXTERN VOID *malloc _ANSI_ARGS_((long unsigned int ));
EXTERN VOID *realloc _ANSI_ARGS_((VOID *, long unsigned int ));
#endif

/*
 * DEFINES for error reporting
 */

#define ARGS1		"%s: \"%s\": error: %s"
#define ARGS2   	"%s: \"%s\" \"%s\": error: %s"
#define ARGS3		"%s: \"%s\" \"%s\" \"%s\": error: %s"

#define BAD_MODE 	"%s: bad mode \"%s\""

#define OPEN_USAGE 	"gdbm open <name> ?mode?"
#define CLOSE_USAGE 	"gdbm close <name>"
#define INSERT_USAGE 	"gdbm insert <name> <key> <content>"
#define STORE_USAGE 	"gdbm store <name> <key> <content>"
#define FETCH_USAGE 	"gdbm fetch <name> <key>"
#define EXISTS_USAGE 	"gdbm exists <name> <key>"
#define DELETE_USAGE 	"gdbm delete <name> <key>"
#define LIST_USAGE 	"gdbm list <name>"
#define REORG_USAGE	"gdbm reorganize <name>"
#define GDBM_USAGE	"gdbm <command> <name> ?parameter? ..."
#define FIRST_USAGE	"gdbm firstkey <name>"
#define NEXT_USAGE	"gdbm nextkey <name> <lastkey>"

/*
 * a database ..
 */

typedef struct db {
	char *name ;
	int mode ;
	GDBM_FILE db ;
} DB ;

/*
 * all open databases
 */

Tcl_HashTable databases ;

/*
 * counter for handles
 */

static handle ;

/*
 * prototypes for internal functions
 */

static int tcl_gdbm_close _ANSI_ARGS_((ClientData client, Tcl_Interp *interp, 
		   int argc, char **argv));
static int tcl_gdbm_insert _ANSI_ARGS_((ClientData client, Tcl_Interp *interp, 
		    int argc, char **argv));
static int tcl_gdbm_store _ANSI_ARGS_((ClientData client, Tcl_Interp *interp,
		   int argc, char **argv));
static int tcl_gdbm_reorg  _ANSI_ARGS_((ClientData client, Tcl_Interp *interp,
		   int argc, char **argv));
static int tcl_gdbm_fetch _ANSI_ARGS_((ClientData client, Tcl_Interp *interp,
		   int argc, char **argv));
static int tcl_gdbm_exists _ANSI_ARGS_((ClientData client, Tcl_Interp *interp,
		   int argc, char **argv));
static int tcl_gdbm_delete _ANSI_ARGS_((ClientData client, Tcl_Interp *interp,
		    int argc, char **argv));
static int tcl_gdbm_list _ANSI_ARGS_((ClientData client, Tcl_Interp *interp,
		  int argc, char **argv));
static int tcl_gdbm_open _ANSI_ARGS_((ClientData client, Tcl_Interp *interp,
		  int argc, char **argv));
static int tcl_gdbm_firstkey _ANSI_ARGS_((ClientData client, 
					  Tcl_Interp *interp, 
					  int argc, char **argv));
static int tcl_gdbm_nextkey _ANSI_ARGS_((ClientData client, 
					 Tcl_Interp *interp, 
					 int argc, char **argv));

/* ****************************************************************** */

/*
 * tcl_gdbm_close
 */

static int 
tcl_gdbm_close (client, interp, argc, argv)
	ClientData client;
	Tcl_Interp *interp;
	int argc;
	char **argv;

{
	DB *db ;
	Tcl_HashEntry *entry ;
	char buf[512] ;

	/*
	 * check argument number
	 */
	if (argc != 2) {
		Tcl_AppendResult (interp,CLOSE_USAGE,(char*)0);
		return(TCL_ERROR);
	}
	
	entry = Tcl_FindHashEntry(&databases,argv[1]);

	if (!entry) {
		/*
		 * db not in hashtab
		 */
		sprintf(buf,ARGS1,argv[0],argv[1],"no such database");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}

	/*
	 * retrieve db from hash table
	 */

	db = (DB*) Tcl_GetHashValue (entry);

	/*
	 * close db and remove it from tables
	 */

	gdbm_close (db->db) ;

	free (db->name);
	free ((char*)db);
	Tcl_DeleteHashEntry(entry);

	/*
	 * ok
	 */

	return (TCL_OK);

}

/* ****************************************************************** */

/*
 * tcl_gdbm_insert <name> <key> <content>
 */

static int 
tcl_gdbm_insert (client, interp, argc, argv)
	ClientData client;
	Tcl_Interp *interp;
	int argc;
	char **argv;

{
	char buf[512] ;
	datum key, content ;
	int ret ;
	const char *error ;
	DB *db ;
	Tcl_HashEntry *entry ;

	/*
	 * check argument number
	 */
	if (argc != 4) {
		Tcl_AppendResult (interp,INSERT_USAGE,(char*)0);
		return(TCL_ERROR);
	}
	
	entry = Tcl_FindHashEntry(&databases,argv[1]);

	if (!entry) {
		/*
		 * db not in hashtab
		 */
		sprintf(buf,ARGS1,argv[0],argv[1],"no such database");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}

	/*
	 * retrieve db
	 */

	db = (DB*) Tcl_GetHashValue (entry);

	/* prepare data and store it 
	 * if the key is allready stored in db 
	 * an error will be reported
	 */

	key.dptr = argv[2];
	key.dsize = strlen(argv[2]) + 1 ;

	content.dptr = argv[3];
	content.dsize = strlen(argv[3]) + 1 ;

	ret = gdbm_store(db->db, key, content, GDBM_INSERT);
	if (ret != 0) {
		error = gdbm_strerror(gdbm_errno);
		sprintf(buf, ARGS3,argv[0],argv[1],argv[2],argv[3],error);
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}

	return TCL_OK;
}

/* ****************************************************************** */

/*
 * tcl_gdbm_store <name> <key> <content>
 */

static int 
tcl_gdbm_store (client, interp, argc, argv)
	ClientData client;
	Tcl_Interp *interp;
	int argc;
	char **argv;

{
	char buf[512] ;
	datum key, content ;
	int ret ;
	const char *error ;
	DB *db ;
	Tcl_HashEntry *entry ;

	/*
	 * check argument number
	 */
	if (argc != 4) {
		Tcl_AppendResult (interp,STORE_USAGE,(char*)0);
		return(TCL_ERROR);
	}
	
	entry = Tcl_FindHashEntry(&databases,argv[1]);

	if (!entry) {
		/*
		 * db not in hashtab
		 */
		sprintf(buf,ARGS1,argv[0],argv[1],"no such database");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}

	/*
	 * retrieve db
	 */

	db = (DB*) Tcl_GetHashValue (entry);

	/* prepare data and store it 
	 * if the key is allready stored in db the new contents will
	 * will replace the old
	 */

	key.dptr = argv[2];
	key.dsize = strlen(argv[2]) + 1 ;

	content.dptr = argv[3];
	content.dsize = strlen(argv[3]) + 1 ;

	ret = gdbm_store(db->db, key, content, GDBM_REPLACE);
	if (ret != 0) {
		error = gdbm_strerror(gdbm_errno);
		sprintf(buf, ARGS3,argv[0],argv[1],argv[2],argv[3],error);
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}

	return TCL_OK;
}

/* ****************************************************************** */

/*
 * tcl_gdbm_reorg <name>
 */

static int 
tcl_gdbm_reorg (client, interp, argc, argv)
	ClientData client;
	Tcl_Interp *interp;
	int argc;
	char **argv;

{
	int ret ;
	char buf[512] ;
	DB *db ;
	Tcl_HashEntry *entry ;

	/*
	 * check argument number
	 */
	if (argc != 2) {
		Tcl_AppendResult (interp,REORG_USAGE,(char*)0);
		return(TCL_ERROR);
	}
	
	entry = Tcl_FindHashEntry(&databases,argv[1]);

	if (!entry) {
		/*
		 * db not in hashtab
		 */
		sprintf(buf,ARGS1,argv[0],argv[1],"no such database");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}

	/*
	 * retrieve db
	 */

	db = (DB*) Tcl_GetHashValue(entry);
	ret = gdbm_reorganize(db->db) ;
	
	if (ret != 0) {
		/* 
		 * error
		 */
		sprintf(buf,ARGS1,argv[0],argv[1],"can't reorganize");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}

	return TCL_OK ;
}


/* ****************************************************************** */

/*
 * tcl_gdbm_fetch <name> <key>
 */

static int 
tcl_gdbm_fetch (client, interp, argc, argv)
	ClientData client;
	Tcl_Interp *interp;
	int argc;
	char **argv;

{
	DB *db ;
	Tcl_HashEntry *entry ;
	datum key, content ;
	char buf[512] ;

	/*
	 * check argument number
	 */
	if (argc != 3) {
		Tcl_AppendResult (interp,FETCH_USAGE,(char*)0);
		return(TCL_ERROR);
	}
	
	entry = Tcl_FindHashEntry(&databases,argv[1]);

	if (!entry) {
		/*
		 * db not in hashtab
		 */
		sprintf(buf,ARGS1,argv[0],argv[1],"no such database");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}
	
	db = (DB*) Tcl_GetHashValue(entry);

	key.dptr = argv[2];
	key.dsize = strlen(argv[2]) + 1 ;

	content = gdbm_fetch (db->db,key) ;
	
	if (content.dptr) {
		/*
		 * found
		 */
		Tcl_AppendResult (interp, content.dptr, (char*)0);
		free(content.dptr);
		return TCL_OK ;
	}
	
	Tcl_AppendResult (interp, "key not found", (char*)0);
	return TCL_ERROR ;
}

/*
 * tcl_gdbm_exists <name> <key>
 */

static int 
tcl_gdbm_exists (client, interp, argc, argv)
	ClientData client;
	Tcl_Interp *interp;
	int argc;
	char **argv;

{
	DB *db ;
	Tcl_HashEntry *entry ;
	datum key ;
	int exists ;
	char buf[512] ;

	/*
	 * check argument number
	 */
	if (argc != 3) {
		Tcl_AppendResult (interp,EXISTS_USAGE,(char*)0);
		return(TCL_ERROR);
	}
	
	entry = Tcl_FindHashEntry(&databases,argv[1]);

	if (!entry) {
		/*
		 * db not in hashtab
		 */
		sprintf(buf,ARGS1,argv[0],argv[1],"no such database");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}
	
	db = (DB*) Tcl_GetHashValue(entry);

	key.dptr = argv[2];
	key.dsize = strlen(argv[2]) + 1 ;

	exists = gdbm_exists (db->db,key) ;
	
	if (exists) {
		interp->result = "1" ;
	} else {
		interp->result = "0" ;
	}

	return TCL_OK ;
}
	

/* ****************************************************************** */

/*
 * tcl_gdbm_delete <name> <key>
 */

static int 
tcl_gdbm_delete (client, interp, argc, argv)
	ClientData client;
	Tcl_Interp *interp;
	int argc;
	char **argv;

{
	DB *db ;
	Tcl_HashEntry *entry;
	datum key;
	char buf[512] ;
	gdbm_error ret ;


	/*
	 * check argument number
	 */
	if (argc != 3) {
		Tcl_AppendResult (interp,DELETE_USAGE,(char*)0);
		return(TCL_ERROR);
	}
	
	entry = Tcl_FindHashEntry(&databases,argv[1]);

	if (!entry) {
		/*
		 * db not in hashtab
		 */
		sprintf(buf,ARGS1,argv[0],argv[1],"no such database");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}

	db = (DB*) Tcl_GetHashValue(entry);

	/* check for read mode */

	if (db->mode == GDBM_READER) {
		sprintf(buf,ARGS1,argv[0],argv[1],"is opened for reading");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}

	key.dptr = argv[2];
	key.dsize = strlen(argv[2]) + 1 ;

	ret = gdbm_delete(db->db, key) ;
	
	if (ret == 0) {
		return TCL_OK ;
	} else {
		/*
		 * not found
		 */
		sprintf(buf,ARGS2,argv[0],argv[1],argv[2], "can't delete");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}
}

/* ****************************************************************** */

/*
 * tcl_gdbm_list <name>
 * the key is not allowed to be longer than 1023 bytes!
 */

static int 
tcl_gdbm_list (client, interp, argc, argv)
	ClientData client;
	Tcl_Interp *interp;
	int argc;
	char **argv;

{
	DB *db ;
	GDBM_FILE dbf ;
	Tcl_DString result ;
	Tcl_HashEntry *entry ;
	datum key ;
	char buf[1024];

	/*
	 * check argument number
	 */
	if (argc != 2) {
		Tcl_AppendResult (interp,LIST_USAGE,(char*)0);
		return(TCL_ERROR);
	}
	
	entry = Tcl_FindHashEntry(&databases,argv[1]);

	if (!entry) {
		/*
		 * db not in hashtab
		 */
		sprintf(buf,ARGS1,argv[0],argv[1],"no such database");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}

	db = (DB*) Tcl_GetHashValue(entry) ;
	dbf = db->db;
	Tcl_DStringInit(&result);

	key = gdbm_firstkey(dbf);
	if (!key.dptr) {
		/*
		 * empty db
		 */
		return TCL_OK ;
	}

	/* 
	 * copy key to result and free original key
	 */

	do {
		Tcl_DStringAppendElement(&result,key.dptr);
		/*
		 * remember key for gdbm_nextkey()
		 */
		strcpy(buf,key.dptr);
		free(key.dptr);
		key.dptr = buf ;
		key = gdbm_nextkey(dbf,key) ;
	} while (key.dptr) ;

	Tcl_DStringResult(interp, &result);
	return TCL_OK ;
}

/*
 * tcl_gdbm_firstkey
 */

static int 
tcl_gdbm_firstkey (client, interp, argc, argv)
	ClientData client;
	Tcl_Interp *interp;
	int argc;
	char **argv;

{
	DB *db ;
	GDBM_FILE dbf ;
	Tcl_HashEntry *entry ;
	datum key ;
	char buf[512];

	/*
	 * check argument number
	 */
	if (argc != 2) {
		Tcl_AppendResult (interp,FIRST_USAGE,(char*)0);
		return(TCL_ERROR);
	}
	
	entry = Tcl_FindHashEntry(&databases,argv[1]);

	if (!entry) {
		/*
		 * db not in hashtab
		 */
		sprintf(buf,ARGS1,argv[0],argv[1],"no such database");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}

	db = (DB*) Tcl_GetHashValue(entry) ;
	dbf = db->db;

	key = gdbm_firstkey(dbf);
	if (!key.dptr) {
		/*
		 * empty db
		 */
		return TCL_OK ;
	}

	Tcl_AppendResult (interp,key.dptr,(char*)0);
	free (key.dptr);
	return TCL_OK ;
}


/*
 * tcl_gdbm_nextkey
 */

static int 
tcl_gdbm_nextkey (client, interp, argc, argv)
	ClientData client;
	Tcl_Interp *interp;
	int argc;
	char **argv;

{
	DB *db ;
	GDBM_FILE dbf ;
	Tcl_HashEntry *entry ;
	datum oldkey, newkey ;
	char buf[512];

	/*
	 * check argument number
	 */
	if (argc != 3) {
		Tcl_AppendResult (interp,NEXT_USAGE,(char*)0);
		return(TCL_ERROR);
	}
	
	entry = Tcl_FindHashEntry(&databases,argv[1]);

	if (!entry) {
		/*
		 * db not in hashtab
		 */
		sprintf(buf,ARGS1,argv[0],argv[1],"no such database");
		Tcl_AppendResult (interp,buf,(char*)0);
		return TCL_ERROR ;
	}

	db = (DB*) Tcl_GetHashValue(entry) ;
	dbf = db->db;

	
	oldkey.dsize = strlen(argv[2]) + 1;
	oldkey.dptr = argv[2] ;
	
	newkey = gdbm_nextkey(db->db,oldkey);

	if (!newkey.dptr) {
		/*
		 * empty db
		 */
		return TCL_OK ;
	}

	Tcl_AppendResult (interp,newkey.dptr,(char*)0);
	free (newkey.dptr);
	return TCL_OK ;
}


/* ****************************************************************** */

/*
 * tcl_gdbm_open
 * 
 * mode in {r,rw,rwc,rwn}
 */

static int 
tcl_gdbm_open (client, interp, argc, argv)

	ClientData client;
	Tcl_Interp *interp;
	int argc;
	char **argv;

{
	int mode, new ;
	char buf[512] ;
	const char *db_error ;
	DB *db ;
	Tcl_HashEntry *entry ;

	/*
	 * check argument number
	 */
	if (argc < 2 || argc > 3) {
		Tcl_AppendResult (interp,OPEN_USAGE,(char*)0);
		return(TCL_ERROR);
	}

	mode = GDBM_READER ;

	/*
	 * check mode string if given
	 */
	if (argc == 3) {
		
		if (strcmp(argv[2],"r")==0)
			mode = GDBM_READER ;
		else if (strcmp(argv[2],"rw")==0)
			mode = GDBM_WRITER ;
		else if (strcmp(argv[2],"rwc")==0)
			mode = GDBM_WRCREAT ;
		else if (strcmp(argv[2],"rwn")==0)
			mode = GDBM_NEWDB ;
		else {
			sprintf(buf, BAD_MODE, argv[0], argv[2]);
			Tcl_AppendResult (interp,buf,(char *)0);
			return (TCL_ERROR);
		}
	}

	/*
	 * open gdbm and register it
	 */

	/* name not in hashtab - create new db */

	db = (DB*) malloc (sizeof(DB)) ;

	if (!db) {
		/* malloc problem */
		perror ("malloc in db_open");
		exit (-1);
	}
	
	/*
	 * create new name and malloc space for it
	 * malloc extra space for name
	 */

	sprintf(buf,"gdbm%i",handle);
	handle++ ;

	db->name = (char *) malloc (strlen(buf)+1) ;
	if (!db->name) {
		/* malloc problem */
		perror ("malloc in db_open");
		exit (-1);
	}
	strcpy(db->name,buf);

	db->mode = mode ;
	db->db = gdbm_open(argv[1],0,mode,0664,(VOID*)NULL);

	if (!db->db) {
		/*
		 * error occurred
		 * free previously allocated memory
		 * gdbm_errno indicates error
		 */
		free (db->name);
		free ((char*) db);
		db = (DB*) NULL ;

		db_error = gdbm_strerror(gdbm_errno);
		if (argc == 3) {
			sprintf(buf,ARGS2,argv[0],argv[1],argv[2],db_error);
		} else {
			sprintf(buf,ARGS1,argv[0],argv[1],db_error);
		}
		Tcl_AppendResult (interp,buf,(char*)0);
		return (TCL_ERROR);

	} else {
		/* 
		 * success
		 * enter db to hashtab
		 */
		entry = Tcl_CreateHashEntry(&databases,db->name,&new);
		Tcl_SetHashValue(entry,(char*)db);
		Tcl_AppendResult (interp,db->name,(char*)0);
		return TCL_OK;
	}

}

/*
 * tcl_gdbm
 * central command dispatcher
 */

static int 
tcl_gdbm (client, interp, argc, argv)

	ClientData client;
	Tcl_Interp *interp;
	int argc;
	char **argv;

{
	int i;
	char msg[80] ;

	static char *cmds[] = {
		"close",
		"delete",
		"exists",
		"fetch",
		"firstkey",
		"insert",
		"list",
		"nextkey",
		"open",
		"reorganize",
		"store",
		0
	};

	static (*f[])() = {
		tcl_gdbm_close,
		tcl_gdbm_delete,
		tcl_gdbm_exists,
		tcl_gdbm_fetch,
		tcl_gdbm_firstkey,
		tcl_gdbm_insert,
		tcl_gdbm_list,
		tcl_gdbm_nextkey,
		tcl_gdbm_open,
		tcl_gdbm_reorg,
		tcl_gdbm_store
	};

	/*
	 * check argument number
	 */
	if (argc < 3) {
		Tcl_AppendResult (interp,GDBM_USAGE,(char*)0);
		return(TCL_ERROR);
	}
	
	for (i = 0 ; cmds[i] != NULL; i++) {
		if (strcmp(cmds[i],argv[1]) == 0) {
			/* 
			 * call appropriate function
			 */
			return (*f[i])(client,interp,argc-1,&argv[1]);
		}
	}
	
	/* command not found */

	sprintf (msg,"%s %s: no such command",argv[0],argv[1]);
	Tcl_AppendResult (interp,msg,(char*)0);
	return(TCL_ERROR);
}

/*
 * Gdbm_Init
 * register commands, init data structures
 */

int 
Tclgdbm_Init (Tcl_Interp *interp)
{	
        if (Tcl_PkgRequire(interp, "Tcl", TCL_VERSION, 0) == NULL) {
            return TCL_ERROR;
        }
        if (Tcl_PkgProvide(interp, "Tclgdbm", VERSION) != TCL_OK) {
            return TCL_ERROR;
        }

	Tcl_CreateCommand(interp,"gdbm", tcl_gdbm,
			  (ClientData)0, (Tcl_CmdDeleteProc *)0);

	/*
	 * init hash table
	 */

	Tcl_InitHashTable (&databases,TCL_STRING_KEYS);

	/*
	 * set handle to zero
	 */

	handle = 0 ;

	return TCL_OK ;
}
