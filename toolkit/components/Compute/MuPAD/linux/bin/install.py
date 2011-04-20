#!/usr/bin/python
import os, sys
import string, re
import shutil
import getopt
import time
import smtplib
import cPickle
import commands
import  subprocess, shlex 
from optparse import OptionParser
from operator import indexOf

def report(logpath, message):
    "Write message to the log and print it to the console"
    print message
    lines = re.split( '\n', message )
    cmd = "echo "
    for l in lines :
	    args = l+' >>'+logpath
	    do_os_command( cmd, args )
    return

def do_os_command( cmd, args):
    "Execute a command in the operating system"
    ec = 0
    ec = subprocess.call( cmd+" "+args, shell=True, stderr=subprocess.STDOUT, env=os.environ )
    return ec

def run_command( cmd ):
    "Execute a command in the operating system. Takes a string."
    command = shlex.split( cmd )
    p = subprocess.Popen( command, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
    (result, error) = p.communicate()
    if result[-1] == '\n' :
	    result = result[:-1]
    return result
        
def setup_environment():
    env = {}
    env['homedir'] = os.environ['HOME']
    dir = os.path.join( env['homedir'], '.swppro')
    if  not os.path.isdir( dir ) :
	   os.mkdir( dir, 0770 )
    logfile = os.path.join( dir, 'install.log')
    if os.path.isfile( logfile ) :
	    os.remove( logfile )
    cmd = "touch"
    ec = do_os_command( cmd, logfile)
    env['logfile'] = logfile
    
    #  Set OS environment variables
    env['sysname'], nxx, rxx, vxx, env['machine'] = os.uname()
    return env

def linux_distribution( env ) :
    logfile = env['logfile']
    distro = 'unknown0'
    if os.path.isfile( '/etc/SuSE-release' ) :
	distro = 'suse'
    if os.path.isfile( '/etc/redhat-release' ) :
	distro = 'redhat'
    if os.path.isfile( '/etc/fedora-release' ) :
	distro = 'fedora'
    if os.path.isfile( '/etc/slackware-version' ) :
	distro = 'slackware'
    if os.path.isfile( '/etc/debian_version' ) :
	distro = 'debian'
    env['distro'] = distro
    return distro

def has_bin_directory() :
    result = 'no'
    dir = os.path.join( env['homedir'], 'bin' )
    if os.path.isdir( dir ) :
	    result = 'yes'
    return result

def find_installation( env ) :
    base = env['base']
    logfile = env['logfile']
    installdir = os.path.join( base, "SWP-6.0")
    if os.path.isdir( installdir ) :
	    return installdir
    installdir = os.path.join( env['homedir'], "SWP-6.0")
    if os.path.isdir( installdir ) :
	    env['base'] = installdir
	    return installdir
    return ''

def no_installation( env ) :
    logfile = env['logfile']
    report( logfile, "Could not find the current installation.")
    report( logfile, "Use the -b option to indicate where it is. Exiting.")
    exit()
    
def verify_root( env, curuser ):
    if curuser == 'root' :
	return
    report ( env['logfile'], "You must do this install as root. Exiting.")
    exit()
    
def move_app_tree( env ) :
    logfile = env['logfile']
    BASE = env['base']
    CURDIR = env['CURDIR']
    INSTALLNAME = env['INSTALLNAME']
    CURUSER = env['CURUSER']
    if 	BASE == '/usr/local' :
	    verify_root( env, CURUSER)
    ( parent, dir) = os.path.split( CURDIR )
    if dir == 'swppro' :
	    os.chdir( parent )
	    CURDIR = parent
    dst = os.path.join( BASE, INSTALLNAME)
    if os.path.exists( dst ) :
        shutil.rmtree( dst)
    shutil.copytree( DIRNAME, dst)

def path_from_list( lst ):
    path = os.path.join( '/', lst[0])
    for i in lst[1:] :
	    path = os.path.join( path, i)
    return path

def list_from_path( path ) :
    path = os.path.expanduser( path )
    result = re.split( '/', path)
    return result
    
env = setup_environment()

logfile = env['logfile']
cmd = 'whoami'
env['CURUSER'] = run_command( cmd )
env['CURDIR'] = os.getcwd()
env['startdir'] = env['CURDIR']
env['INSTALLNAME']='SWP-6.0'
env['DIRNAME']='swppro'
env['LICENSEDIR']='.mupadpro40'
env['LICENSEFILE']='.mupadpro40/.license'
env['DISTRO'] = linux_distribution( env )
env['HASBIN'] = has_bin_directory()

CURUSER = env['CURUSER']
CURDIR = env['CURDIR']
INSTALLNAME = env['INSTALLNAME']
DIRNAME = env['DIRNAME']
LICENSEDIR = env['LICENSEDIR']
LICENSEFILE = env['LICENSEFILE']
DISTRO = env['DISTRO']
HASBIN = env['HASBIN']
BASE='/usr/local'
if not CURUSER == 'root' :
    BASE = env['homedir']
env['base'] = BASE
env['bindirbase'] = BASE

# Get options
parser = OptionParser()
parser.add_option( "-b", "--base", dest="base", default="none", help="Installation directory.")
parser.add_option( "-u", dest="update", action="store_true", default=False, help="Update installation")
parser.add_option( "-r", dest="remove", action="store_true", default=False, help="Remove installation")
(options, arguments) = parser.parse_args()
if options.base == 'none' :
    BASE = env['base']
else :
    if options.base == '.' or options.base == '' :
        BASE = env['homedir']
    if not options.base == '' :
        BASE = options.base
if options.update and options.remove :
    report( logfile, "Remove and Update options cannot be combined.")
    exit()
env['base'] = BASE

report( logfile, "Current user is "+CURUSER  )
report( logfile, "Linux distribution: "+DISTRO )

if options.update :
    report( logfile, "Update current installaion.")

if options.remove :
    report( logfile, "Remove current installaion.")

# Update
if options.update :
    CURUSER = env['CURUSER']
    CURDIR = env['CURDIR']
    DIRNAME = env['DIRNAME']
    INSTALLNAME = env['INSTALLNAME']
    installdir = find_installation( env )
    if installdir == '' :
	    no_installation( env )
    move_app_tree( env )
    report( logfile, "Update completed at "+installdir)
    exit()

# Uninstall
if options.remove :
    CURUSER = env['CURUSER']
    CURDIR = env['CURDIR']
    DIRNAME = env['DIRNAME']
    INSTALLNAME = env['INSTALLNAME']
    installdir = find_installation( env )
    if installdir == '' :
	no_installation( env )
    if HASBIN :
	progname = 'swppro'
    else :
    	progname = 'runswp'
    os.chdir( os.path.join( env['bindirbase'], 'bin'))
    os.remove( progname)
    os.chdir( '..' )
    if os.path.isdir( installdir ) :
    	    shutil.rmtree( installdir )
    try:
	realhome = os.path.join( '/home', os.environ['SUDO_USER'])
    except:
	realhome = os.path.join( '/home', os.environ['USER'])
    licensedir = os.path.join( realhome, LICENSEDIR )
    testfile = os.path.join( licensedir, 'SWP' )
    if os.path.isfile( testfile ) :
    	    shutil.rmtree( licensedir )
    profile = os.path.join( realhome, '.swppro')
    if os.path.isdir( profile ) :
    	    shutil.rmtree( profile )
    report( logfile, "Uninstall complete." )
    exit()
    
# Install
# Check for existing installation
installdir = find_installation( env )
if not installdir == '' :
    # whine
    report( logfile, "SWP-6.0 is already installed.")
    report( logfile, "Use -u option to update. Exiting.")
    exit()
# move application files
move_app_tree( env )
# Create startup script
if HASBIN == 'yes' :
    os.chdir( os.path.join( env['bindirbase'], 'bin'))
    progname = 'swppro'
else :
    progname = 'runswp'
string = "#!/bin/sh\nLD_LIBRARY_PATH="+BASE+"/SWP-6.0/MuPAD/linux/lib:$LD_LIBRARY_PATH "+BASE+"/SWP-6.0/swppro"
exe = open( progname, 'w+')
exe.write( string )
exe.close()
os.chmod( progname, 0755)
# move license file
path2 = sys.argv[0]
try:
	realhome = os.path.join( '/home', os.environ['SUDO_USER'])
except:
	realhome = os.path.join( '/home', os.environ['USER'])
lst = list_from_path( os.path.join(realhome, path2))
if not lst[0] :
	lst = lst[1:]
licensedir = os.path.join( '/', lst[0], lst[1], '.mupadpro40')
copydir = path_from_list( lst[:-2])
if not os.path.isdir( licensedir ) :
	os.mkdir( licensedir )
	do_os_command( 'touch ', os.path.join( licensedir, 'SWP'))
	src = os.path.join( copydir, 'swppro', 'MuPAD', 'license', 'license')
	dst = os.path.join( licensedir, '.license')
	shutil.copy( src, dst)
report( logfile, "Installation completed.")
