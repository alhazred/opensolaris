/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include <ti_api.h>

#include "install.h"


/*
 * create_fdisk_target()
 */

static int
prepare_fdisk_target(nvlist_t *target_attrs, char *disk_name, boolean_t whole_disk)
{

	assert(target_attrs != NULL);

	/* add atributes requiring creating Solaris2 partition on whole disk */

	/* disk name */

	if (nvlist_add_string(target_attrs, TI_ATTR_FDISK_DISK_NAME,disk_name) != 0) {
		(void) fprintf(stderr, "Couldn't add TI_ATTR_FDISK_DISK_NAME to nvlist\n");
		return (-1);
	}

	/*
	 * use whole disk for Solaris partition ?
	 * If not, fdisk partition table needs to be provided in file
	 */

	if (whole_disk) {
		if (nvlist_add_boolean_value(target_attrs,TI_ATTR_FDISK_WDISK_FL, B_TRUE) != 0) {
			(void) fprintf(stderr, "ERR: Couldn't add TI_ATTR_FDISK_WDISK_FL to nvlist\n");
			return (-1);
		}
	}
	 

	return (0);
}


/*
 * prepare_vtoc_target()
 */

static int
prepare_vtoc_target(nvlist_t *target_attrs, char *disk_name, boolean_t default_layout)
{

	assert(target_attrs != NULL);

	/* add atributes requiring creating VTOC */

	/* disk name */

	if (nvlist_add_string(target_attrs, TI_ATTR_SLICE_DISK_NAME, disk_name) != 0) {
		(void) fprintf(stderr, "Couldn't add TI_ATTR_SLICE_DISK_NAME to nvlist\n");
		return (-1);
	}

	/*
	 * create default VTOC layout
	 */

	if (nvlist_add_boolean_value(target_attrs, TI_ATTR_SLICE_DEFAULT_LAYOUT, B_TRUE) != 0) {
		(void) fprintf(stderr, "ERR: Couldn't add TI_ATTR_SLICE_DEFAULT_LAYOUT to nvlist\n");
		return (-1);
	}

	return (0);
}


/*
 * prepare_zfs_rpool_target()
 */

static int
prepare_zfs_rpool_target(nvlist_t *target_attrs, char *rpool_name, char *disk_name) {
	/* target type */

	if (nvlist_add_uint32(target_attrs, TI_ATTR_TARGET_TYPE, TI_TARGET_TYPE_ZFS_RPOOL) != 0) {
		(void) fprintf(stderr, "Couldn't add TI_ATTR_TARGET_TYPE to nvlist\n");
		return (-1);
	}

	/* root pool name */

	if (nvlist_add_string(target_attrs, TI_ATTR_ZFS_RPOOL_NAME, rpool_name) != 0) {
		(void) fprintf(stderr, "Couldn't add TI_ATTR_ZFS_RPOOL_NAME to nvlist\n");
		return (-1);
	}

	/* device name */

	if (nvlist_add_string(target_attrs, TI_ATTR_ZFS_RPOOL_DEVICE, disk_name) != 0) {
		(void) fprintf(stderr, "Couldn't add TI_ATTR_ZFS_RPOOL_DEVICE to nvlist\n");
		return (-1);
	}

	return (0);
}


/*
 * prepare_zfs_fs_target()
 */

static int
prepare_zfs_fs_target(nvlist_t *target_attrs, char *pool_name, char *fs_name) {
	char		fs_num = 1;
	char		*fs_names[1];
	
	fs_names[0] = fs_name;

	/* target type */

	if (nvlist_add_uint32(target_attrs, TI_ATTR_TARGET_TYPE, TI_TARGET_TYPE_ZFS_FS) != 0) {
		(void) fprintf(stderr, "Couldn't add TI_ATTR_TARGET_TYPE to nvlist\n");
		return (-1);
	}

	/* only one filesystem will be created */

	if (nvlist_add_uint16(target_attrs, TI_ATTR_ZFS_FS_NUM, fs_num) != 0) {
		(void) fprintf(stderr, "Couldn't add TI_ATTR_ZFS_FS_NUM to nvlist\n");
		return (-1);
	}

	/* pool name */

	if (nvlist_add_string(target_attrs, TI_ATTR_ZFS_FS_POOL_NAME, pool_name) != 0) {
		(void) fprintf(stderr, "Couldn't add TI_ATTR_ZFS_FS_POOL_NAME to nvlist\n");
		return (-1);
	}

	/* ZFS filesystem names */

	if (nvlist_add_string_array(target_attrs, TI_ATTR_ZFS_FS_NAMES, fs_names, fs_num) != 0) {
		(void) fprintf(stderr, "Couldn't add TI_ATTR_ZFS_FS_NAMES to nvlist\n");
		return (-1);
	}

	return (0);
}


