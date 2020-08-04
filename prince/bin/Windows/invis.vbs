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

'WshShell.Run """" & args.Item(1) & "\\convert.exe" & """ " & args.Item(0) & "\\graphics\\" & args.Item(2) & ".bmp " & " -crop " & cropping & "%%x+0+0! " & args.Item(0) & "\\gcache\\" & args.Item(2) & ".png", 0, true

'BBM I modified this for more general use when exporting a plot snapshot. The graphics and gcache directories now
'must be included in the parameters when calling this for the default case -- creating snapshots for printing

'Item 0 is the path of utility programs'
'Item 1 is the source bmp file directory'
'Item 2 is the destination png file directory'
'Item 3 is the base file name'

WshShell.Run """" & args.Item(0) & "\convert.exe" & """ " & args.Item(1) & args.Item(3) & ".bmp " & " -crop " & cropping & "%%x+0+0! " & args.Item(2) & args.Item(3) & ".png", 0, true


