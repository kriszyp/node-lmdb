/* backend.c - deals with backend subsystem */
/* $OpenLDAP$ */
/* This work is part of OpenLDAP Software <http://www.openldap.org/>.
 *
 * Copyright 2001-2004 The OpenLDAP Foundation.
 * Portions Copyright 2001-2003 Pierangelo Masarati.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.
 *
 * A copy of this license is available in file LICENSE in the
 * top-level directory of the distribution or, alternatively, at
 * <http://www.OpenLDAP.org/license.html>.
 */
/* ACKNOWLEDGEMENTS:
 * This work was initially developed by Pierangelo Masarati for inclusion
 * in OpenLDAP Software.
 */


#include "portable.h"

#include <stdio.h>
#include <ac/string.h>

#include "slap.h"
#include "back-monitor.h"

/*
 * initializes backend subentries
 */
int
monitor_subsys_backend_init(
	BackendDB		*be,
	monitor_subsys_t	*ms
)
{
	monitor_info_t		*mi;
	Entry			*e_backend, **ep;
	int			i;
	monitor_entry_t		*mp;
	monitor_subsys_t	*ms_database;

	mi = ( monitor_info_t * )be->be_private;

	ms_database = monitor_back_get_subsys( SLAPD_MONITOR_DATABASE_NAME );
	if ( ms_database == NULL ) {
		Debug( LDAP_DEBUG_ANY,
			"monitor_subsys_backend_init: "
			"unable to get "
			"\"" SLAPD_MONITOR_DATABASE_NAME "\" "
			"subsystem\n",
			0, 0, 0 );
		return -1;
	}

	if ( monitor_cache_get( mi, &ms->mss_ndn, &e_backend ) )
	{
		Debug( LDAP_DEBUG_ANY,
			"monitor_subsys_backend_init: "
			"unable to get entry \"%s\"\n",
			ms->mss_ndn.bv_val, 0, 0 );
		return( -1 );
	}

	mp = ( monitor_entry_t * )e_backend->e_private;
	mp->mp_children = NULL;
	ep = &mp->mp_children;

	for ( i = 0; i < nBackendInfo; i++ ) {
		char 		buf[ BACKMONITOR_BUFSIZE ];
		BackendInfo 	*bi;
		struct berval 	bv;
		int		j;
		Entry		*e;

		bi = &backendInfo[ i ];

		snprintf( buf, sizeof( buf ),
				"dn: cn=Backend %d,%s\n"
				"objectClass: %s\n"
				"structuralObjectClass: %s\n"
				"cn: Backend %d\n"
				"creatorsName: %s\n"
				"modifiersName: %s\n"
				"createTimestamp: %s\n"
				"modifyTimestamp: %s\n",
				i,
				ms->mss_dn.bv_val,
				mi->mi_oc_monitoredObject->soc_cname.bv_val,
				mi->mi_oc_monitoredObject->soc_cname.bv_val,
				i,
				mi->mi_creatorsName.bv_val,
				mi->mi_creatorsName.bv_val,
				mi->mi_startTime.bv_val,
				mi->mi_startTime.bv_val );
		
		e = str2entry( buf );
		if ( e == NULL ) {
			Debug( LDAP_DEBUG_ANY,
				"monitor_subsys_backend_init: "
				"unable to create entry \"cn=Backend %d,%s\"\n",
				i, ms->mss_ndn.bv_val, 0 );
			return( -1 );
		}
		
		bv.bv_val = bi->bi_type;
		bv.bv_len = strlen( bv.bv_val );

		attr_merge_normalize_one( e, mi->mi_ad_monitoredInfo,
				&bv, NULL );
		attr_merge_normalize_one( e_backend, mi->mi_ad_monitoredInfo,
				&bv, NULL );

		if ( bi->bi_controls ) {
			int j;

			for ( j = 0; bi->bi_controls[ j ]; j++ ) {
				bv.bv_val = bi->bi_controls[ j ];
				bv.bv_len = strlen( bv.bv_val );
				attr_merge_one( e, slap_schema.si_ad_supportedControl, &bv, NULL );
			}
		}

		for ( j = 0; j < nBackendDB; j++ ) {
			BackendDB	*be = &backendDB[ j ];
			char		buf[ SLAP_LDAPDN_MAXLEN ];
			struct berval	dn;
			
			if ( be->bd_info != bi ) {
				continue;
			}

			snprintf( buf, sizeof( buf ), "cn=Database %d,%s",
					j, ms_database->mss_dn.bv_val );
			dn.bv_val = buf;
			dn.bv_len = strlen( buf );

			attr_merge_normalize_one( e, mi->mi_ad_seeAlso,
					&dn, NULL );
		}
		
		mp = monitor_entrypriv_create();
		if ( mp == NULL ) {
			return -1;
		}
		e->e_private = ( void * )mp;
		mp->mp_info = ms;
		mp->mp_flags = ms->mss_flags | MONITOR_F_SUB;

		if ( monitor_cache_add( mi, e ) ) {
			Debug( LDAP_DEBUG_ANY,
				"monitor_subsys_backend_init: "
				"unable to add entry \"cn=Backend %d,%s\"\n",
				i,
			       	ms->mss_ndn.bv_val, 0 );
			return( -1 );
		}

		*ep = e;
		ep = &mp->mp_next;
	}
	
	monitor_cache_release( mi, e_backend );

	return( 0 );
}

