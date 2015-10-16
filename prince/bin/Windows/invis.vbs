set args = WScript.Arguments
num = args.Count

Set WshShell = WScript.CreateObject("WScript.Shell")
dpi = WshShell.RegRead("HKCU\Control Panel\Desktop\WindowMetrics\AppliedDPI")
cropping = (96/dpi)*100

WshShell.Run """" & args.Item(2) & "\\convert.exe" & """ " & args.Item(1) & "\\graphics\\" & args.Item(3) & ".bmp " & " -crop " & cropping & "%%x+0+0! " & args.Item(1) & "\\gcache\\" & args.Item(3) & ".png", 0, False