:: --------------------------------------------------------------------
::  Wrapper script to start a UniConvertor application once it is installed
::
::  Copyright (C) 2007-2010 Igor E. Novikov
::
::  This library is free software; you can redistribute it and/or
::  modify it under the terms of the GNU Lesser General Public
::  License as published by the Free Software Foundation; either
::  version 2.1 of the License, or (at your option) any later version.
::
::  This library is distributed in the hope that it will be useful,
::  but WITHOUT ANY WARRANTY; without even the implied warranty of
::  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
::  Lesser General Public License for more details.
::
::  You should have received a copy of the GNU Lesser General Public
::  License along with this library; if not, write to the Free Software
::  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
:: ---------------------------------------------------------------------

@echo off  
Set curDir=%~dp0

PATH=%curDir%;%curDir%DLLs;%PATH%
set PYTHONPATH=

if "%~3"=="" (
   pyVM -c "from uniconvertor import uniconv_run; uniconv_run();" "%~1" "%~2"
) else (
   pyVM -c "from uniconvertor import uniconv_run; uniconv_run();" "%~1" "%~2" "%~3"
)
