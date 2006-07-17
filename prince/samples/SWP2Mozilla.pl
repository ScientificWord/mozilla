# Converts output from SWP for Netscape into form better for editting
# Usage SWP2Mozilla.pl < output.xml > output.xhtml

while (<>) {
  s/\<mml:/\</g;
  s/\<\/mml:/\<\//g;
  s/\<math/\<math xmlns=\"http:\/\/www.w3.org\/1998\/Math\/MathML\"/g;
  s/\<link\ href.*\>/\<link href=\"TestComputation.css\" rel=\"stylesheet\" type=\"text\/css\"\/\>/g;
  print $_;
}