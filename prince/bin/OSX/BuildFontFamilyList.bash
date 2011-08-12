# build list of family names of ttf and otf fonts
cd ${2:-~/Library/Application\ Support/SWPPro/Profiles/*default}
fc-list : file family | grep -e 'ttf' -e 'otf' | sed 's/^.*: //' | sort | uniq >fontfamilies.txt
