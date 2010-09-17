September 17, 2010

This is the Linux version of Scientific Workplace 6.0. For this Alpha test
we are using a simple install script, install-sh.

This has been and run on SuSe 10. It should install without problems on Fedora
and Ubuntu distributions. There may be missing libraries on simple Fedora
installations.

INSTALLATI0N

1. Unpack the distribution file with the command:
      tar xjf <distribution-file>
   Note that bzip2 must be installed on your system to do this.
   
2. A new directory named swppro has been created in the directory where you
   unpacked.
      cd swppro
      
3. You must install as root. The installation currently takes 142 MB of disk
   space. Run this command to install.
      sudo ./install-sh
   You will see a few lines of output ending with
      Installation completed.
   
   Execute by typing
      swppro<cr>
      
   You can uninstall by running the command
      sudo ./uninstall-sh
      
4. The installation process makes a copy of the swppro directory at
   /usr/local/SWP-6.0. If space is a problem on your system, you can remove
   the swppro directory after the installation is finished. You can also 
   remove the .bz2 distrubution file. If you later want to uninstall,
     sudo /usr/local/SWP-6.0/uninstall-sh
