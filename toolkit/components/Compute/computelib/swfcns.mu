//*  swfcns.mu - computation support functions for use with MuPAD 4.0
//*
//* Copyright (c) 1999 - 2007  MacKichan Software, Inc.
//*
//*                    All Rights Reserved

//Fixes printing of matrices, intervals and RootOf.
Pref::mackichan(TRUE);

//Assign non-zero assumptions to units
assume(mQU,Type::Positive):
assume(kgQU,Type::Positive):
assume(sQU,Type::Positive):
assume(AQU,Type::Positive):
assume(tmpKQU,Type::Positive):
assume(radQU,Type::Positive):
assume(srQU,Type::Positive):
assume(cdQU,Type::Positive):
assume(molQU,Type::Positive):

tcipwr := proc(xx,nn)
begin
  if (testtype(xx,Dom::Matrix) and (linalg::ncols(xx) <> linalg::nrows(xx))) then
    map( xx, (x) -> (x^nn) );
  else
    xx^nn;
  end_if;
end_proc:

tcipowermod := proc(xx,nn,mm)
  local II,m;
begin
  if (testtype(xx,Dom::Matrix)) then
    if (nops(tciindets(xx)) = 0) then
      II := Dom::Matrix( Dom::IntegerMod(mm) );
      m := II::convert(xx);
    else
      II := Dom::Matrix(
              Dom::Fraction(
                Dom::MultivariatePolynomial([op(tciindets(xx))],Dom::IntegerMod(mm)) ) );
      m :=II::convert(xx);
    end_if;
    matrix::convert(m^nn);  // might FAIL if nn negative
  else
    powermod(xx,nn,mm);
  end_if;
end_proc:
  
tcimpy2 := proc(A,B)
begin
  if (testtype(A,linalg::VectorOf(Type::AnyType)) and testtype(B,Dom::Matrix)) then
    if ((linalg::ncols(A) <> linalg::nrows(B)) and (linalg::nrows(A) = linalg::nrows(B))) then
      linalg::transpose(A)*B;
    else
      A*B;
    end_if
  else
    A*B;
  end_if;
end_proc:

tcimpy := proc()
  local t;
begin
  if args(0) = 0 then 
    1 
  else
    t := 1;
    for i from 1 to args(0) do 
      t := tcimpy2(t,args(i))
    end_for;
    eval(t)
  end_if
end_proc:

tciadd2 := proc(a,b)
begin
  if( testtype(a,Dom::Matrix) and testtype(b,Dom::Matrix) ) then
    a+b                                    
  elif( testtype(a,Dom::Matrix) ) then
    if ( linalg::ncols(a) = linalg::nrows(a) ) then 
      a+(b*matrix(linalg::ncols(a),linalg::ncols(a),1,Diagonal))
    else
      a+b*matrix(linalg::nrows(a),linalg::ncols(a),1)
    end_if;
  elif( testtype(b,Dom::Matrix) ) then
    if ( linalg::ncols(b)=linalg::nrows(b) ) then
      a*matrix(linalg::ncols(b),linalg::ncols(b),1,Diagonal)+b
    else
      a*matrix(linalg::nrows(b),linalg::ncols(b),1)+b
    end_if;
  else
    a+b
  end_if; 
end_proc:  

tciadd := proc()
  local i,t;
begin
   t := args(1);
   for i from 2 to args(0) do 
     t := tciadd2(t,args(i)) 
   end_for;
   eval(t);
end_proc:

tciexpand := proc()
begin
  if( testtype(args(1),Dom::Matrix) ) then
    return(map(args(),expand));
  else 
    return(expand(args()));
  end_if;
end_proc:

tcifactor := proc() 
  local i,j,x,m;
begin
   if args(0) = 0 or args(0) > 2 then
     error("Wrong number of arguments")
   end_if;
   x:=args(1);
   m:=0;
   if args(0) = 2 then
     m:=args(2)
   end_if;
   if( testtype(args(1),Dom::Matrix) )  then
     map(x,tcifac2,m);
   else
     tcifac2(x,m)
   end_if;
end_proc:

tcifac2 := proc(x,m) 
  local t, ans;
begin
  t := eval(x);
  if m <> 0 then 
    return( expr(factor(poly(t,[op(tciindets(t))],IntMod(m)))) )
  elif ( testtype(t,DOM_INT) ) then 
    ans := ifactor(t) 
  else  
    ans := factor(t) 
  end_if;
end_proc:

tcigcd := proc() 
begin
    gcd(args());
end_proc:

tcilcm := proc() 
begin
    lcm(args());
end_proc:

tcicsgn := proc(z)
begin
  if testtype(z,Dom::Complex) = FALSE then
    procname( args() );
  elif float(Re(z)) > 0 then   // convert to float in case of PI/EULER
     1;
  elif float(Re(z)) < 0 then
    -1;
  elif float(Im(z)) > 0 then
     1;
  elif float(Im(z)) < 0 then
    -1;
  else
    0;
  end_if;
end_proc:

tcipsi := proc()
begin
  if args(0) = 1 then
    psi(args(1))
  elif args(0) = 2 then
    psi(args(1),args(2))
  end_if;
end_proc:

tcilog := proc()
begin
  if (args(0) = 2) then
    log(args(1),args(2));
  else
    ln(args(1));
  end_if;
end_proc:

tciargument := proc(x)
begin
  if (testtype(x,Dom::Complex)) then
    return(arg(Re(x),Im(x)));
  else
    return(0);
  end_if
end_proc:

tcihermite := proc(M)
begin
  linalg::hermiteForm(M);
end_proc:
                
tcismith := proc(A)
begin
  linalg::smithForm(A);
end_proc:

tciffgausselim := proc(A)
  local II,m;
begin
  II := Dom::Matrix(Dom::Integer);
  m := II::convert(A);
  if (m = FAIL) then
    if (nops(tciindets(A)) = 0) then
      error("Matrix entries must be polynomials over the rationals" );
    end_if;
    II := Dom::Matrix( Dom::MultivariatePolynomial([op(tciindets(A))],Dom::Rational) );
    m :=II::convert(A);
    if (m = FAIL) then
      error( "Matrix entries must be polynomials over the rationals" );
    end_if;
  end_if;
  linalg::gaussElim(m);
end_proc:

tcidprod := proc(A,B) 
begin
  if (testtype(A,Dom::Matrix)) then
    if (testtype(B,Dom::Matrix)) then
      return( linalg::scalarProduct( A, B ) );
    elif (testtype(B,DOM_LIST)) then
      return( linalg::scalarProduct( A, matrix(B) ) );
    end_if;
  elif (testtype(A,DOM_LIST)) then
    if (testtype(B,Dom::Matrix)) then
      return( linalg::scalarProduct( matrix(A), B ) );
    elif (testtype(B,DOM_LIST)) then
      return( linalg::scalarProduct( matrix(A), matrix(B) ) );
    end_if;
  end_if;
  tcimpy(A,B);  // last ditch possibility 
end_proc:

tcixprod := proc(A,B)
begin
  if (testtype(A,Dom::Matrix)) then
    if (testtype(B,Dom::Matrix)) then
      return( linalg::crossProduct( A, B ) );
    elif (testtype(B,DOM_LIST)) then
      return( linalg::crossProduct( A, matrix(B) ) );
    end_if;
  elif (testtype(A,DOM_LIST)) then
    if (testtype(B,Dom::Matrix)) then
      return( linalg::crossProduct( matrix(A), B ) );
    elif (testtype(B,DOM_LIST)) then
      return( linalg::crossProduct( matrix(A), matrix(B) ) );
    end_if;
  end_if;
  tcimpy(A,B);  // last ditch possibility
end_proc:

tciindets := proc(a)
begin
  indets(a) minus Type::ConstantIdents;
end_proc:

tcidiv := proc() 
  local x,a,b,c,s,g,f;     //fraction is args(1)/args(2) = g/f
begin
  if args(0) = 3 then 
    x := args(3);
  elif args(0) = 2 then        
    // find the polynomial variable x
    a := tciindets(args(1));  
    b := tciindets(args(2));
    if ( nops(a) = 1 ) then 
      x := op(a,1)
    elif ( nops(b) = 1 ) then 
      x := op(b,1)
    else
      c := a intersect b;
      if nops(c) = 1 then 
        x := op(c,1);
      else 
        return( hold(needvars) ) 
      end_if;
    end_if; 
  else
    error("polynomial divide needs two or three arguments")
  end_if;
  g := poly(args(1),[x]);  // convert to univariate polynomials
  f := poly(args(2),[x]);
  if( degree(g,x) < degree(f,x) ) then
    return(args(1)/args(2))
  elif( divide( g,f,Exact) <> FAIL ) then
    return( expr(divide( g,f,Exact)) )
  else
    return( expr(divide( g,f,Quo )) + expr(divide( g,f,Rem ))/args(2) )
  end_if
end_proc:

tcicompanion := proc(p)
  local x,a;