/*
 * main ti
 */
int
ti_prepare(int ops, char *disk_name) {
	/* create Solaris2 partition on whole disk */

	boolean_t	fl_wholedisk = B_TRUE;
	nvlist_t	*target_attrs = NULL;
	char            *zfs_root_pool_name = ZPOOL;                                                           
	char            *zfs_fs_name = ZFSROOT;                                                           

	/* Create nvlist containing attributes describing the target */

	if (disk_name == NULL) {
		(void) fprintf(stderr, "ERR: Device name is required\n");
		return (-1);
	}

	if (nvlist_alloc(&target_attrs, TI_TARGET_NVLIST_TYPE, 0) != 0) {
		(void) fprintf(stderr, "ERR: Couldn't create nvlist describing the target\n");
		return (-1);
	}

	if (ops == 1) {

		/* set target type attribute */

		if (nvlist_add_uint32(target_attrs, TI_ATTR_TARGET_TYPE, TI_TARGET_TYPE_FDISK) != 0) {
			(void) fprintf(stderr, "ERR: Couldn't add TI_ATTR_TARGET_TYPE to nvlist\n");
			nvlist_free(target_attrs);
			exit(1);
		}

		if (prepare_fdisk_target(target_attrs, disk_name, fl_wholedisk) != 0) {
			(void) fprintf(stderr, "ERR: preparing of fdisk target failed\n");
			exit(1);
		} 

		/* create target */

		if (ti_create_target(target_attrs, NULL) != TI_E_SUCCESS) {
			(void) fprintf(stderr, "ERR: creating of fdisk target failed\n");
			exit(1);
		} 
	}


	if (ops == 2) {

	    /* set target type attribute */

		if (nvlist_add_uint32(target_attrs,TI_ATTR_TARGET_TYPE, TI_TARGET_TYPE_VTOC) != 0) {
			(void) fprintf(stderr, "ERR: Couldn't add TI_ATTR_TARGET_TYPE to nvlist\n");
			nvlist_free(target_attrs);
			exit(1);
		}

		if (prepare_vtoc_target(target_attrs, disk_name, B_TRUE) != 0) {
			(void) fprintf(stderr,"ERR: preparing of VTOC target failed\n");
			nvlist_free(target_attrs);
			exit(1);
		} 

		/* create target */

		if (ti_create_target(target_attrs, NULL) != TI_E_SUCCESS) {
			(void) fprintf(stderr, "ERR: creating of VTOC target failed\n");
			exit(1);
		} 
			
		

	}

	if (ops == 3) {
	
		/* ZFS root pool */
	    if (prepare_zfs_rpool_target(target_attrs, zfs_root_pool_name, disk_name) != 0) {
			(void) fprintf(stderr, "ERR: preparing of ZFS root pool target failed\n");
			exit(1);
	    } 

	    /* create target */

	    if (ti_create_target(target_attrs, NULL) != TI_E_SUCCESS) {
			(void) fprintf(stderr, "ERR: creating of ZFS root pool target failed\n");
			exit(1);
	    } 
			
			

	    /* ZFS filesystem */
		
	    if (prepare_zfs_fs_target(target_attrs, zfs_root_pool_name, zfs_fs_name) != 0) {
			(void) fprintf(stderr, "ERR: preparing of ZFS filesystem target failed\n");
			exit(1);
	    } 

	    /* create target */

	    if (ti_create_target(target_attrs, NULL) != TI_E_SUCCESS) {
			(void) fprintf(stderr, "ERR: creating of ZFS filesystem target failed\n");
			exit(1);
	    } 
	}



	/* call TI for creating the target */

	ti_create_target(target_attrs, NULL);
	nvlist_free(target_attrs);

	return (0);
}
