/* $OpenLDAP$ */
/*
 * Copyright 1999-2003 The OpenLDAP Foundation, All Rights Reserved.
 * COPYING RESTRICTIONS APPLY, see COPYRIGHT file
 */
/*
 *	 Copyright 1999, John C. Quillan, All rights reserved.
 *	 Portions Copyright 2002, myinternet Limited. All rights reserved.
 *
 *	 Redistribution and use in source and binary forms are permitted only
 *	 as authorized by the OpenLDAP Public License.	A copy of this
 *	 license is available at http://www.OpenLDAP.org/license.html or
 *	 in file LICENSE in the top-level directory of the distribution.
 */

#include <EXTERN.h>
#include <perl.h>
#undef _ /* #defined by both Perl and ac/localize.h */

#ifdef HAVE_WIN32_ASPERL
#include "asperl_undefs.h"
#endif

#include "portable.h"
	
#include <stdio.h>

#include "slap.h"

#include "perl_back.h"

/**********************************************************
 *
 * Close
 *
 **********************************************************/

int
perl_back_close(
	BackendInfo *bd
)
{
	ldap_pvt_thread_mutex_lock( &perl_interpreter_mutex );	

	perl_destruct(PERL_INTERPRETER);

	ldap_pvt_thread_mutex_unlock( &perl_interpreter_mutex );	

	return 0;
}

int
perl_back_destroy(
	BackendInfo *bd
)
{
	perl_free(PERL_INTERPRETER);
	PERL_INTERPRETER = NULL;

	ldap_pvt_thread_mutex_destroy( &perl_interpreter_mutex );	

	return 0;
}

int
perl_back_db_destroy(
	BackendDB *be
)
{
	free( be->be_private );
	be->be_private = NULL;

	return 0;
}