begin
  delete x;
  // find polynomial variable
  if args(0) = 2 then 
    x := args(2);
  else
    a := tciindets(p);
    if (nops(a) = 1) then
      x := op(a,1);
    else
      return(hold(needvars)) 
    end_if;
  end_if;
  linalg::companion(p,x); 
end_proc:

tciroot := proc() 
  local var, vset, p, l, r;
begin
  p := args(1);     // polynomial is args(1)
  if args(0) = 2 then 
    var := args(2)
  elif args(0) = 1 then                
    // Find the polynomial variable x
    vset := tciindets(p);
    if ( nops(vset) = 1 ) then 
      var := op(vset,1)
    else 
      return(hold(needvars)) 
    end_if;
  else
    error("polynomial root finder needs one or two arguments")
  end_if;
  if degree(p,var) < 5  then
    l := solve(p,var,Multiple,MaxDegree=4);
  else
    l := float(hold(solve)(p,var,Multiple));
  end_if;
  if (testtype(l,Dom::Multiset)) then
    op( map( [op(l)], r -> r[1]$r[2] ) );
  else
    // ignore multiple roots
    solve(p,var,MaxDegree=2);
  end_if;
end_proc:

list2sum := proc()
  local ans, sv, m;
begin
  if (nops(args()) = 1) then
    return(expr2text(args()[1]));
  end_if;
  ans := "";
  if args()[1] = -1 then
    ans := ans."-";
  elif args()[1] <> 1 then
    ans := ans.expr2text(args()[1]);
  end_if;
  for i from 2 to nops(args()) do
    sv := expr2text(args()[i]);
    if (stringlib::match( sv, "-" ) <> 1) then   // put + if next term doesn't start with -
      ans := ans."+";
    end_if;
    ans := ans.sv;
  end_for;
  return(ans)
end_proc:

tcisort := proc() 
  local var, vset;    // polynomial is args(1)
begin
  if args(0) = 2 then 
    var := op(args(2))  // convert from set to sequence
  elif args(0) = 1 then              
    // find the polynomial variable x
    vset := tciindets(args(1));
    if( nops(vset) = 1 ) then 
      var := op(vset,1)
    else 
      return( hold(needvars) ) 
    end_if;
  else
    error("polynomial sort needs one or two arguments")
  end_if;
  map( polylib::sortMonomials(args(1),[var]), expr );
  list2sum( op(%) );
end_proc:

tcicollect := proc(expr,vars)
begin
    if testtype(vars,DOM_SET) then
        collect(expr,[op(vars)]);
    else
        collect(expr,vars);
    end_if;
end_proc:

tcicombine := proc(x,y) 
begin
  if (testtype(x,Dom::Matrix)) then
    return(map(x,tcicomb2,y));
  else
    return(tcicomb2(x,y));
  end_if;
end_proc:

tcicomb2 := proc(x,y) 
begin
  if( y=0 ) then // trig
    combine(x,sincos)
  elif( y=1 ) then // exp
    combine(x,exp)
  elif( y=2 ) then  // power (_power doesn't have the desired effect)
    combine(x)
  elif( y=3 ) then // log (used to use ln)
    combine(x,log)
  elif( y=5 ) then // Psi
    combine(x)
  elif( y=6 ) then // arctan
    combine(x,arctan)
  elif( y=7 ) then // hyperbolic trig
    combine(x,sinhcosh)
  else  // combine with no additional parameters#             
    combine(x)
  end_if
end_proc:

getvar := proc(e) 
  local i,j,r;
begin
  if (nops(e) = 1) then
    if (testtype(e,"function")) then
      return( getvar(op(e,1)) );
    elif (testtype(e,Dom::Matrix)) then
      for i from 1 to linalg::nrows(e) do
        for j from 1 to linalg::ncols(e) do
          r := getvar( e[i,j] );
          if (r <> 0) then
            return(r);
          end_if;
        end_for;
      end_for;
    elif (testtype(e,DOM_IDENT)) then
      return(e);
    end_if;
  elif (nops(e) > 1) then
    for i from nops(e) downto 1 do
      if (testtype(op(e,i),DOM_IDENT)) then
        return(op(e,i));
      else
        r := getvar( op(e,i) );
        if (r <> 0) then
          return(r);
        end_if;
      end_if;
    end_for;
  end_if;
  return(0);
end_proc:

tciparfrac := proc() 
begin
  if (args(0)=1) then
    if (nops(tciindets(args(1))) = 1) then
      return(partfrac(args(1)));
    else
      return(hold(needvars));
    end_if;
  else
    return(partfrac(args(1),args(2)));
  end_if;
end_proc:

tciint := proc()
begin
  if (testtype(args(1),Dom::Matrix)) then
      return(map(args(1),tciint1,op(args(),2..args(0))));
    else
      return(tciint1(args()));
    end_if;
end_proc:

// If we get too many warning messages, consider  intlib::printWarnings(FALSE)
tciint1 := proc()
  local var;
begin
  if args(0) = 2 then 
    return( simplify(int( args(1),args(2) )) ); 
  end_if;
  if (args(0) = 4) then
    return( simplify(int( args(1),args(2)=args(3)..args(4) )) );
  end_if;
  var := getvar( args(1) );
  if ((args(0) = 1) and (var <> 0)) then
    return( simplify(int(args(1),var)) );
  end_if;
  if ((args(0) = 3) and (var <> 0)) then
    return( simplify(int( args(1),var=args(2)..args(3) )) );
  end_if;
  error( "Wrong number or format of arguments for integration." );
end_proc:

tcisum := proc() 
  local var, mapit;
begin
  if args(0) = 0 then 
    error("Sum has no argument");
  else 
    mapit := testtype(args(1),Dom::Matrix);
  end_if;
    
  if args(0) = 2 then 
    if mapit then 
       return(map(args(1),sum,args(2)));
    else 
      return(sum(args(1),args(2))); 
    end_if;
  end_if;
    
  if args(0) = 4 then 
    if mapit then 
      return(map(args(1),sum,args(2)=args(3)..args(4)));
    else 
      return(sum(args(1),args(2)=args(3)..args(4))); 
    end_if;
  end_if;
  var := getvar(args(1));
  if args(0) = 1 and var <> 0 then
    if mapit then
      return(map(args(1),sum,var));
    else
      return(sum(args(1),var));
    end_if;
  end_if;
  if args(0) = 3 then 
    if mapit then
      return(map(args(1),sum,var=args(2)..args(3)));
    else
      return(sum(args(1),var=args(2)..args(3)));
    end_if;
  end_if;
  return( args(1) );
end_proc:

tciprod := proc() 
  local var, mapit;
begin
  if args(0) = 0 then 
    error("Product has no argument") ;
  else 
    mapit := testtype(args(1),Dom::Matrix);
  end_if;

  if args(0) = 2 then 
    if mapit then 
      return( map(args(1),product,args(2)) );
    else 
      return( product(args(1),args(2)) );
    end_if;
  end_if;
  if args(0) = 4 then 
    if mapit then 
      RETURN( map(args(1),product,args(2)=args(3)..args(4)) )
    else 
      return( product(args(1), args(2)=args(3)..args(4)) ) 
    end_if;
  end_if;
  var := getvar( args(1) );
  if args(0) = 1 and var <> 0 then
    if mapit then
      return( map(args(1),product,var) );
    else
      return( product(args[1],var) );
    end_if;
  end_if;
  if args(0) = 3 then 
    if mapit then
      return( map(args(1),product,var=args(2)..args(3)) );
    else
      return( product(args(1), var=args(2)..args(3)) );
    end_if;
  end_if;
  return( args(1) );
end_proc:

tciremoveunits := proc(s)
  local st;
begin
  st := expr2text( s );
  if (length(st) <= 2) then  // units have at leave 3 characters
    s
  elif (substring(st,length(st)-1,2) <> "QU") then
    s
  else
    null()
  end_if
end_proc:


//Note how these global variables are set by the main program,
// possibly before this file is loaded.
// Default values for solve options
//   tciMaxDegree := 3;
//   tciPrincipalValue := FALSE;
//   tciIgnoreSpecialCases := FALSE;

// Add solve options to solve command
tcisolve2 := proc(stuff,vars)
  local opts, sys, myvars;
begin
  opts := MaxDegree=tciMaxDegree;
  if (tciPrincipalValue) then
    opts := opts, PrincipalValue;
  end_if;
  if (tciIgnoreSpecialCases) then
    opts := opts, IgnoreSpecialCases;
  end_if;
  if testtype(stuff,DOM_SET) and nops(stuff)=1 then
    sys := op(stuff);
  else
    sys := stuff;
  end_if;
  if testtype(vars,DOM_SET) and nops(vars)=1 then
    myvars := op(vars);
  else
    myvars := vars;
  end_if;
  solve( sys, myvars, opts );
end:

// global variable used to parametrize solutions just like Maple
_t;

tcisolve := proc() 
  local ii, vars, b, a, n, v, ans;
