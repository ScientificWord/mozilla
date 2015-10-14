set args = WScript.Arguments
num = args.Count

Set WshShell = WScript.CreateObject("WScript.Shell")

WshShell.Run """" & args.Item(2) & "\\convert.exe" & """ " & args.Item(1) & "\\graphics\\" & args.Item(3) & ".bmp " & " -crop 66.67%%x+0+0! " & args.Item(1) & "\\gcache\\" & args.Item(3) & ".png", 0, False