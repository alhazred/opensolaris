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

/*
 * MilaX console installer July 23 2009 
 * Alexander Eremin <eremin@milax.org> 
 */

#include <td_dd.h>                                                                                            
#include <td_api.h>            

#include "install.h"


char*
just_the_disk_name(char *src) {
	int 		len;
	char 		*pos;

	len=strlen(src);
	pos=src+len-2;
	*pos='\0';
	
	return (src);
}

char*
just_the_part_num(char *src) {
	int		len;
	char* 		pos;
	
	len=strlen(src);
	pos=src+len-1;
	
	return (pos);
}					
/*
 * discover_disks()
 */
int
discover_disks() {

	nvlist_t        *attrs;  
	int		i, ndisks;
	uint64_t	ui64;
	uint32_t        b, n;  
	char		*name;
	int		num;
	int 		total;
	int		input;
	
	if (td_discover(TD_OT_DISK, &ndisks) != 0) {
		(void) fprintf(stderr,"Couldn't discover disks\n");
		 exit (1);
	}
	
	total=0;
	for (i = 0; i < ndisks; i++) {
	    if (td_get_next(TD_OT_DISK) != 0) {
		    (void) fprintf(stderr,"Couldn't get next disk\n");
		    exit (1);
	    }
	
	    total=i+1;
	    if (total==1) {
		    (void) printf("On which disk do you want to install system:\n");

		    (void) printf("-----------------------------------------------------------\n"
			    " num |        name|        type| size[MB]| boot| removable|\n"
			    "-----------------------------------------------------------\n");
	    }
	    // if many disks we need pager
	    if (i == 12 || i == 24) {                                                                         
		    if (i != ndisks) {
			    (void) printf("--more--\n");                                                                             
		    	    input = getc(stdin);                                                                                 
		    	    printf("\033[A"); /* up cursor */                                                                 
			    /* Erase the "More" message */                                                
		    	    putc('\r', stdout);                                                                               
			    if (input == 'q')                                                                                     
				    break;   
		    }	                                                                                             
	    }                     
	    
	    
	    
	    // 
    	    attrs = td_attributes_get(TD_OT_DISK);                                                    
	    (void) printf("%4d |", i + 1);

	    if (nvlist_lookup_string(attrs, TD_DISK_ATTR_NAME, &name) == 0) 
		    (void) printf("%12s|", name);
	    else 
    		    (void) printf("%12s|", "-");

	    /* disk type - ata,usb,scsi,fiber channel */

	    if (nvlist_lookup_string(attrs, TD_DISK_ATTR_CTYPE, &name) == 0) 
		    (void) printf("%12s|", name);
	    else 
		    (void) printf("%12s|", "-");


	    /* block size */

	    if (nvlist_lookup_uint32(attrs, TD_DISK_ATTR_BLOCKSIZE, &b) != 0) 
		    b = 0;

	    /* total # of blocks */

	    if (nvlist_lookup_uint64(attrs, TD_DISK_ATTR_SIZE, &ui64) != 0) 
		    ui64 = 0;

	    /* total size in [MB] */

	    if (b*ui64 != 0) 
		    (void) printf("%9lld|", ((uint64_t)b * ui64)/(1024 * 1024));
	    else 
		    (void) printf("%9s|", "-");
	
	    //boot
	    if (nvlist_lookup_boolean(attrs, TD_DISK_ATTR_CURRBOOT) == 0)                                
        	    (void) printf("%5s|", "yes");                                                         
	    else                                                                                       
		    (void) printf("%5s|", "no");                                                          
	                                                              
	    /* removable ? */

	    if (nvlist_lookup_boolean(attrs, TD_DISK_ATTR_REMOVABLE) == 0) 
		    (void) printf("%10s|", "yes");
	    else 
		    (void) printf("%10s|", "no");

	    (void) printf("\n");

	}
	if (total==0) {
		(void) printf ("No disks found\n");
    		exit(1);
	} else {	

		// hide cursor
		(void) printf("\033[?25l");


		(void)	printf("-----------------------------------------------------------\n");
	
		(void) printf ("Enter a disk number: ");
    		(void) fflush(stdin);
	 
		(void) scanf ("%d" , & num);
	
		while (num < 1 || num > total) {
			(void) printf ("Wrong disk number"
				" \nEnter disk number: ");
			(void) fflush(stdin);
		 
			(void) scanf ("%d" , & num);
		}
	
		if (td_reset(TD_OT_DISK) != 0) {
			(void) fprintf(stderr,"Couldn't reset disks\n");
			exit (1);
		}
	  
		for (i = 0; i < ndisks; i++) {

			if (td_get_next(TD_OT_DISK) != 0) {
				(void) fprintf(stderr,"Couldn't get next disk\n");
				exit (1);
			}

	    
			attrs = td_attributes_get(TD_OT_DISK);                                                    

			if (nvlist_lookup_string(attrs, TD_DISK_ATTR_NAME, &name) == 0) {	
	
				if (num == i + 1) {
					seldisk=name;
	    				break;	
				}
			}																											     
		}
	}
	
	return (0);
}


