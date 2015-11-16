set args = WScript.Arguments
num = args.Count

Set WshShell = WScript.CreateObject("WScript.Shell")
dpi = WshShell.RegRead("HKCU\Control Panel\Desktop\WindowMetrics\AppliedDPI")
'Catch the error
Select Case Err
  Case 0:
    'Error Code 0 = 'success'
    cropping = (96/dpi)*100
  Case Else
    'Any other error code is a failure code
    cropping = 100
End Select

WshShell.Run """" & args.Item(1) & "\\convert.exe" & """ " & args.Item(0) & "\\graphics\\" & args.Item(2) & ".bmp " & " -crop " & cropping & "%%x+0+0! " & args.Item(0) & "\\gcache\\" & args.Item(2) & ".png", 0, False