begin
  if args(0) < 1  then 
    return(0) 
  end_if;
  if args(0) = 1  then
    vars := map( tciindets( args(1) ), tciremoveunits );
    if nops(vars) = nops(args(1)) then
         ans := tcisolve2(args(1),vars)
    else
         return( hold(needvars) )
    end_if;
  elif( testtype(args(1),Dom::Matrix) and testtype(args(2),Dom::Matrix) ) then
    if (linalg::nrows(args(1)) > linalg::ncols(args(1))) then
      ans := numeric::leastSquares( args(1), args(2), Symbolic );
      ans := ans[1];
    else
      ans := linalg::matlinsolve( args(1), args(2), [_t[i] $ i=1..linalg::ncols(args(1))] );
    end_if;
  elif( testtype(args(1),DOM_SET) and nops(args(1)) = 1 and testtype(args(2),DOM_SET) and nops(args(2)) = 1 ) then
      ans := tcisolve2(op(args(1)),op(args(2)));
    else
    ans := tcisolve2(args(1),args(2));
  end_if;
    if (nops(ans) = 0) or (stringlib::match(expr2text(ans),"solve") = 1) then      
        return( hold(NoSol) );
    elif (type(ans) <> piecewise) and testtype(ans,DOM_SET) then
        return( eval(op(ans)) );  // convert from set to sequence
    else
        return( eval(ans) );
    end_if;
end_proc:

tciisolve := proc(eqs) 
  local eq, v;
begin
  if nops(eqs) > 1 then
    error("tciisolve can only solve one equation");
  end_if;
  eq := op(eqs,1);  // eqs is a set.  currently only solve polys
  v := getvar(eq);
  solve( eq, v, Domain=Dom::Integer );
end_proc:

tciassume := proc()
begin
  assume(args(1), args(2));
  getprop(args(1));
end_proc:


tciadditionally := proc(v,p)
begin
    assumeAlso(v, p);
    getprop(v);
end_proc:


tcidsolve := proc(eqns,vars)
  local ans;
begin
  if (nops(vars) = 1) then
    ans := solve( ode( eqns, op(vars) ) );
  else
    ans := solve( ode( eqns, vars ) );
  end_if;
    if (nops(ans) = 0) or (stringlib::match(expr2text(ans),"solve") = 1) then      
        null();  // should return(NoSol)
  else
    ans;
  end_if;
end_proc:

// for Laplace solutions of ODEs.
tcildsolve := proc(eqns,vars)
  local ans;
begin
  if (nops(vars) = 1) then
    ans := ode::laplace( eqns, op(vars) );
  else
    ans := ode::laplace( eqns, vars );
  end_if;
    if (nops(ans) = 0) or (stringlib::match(expr2text(ans),"transform::laplace") = 1) then
        null();  // should return(NoSol)
  else
    ans;
  end_if;
end_proc:

tcisdsolve := proc(eqns,vars)
  local ovar;
begin
  if (nops(vars) = 1) then
    ovar := op(vars);
    ode::series( ode( eqns, ovar ), op(ovar) );
  else
    ode::series( ode( eqns, vars ) );
  end_if
end_proc:

//for numerical solution of ODEs.
// Old Example
//   tcindsolve({diff(q1yODE(q1t),q1t)=sin(q1yODE(q1t)*q1t),q1yODE(0)=3},[q1yODE(q1t)],[hold(q1y)],4);
// New Example
//   tcindsolve({q1yODE'(q1t)=sin(q1yODE(q1t)*q1t),q1yODE(0)=3},[q1yODE(q1t)],[hold(q1y)],4);
// second order
//   tcindsolve({q1zODE''(q1t)=sin(q1zODE(q1t)*q1t),D(q1zODE)(0)=1,q1zODE(0)=0.3},[q1zODE(q1t)],[hold(q1z)],5);
// four functions
//   tcindsolve({q2xODE'(q2t)=q2xODE,q2yODE'(q2t)=q2yODE,q2zODE'(q2t)=q2zODE,q2wODE'(q2t)=q2wODE,q2xODE(0)=1,q2yODE(0)=1,q2zODE(0)=1,q2wODE(0)=1},[q2xODE(q2t),q2yODE(q2t),q2zODE(q2t),q2wODE(q2t)],[hold(q2x),hold(q2y),hold(q2z),hold(q2w)],6);
   
tcindsolve := proc(eqns,vars,names,procnum)
  option escape;
  local svars, evars, ivar, n;
begin
  ivar := op(vars[1],1);
  svars := misc::subExpressions(eqns,"D");  // thanks to "Tips and Tricks" in the source.
  svars := [op(map(svars, field -> op(field)(ivar)))];
  evars := {op(svars)} minus {op(vars)}; // svars includes higher order functions
  svars := vars . [op(evars)];  // maintain correspondence between vars and names

  map( names, _delete );       // erase function names

  ftp[procnum] := numeric::odesolve2( numeric::ode2vectorfield(eqns,svars) );
  for n from 1 to nops(names) do
    evalassign(names[n],
               subs(proc() name `#f`; begin
                       if map({float(args(i)) $ i=1..args(0)},domtype)<>{DOM_FLOAT} then
                          names[`#n`]( args() );
                         else
                          op(ftp[procnum](args(i) $ i=1..args(0)), `#n`)
                         end_if
                       end_proc,
                    `#n` = n, `#f` = names[n]),
               1);
  end_for;

  ftprefs[procnum] := nops(names);
  return( names );
end_proc:


//  Handle reference counting on procedures which are solutions to
//  ODEs. When you 'delete' it once for each dependent variable
//  it finally goes away. ... this doesn't work right yet.

tcidelodeproc := proc(procnum)
begin
    if(ftprefs[procnum] > 1) then
        ftprefs[procnum] := ftprefs[procnum] - 1;
    else
        delete ftprefs[procnum];
        delete ftp[procnum];
    end_if;
end_proc:


// 2-norm of a square matrix
// expensive!
twonorm := proc(A)
begin
  if (linalg::ncols(A) > 3) then
    sqrt( tcimax( numeric::eigenvalues( A*tcihtranspose(A) ) ) );
  else
    sqrt( tcimax( numeric::eigenvalues( A*tcihtranspose(A) ) ) );
  end_if;
end_proc:

// Pull out two norm or square matrix, since not implemented
tcinorm := proc(a,t)
begin
  if testtype(a,Dom::Matrix) then
    if (linalg::ncols(a) = linalg::nrows(a) and t = 2) then
      return( twonorm(a) );
    elif (linalg::nrows(a) = 1) then     // vectors come in like this
      return( norm(linalg::transpose(a),t) );
    end_if;
  end_if;
  norm(a,t);
end_proc:

tcispectralradius := proc(A)
  local startvec, l;
begin
  startvec := [random(-50..50)() $ i=1..linalg::nrows(A)];
  l := numeric::spectralradius( A, startvec, 100);
  l[1];
end_proc:

//  Condition number of a matrix
tcicond := proc(A)
begin
  float(tcinorm(A,2)*tcinorm(A^(-1),2));
end_proc:

tcideftest := proc(a)
begin
  if linalg::isPosDef(a) = TRUE then
    return("posdef");
  else
    return("indef");
  end_if;
end_proc:

tciprime := proc(f,vv,n)
  local  ans, ftemp, ii, jj, var, nn;
  save x;      
begin
  var := op(vv);
  if (n=0)  then
    return(f(var));
  end_if;
  if (testtype(n,DOM_LIST)) then
    return(D([op(n)],f)(var));
  end_if;      
  delete x;    // need free variable
  if (not testtype(f(x),DOM_PROC)) then
    return((D@@n)(f)(var));
  end_if;      
  if (testtype(f(x),Dom::Matrix)) then      
    ftemp := map(f(x),diff,x$n);
  else 
    ftemp := simplify(diff(f(x),x$n));      
  end_if;
  if( stringlib::contains(expr2text(ftemp),"diff")) then      
    error("MuPAD cannot do this differentiation");      
  end_if;
  if (testtype(var,Dom::Matrix)) then
    ans := matrix(linalg::nrows(var),linalg::ncols(var));      
    for ii from 1 to linalg::nrows(var) do
      for jj from 1 to linalg::ncols(var) do
        x := var[ii,jj];
        ans[ii,jj] := eval(ftemp);      
      end_for;
    end_for;
    return(eval(ans));
  else
    return(subs(eval(ftemp),x=var));      
  end_if;
end_proc:
     
tcidiff := proc()
  local i;
begin
  if ( testtype(args(1),Dom::Matrix) or testtype(args(1),DOM_LIST) or testtype(args(1),DOM_SET) ) then
    map(args(1),diff,args(i)$i=2..args(0));
  else 
    simplify(diff(args()));
  end_if;
end_proc:

tcisimplify := proc()
begin
  if ( hastype(args(1),Type::AlgebraicConstant) ) then
    simplify( simplify(args(),sqrt) );  // AKA radsimp
  else 
    simplify(args());
  end_if;
end_proc:

