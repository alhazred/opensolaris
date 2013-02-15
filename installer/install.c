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
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include <libgen.h>  
#include "install.h"

int showbar(int perc) {                                                                                        
                                                                                                              
	char	line[80];                                                                                            
	int 	i, pos;                                                                                               
	                                                                                                              
	if(perc>100 || perc<0)                                                                                    
		return 1;                                                                                             
	pos=70*perc/100;                                                                                          
	line[0]='|';                                                                                              
	for(i=1; i<pos; i++)                                                                                      
		line[i]='-';                                                                                      
	                                                                                                         
	for(; i<70; i++)                                                                                          
		line[i]=' ';                                                                                          
                                                                                                             
	line[i++]='|';                                                                                            
	for(; i<80; i++)                                                                                      
		line[i]='\0';                                                                                         
                                                                                                             
	(void) sprintf(line+71, "%5d%%", perc);                                                                          
	(void) printf("%s\r", line);                                                                                     
	(void) fflush(stdout);                                                                                           
	return 0;                                                                                                 
}                         

int transfer(char *disk) {
	int	error = 0;
	
	// start copying main data over to it
	(void) printf ("Starting to transfer data - this may take some time..\n");
	(void) showbar(0);                                                                                                          

//	(void) snprintf(cmd, sizeof (cmd), "umount /lib/libc.so.1 2>/dev/null >/dev/null");                                
//	if (system_cmd(cmd) == -1)
//		error++;                              
	(void) showbar(5);                                                                                                          

	// copy /
	(void) snprintf(cmd, sizeof (cmd), "find / -xdev -depth | cpio -pudm /%s/%s/ 2>/dev/null >/dev/null",ZPOOL,ZFSROOT);                                
	if (system_cmd(cmd) == -1)
		error++;                              
	(void) showbar(10);

	// /usr
	(void) snprintf(cmd, sizeof (cmd), "find /usr -xdev -depth | cpio -pudm /%s/%s/ 2>/dev/null >/dev/null",ZPOOL,ZFSROOT);                                
	if (system_cmd(cmd) == -1)
		error++;                              
	(void) showbar(50);                                                                                                          

	// /mnt/pkg
	(void) snprintf(cmd, sizeof (cmd), "find /var/pkg -depth | cpio -pudm /%s/%s/ 2>/dev/null >/dev/null",ZPOOL,ZFSROOT);                                
	if (system_cmd(cmd) == -1)
		error++;                              
	(void) showbar(75);                                                                                                          

	// /admin
	(void) snprintf(cmd, sizeof (cmd), "find /admin -depth | cpio -pudm /%s/%s/ 2>/dev/null >/dev/null",ZPOOL,ZFSROOT);                                
	if (system_cmd(cmd) == -1)
		error++;                              
	(void) showbar(80);                                                                                                          

	// /root
	(void) snprintf(cmd, sizeof (cmd), "find /root -depth | cpio -pudm /%s/%s/ 2>/dev/null >/dev/null",ZPOOL,ZFSROOT);                                
	if (system_cmd(cmd) == -1)
		error++;                              
	(void) showbar(85);                                                                                                          
	
	// /dev
	(void) snprintf(cmd, sizeof (cmd), "find /dev -depth | cpio -pudm /%s/%s/ 2>/dev/null >/dev/null",ZPOOL,ZFSROOT);                                
	if (system_cmd(cmd) == -1)
		error++;                              
	(void) showbar(90);                                                                                                          
	
	// devices
	(void) snprintf(cmd, sizeof (cmd), "find /devices -depth | cpio -pudm /%s/%s/ 2>/dev/null >/dev/null",ZPOOL,ZFSROOT);                                
	if (system_cmd(cmd) == -1)
		error++;                              
	(void) showbar(95);                                                                                                          

	// etc
	(void) snprintf(cmd, sizeof (cmd), "find /etc -depth | cpio -pudm /%s/%s/ 2>/dev/null >/dev/null",ZPOOL,ZFSROOT);                                
	if (system_cmd(cmd) == -1)
		error++;                              
//	(void) showbar(90);                                                                                                          

//	(void) snprintf(cmd, sizeof (cmd), "mount -O -F lofs /usr/lib/libc/libc_hwcap1.so.1 /lib/libc.so.1 2>/dev/null >/dev/null");
//	if (system_cmd(cmd) == -1)
//		error++;                              
	(void) showbar(100);                                                                                                          
	(void) printf("\n");
	
	if (error !=0)
		(void) fprintf(stderr,"Transfer Tasks finished with errors\n");
	
	// install-finish
	(void) snprintf(cmd, sizeof (cmd), "install-finish %s", disk);
	if (system_cmd(cmd) == -1) {
		(void) fprintf(stderr,"Post-transfer Installation Tasks finished with errors\n");
		exit (1);
	}
	else {
		(void) printf("Installation is complete\n");
	}                                                                                                          
	// show cursor
	(void) printf("\033[?25h");     
	exit (0);	       
}

int 
prepare_target(boolean_t wholedisk,char *seldisk) {
    
	struct 	stat st;

	if (wholedisk) {                                                                                         
		if (ti_prepare(1, seldisk) != 0) {                                                                      
	    		(void) fprintf(stderr,"Can't create Solaris partition\n");
			exit(1);
		}	                                                            
	}                                                                                                         
	
	if (ti_prepare(2, seldisk) != 0) {                                                                          
		(void) fprintf(stderr,"Can't make default layout\n");                                                                
    		exit(1);
	}	                                                                                                
	
	(void) snprintf(cmd, sizeof (cmd), "/%s",ZPOOL);     
	if(stat(cmd,&st) == 0) {
		(void) printf("Sorry ZFS pool %s already exists."
		"Please destroy it and run installer again\n",ZPOOL);
		exit(1);
	}

	(void) strcat(seldisk,"s0");                                                                                     
	
	if (ti_prepare(3, seldisk) != 0) {                                                                          
		(void) fprintf(stderr,"Sorry, unable to create root pool. Exiting now\n");                                                                           
		exit(1);
	}
    
	// compression                                                                                             
					                                                                                                                
	(void) snprintf(cmd, sizeof (cmd), "zfs set compression=on %s/%s",ZPOOL,ZFSROOT);                                
	                                                                            
	if (system_cmd(cmd) == -1)                                                                          
		(void) fprintf(stderr,"Can't set zfs compression\n");                              

	// bootfs
	(void) snprintf(cmd, sizeof (cmd), "zpool set bootfs=%s/%s %s",ZPOOL,ZFSROOT,ZPOOL);                                
	                                                                            
	if (system_cmd(cmd) == -1)                                                                          
		(void) fprintf(stderr,"Can't set bootfs\n");                              
	
	(void) transfer(seldisk);

}

