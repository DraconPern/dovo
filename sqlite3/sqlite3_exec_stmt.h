#ifndef _SQLITE3_EXEC_STMT_H_
#define _SQLITE3_EXEC_STMT_H_

#ifdef __cplusplus
extern "C" {
#endif
	
int sqlite3_exec_stmt(
  sqlite3_stmt *pStmt,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  void *pArg,                 /* First argument to xCallback() */
  char **pzErrMsg             /* Write error messages here */
);


#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif

#endif 