/*
 * discover_partitions()
 */
static int
discover_partitions() {
	nvlist_t	*attrs;
	int		i;
	int		nparts;
	uint32_t	bid;
	uint32_t        b, n;
	uint32_t	ptype;
	char		*type;
	char 		*pname;
	char 		*dname;
		

	if (td_discover(TD_OT_PARTITION, &nparts) != 0) {
		(void) fprintf(stderr,"Couldn't discover partitions\n");
		exit (1);
	}

	solpart = 0;
	int dparts = 0;	

	for (i = 0; i < nparts; i++) {
		if (td_get_next(TD_OT_PARTITION) != 0) {
			(void) printf("Couldn't get next partition\n");

			return (-1);
		}


	attrs = td_attributes_get(TD_OT_PARTITION);
	
	if (nvlist_lookup_string(attrs, TD_PART_ATTR_NAME, &pname) == 0) {
		
		strcpy(dname,pname);

		if (strcmp(seldisk,just_the_disk_name(dname))==0) {
				
			dparts++;
			if (dparts==1) {
				(void)printf("Partitions on the disk %s\n",seldisk);                                   

				(void) printf("------------------------------------------------\n"
					" num |        name|        type| size[MB]| boot|\n"
					"------------------------------------------------\n");
			}
				    
			(void) printf("%4d |%12s|", dparts,pname);

			if (nvlist_lookup_uint32(attrs, TD_PART_ATTR_TYPE, &ptype) == 0) {
	
			switch (ptype) {                                                                          
	    		case 65:                                                                                
	    			type = "linux";                                                                             
	    			break;
	    		case 66:                                                                                
	    			type = "linux swap";                                                                             
	    			break;
	    		case 67:         
	    		case 131:                                                                       
	    			type = "linux native";                                                                             
	    			break;
	    		case 130:                                                                                
	    			type = "solaris";                                                                             
	    			break;
	    		case 165:                                                                                
	    			type = "bsd";                                                                             
	    			break;
	    		case 190:                                                                                
	    			type = "solaris boot";                                                                             
	    			break;
	    		case 191:                                                                                
	    			type = "solaris 2";                                                                             
	    			solpart=1;
	    			memset(solpart_name, 0, sizeof(solpart_name));         
				strncpy(solpart_name, pname, sizeof(solpart_name));
				solpart_num=just_the_part_num(pname);
				break;
			default:                                                                                          
        			type = "unknown";                                                                         
        			break;                                                                                    
        		}                              
			(void) printf("%12s|", type);
			}
		    
			if (nvlist_lookup_uint32(attrs,TD_PART_ATTR_SIZE , &n) == 0) {
				(void) printf("%9d|", n/(2*1024));
				if (ptype==191) {
				solpart_size=n/(2*1024);
				}
			    } else {
				(void) printf("%9s|", "-");
			}

			if (nvlist_lookup_uint32(attrs, TD_PART_ATTR_BOOTID, &bid) == 0) {
				(void) printf("%5s|", bid & 0x80 ? "yes" : "no");
			} else {
				(void) printf("%5s|", "-");
			}
	    
			(void) printf("\n");
		    }    
	    }
	}

	if (dparts==0)
	    (void) printf("No partitions found\n");    
	else
	    (void) printf("------------------------------------------------\n");
}