tcirewrite := proc(ntype,ex)
begin
  if (ntype = 1) then
    numeric::rationalize(ex);
  elif (ntype = 2) then  // Float
    float(ex);
  elif (ntype = 3) then  // Exponential
    rewrite(ex,exp);
  elif (ntype = 4) then
    rewrite(ex,fact);
  elif (ntype = 5) then
    rewrite(ex,gamma);
  elif (ntype = 6) then
    rewrite(ex,ln);
  elif (ntype = 7) then
    rewrite(ex,sincos);
  elif (ntype = 8) then
    rewrite(ex,sinhcosh);
  elif (ntype = 9) then
    rewrite(ex,tan);
  elif (ntype = 10) then  // Polar
    abs(ex)*exp(I*arg(Re(ex),Im(ex)));
  elif (ntype = 11) then  // Rectangular
    rectform(ex);
  elif (ntype = 12) then  // Normal
    normal(ex);
  elif (ntype = 13) then
    rewrite(ex,sin);
  elif (ntype = 14) then
    rewrite(ex,cos);
  elif (ntype = 15) then
    rewrite(ex,arcsin);
  elif (ntype = 16) then
    rewrite(ex,arccos);
  elif (ntype = 17) then
    rewrite(ex,arctan);
  elif (ntype = 18) then
    rewrite(ex,arccot);
  else
    error("Unknown rewrite type");
  end_if;
end_proc:

tciequationstomatrix := proc()
  local l,vars;
begin
  vars := args(args(0));
  if ( testtype(vars,Dom::Matrix) ) then
    vars := [op(vars)];
  end_if;
  if ( testtype(args(1),Dom::Matrix) ) then
    l := [op(args(1))];
  elif ( testtype(args(1),DOM_LIST) or testtype(args(1),DOM_SET) ) then
    l := args(1);
  else
    l := [args(1..args(0)-1)];
  end_if;
  linalg::expr2Matrix( l, vars );
end_proc:

SWPmatrix := proc(m,n,lis) 
begin
  lis;
end_proc:

// MuPAD doesn't yet have matrix2Expr
tcimatrixtoequations := proc(A,v)
  local vars, c, nrows, r, S;
begin
  nrows := linalg::nrows(A);
  vars := linalg::transpose(v);
  if (linalg::nrows(A) = linalg::ncols(A)) then
    c := matrix(linalg::nrows(A),1);
    S := A;
  else
    c := linalg::col(A,linalg::ncols(A));
    S := linalg::delCol(A,linalg::ncols(A));
  end_if;
  { op(linalg::row(S,r)*vars,1) = c[r] $ r = 1..nrows }; //linalg::scalarProduct is wrong
end_proc:


tcizmat := proc(nrows,ncols)
begin
  matrix(nrows,ncols);
end_proc:

tciimat := proc(nrows,ncols)
begin
  tciBand( [1], nrows, ncols );
end_proc:

tciJBmat := proc(diag,nrows,ncols)
begin
  tciBand( [0,diag,1], nrows, ncols );
end_proc:

tciBand := proc(entrylist,nrows,ncols)
  local A, d;
begin
  d := max(nrows,ncols);
  A := matrix(d,d,entrylist,Banded);
  if( d > ncols ) then
    A := linalg::delCol(A,(ncols+1)..d);
  elif( d > nrows ) then
    A := linalg::delRow(A,(nrows+1)..d);
  end_if;
  eval(A);
end_proc:

tcistatdata := proc(M)
  local A;
begin
  A := M;
  if linalg::nrows(A) = 1 then
    A := linalg::transpose(A);   // vectors come in this way
  end_if;
  if testtype(A[1,1],DOM_IDENT) then
    A := linalg::delRow(A,1);
  end_if;
  return(A);
end_proc:

tcimean1 := proc(A)  // A is a 1 column matrix
begin
  stats::mean( [op(A)] );
end_proc:

tcimean := proc()
  local A;
begin
  if testtype(args(1),Dom::Matrix) then
    A := tcistatdata( args(1) );
    if linalg::ncols(A) = 1 then
      tcimean1( A );
    else
      map( linalg::col(A,1..linalg::ncols(A)), tcimean1 );
    end_if;
  elif testtype(args(1),DOM_LIST) then
    stats::mean( args(1) );
  else
    stats::mean( [args()] );
  end_if;
end_proc:

tcimedian1 := proc(A)  // A is a 1 column matrix
begin
  stats::median( [op(A)] );
end_proc:

tcimedian := proc()
  local A;
begin
  if testtype(args(1),Dom::Matrix) then
    A := tcistatdata( args(1) );
    if linalg::ncols(A) = 1 then
      tcimedian1( A );
    else
      map( linalg::col(A,1..linalg::ncols(A)), tcimedian1 );
    end_if;
  elif testtype(args(1),DOM_LIST) then
    stats::median( args(1) );
  else
    stats::median( [args()] );
  end_if;
end_proc:

tcimode1 := proc(A)  // A is a 1 column matrix
begin
  stats::modal( [op(A)] );
end_proc:

tcimode := proc()
  local A;
begin
  if testtype(args(1),Dom::Matrix) then
    A := tcistatdata( args(1) );
    if linalg::ncols(A) = 1 then
      tcimode1( A );
    else
      map( linalg::col(A,1..linalg::ncols(A)), tcimode1 );
    end_if;
  elif testtype(args(1),DOM_LIST) then
    stats::modal( args(1) );
  else
    stats::modal( [args()] );
  end_if;
end_proc:

tcigeometricmean1 := proc(A)  // A is a 1 column matrix
begin
  stats::geometricMean( [op(A)] );
end_proc:

tcigeometricmean := proc()
  local A;
begin
  if testtype(args(1),Dom::Matrix) then
    A := tcistatdata( args(1) );
    if linalg::ncols(A) = 1 then
      tcigeometricmean1( A );
    else
      map( linalg::col(A,1..linalg::ncols(A)), tcigeometricmean1 );
    end_if;
  elif testtype(args(1),DOM_LIST) then
    stats::geometricMean( args(1) );
  else
    stats::geometricMean( [args()] );
  end_if;
end_proc:

tciharmonicmean1 := proc(A)  // A is a 1 column matrix
begin
  stats::harmonicMean( [op(A)] );
end_proc:

tciharmonicmean := proc()
  local A;
begin
  if testtype(args(1),Dom::Matrix) then
    A := tcistatdata( args(1) );
    if linalg::ncols(A) = 1 then
      tciharmonicmean1( A );
    else
      map( linalg::col(A,1..linalg::ncols(A)), tciharmonicmean1 );
    end_if;
  elif testtype(args(1),DOM_LIST) then
    stats::harmonicMean( args(1) );
  else
    stats::harmonicMean( [args()] );
  end_if;
end_proc:

tcisdev1 := proc(A)    // A is a 1 column matrix
begin
  stats::stdev( [op(A)], Sample );
end_proc:

tcisdev := proc()
  local A;
begin
  if testtype(args(1),Dom::Matrix) then
    A := tcistatdata( args(1) );
    if linalg::ncols(A) = 1 then
      tcisdev1( A );
    else
      map( linalg::col(A,1..linalg::ncols(A)), tcisdev1 );
    end_if;
  elif testtype(args(1),DOM_LIST) then
    stats::stdev( args(1), Sample );
  else
    stats::stdev( [args()], Sample );
  end_if;
end_proc:

tcimdev1 := proc(A)    // A is a 1 column matrix or a list
  local l, k, N, m, s;
begin
  if testtype(A,Dom::Matrix) then
    l := [op(A)];
  else
    l := A;
  end_if;
  N := nops(l);
  m := stats::mean(l);
  s := 0;
  for k from 1 to N do
    s := s + abs(l[k]-m);
  end_for;
  s / N;
end_proc:

tcimdev := proc()
  local A;
begin
  if testtype(args(1),Dom::Matrix) then
    A := tcistatdata( args(1) );
    if linalg::ncols(A) = 1 then
      tcimdev1( A );
    else
      map( linalg::col(A,1..linalg::ncols(A)), tcimdev1 );
    end_if;
  else
    tcimdev1( [args()] );
  end_if;
end_proc:

tcivariance1 := proc(A)   // A is a 1 column matrix
begin
  stats::variance( [op(A)] );
end_proc:

tcivariance := proc()
  local A;
begin
   if testtype(args(1),Dom::Matrix) then
    A := tcistatdata( args(1) );
    if linalg::ncols(A) = 1 then
      tcivariance1( A );
    else
      map( linalg::col(A,1..linalg::ncols(A)), tcivariance1 );
    end_if;
  elif testtype(args(1),DOM_LIST) then
    stats::variance( args(1) );
  else
    stats::variance( [args()] );
  end_if;
end_proc:

tcimoment1 := proc(A,r,o)    // A is a 1 column matrix or a list
  local l, k, N, a, s;
begin
  if testtype(A,Dom::Matrix) then
    l := [op(A)];
  else
    l := A;
  end_if;
  if (testtype(o,Type::Numeric)) then
    a := o;
  else
    a := stats::mean(l);
  end_if;
  N := nops(l);
  s := 0;
  for k from 1 to N do
    s := s + float((l[k]-a)^r);
  end_for;
  s / N;
