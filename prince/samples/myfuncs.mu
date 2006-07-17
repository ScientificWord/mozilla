// some simple test functions for Prince

myfunc1 := x -> x^2:

myfunc2 := proc(arg)
  save x;
begin
  delete x;
  if testtype(arg,DOM_INT) then
    matrix(arg,arg,x->1,Diagonal);
  elif testtype(arg,Type::Numeric) then
    numeric::int(exp(x^2),x=0..arg);
  elif testtype(arg,DOM_IDENT) then
    arg^2+1/arg^2;
  else
    {arg,indets(arg)};
  end_if;
end_proc: