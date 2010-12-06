#/ /usr/bin/bash
set -x
echo $0 $1 $2 > /Users/barry/dump
echo "$2"  >> /Users/barry/dump
echo "$2"  >> /Users/barry/dump
cd "$2" >> /Users/barry/dump
pwd  >> /Users/barry/dump
cd "$2"
set +x
otfinfo -a /Library/Fonts/*.otf > bigfontlist.txt



			  
			  