end_proc:

tcimoment := proc()
  local A, r, orig;
begin
  if args(0) < 3 then
    error("moment needs 3 arguments")
  end_if;
  r := args(args(0)-1);
  orig := args(args(0));
  if testtype(args(1),Dom::Matrix) then
    A := tcistatdata( args(1) );
    if linalg::ncols(A) = 1 then
      tcimoment1( A,r,orig );
    else
      map( linalg::col(A,1..linalg::ncols(A)), tcimoment1,r,orig );
    end_if;
  elif testtype(args(1),DOM_LIST) then
     tcimoment1(args(1),r,orig);
  else
     A := [args(1..args(0)-2)];
     tcimoment1(A,r,orig);
  end_if;
end_proc:

tciquantile1 := proc(A,alpha)  // A is a 1 column matrix
begin
  stats::empiricalQuantile([op(A)])(alpha,Averaged);
end_proc:

// note strange return list
tciquantile := proc()
  local A, alpha;
begin
  if args(0) < 2 then
    error("quantile needs 2 arguments")
  end_if;
  alpha := args(args(0));
  if testtype(args(1),Dom::Matrix) then
    A := tcistatdata( args(1) );
    if linalg::ncols(A) = 1 then
      alpha, tciquantile1( A, alpha );
    else
      alpha, map( linalg::col(A,1..linalg::ncols(A)), tciquantile1, alpha );
    end_if;
  elif testtype(args(1),DOM_LIST) then
    alpha, stats::empiricalQuantile(args(1))(alpha,Averaged);
  else
    alpha, stats::empiricalQuantile([args(1..args(0)-1)])(alpha,Averaged);
  end_if;
end_proc:

tcicorr := proc(A)
  local n, ans, i, j, list1, list2;
begin
  if not testtype(A,Dom::Matrix) then
    error("selection must be a matrix")
  end_if;

  A := tcistatdata(A);
  n := linalg::ncols(A);
  ans := matrix(n,n);
  for i from 1 to n do
    for j from 1 to n do
      list1 := linalg::col(A,i);
      list2 := linalg::col(A,j);
      ans[i,j] := stats::correlation([op(list1)],[op(list2)]);
    end_for;
  end_for;
  float(ans):
end_proc:

tcicovar2 := proc(l1,l2)
  local m1, m2, N, s;
begin
  if (nops(l1) <> nops(l2)) then
    error("lists must be same size");
  end_if;
  N := nops(l1);
  m1 := stats::mean(l1);
  m2 := stats::mean(l2);
  s := 0;
  for k from 1 to N do  // sum should work, but this is probably more efficient
    s := s + float((l1[k]-m1)*(l2[k]-m2));
  end_for;
  s/(N-1);
end_proc:

tcicovar := proc(A)
  local n, m, ans, i, j, list1, list2;
begin
  if not testtype(A,Dom::Matrix) then
    error("selection must be a matrix")
  end_if;

  A := tcistatdata(A);
  n := linalg::ncols(A);
  m := linalg::nrows(A);
  ans := matrix(n,n);
  for i from 1 to n do
    for j from 1 to n do
      list1 := linalg::col(A,i);
      list2 := linalg::col(A,j);
      ans[i,j] := tcicovar2([op(list1)],[op(list2)]);
      if (n > 1) then
        ans[i,j] = ans[i,j]*m/(m-1);
      end_if;
    end_for;
  end_for;
  float(ans);
end_proc:


tcipolyfit := proc(A,deg,depvar)
  local key, b, c, ans, S, i;
  save a;
begin
  if not testtype(A,Dom::Matrix) then
    error("selection must be a matrix")
  end_if;
  if linalg::ncols(A) <> 2 then
    error("selection must be a 2 column matrix")
  end_if;
  
  if testtype(A[1,1],DOM_IDENT) then
    key := [op(linalg::row(A,1))];
    A := linalg::delRow(A,1);
  else
    key := [x,y];  // bogus if x,y have values
  end_if;
  if deg >= linalg::nrows(A) then
    deg := linalg::nrows(A)-1;
  end_if;
  if depvar = 1 then    // last column is dependent variable
    ans := subsop( a[i]*key[1]^i $ i=0..deg, 0=_plus );
    S   := [ a[i] $ i=0..deg ];
    c := stats::reg( [op(linalg::col(A,1))], [op(linalg::col(A,2))], expr(ans), [key[1]], S );
    c := c[1];
    return( key[2] = subsop( c[i+1]*key[1]^i $ i=0..deg, 0=_plus ) );
  else                   // first column is dependent variable
    ans := subsop( a[i]*key[2]^i $ i=0..deg, 0=_plus );
    S   := [ a[i] $ i=0..deg ];
    c := stats::reg( [op(linalg::col(A,2))], [op(linalg::col(A,1))], expr(ans), [key[2]], S );
    c := c[1];
    return( key[1] = subsop( c[i+1]*key[2]^i $ i=0..deg, 0=_plus ) );
  end_if;
end_proc:

tcimregress := proc(A,hasconst,depvar)
  local key, b, c, cc, d, i, nc, dkey, ikeys, regdata, fitfunc, fittedfunc;
  save a, t;
begin
  if not testtype(A,Dom::Matrix) then
    error("selection must be a matrix")
  end_if;
  nc := linalg::ncols(A);
  if nc < 2 then
    error("selection must be a matrix with at least 2 columns")
  end_if;
  if not testtype(A[1,1],DOM_IDENT) then
    error("columns must be labelled")
  end_if;
  
  if depvar = 0 then  // first column is dependent variable
    A := linalg::swapCol( A, 1, nc )
  end_if;
  key := [op(linalg::row(A,1))];
  A := linalg::delRow(A,1);
  regdata := map( linalg::row(A,1..linalg::nrows(A)), (t) -> [op(t)] );
  dkey  := key[nc];
  ikeys := [op(key,1..nc-1)];
  if (nc = 2) and (hasconst = 1) then
    // special case where we can use stats::linReg
    cc := stats::linReg( regdata );
    c := cc[1];
    return(dkey = ikeys[1]*c[2] + c[1]);
  end_if;
  // now in the general case
  if nc > 2 then
    fitfunc := subsop( a[i]*ikeys[i] $ i=1..nc-1, 0=_plus );
  else
    fitfunc := a[1]*ikeys[1];
  end_if;
  if hasconst = 1 then  // linear
    c := stats::reg( regdata, fitfunc+a[nc], ikeys, [a[i] $ i=1..nc] );
    cc := c[1];
    if nc > 2 then
      fittedfunc := subsop( cc[i]*ikeys[i] $ i=1..nc-1, 0=_plus );
    else
      fittedfunc := cc[1]*ikeys[1];
    end_if;
    return(dkey = fittedfunc + cc[nc]);
  else                 // no constant term
    c := stats::reg( regdata, fitfunc, ikeys, [a[i] $ i=1..nc-1] );
    cc := c[1];
    if nc > 2 then
      fittedfunc := subsop( cc[i]*ikeys[i] $ i=1..nc-1, 0=_plus );
    else
      fittedfunc := cc[1]*ikeys[1];
    end_if;
    return(dkey = fittedfunc);
  end_if;
end_proc:


tciaprxarea := proc(e,vv,rnge,nint,atype)
  local var;
begin
  if vv = 0 then
    var := getvar(e)
  else
    var := vv
  end_if;
  if var = 0 then
    var := x
  end_if;
  if atype = 1 then  // left
    student::riemann( e, var=rnge, nint, Left );
  elif atype = 2 then    // midpoint
    student::riemann( e, var=rnge, nint, Middle );
  elif atype = 3 then    // right
    student::riemann( e, var=rnge, nint, Right );
  elif atype = 4 then // Simpson
    student::simpson( e, var=rnge, nint );
  elif atype = 5 then // trapezoid
    student::trapezoid( e, var=rnge, nint );
  else
    error( "Unknown integration method" );
  end_if;
end_proc:

tcipotential := proc(vec,vars)
  local poss;
begin
  poss := linalg::potential(vec,vars);
  if poss = FALSE then
    0;
  else
    poss;
  end_if;
end_proc:

tcivecpotent := proc(vec,vars)
begin
 linalg::vectorPotential(vec,vars);
end_proc:

tcijacobian := proc(a,vars)
begin
  linalg::jacobian(a,vars);
end_proc:

tcicurl := proc(a,vars)
begin
   linalg::curl(a,vars);
end_proc:

tcidiverge := proc(a,vars)
  local i;
begin
   if (args(0) = 2) then
     linalg::divergence(a,vars);
   else
     linalg::divergence( [args(i) $ i = 1..args(0)-1], args(args(0)) );
   end_if;
end_proc:

tcihessian := proc(a,vars)
begin
   linalg::hessian(a,vars);
end_proc:

tcigrad := proc(a,vars)
begin
   linalg::grad(a,vars);
end_proc:

tcilaplacian := proc(a,vars)
begin
   linalg::laplacian(a,vars);
