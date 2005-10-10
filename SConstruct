#! /usr/bin/env python

"""
help       -> scons -h
compile    -> scons
clean      -> scons -c
install    -> scons install
uninstall  -> scons -c install
configure  -> scons configure debug=full extraincludes=/usr/local/include:/tmp/include prefix=/usr/local

Run from a subdirectory -> scons -u (or export SCONSFLAGS="-u" in your ~/.bashrc)

Documentation is in bksys/design
"""

# QUICKSTART (or look in bksys/design)
# cd kdelibs
# tar xjvf bksys/scons-mini.tar.bz2
# ./scons

# WARNING
# Nothing will work out of the box until the config.h system is ready...

# Suggested plan ? (ita)
# * convert some kde4 code as a base for further developments : ok
# * find the variables for the configuration modules          : ok
# * create the config.h system                                : ok
# * write the configuration modules                           : ~
# * fix the installation process and the kobject builder      :
# * use an automatic converter for the rest of the tree       :

###################################################################
# LOAD THE ENVIRONMENT AND SET UP THE TOOLS
###################################################################

## We assume that 'bksys' is our admin directory
import sys, os
sys.path.append('bksys')

## Import the main configuration tool
from generic import configure
config = {
	'modules'  : 'generic lowlevel compiler kde4 liblua50',
	'builddir' : 'build', # put all object files under 'build/'
	'config.h' : 1, # mechanism should be ok
	'rpath'    : 1, # incomplete
	'bootstrap': 1, # incomplete
	'colorful' : not os.environ.has_key('NOCOLORS'), # only with scons >= 0.96.91 - now miniscons
}

# and the config.h
env=configure(config)

# now the kdemacros (TODO put this into a bootstrap section somehow ?)
#dest = open(env.join('build','kdemacros.h'), 'w')
#dest.write('#include <kdemacros.h.in>\n')
#dest.close()

#import os
#try:
#   os.mkdir('build'+os.sep+'kjs')
#except OSError:
#   pass

#dest = open(env.join('build','kjs','global.h'), 'w')
#dest.write('#include "global.h.in"\n')
#dest.close()

###################################################################
# SCRIPTS FOR BUILDING THE TARGETS
###################################################################
subdirs="""
libyzis
qyzis
nyzis
kyzis
scripts
syntax
translations
"""

# TODO this will not stay like this ..
dirs=[] # the dirs to process are appended to this var in the loop below
for dir in subdirs.split():
	jdir =  env.join('#build', dir)
	try:
		os.mkdir(jdir[1:])
	except OSError:
		pass
	env.BuildDir( jdir, dir, duplicate=0)
	dirs.append( jdir )
env.subdirs(dirs)

###################################################################
# CONVENIENCE FUNCTIONS TO EMULATE 'make dist' and 'make distclean'
###################################################################

# if you forget to add a version number, the one in the file VERSION will be used instead
env.dist('yzis', '1.0.0pre0')

