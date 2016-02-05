#!/usr/bin/ruby
#                                                                                                            
# CDDL HEADER START                                                                                           
#                                                                                                             
# The contents of this file are subject to the terms of the                                                   
# Common Development and Distribution License (the "License").                                                
# You may not use this file except in compliance with the License.                                            
#                                                                                                             
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE                                         
# or http://www.opensolaris.org/os/licensing.                                                                 
# See the License for the specific language governing permissions                                             
# and limitations under the License.                                                                          
#                                                                                                             
# When distributing Covered Code, include this CDDL HEADER in each                                            
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.                                           
# If applicable, add the following below this CDDL HEADER, with the                                           
# fields enclosed by brackets "[]" replaced with your own identifying                                         
# information: Portions Copyright [yyyy] [name of copyright owner]                                            
#                                                                                                             
# CDDL HEADER END                                                                                             
#                                                                                                             
# Copyright 2007 Sun Microsystems, Inc.  All rights reserved.                                                 
# Use is subject to license terms.                                                                            
#                                                                                                             
# Small hack script to get pkg contents from repo without pkg
# http://www.milax.org
#

require 'net/http'   
require 'fileutils'                                                                                          
require 'thread'


    if ARGV.length == 0 || ARGV.length < 2
	puts "Usage: getpkg <inst_dir> <manifest url>" 
	exit 0 
    end 
    
    # dir must have "/" at the end
    base = ARGV[0]
    if !base.match(/\/$/)                                                                                     
        base = base + "/"                                                                                             
    end              
    url = ARGV[1].split("pkg.opensolaris.org")
	
    # get manifest from repo
    Net::HTTP.start("pkg.opensolaris.org") { |http|
        resp = http.get(url[1])
        open("manifest", "wb") { |file|
	    file.write(resp.body)
	}
    }

    # read manifest
    f = File.new("manifest")
    print "Processing files "	

    begin
    while (line = f.readline)
        a = %w[ | / - \\ ]

        t = Thread.new {
	    line.chomp
            # get dir path
	    if line =~ /^dir/
                dir = line.scan(/path=[^\s]*/).to_s.split("=")[1]
	            
	        # get mode,group,owner,hash
	   	mode = line.scan(/mode=[0-9]*/).to_s.split("=")[1]
                owner = line.scan(/owner=[a-z]*/).to_s.split("=")[1].to_s
	    	group = line.scan(/group=[a-z]*/).to_s.split("=")[1].to_s
	    	FileUtils.mkdir_p(base + dir, :mode=>Integer(mode))
	    	FileUtils.chown(owner,group, base + dir)
	    
            end
	
            # get file name, path    
            if line =~ /^file/
                path = line.scan(/path=[^\s]*/).to_s.split("=")[1]
		if path.match(/\//)
		    filetmp = path.split("/")
		    file = filetmp[(filetmp.size-1)] 
		    fdir = path.gsub(file,"")
		end
        
        	# get mode,group,owner,hash
	    	hash = line.split(/\s/)[1].to_s
		mode = line.scan(/mode=[0-9]*/).to_s.split("=")[1]
		owner = line.scan(/owner=[a-z]*/).to_s.split("=")[1].to_s
	    	group = line.scan(/group=[a-z]*/).to_s.split("=")[1].to_s
    
		# get file from repo, gunzip, chown and chmod
	    	Net::HTTP.start("pkg.opensolaris.org") { |http|                                                               
		    resp = http.get("/file/0/" + hash)                      
		    
		    FileUtils.mkdir_p base + fdir
	    	    open(base + fdir + file + ".gz", "wb") { |filed|                                                                               
		        fileiled.write(resp.body)
			filed.chmod( Integer(mode) )
		    }                                                                                                            
		}                                                                                                            
		    
		system("gunzip -f " + base + fdir + file + ".gz")
	    	FileUtils.chown(owner,group, base + fdir + file)
		    
            end	
    	    } 
	    
	    # small spinner
	    while t.alive?
	        print a.unshift(a.pop).last
	        sleep 0.1
	        print "\b"
    	    end
    	    t.exit	
        end
        rescue EOFError
            f.close
        end
	
        # now processing links
  	f = File.new("manifest")
  	begin
   	while (line = f.readline)
    	    a = %w[ | / - \\ ]
	    t= Thread.new {
  		line.chomp

		if line =~ /^link|^hardlink/
	    	    path = line.scan(/path=[^\s]*/).to_s.split("=")[1]
		    if path.match(/\//)
		        filetmp = path.split("/")
			file = filetmp[(filetmp.size-1)] 
		        fdir = path.gsub(file,"")
		    end
		    target = line.scan(/target=[^\s]*/).to_s.split("=")[1]
		    FileUtils.mkdir_p base + fdir
		    Dir.chdir base + fdir
		    if line =~ /^link/
			File.symlink(target,file)      
		    elsif line =~ /^hardlink/
			File.link(target,file)      
	    	    end
 		end
	    } 
 
	    while t.alive?
		print a.unshift(a.pop).last
		sleep 0.1
		print "\b"
	    end
	    t.exit
   	end
   	rescue EOFError
	    f.close
   	end
 	puts
