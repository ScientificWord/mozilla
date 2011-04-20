Aprin 20, 2011

This is the Linux version of Scientific Workplace 6.0. For this Beta test
we are using a simple Python install script, install.py.

This has been installed and run on SuSe 10, SuSe 11, and Ubuntu. It should install without problems on Fedora. 
There may be missing libraries on simple Fedora
installations, particularly on 64-bit Fedora systems. More detailed Fedora installs will
be forthcoming.

INSTALLATI0N

1. Unpack the distribution file in your home directory with the command:
      tar xjf <distribution-file>
   Note that bzip2 must be installed on your system to do this.
   
2. A new directory named swppro has been created in your home directory.
      
3. You may install as root or as yourself. The installation currently takes 137 MB of disk
   space. Run this command to install as root.
      sudo swppro/install.py
   You will see a few lines of output ending with
      Installation completed.
   
   Execute by typing
      swppro<cr>
      
   You can uninstall by running the command
      sudo swppro/install.py -r
      
   If you simply want to update an existing installation,
      sudo swppro/install.py -u
      
4. The installation process makes a copy of the swppro directory at
   /usr/local/SWP-6.0. If space is a problem on your system, you can remove
   the swppro directory after the installation is finished. You can also 
   remove the .bz2 distrubution file. If you later want to uninstall,
      sudo /usr/local/SWP-6.0/install.py -r
     
4. To install in your own home directory:
      swppro/install.py
      
   The SWP-6.0 directory will be created in your home directory, and the executable
   file swppro will be put in your bin directory if you have one. If you don't have
   a bin directory, the executable will also be in your home directory, but it will 
   be called runswp (since there is already something called swppro there).
