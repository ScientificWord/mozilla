--  xelatex.applescript
--
--  Created by Barry MacKichan on 2011-10-06.
--  Copyright (c) 2011 MacKichan Software, Inc.. All rights reserved.
--

on run argv
	tell application "terminal"
		do script "cd " & item 1 of argv
		do script "pdflatex -jobname=" & item 3 of argv & " " & item 2 of argv in window 1
		do script "touch sentinel" in window 1
		do script "exit" in window 1
	end tell
end run