end_proc:

tciwronskian := proc(a)
  local var;
begin
  delete var;
  // find differentiation variable
  if args(0) = 2 then 
    var := args(2);
  else
    var := tciindets(a);
    if (nops(var) = 1) then
      var := op(var,1);
    else
      return(hold(needvars)) 
    end_if;
  end_if;
  ode::wronskianMatrix([op(a)],var)
end_proc:

// MuPAD wants the part to be integrated rather than the part to differentiate
tcibyparts := proc(intf,v)
  local e;
begin
  e := op(intf,1);
  return(intlib::byparts(intf,e/v));
end_proc:

tcichangevar := proc(s,i)
begin
   return(intlib::changevar(i,s));
end_proc:

// solve unconstrained extrema problems
//  (this function is not very robust if solve returns something besides a set)
tciextrema1 := proc( eqn, vars )
  local cands, pts, p, sys, baseinterval;
begin
  sys := map( vars, (z) -> diff(eqn,z)=0 );
  pts := solve( sys, vars );
  if (type(pts) = "_in") then
    if (type(op(pts,2)) = Dom::ImageSet) then
      sys := {Dom::ImageSet::getElement(op(pts,2))};  // should get more elements!
      if sys = {FAIL} then
        return( [{},{}] );
      end_if;
    elif (type(op(pts,2)) = DOM_SET) then
      sys := op(pts,2);
    elif (type(op(pts,2)) = "_union") then
      baseinterval := Dom::Interval(0,2*PI);  // how to find period?
      sys := map( op(pts,2), (z) -> z intersect baseinterval );
    else
      return( [{},{}] );
    end_if;
    p := op(vars,1);  // can this only happen in univariate case?
    pts := map( sys, (z) -> [p=z] );
  elif (type(pts) = "solve") then
    return( [{},{}] );
  end_if;
  cands := {};
  for p in pts do
    cands := cands union {eval(subs(eqn,p))};
  end_for;
  return( [cands,pts] );
end_proc:

tciextrema := proc()
  local vars, ans, i;
begin
  if (args(0) >= 2 and nops(args(2)) > 0) then
    error( "Constrained extrema not yet available." );
  end_if;

  if (args(0) <= 2) then
    vars := tciindets(args(1));
    if args(0) >= 2 then
      vars := vars union tciindets(args(2));
    end_if;
    if nops(vars) = 1 then
      if args(0) = 1 then 
        ans := tciextrema1(args(1),vars);
      else
        error( "Constrained extrema not yet available." );
      end_if;
    else 
      if args(0) = 1 then 
        return( hold(needvars) );
      else
        return( hold(needconstrainedvars) )
      end_if;
    end_if;
  elif (args(0) = 3) then
    ans := tciextrema1(args(1),args(3));
  else
    error( "Wrong number of arguments" );
  end_if;

  if (nops(ans[1])=0) then
    return( hold(at) );  // just like Maple version
  else
    return( ans[1], hold(at), ans[2] );
  end_if;
end_proc:


tcifrob := proc(A)
  local L;
begin
   L := linalg::frobeniusForm(A,All);
   L[2],L[1],(L[2])^(-1);
end:

// have to fiddle to get off-diagonal entries above diagonal
tcijordan := proc(A)
  local x, q, j;
begin
  x := linalg::jordanForm(linalg::transpose(A),All);
  if x = FAIL then
    FAIL;
  else
    q := expand(op(x,2));
    j := expand(op(x,1));
    linalg::transpose(q^(-1)),linalg::transpose(j),linalg::transpose(q);
  end_if;
end_proc:

tciinverse := proc(A)
begin
  if (tcishouldfloat(A)) then
    numeric::inverse(A);
  else
    A^(-1);
  end_if;
end_proc:

tcideterminant := proc(A)
begin
  if (tcishouldfloat(A)) then
    numeric::det(A);
  else
    linalg::det(A);
  end_if;
end_proc:

tciexponential := proc(A)
begin
  if (tcishouldfloat(A)) then
    numeric::expMatrix(A);
  else
    exp(A);
  end_if;
end_proc:

tcihtranspose := proc(A)
begin
  conjugate(linalg::transpose(A));
end_proc:

tcisingvals := proc(A)
begin
  // a symbolic solution is also possible
  numeric::singularvalues(A);
end_proc:

tciSVD := proc(A)
  local mD,l,ll,lr;
begin
  l := numeric::singularvectors(A,NoErrors);
  ll := matrix( l[1] );  // force arrays to be matrices
  lr := matrix( l[3] );
  mD := matrix( linalg::ncols(ll),linalg::nrows(lr), l[2], Diagonal );
  ll, mD, tcihtranspose(lr);
end_proc:

tcicolspace := proc(A)
begin
  linalg::basis( linalg::col( A, 1..linalg::ncols(A) ));
end_proc:

tcirowspace := proc(A)
begin
  linalg::basis( linalg::row( A, 1..linalg::nrows(A) ));
end_proc:

tcifsolve := proc()
  local vars;
begin
  if args(0) = 2 then
    numeric::fsolve( args(), RestrictedSearch );
  else
    float( hold(solve)(args()) );
  end_if;
end_proc:

tcirsolve := proc(x,n)
  local v, ex, vars, funcname, ic, sol;
begin
  if (nops(n) > 1) then
    error( "Can't solve for more than one function yet." );
  end_if;
  if testtype(x,DOM_SET) then
    ex := op(x,1);
    ic := {op(x,2..nops(x))};
  else
    ex := x;
    ic := {};
  end_if;
  vars := tciindets(ex);  //hoping for the best (looking for argument variable)
  funcname := op(n,1)(op(vars,1));   // protect against the worst.
  for v in vars do
    funcname := op(n,1)(v);
    if (has({op(ex)},funcname)) then
      break;
    end_if;
  end_for;
  sol := solve( rec( ex, funcname, ic ) );
  { funcname = op(sol) };
end_proc:

tciterate := proc(f,a,n)
  local func, ii, vec, nextone;
begin
  if testtype(f,DOM_EXPR) then
    func := fp::expr_unapply(f);
  else
    func := f;
  end_if;
  vec := matrix(n+1,1);
  vec[1,1] := a;
  for ii from 1 to n do 
    nextone := eval(func(vec[ii,1]));
    if not testtype(nextone,Type::Numeric) then
      nextone := float(func(vec[ii,1]))
    end_if;
    vec[ii+1,1] := nextone; 
  end_for;
  return( vec );
end_proc:

tcimap := proc(expr,f)
  local func;
begin
  if testtype(f,DOM_EXPR) then
    func := fp::expr_unapply(f);
  else
    func := f;
  end_if;
  map(expr,func);
end_proc:

tcidual := proc(f,l,v)
  local a;
begin
  a := linopt::dual([l,f]);
  a[2],{op(a[1])};  // special, strange Maple form
end_proc:

tcimaximize := proc(f,l)
  local a;
begin
  a := linopt::maximize( [l,f] );
  if a[1] = OPTIMAL then
    a[2];
  else
    a[1];   // Maple returns nothing here
  end_if;
end_proc:

tciminimize := proc(f,l)
  local a;
begin
  a := linopt::minimize( [l,f] );
  if a[1] = OPTIMAL then
    a[2];
  else
    a[1];
  end_if;
end_proc:

// Return TRUE if this is really a numeric matrix and more suited for numerical computations instead of symbolic
tcishouldfloat := proc(m)
  local numericMode;
begin
  numericMode:= _lazy_or(
    contains(map({op(m)}, domtype), DOM_FLOAT),
    contains(map(select({op(m)}, x -> (domtype(x) = DOM_COMPLEX)), 
                 domtype@op, 2),
             DOM_FLOAT));
  if numericMode then
    if map({op(m)}, domtype@float) minus
       {DOM_FLOAT, DOM_COMPLEX} <> {} then
       numericMode:= FALSE;
    end_if;
  end_if;
  numericMode;
end_proc:

tciQR := proc(Z)
  local l;
begin
  if (tcishouldfloat(Z)) then
    l := numeric::factorQR(Z);
  else
    l := linalg::factorQR(Z);
  end_if;
  l[1], l[2];
end_proc:

tciLU := proc(Z)
  local P,p,l,i,L,U;
begin
  if (tcishouldfloat(Z)) then
    l := numeric::factorLU(Z);
  else
    l := linalg::factorLU(Z);
  end_if;
  p := l[3];
  P := matrix( nops(p), nops(p) ); // build permutation matrix
  for i from 1 to nops(p) do
    P[p[i],i] := 1;
  end_for;
  L := l[1];
  U := l[2];
  P, L, U;
end_proc:

tcicholesky := proc(Z)
  local R;
begin
  if (tcishouldfloat(Z)) then
    R := numeric::factorCholesky(Z,NoCheck);
  else
    R := linalg::factorCholesky(Z,NoCheck);
  end_if;
  R, linalg::transpose(R);
end_proc:

// make look like Maple output
tcieigenvalues := proc(M)
  local fM;
begin
  if (tcishouldfloat(M)) then
    op(numeric::eigenvalues(M));
  else
    op(linalg::eigenvalues(M));
  end_if;
end_proc:

tcieigenvectors := proc(M)
  local fM, i, vals, m, res;
begin
  if (tcishouldfloat(M)) then
    [vals,m,res] := numeric::eigenvectors(M);
    m := matrix(m);   // convert from ARRAY
    [vals[i],1,{linalg::col(m,i)}] $ i = 1..nops(vals);
  else
    m := linalg::eigenvectors(M);        // convert overall list to sequence
    if (m = FAIL) then
      FAIL;
    elif (nops(m) = 1) then
      op( map( m, (l) -> [l[1],l[2],{op(l[3])}] ) );  // convert eigenvector to set of such
    else
      map( op(m), (l) -> [l[1],l[2],{op(l[3])}] );  // convert list of eigenvectors to set of such
    end_if;
  end_if;
end_proc:

//convert a fraction to a mixed number
tcimixnum := proc(x)
  local quo, rem;
begin
  quo := trunc(x);
  rem := frac(x);
  if quo = 0 then
    return(rem);
  elif rem = 0 then
    return(quo);
  else
    return(quo,"#",rem)
  end_if;
end_proc:

tcimod2 := proc(x,p)
  local a;
begin
  a := tciindets(p);
  if (nops(a) = 0) then
    if testtype(x,Dom::Rational) then
      modp(x,p);
    elif testtype(x,"_equal") then
      map(x,tcimod2,p);
    elif testtype(x,DOM_EXPR) then
      mapcoeffs(x,tcimod2,p);
    else
      x;
    end_if;
  else
    divide(x,p,[op(a,1)],Rem);
  end_if;
end_proc:

tcimod := proc(x,p)
  local xx,xxx;
begin
  xx := normal(expand(x));
  if (testtype(xx,Dom::Matrix)) then
    map(xx,tcimod,p);
  elif nops(tciindets(p)) = 0 then
    if testtype(xx,"_power") then
      subsop(xx,1=tcimod(op(xx,1),p));  // mod the base
    elif testtype(xx,"_mult") then
      map(xx,tcimod,p);                 // mod the terms
    elif testtype(xx,DOM_EXPR) then
      mapcoeffs(xx,tcimod,p);
    else
      tcimod2(xx,p);
    end_if;
  else
    tcimod2(xx,p);
  end_if;
end_proc:

tcigamma := proc()
begin
  if args(0)=1 then
    gamma(args(1))
  elif args(0)=2 then
    igamma(args(1),args(2))
  end_if;
end_proc:

///////////////// Distribution Functions //////////////////////

tcinormaldist := proc()
  local x,mu,sigma;
begin
  x := args(1);
  if (args(0) = 3) then
    sigma := args(3);
    mu := args(2);
    if (float(sigma) < 0.0) then
      error( "bad argument" );
    end_if;
  else
    sigma := 1;
    mu := 0;
  end_if;
  (stats::normalCDF(mu,sigma^2))(x)
end_proc:

tcinormalden := proc(x,mu,sigma)
begin
  (stats::normalPDF(mu,sigma^2))(x)
end_proc:

tcinormalinv := proc(x,mu,sigma)
begin
  (stats::normalQuantile(mu,sigma^2))(x)
end_proc:


tcitden := proc(x,v)
begin
  (stats::tPDF(v))(x)
end_proc:

tcitdist := proc(x,v)
begin
  (stats::tCDF(v))(x)
end_proc:

tcitinv := proc(x,v)
begin
  (stats::tQuantile(v))(x)
end_proc:


tcichisquareden := proc(x,mu)
begin
  (stats::chisquarePDF(mu))(x)
end_proc:

tcichisquaredist := proc(x,mu)
  local c;
begin
  (stats::chisquareCDF(mu))(x)
end_proc:

tcichisquareinv := proc(x,mu)
begin
  (stats::chisquareQuantile(mu))(x)
end_proc:


tcifden := proc(x,n,m)
begin
  (stats::fPDF(n,m))(x)
end_proc:

tcifdist := proc(x,n,m)
begin
  (stats::fCDF(n,m))(x)
end_proc:

tcifinv := proc(x,n,m)
begin
  (stats::fQuantile(n,m))(x)
end_proc:


tciexpden := proc(x,mu)
begin
  (stats::exponentialPDF(0,1/mu))(x)
end_proc:

tciexpdist := proc(x,mu)
begin
  (stats::exponentialCDF(0,1/mu))(x)
end_proc:

tciexpinv := proc(x,mu)
begin
  (stats::exponentialQuantile(0,1/mu))(x)
end_proc:


tciweibullden := proc(x,a,b)
begin
  (stats::weibullPDF(a,b))(x)
end_proc:

tciweibulldist := proc(x,a,b)
begin
  (stats::weibullCDF(a,b))(x)
end_proc:

tciweibullinv := proc(x,a,b)
begin
  (stats::weibullQuantile(a,b))(x)
end_proc:


tcigammaden := proc(x,a,b)
begin
  (stats::gammaPDF(a,b))(x)
end_proc:

tcigammadist := proc(x,a,b)
begin
  (stats::gammaCDF(a,b))(x)
end_proc:

tcigammainv := proc(x,a,b)
begin
  (stats::gammaQuantile(a,b))(x)
end_proc:


tcibetaden := proc(x,v,w)
begin
  (stats::betaPDF(v,w))(x)
end_proc:

tcibetadist := proc(x,v,w)
  local den;
begin
  (stats::betaCDF(v,w))(x)
end_proc:

tcibetainv := proc(x,v,w)
begin
  (stats::betaQuantile(v,w))(x)
end_proc:


tcicauchyden := proc(x,a,b)
begin
  (stats::cauchyPDF(a,b))(x)
end_proc:

tcicauchydist := proc(x,a,b)
begin
  (stats::cauchyCDF(a,b))(x)
end_proc:

tcicauchyinv := proc(x,a,b)
begin
  (stats::cauchyQuantile(a,b))(x)
end_proc:


tciuniformden := proc(x,a,b)
begin
  (stats::uniformPDF(a,b))(x)
end_proc:

tciuniformdist := proc(x,a,b)
begin
  (stats::uniformCDF(a,b))(x)
end_proc:

tciuniforminv := proc(x,a,b)
begin
  (stats::uniformQuantile(a,b))(x)
end_proc:


tcihypden := proc(x,N,S,n) 
begin
  (stats::hypergeometricPF(N,S,n))(x)
end_proc:

tcihypdist := proc(x,N,S,n)
begin
  (stats::hypergeometricCDF(N,S,n))(x)
end_proc:

tcihypinv := proc(x,N,S,n)
  local tot,k;
begin
  (stats::hypergeometricQuantile(N,S,n))(x)
end_proc:


tcibinomialden := proc(x,n,p)
begin
  (stats::binomialPF(n,p))(x)
end_proc:

tcibinomialdist := proc(x,n,p)
begin
  (stats::binomialCDF(n,p))(x)
end_proc:

tcibininv := proc(x,n,p)
begin
  (stats::binomialQuantile(n,p))(x)
end_proc:


tcipoissonden := proc(k,mu)
begin
  (stats::poissonPF(mu))(k)
end_proc:

tcipoissondist := proc(x,mu)
  local k;
begin
  (stats::poissonCDF(mu))(x)
end_proc:

tcipoissoninv := proc(y,mu)
begin
  (stats::poissonQuantile(mu))(y)
end_proc:

///////////////// end Distribution Functions //////////////////


tcicharpoly := proc(M)
  local v, k, vars;
begin
  if args(0)=1 then
    vars := tciindets(M);
    for k in vars do   // crude check for names like q1X
      if (stringlib::contains(expr2text(k),"X")) then      
        return(hold(needvars));
      end_if;
    end_for;
    v := X;
  else
    v := args(2);
  end_if;
  linalg::charpoly(M,v);
end_proc:

tciminpoly := proc(M)
  local v, k, vars;
begin
  if args(0)=1 then
    vars := tciindets( M );
    for k in vars do   // crude check for names like q1X
      if (stringlib::contains(expr2text(k),"X")) then      
        return( hold(needvars) );
      end_if;
    end_for;
    v := X;
  else
    v := args(2);
  end_if;
  linalg::minpoly( M, v );
end_proc:

tciistrue := proc(eq)
begin
  if type(eq) = "_equal" then
    testeq( tcisimplify( normal(lhs(eq)) ), tcisimplify( normal(rhs(eq)) ), Steps=100 );
  else
    is( tcisimplify( normal(eq) ) );
  end_if;
end_proc:

// compute max of sequence, attempting to float args first
tcimax2 := proc()
  local k, l, m, n;
begin
  n := args(0);
  if (n <= 1) then
    return( args(1) );
  end_if;
  l := float(args(k))=args(k) $ k=1..n;
  m := max(op(l, [k, 1]) $ k=1..n);
  subs(m, select(l, has, op(m,k)) $ k=1..nops(m));
end_proc:

