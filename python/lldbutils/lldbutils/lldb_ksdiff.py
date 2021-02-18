# lldb_ksdiff.py v2.4.210127
# to manually install put this in your ~/.lldbinit
# """
# command script import /Applications/Kaleidoscope.app/Contents/Resources/Integration/scripts/lldb_ksdiff.py
# """
import lldb
import subprocess
from subprocess import PIPE
import tempfile
import os
import sys
import re
import time
import math
import string

diff_tool_path = '/usr/local/bin/ksdiff'


def kspo(debugger, command, result, dict):
	"""\
kspo - pipe objects/object descriptions to Kaleidoscope for inspection and diffing
Usage:
	kspo <object expression>
	
Examples:
	# Compare the descriptions of two array objects
	kspo myArray1
	kspo myArray2

	# Compare two view hierarchies by dumping recursive descriptions
	kspo view1.recursiveDescription
	kspo view2.recursiveDescription

	# Compare two images by dumping pngs in UIKit
	kspo UIImagePNGRepresentation([UIImage imageNamed:@"FlashOff"])
	kspo UIImagePNGRepresentation([UIImage imageNamed:@"FlashOn2"])
	"""
	
#  if not command or command.isspace():
# result.SetError(f"Missing arguments\n")
# return
	
	tmp_file_name = dump_object_result_to_tmp_file(debugger, command, result)
	
	if not tmp_file_name:
		return
	
	send_file_to_kaleidoscope(result, "lldb %s" % (debugger.GetSelectedTarget()), tmp_file_name)
	

def ksp(debugger, command, result, dict):
	"""\
ksp - pipe the output of an arbitary lldb command to Kaleidoscope for inspection and diffing
Usage:
	ksp <any lldb command>
	
Examples:
	# Compare register state
	ksp register read
	ksp register read
	"""
	if not command or command.isspace():
		result.SetError("Missing arguments\n%s" %(ksp.__doc__))
		return
	
	tmp_file_name = dump_command_result_to_tmp_file(debugger, command, result)
	
	if not tmp_file_name:
		return
	send_file_to_kaleidoscope(result, "lldb %s" % (debugger.GetSelectedTarget()), tmp_file_name)

def send_file_to_kaleidoscope(result, title, filename):
	args = [diff_tool_path, "-l", title, filename, "-s"]
	subl = subprocess.Popen(args, stdout=PIPE, stderr=PIPE)
	subl.wait()
	
#	result.SetError(f"CLI: {args} {time.monotonic_ns()}")
	os.remove(filename)
	
	if subl.returncode != 0:
		out, err = subl.communicate()
		result.SetError("CLI: %s\nksdiff error: %s" % (args,err.decode('utf-8')))
		return
	

def dump_object_result_to_tmp_file(debugger, expression, result):
	target = debugger.GetSelectedTarget()
	process = target.GetProcess()
	mainThread = process.GetThreadAtIndex(0)
	currentFrame = mainThread.GetSelectedFrame()
	
	
	expr_options = lldb.SBExpressionOptions()
	expr_options.SetIgnoreBreakpoints(False);
	expr_options.SetFetchDynamicValue(lldb.eDynamicCanRunTarget);
	expr_options.SetTimeoutInMicroSeconds (30*1000*1000) # 30 second timeout
	expr_options.SetTryAllThreads (True)
	expr_options.SetUnwindOnError(True)
	expr_options.SetGenerateDebugInfo(True)
#	expr_options.SetLanguage (lldb.eLanguageTypeObjC)
	expr_options.SetCoerceResultToId(False)
	
	result_value = currentFrame.EvaluateExpression("%s" % (expression), expr_options)
	
#	print(f"Value Type: {result_value.GetValueType()}")
#	print(f"Type Name: {result_value.GetTypeName()}")
#	print(f"Declaration: {result_value.GetDeclaration()}")
#	print(f"Display Type Name: {result_value.GetDisplayTypeName()}")
#	print(f"Summary: {result_value.GetSummary()}")
#	print(f"Object Description: {result_value.GetObjectDescription()}")
#	print(f"Pointer: {result_value.GetLocation()}")


#	print(f"Target: {debugger.GetSelectedTarget()}")
	
	error = result_value.GetError()
	if not error.Success():
		result.SetError(error)
		return
	
	target_filename = target_filename_for_value(result_value)
	
	result_content = result_value.GetObjectDescription() or result_value.GetSummary()
#	if !result_content:
#		result_content = result_value.GetSummary()
	output = "%s" % (result_content)
	
	m = re.search(r'^<(([0-9A-Fa-f]{2,8} ?)+)>$', output)
	
	bytes = bytearray.fromhex(m.group(1)) if m else output.encode('utf8')
	
	target_filename += ".dump" if m else ".txt"
	
	filepath = os.path.join(tempfile.gettempdir(),target_filename)
	
	o_file = open(filepath, "bw")
#	o_file = tempfile.NamedTemporaryFile(delete=False, prefix=target_filename, suffix=(".dump" if m else ".txt"))
	o_file.write(bytes)
	o_file.close()

	return o_file.name

def target_filename_for_value(value):
	time_part = time.strftime("%H-%M-%S")
	nano_time_part = "%s" % (round(math.modf(time.perf_counter())[0],4))[1:]
	
	helpful_part = value.GetDisplayTypeName().encode('ascii','ignore').decode('ascii').replace(' ','_')
	valid_chars = "-_.{0}{1}".format(string.ascii_letters, string.digits)
	helpful_part = "".join(ch for ch in helpful_part if ch in valid_chars)
	helpful_part = helpful_part.strip("_ \t\n-")
	
	target_filename = "%s_%s%s" % (helpful_part,time_part,nano_time_part)
	return target_filename	

def dump_command_result_to_tmp_file(debugger, command, result):
	res = lldb.SBCommandReturnObject()
	comminter = debugger.GetCommandInterpreter()
	comminter.HandleCommand(command, res)
	if not res.Succeeded():
		result.SetError("command '%s'\n failed with %s" % (command, res.GetError()))
		return
	output = res.GetOutput()
	o_file = tempfile.NamedTemporaryFile(delete=False)
	o_file.write(output.encode('utf-8'))
	o_file.close()
	return o_file.name


def init(debugger):
    debugger.HandleCommand('command script add -f lldbutils.lldb_ksdiff.kspo kspo')
    debugger.HandleCommand('command script add -f lldbutils.lldb_ksdiff.ksp ksp')