int 
system_cmd(char *cmd) {                                                                                                                                 

	int     	ret;                                                                                                                                             
	                                                                                                                                                             
	ret = system(cmd);                                                                                                                                         
	if (WEXITSTATUS(ret) != 0)                                                                                                              
		return (-1);                                                                                                                                         
				                                                                                                                                                                 
	return 0;                                                                                                                                                
}          

int get_target() {
     
	int		ret, num;

	wholedisk = B_FALSE;
    
	(void) td_discovery_release();

	(void) discover_partitions();
    
	if (solpart==0) {
		(void) printf("Cannot find the Solaris partitions on disk %s\n", seldisk);
    		(void) printf("Using whole disk? (All data will be lost)\n");
    		(void) printf("1. Yes\n");
    		(void) printf("2. No (Fdisk will start)\n");
		(void) fflush(stdin);
	     
    		(void) scanf("%d", & num);
		while (num < 1 || num > 2) {
        	        (void) printf("Enter 1 or 2\n");
        		(void) fflush(stdin);
		        
			(void) scanf("%d", & num);
    		}
    		if (num == 1) {
        	        wholedisk = B_TRUE;
    		} else {
            	        (void) snprintf(cmd, sizeof (cmd), "/usr/sbin/fdisk /dev/rdsk/%sp0",seldisk);                                
			if (system_cmd(cmd) != 0) {                                                                          
			        (void) printf("Please create Solaris partition manually and restart installer\n"); 
				exit (1);
			}
			get_target();	    
    		}

	} else {

	    // we have Solaris part
		(void) printf("Found existing Solaris partition: %s (%d MB)\n", solpart_name,solpart_size);
		(void) printf("Where should system be installed?\n");
    		(void) printf("1. Using whole disk\n");
    		(void) printf("2. Using existing Solaris partition\n");
    		(void) fflush(stdin);
	     
		(void) scanf("%d", & num);
    		while (num < 1 || num > 2) {
        	        (void) printf("Enter 1 or 2\n");
        		(void) fflush(stdin);
		     
			(void) scanf("%d", & num);
    		}
            
		if (num == 1) 
        	        wholedisk = B_TRUE;
	}
    
	if (wholedisk) {
    	        (void) printf("System will be installed on disk %s. Continue?\n", seldisk);
	} else {
    	        if (solpart_size < 400) {
	    	        (void) printf("Not enough free space on %s (%d MB). At least 400MB required\n", solpart_name, solpart_size);
    		        (void) printf("Please correct and restart installer\n");
        	        exit(1);
    		}
    		if (solpart_act == 0) {
        	        (void) printf("Solaris partition %s is not active. Activate now?\n", solpart_name);
        		(void) printf("1. Yes\n");
			(void) printf("2. No\n");
			(void) fflush(stdin);
		     
			(void) scanf("%d", & num);
			while (num < 1 || num > 2) {
    			        (void) printf("Enter 1 or 2\n");
    				(void) fflush(stdin);
			     
				(void) scanf("%d", & num);
			}
			if (num == 1) { 
			        (void) snprintf(cmd, sizeof (cmd), "printf '2\n%s\n6'|fdisk /dev/rdsk/%sp0 2>/dev/null >/dev/null",solpart_num,seldisk);                                
				if (system_cmd(cmd) == -1)                                                                          
                		        (void) printf("Can't activate partition.You must activate partition manually\n");                   
			} else {
			        (void) printf ("You must activate partition before reboot\n");
			}
	    
    		}
        
		(void) printf("System will be installed on partition %s. Continue?\n", solpart_name);
	}

	(void) printf("1. Yes\n");
	(void) printf("2. No\n");
	(void) fflush(stdin);
     
	(void) scanf("%d", & num);

	while (num < 1 || num > 2) {
    	        (void) printf("Enter 1 or 2\n");
    		(void) fflush(stdin);
	     
		(void) scanf("%d", & num);
	}
	
	if (num == 1) 
    		(void) prepare_target(wholedisk, seldisk);
	else 
    		exit(0);
    
}

			 
/*
 * main()
 */
int
main(int argc, char *argv[]) {

	(void) system("clear");
    
	// hide cursor
	(void) printf("\033[?25l");
	
	(void) discover_disks();

	(void) get_target();

	return (0);
}