tcimax := proc()
begin
  if ( args(0)=1 ) then
    if (testtype(args(1),DOM_LIST) or testtype(args(1),DOM_SET)) then
      return( tcimax2( op(args(1)) ) ); 
    elif (testtype(args(1),Dom::Matrix)) then
      return( tcimax2( op(args(1)) ) ); 
    else
      return( args(1) );
    end_if;
  elif ( args(0)=2 ) then
    if (testtype(args(1),DOM_BOOL) and testtype(args(2),DOM_BOOL)) then
      return( args(1) or args(2) );
    elif (testtype(args(1),Type::Relation) and testtype(args(2),Type::Relation)) then
      return( is(args(1)) or is(args(2)) );
    elif (testtype(args(1),DOM_BOOL) and testtype(args(2),Type::Relation)) then
      return( args(1) or is(args(2)) );
    elif (testtype(args(1),Type::Relation) and testtype(args(2),DOM_BOOL)) then
      return( is(args(1)) or args(2) );
    else
      return( tcimax2(args()) );
    end_if;
  else
    return( tcimax2(args()) );
  end_if;
end_proc:

// compute min of sequence, attempting to float args first
tcimin2 := proc()
  local k, l, m, n;
begin
  n := args(0);
  if (n <= 1) then
    return( args(1) );
  end_if;
  l := float(args(k))=args(k) $ k=1..n;
  m := min(op(l, [k, 1]) $ k=1..n);
  subs(m, select(l, has, op(m,k)) $ k=1..nops(m));
end_proc:

tcimin := proc()
begin
  if ( args(0)=1 ) then
    if (testtype(args(1),DOM_LIST) or testtype(args(1),DOM_SET)) then
      return( tcimin2( op(args(1)) ) ); 
    elif (testtype(args(1),Dom::Matrix)) then
      return( tcimin2( op(args(1)) ) ); 
    else
      return( args(1) );
    end_if;
  elif (args(0)=2) then
    if (testtype(args(1),DOM_BOOL) and testtype(args(2),DOM_BOOL)) then
      return( args(1) and args(2) );
    elif (testtype(args(1),Type::Relation) and testtype(args(2),Type::Relation)) then
      return( is(args(1)) and is(args(2)) );
    elif (testtype(args(1),DOM_BOOL) and testtype(args(2),Type::Relation)) then
      return( args(1) and is(args(2)) );
    elif (testtype(args(1),Type::Relation) and testtype(args(2),DOM_BOOL)) then
      return( is(args(1)) and args(2) );
    else
      return( tcimin2(args()) );
    end_if;
  else
    return( tcimin2(args()) );
  end_if;
end_proc:

tcirandmat := proc(nr,nc,mtype,range)
  local A,j,k;
begin
  if (testtype(range,"_range") or testtype(range,Dom::Matrix) or testtype(range,DOM_LIST)) then
    A := matrix(nr,nc);
    for j from 1 to nr do
      for k from 1 to nc do
        A[j,k] := random(op(range,1)..op(range,2))();
      end_for;
    end_for;
  elif (testtype(range,DOM_SET)) then
    A := matrix(nr,nc);
    for j from 1 to nr do
      for k from 1 to nc do
        A[j,k] := tcirandfromset(range);
      end_for;
    end_for;
  else  // how did we get here? 
    A := linalg::randomMatrix(nr,nc,Dom::Integer);
  end_if;
  if mtype = 2 then
    // symmetric
    for j from 2 to nr do
      for k from 1 to j-1 do
        A[j,k] := A[k,j];
      end_for;
    end_for;
  elif mtype = 3 then
    // antisymmetric 
    for j from 2 to nr do
      for k from 1 to j-1 do
        A[j,k] := -A[k,j];
      end_for;
    end_for;
  elif mtype = 4 then
    // triangular
    for j from 2 to nr do
      for k from 1 to j-1 do
        A[j,k] := 0;
      end_for;
    end_for;
  end_if;
  eval(A);
end_proc:

tcirandomnumbers := proc()
  local i, n, f, v;
begin
  if args(0)<3 or args(0)>4 then
   error("expecting three or four arguments")
  end_if:
  if args(1)=0 then // beta
    n := args(4);
    f := stats::betaRandom(args(2),args(3));
  elif args(1)=1 then  // binomial
    n := args(4);
    f := stats::binomialRandom(args(2),args(3));
  elif args(1)=2 then  // cauchy
    n := args(4);
    f := stats::cauchyRandom(args(2),args(3));
  elif args(1)=3 then  // chisquare
    n := args(3);
    f := stats::chisquareRandom(args(2));
  elif args(1)=4 then  // exponential
    n := args(3);
    f := stats::exponentialRandom(float(0),float(1)/args(2));
  elif args(1)=5 then  // F
    n := args(4);
    f := stats::fRandom(args(2),args(3));
  elif args(1)=6 then  // gamma
    n := args(4);
    f := stats::gammaRandom(args(2),args(3));
  elif args(1)=7 then  // normal
    n := args(4);
    f := stats::normalRandom(args(2),(args(3))^2);
  elif args(1)=8 then  // Poisson
    n := args(3);
    f := stats::poissonRandom(args(2));
  elif args(1)=9 then  // Student's T
    n := args(3);
    f := stats::tRandom(args(2));
  elif args(1)=10 then  // uniform
    n := args(4);
    f := stats::uniformRandom(args(2),args(3));
  elif args(1)=11 then  // Weibull
    n := args(4);
    f := stats::weibullRandom(args(2),args(3));
  else
    error("Distribution type not recognized");
  end_if;
  f() $ i=1..n;
end_proc:

tcirandmateg := proc(nr,nc,atype,arange)
begin
  tcirandmat(nr,nc,atype,arange);
end_proc:
  
tcisetseed := proc(n)
begin
  assigns := "":
  conditions := "";
  SEED := n;
end_proc:

tcirand := proc()
  local s,n,d1,d2,a,b;
begin
  if (args(0)=0) then
    random();
  elif (args(0)=1) then
    if (testtype(args(1),DOM_SET)) then
      op(args(1),random(1..nops(args(1)))());
      else
      random(args(1))();
    end_if;
  else
       a := min(floor(args(1)), floor(args(2)));
       b := max(floor(args(1)), floor(args(2)));
     random(a..b)();
  end_if;
end_proc:

tcirandfromset := proc(X)
begin
  if (testtype(args(1),DOM_SET)) then
    op(args(1),random(1..nops(args(1)))());
  end_if;
end_proc:

tcidistinct := proc()
begin
  for i from 1 to args(0)  do
     for j from i+1 to args(0) do
        if (args(i)=args(j)) then 
           return(FALSE);   
        end_if;
     end_for;
  end_for;
  return(TRUE)
end_proc:

tcidostring := proc(s)
begin
  LEVEL := 2:
  eval(text2expr(s));
end_proc:

tciloop := proc(stuff, cond) 
begin
  tcidostring(stuff);
  if (length(cond) <> 0) then
    for count from 0 to 300 do
      if (tcidostring(cond)) then
        return();
      end_if;
      tcidostring(stuff);
    end_for;
    assigns := "";
    conditions := "";
      errmsg := "Could not satisfy condition: " . cond;
    error(errmsg);
  end_if;
end_proc:

tcisetassigns:= proc(str)
  local bb;
begin
//  bb := text2expr(str);
//  if type(bb) = "_assign" then
//     str := expr2text(eval(bb));
//  end_if:

  if (length(assigns)=0) then
    assigns := str;
  else
    assigns := assigns . "; " . str;
  end_if;
end_proc:

tcisetconditions := proc(str)
begin
  if (length(conditions)=0) then
    conditions :=str;
  else
    conditions := conditions . " and " . str;
  end_if;
end_proc:

tcisatisfy := proc()
begin
  if (length(assigns)<>0) then 
     if ( length(conditions)=0) then
       tciloop(assigns, "TRUE");
     elif ( length(assigns) <> 0 ) then 
       tciloop(assigns, conditions);
     end_if;
  end_if;
  assigns := "";
  conditions := "";
end_proc:

tcipickunit := proc(a,b)
  //choose 'b' if only one term, otherwise choose 'a'
begin
  if type(eval(b))="_plus" then 
    return(eval(a)) 
  end_if;
  return(eval(b))
end_proc:





SWPplotMatrixToList := proc(itemIn)
local iii,jjj,itemOut;
begin
  if testtype(SWPplotMatrix,itemIn) then
    itemOut := SWPplotMatrix(itemIn):
    itemOut := [itemOut[iii,jjj] $ jjj=1..op(itemOut,[0,3,2]) $ iii=1..op(itemOut,[0,2,2])]:
  elif type(itemIn) = matrix then
    itemOut := expr(itemIn):
    itemOut := [expr(itemOut[iii,jjj]) $ jjj=1..op(itemOut,[0,3,2]) $ iii=1..op(itemOut,[0,2,2])]:
  else
    itemOut := itemIn:
  end_if:
  itemOut:
end_proc:


swpPlotDigits:=10: