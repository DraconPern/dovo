/*

Copyright (c) 2007-2015 Ing-Long Eric Kuo

Distributed under the MIT License.
http://opensource.org/licenses/MIT

*/

#include "sqlite3.h"
#include <stdlib.h>

/*
** Execute SQL code.  Return one of the SQLITE_ success/failure
** codes.  Also write an error message into memory obtained from
** malloc() and make *pzErrMsg point to that message.
**
** If the SQL is a query, then for each row in the query result
** the xCallback() function is called.  pArg becomes the first
** argument to xCallback().  If xCallback=NULL then no callback
** is invoked, even for queries.
*/
int sqlite3_exec_stmt(
	sqlite3_stmt *pStmt,           /* The SQL to be executed */
	sqlite3_callback xCallback, /* Invoke this callback routine */
	void *pArg,                 /* First argument to xCallback() */
	char **pzErrMsg             /* Write error messages here */
	){
		int rc = SQLITE_OK;
		sqlite3 *db = 0;
		char **azCols = 0;

		int nRetry = 0;
		int nChange = 0;
		int nCallback;

		if( pStmt ==0 ) return SQLITE_OK;
		db = sqlite3_db_handle(pStmt);
		while( (rc==SQLITE_OK || (rc==SQLITE_SCHEMA && (++nRetry)<2))){
			int nCol;
			char **azVals = 0;

			nCallback = 0;

			nCol = sqlite3_column_count(pStmt);
			azCols = (char **) malloc(2*nCol*sizeof(const char *) + 1);
			if( azCols==0 ){
				goto exec_out;
			} 

			while( 1 ){
				int i;
				rc = sqlite3_step(pStmt);

				/* Invoke the callback function if required */
				if( xCallback && (SQLITE_ROW==rc ||
					(SQLITE_DONE==rc && !nCallback))
					){
						if( 0==nCallback ){
							for(i=0; i<nCol; i++){
								azCols[i] = (char *)sqlite3_column_name(pStmt, i);
							}
							nCallback++;
						}
						if( rc==SQLITE_ROW ){
							azVals = &azCols[nCol];
							for(i=0; i<nCol; i++){
								azVals[i] = (char *)sqlite3_column_text(pStmt, i);
							}

							if( xCallback(pArg, nCol, azVals, azCols) ){
								rc = SQLITE_ABORT;
								goto exec_out;
							}
						}
				}

				if( rc!=SQLITE_ROW ){                   
					break;
				}
			}
		}

exec_out:
		if( azCols ) free(azCols); 
		sqlite3_reset(pStmt);

		return rc;
}
