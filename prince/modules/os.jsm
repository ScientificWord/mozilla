var EXPORTED_SYMBOLS = ["getOS"];


function getOS(window)
{
  var os;

  switch(window.navigator.platform)
  {
  case 'Win32':
   os = 'win';
   break;
  case 'MacPPC':
  case 'MacIntel':
   os = 'osx';
   break;
  case 'Linux i686':
  case 'Linux i686 (x86_64)':
   os = 'linux';
   break;
  default:
   throw('Error: Unknown OS ' + navigator.platform);
   os = "??";
  }
  return os;
}
