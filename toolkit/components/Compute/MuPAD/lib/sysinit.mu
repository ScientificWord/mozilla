//--------------------------------------------------------------------
// MuPAD - Systeminitialisierungsdatei
//--------------------------------------------------------------------

(// trick to suppress all output

//--------------------------------------------------------------------
// Initialize domains for buit-in types
//--------------------------------------------------------------------

sysassign(sysassign, subsop(sysassign, 2=op(_assign, 2))):
old_assign := _assign;
protected(hold(_assign), None);
sysassign(_assign, sysassign);

sysdelete(Dpoly):   // use polylib::Dpoly

// -------- DOM_ARRAY --------
DOM_ARRAY:= domtype(array(1..1));
DOM_ARRAY::func_call:= proc(arr) name func_call; begin
    map(context(arr),
        proc(elem) begin elem(args(2..args(0))) end_proc,
        context(args(2..args(0)))
    )
end_proc;
DOM_ARRAY::new:= array;
DOM_ARRAY::new_extelement:= proc(d) name new_extelement; begin
    d::new(args(2..args(0)))
end_proc;
DOM_ARRAY::coerce:= proc(x)
    local T, i;
begin
    T:= x::dom;
    case T
    of DOM_HFARRAY do
        return(return(array(op(x, [0, i]) $ i = 2..op(x, [0, 1]) + 1, [op(x)])))
    otherwise
        return(T::convert_to(x, DOM_ARRAY))
    end_case
end_proc;

// -------- DOM_HFARRAY --------
DOM_HFARRAY := domtype(hfarray(1..1, [1])):
DOM_HFARRAY::print := arr -> if PRETTYPRINT and [op(arr,0)][1]<3 then
			       array(op([op(arr,0)][2..-1]), [op(arr)])
			     else
			       hold(hfarray)(op([op(arr, 0)][2..-1]), [op(arr)])
			     end_if:
DOM_HFARRAY::new := hfarray:
DOM_HFARRAY::new_extelement := DOM_ARRAY::new_extelement:
DOM_HFARRAY::coerce:= proc(x)
    local T, i;
begin
    T:= x::dom;
    case T
    of DOM_ARRAY do
        return(return(hfarray(op(x, [0, i]) $ i = 2..op(x, [0, 1]) + 1, [op(x)])))
    otherwise
        return(T::convert_to(x, DOM_HFARRAY))
    end_case
end_proc;
DOM_HFARRAY::hastype :=
proc(ar, t)
begin
  case t
  of DOM_FLOAT do return(TRUE);
  of DOM_COMPLEX do return(not iszero(Im(ar)));
  end_case;
  FALSE;
end_proc:

// -------- DOM_BOOL --------
DOM_BOOL:= domtype(TRUE);
DOM_BOOL::new:= proc() name new; begin args() end_proc;
DOM_BOOL::new_extelement:= DOM_ARRAY::new_extelement;

// -------- DOM_COMPLEX --------
DOM_COMPLEX:= domtype(I);
DOM_COMPLEX::D := 0 ;
DOM_COMPLEX::new:= proc() name new; begin args(1)+I*args(2) end_proc;
DOM_COMPLEX::new_extelement:= DOM_ARRAY::new_extelement;
DOM_COMPLEX::_index := (x, ind) -> if args(0) = 2 and
				      contains({1,2}, ind) then
                                     op(x, ind)
                                   else
                                     FAIL
                                   end_if:

// -------- DOM_DOMAIN --------
DOM_DOMAIN:= domtype(newDomain("DOM_DOMAIN"));
DOM_DOMAIN::func_call:= _domainfunccall ;
DOM_DOMAIN::new:= newDomain;
DOM_DOMAIN::new_extelement:= DOM_ARRAY::new_extelement;
DOM_DOMAIN::_index := (d) -> d::domain_index(args(2..args(0)));

// -------- DOM_EXEC --------
DOM_EXEC:= domtype(op(_assign, 1));
DOM_EXEC::new:= builtin;
DOM_EXEC::new_extelement:= DOM_ARRAY::new_extelement;

// -------- DOM_EXPR --------
DOM_EXPR:= domtype(XX+1);
DOM_EXPR::coerce:= proc(x)
    local T;
begin
    T:= x::dom;
    case T
    of DOM_STRING do
        return(text2expr(x))
    of DOM_POLY do
        return(op(x,1))
    otherwise
        return(T::convert_to(x,DOM_EXPR))
    end_case
end_proc;
DOM_EXPR::new:= proc() name new; begin args(1)(args(2..args(0))) end_proc;
DOM_EXPR::new_extelement:= DOM_ARRAY::new_extelement;

// -------- DOM_FAIL --------
DOM_FAIL:= domtype(FAIL);
DOM_FAIL::D := proc() name D; begin FAIL end_proc;
DOM_FAIL::new:= proc() name new; begin FAIL end_proc;
DOM_FAIL::new_extelement:= DOM_ARRAY::new_extelement;

// -------- DOM_FLOAT --------
DOM_FLOAT:= domtype(1.0);
DOM_FLOAT::D := 0 ;
DOM_FLOAT::new:= DOM_BOOL::new;
DOM_FLOAT::new_extelement:= DOM_ARRAY::new_extelement;
DOM_FLOAT::coerce:= proc(x)
    local T, v;
begin
    T:= x::dom;
    case T
    of DOM_STRING do
        if traperror((v := float(text2expr(x))))=0 and type(v)=DOM_FLOAT then
          return(v)
        else
          return(FAIL);
        end_if;
    otherwise
        return(T::convert_to(x,DOM_FLOAT))
    end_case
end_proc:

// -------- DOM_FUNC_ENV --------
DOM_FUNC_ENV:= domtype(_assign);
DOM_FUNC_ENV::new:= funcenv;
DOM_FUNC_ENV::new_extelement:= DOM_ARRAY::new_extelement;

// -------- DOM_IDENT --------
DOM_IDENT:= domtype(XX);
DOM_IDENT::new:= DOM_BOOL::new;
DOM_IDENT::new_extelement:= DOM_ARRAY::new_extelement;
DOM_IDENT::coerce:= proc(x)
    local T, v;
begin
    T:= x::dom;
    case T
    of DOM_STRING do
        if traperror((v := text2expr(x)))=0 and type(v)=DOM_IDENT then
          return(v)
        else
          return(FAIL);
        end_if;
    otherwise
        return(T::convert_to(x,DOM_IDENT))
    end_case
end_proc:

// -------- DOM_INT --------
DOM_INT:= domtype(1);
DOM_INT::D := 0 ;
DOM_INT::new:= DOM_BOOL::new;
DOM_INT::new_extelement:= DOM_ARRAY::new_extelement;
DOM_INT::phi:= phi;
sysdelete( phi ):   // use numlib::phi
DOM_INT::coerce:= proc(x)
    local T;
begin
    T:= x::dom;
    case T
    of DOM_STRING do
        x := DOM_FLOAT::coerce(x);
        if x=FAIL then return(FAIL); end_if;
        // fall-through
    of DOM_FLOAT do
        if frac(x)=0.0 then
          return(round(x));
        end_if;
        return(FAIL)
    otherwise
        return(T::convert_to(x,DOM_INT))
    end_case
end_proc:

// -------- DOM_NIL --------
DOM_NIL:= domtype(NIL);
DOM_NIL::new:= proc() name new; begin NIL end_proc;
DOM_NIL::new_extelement:= DOM_ARRAY::new_extelement;

// -------- DOM_NULL --------
DOM_NULL:= domtype(null());
DOM_NULL::new:= proc() name new; begin null() end_proc;
DOM_NULL::new_extelement:= DOM_ARRAY::new_extelement;

// -------- DOM_POLY --------
DOM_POLY:= domtype(poly(0, [XX]));
DOM_POLY::new:= poly;
DOM_POLY::new_extelement:= DOM_ARRAY::new_extelement;
DOM_POLY::coerce:= proc(x)
    local T, p, v;
begin
    T:= x::dom;
    case T
    of DOM_EXPR do
        if traperror( (p:= poly(x)) ) <> 0 then
            return( FAIL )
        else
            // t could be FAIL
            return( p )
        end_if
    of DOM_STRING do
        if traperror((v := text2expr(x)))=0 then
          return(DOM_POLY::coerce(v));
        else
          return(FAIL);
        end_if;
    otherwise
        return(T::convert_to(x,DOM_POLY))
    end_case
end_proc:
//--------------------------------------------------------------------
// Polynomials as operators
//--------------------------------------------------------------------

DOM_POLY::func_call:=
 proc(f)		// func_call ueberladen
   name func_call;
   local values;
 begin
   values := context([args(2..args(0))]);
   if nops(values) <> nops(op(f,2)) then
     error("wrong no of args")
   end_if;
   evalp(f, op(zip(op(f,2), values, _equal)))
 end_proc;


// -------- DOM_PROC --------
DOM_PROC:= domtype(proc() begin end_proc);
DOM_PROC::new:= _procdef;
DOM_PROC::print :=
proc(p)
  local Name, param, str;
  option noDebug;
begin
  if contains({op(p,  3)}, hold(arrow)) then
    // subs removes debug nodes in procedure p
    str := stringlib::collapseWhitespace(expr2text(subs(p, 1=1)));
    if length(str) > 28 then
      str := substring(str, 1..25)."...";
    end_if;
    str
  else
    param := op(p,1);
    if param = NIL then param := "" else param := expr2text(param) end;
    Name := op(p,6);
    if Name = NIL or Name =FAIL
      then Name := ""
    else Name := " ".expr2text(Name) end;
    "proc".Name."(".param.") ... end"
  end_if;
end_proc:

// -------- DOM_PROC_ENV --------
DOM_PROC_ENV:= domtype(op(
    proc(x) option escape; begin proc() begin x end_proc end_proc(1), 12));
DOM_PROC_ENV::new:= proc() name new; begin FAIL end_proc;
DOM_PROC_ENV::new_extelement:= DOM_ARRAY::new_extelement;
DOM_PROC_ENV::Content:=()->error("not implemented");

// -------- DOM_VAR --------
DOM_VAR:= domtype(op(proc(x) begin x end_proc, 4));
DOM_VAR::new:=
proc(d,i)
  name new;
  option noDebug;
begin
  subsop(hold(d), 1=d, 2=i, Unsimplified);
end_proc;
DOM_VAR::new_extelement:= DOM_ARRAY::new_extelement;
DOM_VAR::func_call:= proc(v) name func_call; begin
    subsop(hold(XX)(context(args(2..args(0)))), 0=v)
end_proc:

// -------- DOM_RAT --------
DOM_RAT:= domtype(1/2);
DOM_RAT::D := 0 ;
DOM_RAT::new:= proc() name new; begin args(1)/args(2) end_proc;
DOM_RAT::new_extelement:= DOM_ARRAY::new_extelement;
DOM_RAT::coerce:= proc(x)
    local T;
begin
    T:= x::dom;
    case T
    of DOM_STRING do
        x := DOM_FLOAT::coerce(x);
        if x=FAIL then return(FAIL); end_if;
        // fall-through
    of DOM_FLOAT do
        return(numeric::rationalize(x));
    otherwise
        return(T::convert_to(x,DOM_RAT))
    end_case
end_proc:
DOM_RAT::_index := (x, ind) -> if args(0) = 2 and
				  contains({1,2}, ind) then
				 op(x, ind)
			       else
				 FAIL
			       end_if:

// -------- DOM_LIST --------
DOM_LIST:= domtype([]);
DOM_LIST::func_call:= DOM_ARRAY::func_call;
DOM_LIST::new:= proc() name new; begin [args()] end_proc;
DOM_LIST::new_extelement:= DOM_ARRAY::new_extelement;
DOM_LIST::coerce:= proc(x)
    local T;
begin
    T:= x::dom;
    case T
    of DOM_SET do
        return( [op(x)] )
    of DOM_ARRAY do
    of DOM_HFARRAY do
        return( [op(x)] )
    of DOM_POLY do
        return(poly2list(x))
    otherwise
        return(T::convert_to(x,DOM_LIST))
    end_case
end_proc:

// -------- DOM_SET --------
DOM_SET:= domtype({});

// -------- DOM_STRING --------
DOM_STRING:= domtype("");
DOM_STRING::coerce:= x -> expr2text(x):
DOM_STRING::_less := proc(x, y) name _less; begin
  bool(sort([x, y])=[x, y] and x <> y)
end_proc;
DOM_STRING::_leequal := proc(x, y) name _leequal; begin
  bool(sort([x, y])=[x, y])
end_proc;
DOM_STRING::new:= DOM_BOOL::new;
DOM_STRING::new_extelement:= DOM_ARRAY::new_extelement;
// for the internal Print methods
DOM_STRING::Print := id:
DOM_STRING::MMLContent := (Out, data) -> Out::Ccsymbol(output::MMLPresentation::ms(data)):

// -------- DOM_TABLE --------
DOM_TABLE:= domtype(table());
DOM_TABLE::func_call:= DOM_ARRAY::func_call;
DOM_TABLE::new:= table;
DOM_TABLE::new_extelement:= DOM_ARRAY::new_extelement;
DOM_TABLE::coerce:= proc(x)
    local T;
begin
    T:= x::dom;
    return(T::convert_to(x,DOM_TABLE))
end_proc:
DOM_TABLE::lhs:= ()->map([op(args())],lhs):
DOM_TABLE::rhs:=  t->map(lhs(t), e->t[e]):

// -------- DOM_FRAME --------
DOM_FRAME := domtype(_rootFrame());
DOM_FRAME::print :=
proc(x) local result, tmpframe ;
begin
result := "" ;
tmpframe := x ;
while ( op(tmpframe,1) <> NIL ) do
   result := "::".expr2text(hold(``).op(tmpframe,2)).result ;
   tmpframe := op(tmpframe,1) ;
end_while ;
if ( op(x,1) = NIL ) then
   result := "frame ::"
else
   result := "frame ".result ;
end_if ;
end_proc:

//--------------------------------------------------------------------
// Remember values for special functions
//--------------------------------------------------------------------

abs(I) := 1:  abs(PI) := PI:
abs(EULER) := EULER:          abs(CATALAN) := CATALAN:

//--------------------------------------------------------------------
// Define shortcuts for basic arithmetic operators
//--------------------------------------------------------------------

`+`  := _plus:
`-`  := (x, y) -> case args(0) of 1 do -x; break;
                               of 2 do x - y; break; 
                               otherwise
                                 error("wrong number of args");
                  end:
`*`  := _mult:
`/`  := _divide:
`^`  := _power:
`**` := _power:
`=`  := _equal:
`<=` := _leequal:
`<>` := _unequal:
`<`  := _less:
`>`  := (a,b) -> a>b:
`>=` := (a,b) -> a>=b:
`,`  := _exprseq:

//--------------------------------------------------------------------
// If someone is able to create the identifier I, it evaluates
// to the corresponding MuPAD value
//--------------------------------------------------------------------

``."I" := I:

//--------------------------------------------------------------------
// For users which are used to having E:
//--------------------------------------------------------------------

E := hold(exp(1)):


//--------------------------------------------------------------------
// Define standard prefix for 'genident'
//--------------------------------------------------------------------

genident := funcenv(builtin(1056, NIL, "genident", NIL, "X" ),
                    builtin(1101, 0, NIL, "genident" ),
                    NIL ):

//--------------------------------------------------------------------
// Erzeugung des stdlib-Domains (wird von loadproc benoetigt)
// The 'stdlib' library domain serves only as name space for utilies
//--------------------------------------------------------------------

stdlib:= newDomain("stdlib");
stdlib::info:= "Library 'stdlib': the basic functionality of MuPAD":

//--------------------------------------------------------------------------
// Kern-Funktionen sichern (werden in specfunc bzw. polylib umdefiniert)
//--------------------------------------------------------------------------

specfunc:= newDomain("specfunc");
specfunc::info:= "Library 'specfunc': elementary and special functions";

specfunc::sin:= sin;
specfunc::cos:= cos;
specfunc::tan:= tan;
specfunc::cot:= cot;
specfunc::arcsin:= arcsin;
specfunc::arccos:= arccos;
specfunc::arctan:= arctan;
specfunc::sinh:= sinh;
specfunc::cosh:= cosh;
specfunc::tanh:= tanh;
specfunc::coth:= coth;
specfunc::arcsinh:= arcsinh;
specfunc::arccosh:= arccosh;
specfunc::arctanh:= arctanh;
specfunc::exp:= exp;
specfunc::ln:= ln;
specfunc::arg:= arg;
specfunc::sqrt:= sqrt;
specfunc::gamma:= gamma;
specfunc::igamma:= igamma;
specfunc::zeta:= zeta;
specfunc::erf:= erf;
specfunc::erfc:= erfc;
specfunc::sign:= sign;
specfunc::abs:= abs;
specfunc::fact:= fact;
specfunc::psi:=psi;

specfunc::lambertW := lambertW;
delete lambertW;
specfunc::Ei:=Ei;
specfunc::Ci:=Ci;
specfunc::Si:=Si;
specfunc::floor:= floor;
specfunc::ceil:= ceil;
specfunc::frac:= frac;
specfunc::round:= round;
specfunc::trunc:= trunc;


stdlib::evalAt:= evalAt;
sysdelete(evalAt):
stdlib::powermod:= powermod;
sysdelete(powermod):
stdlib::ithprime:= ithprime;
sysdelete(ithprime):
stdlib::pi:= pi;
sysdelete(pi):
stdlib::max:= max;
stdlib::min:= min;
stdlib::anames:= anames;

stdlib::ifactor:= ifactor: // keep the kernel function 'ifactor'
sysdelete( ifactor ):      // use stdlib::ifactor
stdlib::frandom:= frandom;
sysdelete(frandom);

stdlib::eval_params:= _eval_params:
sysdelete(_eval_params):

stdlib::getOptions:= `prog::getOptions`:
sysdelete(`prog::getOptions`):

stdlib::isnonzero := `stdlib::isnonzero`:
sysdelete(`stdlib::isnonzero`):
stdlib::isless    := `stdlib::isless`:
sysdelete(`stdlib::isless`):

stdlib::collapsews := `stdlib::collapsews`:
sysdelete(`stdlib::collapsews`):

stdlib::to64 := `stdlib::to64`:
sysdelete(`stdlib::to64`):
stdlib::from64 := `stdlib::from64`:
sysdelete(`stdlib::from64`):

stdlib::writeContent := `stdlib::writeContent`:
sysdelete(`stdlib::writeContent`):

stdlib::listDir := `stdlib::listDir`:
sysdelete(`stdlib::listDir`):

stdlib::hasfloat := `stdlib::hasfloat`:
sysdelete(`stdlib::hasfloat`):

stdlib::propertiesChanged := `stdlib::propertiesChanged`:
sysdelete(`stdlib::propertiesChanged`):

stdlib::trace := `stdlib::trace`:
sysdelete(`stdlib::trace`):

stdlib::src_line := `stdlib::src_line`:
sysdelete(`stdlib::src_line`):

stdlib::interval := interval:
sysdelete(interval):

// save for misc::maprec.
// NEVER TO BE USED AS stdlib::maprec!
stdlib::maprec := `misc::maprec`:
sysdelete(`misc::maprec`):
stdlib::breakmap := `misc::breakmap`:
sysdelete(`misc::breakmap`):

stdlib::rationalizeDescendHelper := rationalizeDescendHelper;
sysdelete(rationalizeDescendHelper);

// temporarily stored here, use as polylib::setcoeff
stdlib::setcoeff := `polylib::setcoeff`:
sysdelete(`polylib::setcoeff`):

stdlib::scale_exp := `gcdlib::scale_exp`:
sysdelete(`gcdlib::scale_exp`):

// the lib-readbytes searches the READPATH, so move the
// Kernel-ffunction to stdlib::
stdlib::readbytes:= readbytes: // keep the kernel function 'readbytes'
sysdelete( readbytes ):      // use stdlib::ifactor


// free global namespace
stdlib::gprof := gprof:
delete gprof:
stdlib::tcov := tcov:
delete tcov:

//--------------------------------------------------------------------
// Anonymous proc to hide aux variables
//--------------------------------------------------------------------

proc()
    local path, aux_globals, path_sep, filedescr, lib_path, sysread;
begin

// the global aux variables should not be protected
aux_globals := { hold(old_assign), hold(XX) };

//--------------------------------------------------------------------
// if tar-lib exists, use it
// since the library is not yet loaded, we have no pathname()
// therefore we must construct the correct path by hand.
//--------------------------------------------------------------------

case sysname()
of "UNIX" do
    path_sep := "";
    stdlib::PathSep:= "/";
    path:= LIBPATH;
    break;
of "MACOS" do
    path_sep := ":";
    stdlib::PathSep:= ":";
    path:= LIBPATH.":";
    break;
of "MSDOS" do
    path_sep := "";
    stdlib::PathSep:= "\\";
    path:= LIBPATH;
    break;
end_case;

sysread := proc(filename)
             local filedescr,res;
           begin
             filedescr := fopen(LIBPATH.path_sep.filename.".mb");
             if filedescr <> FAIL then
               res := fread(filedescr, Plain);
               fclose(filedescr);
               return(res);
             end_if;
             filedescr := fopen(LIBPATH.path_sep.filename.".mu");
             if filedescr <> FAIL then
               res := fread(filedescr, Plain);
               fclose(filedescr);
               return(res);
             end_if;
             error("Can't open file'".LIBPATH.path_sep.filename.".{mb,mu}'")
           end_proc:

lib_path:= LIBPATH;
filedescr := fopen(LIBPATH.path_sep."lib.tar");

if filedescr <> FAIL then
    fclose(filedescr);
    delete filedescr;
    path:= LIBPATH.path_sep."lib.tar#lib";
    LIBPATH := path;
end_if;
sysread("STDLIB".stdlib::PathSep."read");

//--------------------------------------------------------------------
// Initialisierung der vordefinierten Standardprozeduren, die bei
// Bedarf automatisch nachgeladen werden sollen.
//--------------------------------------------------------------------

sysread("STDLIB".stdlib::PathSep."buildnumber"):

sysread("STDLIB".stdlib::PathSep."alias");
sysread("STDLIB".stdlib::PathSep."unalias");
sysread("STDLIB".stdlib::PathSep."pathname");
if strmatch(_pref(hold(UserOptions)), "ANALYZE") then
  sysread("STDLIB".stdlib::PathSep."loadproc-analyze")
else
  sysread("STDLIB".stdlib::PathSep."loadproc")
end_if;


sysread("LIBFILES".stdlib::PathSep."stdlib");
sysread("LIBFILES".stdlib::PathSep."specfunc");
sysread("LIBFILES".stdlib::PathSep."dom_interval");

delete sysread;

//--------------------------------------------------------------------
// initialize libraries
//--------------------------------------------------------------------

path:= pathname("LIBFILES");

Ax       := loadproc(Ax,        path, "Ax");
Cat      := loadproc(Cat,       path, "Cat");
Dom      := loadproc(Dom,       path, "Dom");
matchlib := loadproc(matchlib,  path, "matchlib");
misc     := loadproc(misc,      path, "misc");
Series   := loadproc(Series,    path, "Series");
plot     := loadproc(plot,      path, "plot");
// alias for plot::new
display  := loadproc(display,   path, "plot");
polylib  := loadproc(polylib,   path, "polylib");
Pref     := loadproc(Pref,      path, "Pref");
property := loadproc(property,  path, "property");
Type     := loadproc(Type,      path, "Type");
adt      := loadproc(adt,       path, "adt");
combinat := loadproc(combinat,  path, "combinat");
domains  := loadproc(domains,   path, "domains");
export   := loadproc(export,    path, "export"); // neu in 5.0
faclib   := loadproc(faclib,    path, "faclib");
fp       := loadproc(fp,        path, "fp");
gcdlib   := loadproc(gcdlib,    path, "gcdlib");
generate := loadproc(generate,  path, "generate");
Graph    := loadproc(Graph,     path, "Graph");
groebner := loadproc(groebner,  path, "groebner");
interval := loadproc(interval,  path, "interval");
intlib   := loadproc(intlib,    path, "intlib");
import   := loadproc(import,    path, "import");
linalg   := loadproc(linalg,    path, "linalg");
listlib  := loadproc(listlib,   path, "listlib");
linopt   := loadproc(linopt,    path, "linopt");
module   := loadproc(module,    path, "module");
numeric  := loadproc(numeric,   path, "numeric");
numlib   := loadproc(numlib,    path, "numlib");
ode      := loadproc(ode,       path, "ode");
orthpoly := loadproc(orthpoly,  path, "orthpoly");
output   := loadproc(output,    path, "output");
prog     := loadproc(prog,      path, "prog");
rec      := loadproc(rec,       path, "rec");
RGB      := loadproc(RGB,       path, "RGB");
Simplify := loadproc(Simplify,  path, "simplify");
Symbol   := loadproc(Symbol   , path, "Symbol"  ):
Rule     := loadproc(Rule,      path, "Rule");
solvelib := loadproc(solvelib,  path, "solvelib");
stats    := loadproc(stats,     path, "stats");
stringlib:= loadproc(stringlib, path, "stringlib");
transform:= loadproc(transform, path, "transform");
unit     := loadproc(unit,      path, "unit");

//////////////////////////////////////////////////////////////////////
//       initialize basic sets
//////////////////////////////////////////////////////////////////////

N_:= loadproc(N_, path,"solvelib"):
Z_:= loadproc(Z_, path,"solvelib"):
Q_:= loadproc(Q_, path,"solvelib"):
R_:= loadproc(R_, path,"solvelib"):
C_:= loadproc(C_, path,"solvelib"):

//--------------------------------------------------------------------
// Some attributes for kernel functions
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Funktionsattribute fuer ``diff''
//--------------------------------------------------------------------

id::diff        := f -> diff(op(f, 1), args(2..args(0))):
_negate::diff   := f -> diff(-op(f,1), args(2..args(0))):
_subtract::diff := f -> diff(op(f,1) - op(f,2), args(2..args(0))):
_invert::diff   := f -> diff(1/op(f,1), args(2..args(0))):
_divide::diff   := f -> diff(op(f,1) / op(f,2), args(2..args(0))):
_equal::diff    := f -> diff(op(f,1),args(2..args(0))) =
                          diff(op(f,2),args(2..args(0))):

//--------------------------------------------------------------------
// Funktionsattribute fuer ``expand''
//--------------------------------------------------------------------

/*
    expand(S minus T)

    implements the rules
    (A union B) minus C -> (A minus C) union (B minus C)
    (A intersect B) minus C -> (A minus C) intersect B minus C

    it does *not* change A minus (B union C) to (A minus B) minus C

*/
_minus::expand:=
proc(a)
 local S, T;
 begin
   assert(nops(a) = 2);
   [S, T]:= [op(a)];
   case type(S)
     of "_union" do
     of "_intersect" do
       return(map(S, _minus, T))
   end_case;
   hold(_minus)(S, T)
 end_proc:

//--------------------------------------------------------------------
// Funktionsattribute fuer ``_plus''
//--------------------------------------------------------------------

_equal::_plus :=
  proc(a, b)
  begin
    case type(b)
      of "_equal" do
      of "_leequal" do
      of "_less" do
       op(b,0)(op(a, 1)+op(b, 1), op(a, 2)+op(b,2));
       break
      of "_unequal" do
       (op(a, 1)+op(b, 1)) <> (op(a, 2)+op(b,2));
       break;
      of DOM_SET do
      of DOM_LIST do
       map(b, x->a+x);
       break;
      of DOM_POLY do 
       _equal::_plus(a, expr(b));
       break 
      otherwise
       op(a,0)(op(a, 1)+b, op(a, 2)+b);
   end_case
  end_proc:

_approx::_plus := 
  proc(a, b)
  begin
    case type(b)
      of "_leequal" do
      of "_less" do
       op(b,0)(op(a, 1)+op(b, 1), op(a, 2)+op(b,2));
       break
      of "_equal" do 
      of "_approx" do
       _approx(op(a, 1)+op(b, 1), op(a, 2)+op(b,2));
       break
      of "_unequal" do
       (op(a, 1)+op(b, 1)) <> (op(a, 2)+op(b,2));
       break;
      of DOM_POLY do 
       _approx::_plus(a, op(b, 1));
       break 
      otherwise
       op(a,0)(op(a, 1)+b, op(a, 2)+b);
   end_case
  end_proc:
  
  
  
_leequal::_plus :=
 proc(a, b)
  begin
    case type(b)
      of "_equal" do
      of "_leequal" do
       op(a,0)(op(a, 1)+op(b, 1), op(a, 2)+op(b,2));
       break
      of "_less" do
       op(a, 1)+op(b, 1) < op(a, 2)+op(b,2);
       break
      of "_unequal" do
       FAIL;
       break;
      of DOM_SET do
      of DOM_LIST do
       map(b, x->a+x);
       break;
      of DOM_POLY do 
       _leequal::_plus(a, expr(b));
       break 
      otherwise
       op(a,0)(op(a, 1)+b, op(a, 2)+b);
   end_case
  end_proc:

_less::_plus    := proc(a, b)
  begin
    case type(b)
      of "_equal" do
      of "_leequal" do
       op(a,0)(op(a, 1)+op(b, 1), op(a, 2)+op(b,2));
       break
      of "_less" do
       op(b,0)(op(a, 1)+op(b, 1), op(a, 2)+op(b,2));
       break
      of "_unequal" do
       FAIL;
       break;
      of DOM_SET do
      of DOM_LIST do
       map(b, x->a+x);
       break;
      of DOM_POLY do 
       _less::_plus(a, expr(b));
       break 
      otherwise
       op(a,0)(op(a, 1)+b, op(a, 2)+b);
   end_case
  end_proc:

_unequal::_plus :=
proc(a, b)
  begin
    case type(b)
      of "_equal" do
       op(a,0)(op(a, 1)+op(b, 1), op(a, 2)+op(b,2));
       break
      of "_leequal" do
      of "_less" do
      of "_unequal" do
       FAIL;
       break;
      of DOM_SET do
      of DOM_LIST do
       map(b, x->a+x);
       break;
      of DOM_POLY do 
       _unequal::_plus(a, expr(b));
       break
      otherwise
       op(a,0)(op(a, 1)+b, op(a, 2)+b);
   end_case
  end_proc:

// (A_1 union ... union A_n)+B = (A_1+B) union ... (A_n+B)
_union::_plus    := (a, b) -> map(a, _plus, b):

_intersect::_plus :=
_minus::_plus     := proc(a, b)
                     local x, y;
                     begin
                        if testtype(b, Type::Arithmetical) then
                          // treat identifiers in a special way
                          if contains(map({op(a)}, domtype), DOM_IDENT) then
                            if has(b, #xyz) then
                              x:= genident()
                            else 
                              x:= #xyz
                            end_if;  
                            Dom::ImageSet(x+ b, x, a)
                          else   
                            map(a, _plus, b)
                          end_if  
                        elif testtype(b, Type::Set) then
                           x:= genident("XXX");
                           y:= genident("XXX");
                           Dom::ImageSet(x+y, [x,y], [a, b])
                        else
                          hold(_plus)(args())
                        end_if
                      end_proc:

_and::_plus       := () -> hold(_plus)(args()):
_or::_plus        := () -> hold(_plus)(args()):
_not::_plus       := () -> hold(_plus)(args()):

//--------------------------------------------------------------------
// Funktionsattribute fuer ``_mult''
//--------------------------------------------------------------------

_equal::_mult :=
  proc(a, b)
  begin
    case type(b)
      of "_equal" do
      of "_leequal" do
      of "_less" do
       op(b,0)(op(a, 1)*op(b, 1), op(a, 2)*op(b,2));
       break
      of DOM_SET do
      of DOM_LIST do
       map(b, x->a*x);
       break;
     of DOM_POLY do
       _equal::_mult(a, expr(b));
       break
      otherwise
       op(a,0)(op(a, 1)*b, op(a, 2)*b);
   end_case
  end_proc:

  _approx::_mult :=
  proc(a, b)
  begin
    case type(b)
    of "_leequal" do
    of "_less" do
      op(b,0)(op(a, 1)*op(b, 1), op(a, 2)*op(b,2));
      break
    of "_equal" do
    of "_approx" do
      _approx(op(a, 1)*op(b, 1), op(a, 2)*op(b,2));
      break
    of DOM_POLY do
      _approx::_mult(a, op(b, 1));
      break
    otherwise
      op(a,0)(op(a, 1)*b, op(a, 2)*b);
    end_case
  end_proc:  
  
  
_unequal::_mult    :=
  proc(a, b)
  begin
    case type(b)
      of "_equal" do
       if is(op(b, 1)=0) = FALSE or is(op(b, 2)=0) = FALSE then
         op(a,0)(op(a, 1)*op(b, 1), op(a, 2)*op(b,2));
       else
         hold(_mult)(args());
       end_if;
       break
      of "_leequal" do
      of "_less" do
      of "_unequal" do
       FAIL;
       break;
      of DOM_SET do
      of DOM_LIST do
       map(b, x->a*x);
       break;
     of DOM_POLY do
       _unequal::_mult(a, expr(b));
       break 
      otherwise
       case is(b=0)
         of FALSE do
           return(op(a,0)(op(a, 1)*b, op(a, 2)*b))
         of TRUE do
           error("Inequalities must not be multiplied by zero")
         of UNKNOWN do
           return(hold(_mult)(args()))
       end_case;
   end_case
  end_proc:


_leequal::_mult:=
 proc(a, b)
 begin
   assert(type(a) = "_leequal");
   case domtype(b)
     of DOM_COMPLEX do
       error("Inequalities must not be multiplied by complex numbers")
     of DOM_INT     do
     of DOM_RAT     do
     of DOM_FLOAT   do
     of DOM_IDENT do
     of DOM_EXPR do
       case type(b)
         of "_equal" do
           if is(b) = FALSE then
             error("Cannot multiply inequality and an equation that is provably false")
           end_if;
           return(piecewise([op(b, 1) >= 0 or op(b, 2) >= 0,
                             op(a,1)*op(b,1) <= op(a,2)*op(b,2)],
                            [op(b, 1) <= 0 or op(b, 2) <= 0,
                             op(a,1)*op(b,1) >= op(a,2)*op(b,2)]))
         of "_unequal" do
           error("Illegal arguments")
         of "_less" do
         of "_leequal" do
           return(piecewise([op(b, 1) >= 0 and op(b, 2) >= 0,
                             op(a,1)*op(b,1) <= op(a,2)*op(b,2)],
                            [op(b, 1) <= 0 and op(b, 2) <= 0,
                             op(a,1)*op(b,1) >= op(a,2)*op(b,2)]))
       end_case;
       if not testtype(b, Type::Arithmetical) then
         error("Unexpected type of second argument")
       else
         return(piecewise([b >= 0, b*op(a, 1) <= b*op(a, 2)],
                        [b <= 0, b*op(a, 2) <= b*op(a, 1)]))
       end_if
     of DOM_POLY do 
       _leequal::_mult(a, expr(b));
       break
   otherwise
       error("Unexpected type of second argument")
   end_case
 end_proc:


_less ::_mult:=
proc(a, b)
 begin
   assert(type(a) = "_less");
   case domtype(b)
     of DOM_COMPLEX do
       error("Inequalities must not be multiplied by complex numbers")
     of DOM_INT     do
     of DOM_RAT     do
     of DOM_FLOAT   do
       if iszero(b) then
         error("Inequalities must not be multiplied by zero")
       // else: fall through
       end_if;
     of DOM_IDENT do
     of DOM_EXPR do
       case type(b)
         of "_equal" do
           if iszero(op(b,1)) or iszero(op(b,2)) then
             error("Inequalities must not be multiplied by zero")
           end_if;
           if is(b) = FALSE then
             error("Cannot multiply inequality and an equation that is provably false")
           end_if;
           return(piecewise([op(b, 1) > 0 or op(b, 2) > 0,
                             op(a,1)*op(b,1)< op(a,2)*op(b,2)],
                            [op(b, 1) < 0 or op(b, 2) < 0,
                             op(a,1)*op(b,1)> op(a,2)*op(b,2)]))
         of "_unequal" do
           error("Illegal arguments")
         of "_less" do
         of "_leequal" do
           return(piecewise([op(b, 1) > 0 and op(b, 2) > 0,
                             op(a,1)*op(b,1)< op(a,2)*op(b,2)],
                            [op(b, 1) < 0 and op(b, 2) < 0,
                             op(a,1)*op(b,1)> op(a,2)*op(b,2)]))
       end_case;
       if not testtype(b, Type::Arithmetical) then
         error("Unexpected type of second argument")
       else
         return(piecewise([b > 0, b*op(a, 1) < b*op(a, 2)],
                        [b < 0, b*op(a, 2) < b*op(a, 1)]))
       end_if
     of DOM_POLY do 
       _less::_mult(a, expr(b));
       break               
     otherwise
       error("Unexpected type of second argument")
   end_case
 end_proc:


_union::_mult      := proc(a, b)
                      begin
                        if(type(a)="_union") then
                          map(a, _mult, b);
                        else // b is a union
                          map(b, x -> (a*x));
                        end_if;
                      end_proc:

 _intersect::_mult :=
 proc()
   local s, inds, argv;
 begin
   argv:= select([args()], _unequal, 1);
   if nops(argv) = 1 then
     return(argv[1])
   end_if;
   s := split(argv, X->type(X)<>DOM_IDENT and testtype(X,Type::Set) );
   assert(nops(s[1]) > 0); // at least the _intersect should be a set
   inds := map( s[1], X->genident() );
   Dom::ImageSet( _mult(op(s[2].inds)), inds, s[1] );
 end_proc:
 
_minus::_mult      := proc(a, b)
                        local x1, x2;
                      begin
                        if testtype(b, Type::Arithmetical) then
                          map(a, _mult, b)
                        elif testtype(b, Type::Set) then
                          x1:= genident();
                          x2:= genident();
                          Dom::ImageSet(x1*x2, [x1, x2], [a, b])
                        else
                          hold(_mult)(args())
                        end_if
                      end_proc:
_and::_mult        := () -> hold(_mult)(args()):
_or::_mult         := () -> hold(_mult)(args()):
_not::_mult        := () -> hold(_mult)(args()):

//--------------------------------------------------------------------
// Funktionsattribute fuer ``_power''
//--------------------------------------------------------------------
 _equal::_power      :=
 proc(a: "_equal", b)
 begin
   if type(b) = "_equal" then
     op(a, 1)^op(b,1) = op(a, 2)^op(b, 2)
   elif testtype(b, Type::Arithmetical) then
     op(a, 1)^b = op(a, 2)^b
   else
     hold(_power)(args())
   end_if
 end_proc:


_leequal::_power    := () -> hold(_power)(args()):
_less::_power       := () -> hold(_power)(args()):
_unequal::_power    := () -> hold(_power)(args()):
_union::_power      :=
 proc(a, b)
 begin
   if type(a) = "_union" then
     if testtype(b, Type::Arithmetical) then 
       map(a, 
       proc(S)
       begin
         if type(S) = DOM_IDENT then
           // it is important that we do not lose the information that S represents a set here; 
           // which _power cannot know
           Dom::ImageSet(#X^b, #X, S)
         else
           S^b
         end_if   
       end_proc 
       )
     elif testtype(b, Type::Set) then
       Dom::ImageSet(#X^#Y, [#X, #Y], [a, b])
     else
       map(a, _power, b)
     end_if  
   else
     assert(type(b) = "_union");
     map(b, x -> a^x)
   end_if
 end_proc: 


_intersect::_power  := () -> hold(_power)(args()):
_minus::_power      := () -> hold(_power)(args()):
_and::_power       := () -> hold(_power)(args()):
_or::_power         := () -> hold(_power)(args()):
_not::_power        := () -> hold(_power)(args()):




//--------------------------------------------------------------------
// Funktionsattribute fuer ``hull''
//  all of these are called after converting the arguments to intervals
//  as far as possible.
//--------------------------------------------------------------------
_union::hull :=
    proc() name hull; local iv, other, i, hi; begin
        iv := []; other := [];
        for i in args() do
           if i::dom = DOM_INTERVAL
           then iv := iv.[i];
           else hi := hull(i);
              if hi::dom = DOM_INTERVAL
              then iv := iv.[i]
              else other := other.[i];
              end_if;
           end_if;
        end_for;
        if nops(other) > 0
        then _union(hull(op(iv)),op(other));
        else hull(op(iv));
        end_if;
    end_proc:

//--------------------------------------------------------------------
// Funktionsattribute fuer ``_index''
// data:= [[1, 2.1], [2, 3.2]]
// data[k][2] produced an error, since data[k] returns
// symbolically. Define _index::_index to avoid this error.
// Now, sum(data[k][2], k = 1..2) works without option hold.
//--------------------------------------------------------------------
_index::_index := () -> hold(_index)(args()):

// To allow diff(x[i], x[j]). Done at the library level,
// to avoid coupling the kernel to kroneckerDelta
_index::diff :=
proc(ex, x)
  local i;
begin
  if type(x)="_index" and nops(ex) = nops(x) and op(ex, 1)=op(x, 1) then
    _mult(kroneckerDelta(op(ex, i), op(x, i)) $ i=2..nops(ex));
  else 0 end_if
end_proc:

//--------------------------------------------------------------------
// Print modp infix, when _mod = mopd, otherwise functional
// same vice versa for mods
// Make expose(modp) and expose(_mod) identical
//--------------------------------------------------------------------

modp::TeX :=
proc(arg1, arg2, arg3)
begin
  if _mod = arg1 then
    generate::TeXoperator(" \\mathbin{\\text{mod}} ", arg3,
			  output::Priority::Mod,
			  op(arg2)):
  else
    generate::TeXfun(expr2text(arg1), arg3, op(arg2));
  end_if;
end_proc:
mods::TeX := modp::TeX:
__mod := subsop(modp, 1=(()->procname(args()))):
_modp := subsop(modp, 1=(()->procname(args())), [2, 1]=1101):
_mods := subsop(mods, 1=(()->procname(args())), [2, 1]=1101):
modp := subsop(modp, 2 = (x -> if _mod = modp then
                                 hold(__mod)(op(x))
                               else
                                 hold(_modp)(op(x))
                               end_if)):
modp::Content:=
  (Out, data) -> if _mod = modp and nops(data) = 2 then
                   Out::Capply(Out::Crem, map(op(data), Out))
                 else
                   Out::stdFunc(data)
                 end:
modp::type := "_mod":
mods := subsop(mods, 2 = (x -> if _mod = mods then
                                 hold(__mod)(op(x))
                               else
                                 hold(_mods)(op(x))
                               end_if)):
mods::Content:=
  (Out, data) -> if _mod = mods and nops(data) = 2 then
                   Out::Capply(Out::Crem, map(op(data), Out))
                 else
                   Out::stdFunc(data)
                 end:
mods::type := "_mod":
_mod := modp:

//------------------------------------------------------------
// nicer output for last
//------------------------------------------------------------
last :=  subsop(last, 2 = (ex -> if op(ex)=1 then "%"
				 else "%".expr2text(op(ex))
				 end_if)):

//------------------------------------------------------------
// evaluating proc definition
//------------------------------------------------------------
`-->` := proc(l, r)
	   option hold;
	   local substs;
	 begin
	   substs := [];
	   l := op(map([l],
		       proc(x)
			 option hold;
			 local n;
		       begin
			 if domtype(x) = DOM_VAR then
			   n := genident();
			   substs := substs.[x=n];
			   n;
			 else
			   x;
			 end_if;
		       end_proc));

	   if nops({l}) <> nops([l]) or
	      map({l}, domtype) <> {DOM_IDENT} then
	     error("procedure definitions with --> must use only identifiers");
	   end_if;
	   map(l, x -> if ("".x)[1] <> "#" then save x; eval(hold(sysdelete)(x)) end);
	   r := context(subs(r, substs));
	   if hastype(r, DOM_VAR) then
	     error("right hand side contains references to local variables");
	   end_if;
	   fp::unapply((r), (l));
	 end_proc:
`-->` := funcenv(`-->`,  builtin(1097, 150, " --> ", "`-->`")):
`-->`::Content := stdlib::genOutFunc("Clambda", 2):

//--------------------------------------------------------------------
// Operator rules for +, *, ^ and @
//--------------------------------------------------------------------

_plus::operator    := _op_plusmult:          // (f+g)(x) --> f(x)+g(x)
_mult::operator    := _op_plusmult:          // (f*g)(x) --> f(x)*g(x)
_equal::operator   := _op_plusmult:          // ((f=g))(x) --> f(x)=g(x)
_less::operator    := _op_plusmult:          // (f<g)(x) --> f(x)<g(x)
_leequal::operator := _op_plusmult:          // (f<=g)(x) --> f(x)<=g(x)
_unequal::operator := _op_plusmult:          // (f<>g)(x) --> f(x)<>g(x)
_and::operator     := _op_plusmult:          // (f and g)(x) --> f(x) and g(x)
_or::operator      := _op_plusmult:          // (f or g)(x) --> f(x) or g(x)
_not::operator     := _op_plusmult:          // (not f)(x) --> not f(x)
_power::operator   := _op_power:             // (f^n)(x) --> f(x)^n
_fconcat::operator := _op_fconcat:           // (f@g)(x) --> f(g(x))


//--------------------------------------------------------------------
// Define "float" attributes for basic arithmetic operators
//--------------------------------------------------------------------

_plus::float  := _float_plus:
_mult::float  := _float_mult:
_power::float := _float_power:
sysdelete(_float_plus, _float_mult, _float_power):

_equal::float     := (x, y) -> (float(x) =  float(y)):
_unequal::float   := (x, y) -> (float(x) <> float(y)):
_less::float      := (x, y) -> (float(x) <  float(y)):
_leequal::float   := (x, y) -> (float(x) <= float(y)):
_not::float       := x -> not float(x):
_and::float       := () -> hold(_and)      (op(map([args()], float))):
_or::float        := () -> hold(_or)       (op(map([args()], float))):
_union::float     := () -> _union(op(map([args()], float))):
_intersect::float := () -> _intersect(op(map([args()], float))):
_index::float     := (x) -> eval(hold(_index)(float(x), args(2..args(0)))):

_range::float     := (x, y) -> float(x)..float(y):
_seqgen::float    := () -> eval(hold(_seqgen)(map(args(), float))):
_seqstep::float   := () -> eval(hold(_seqstep)(map(args(), float))):
_seqin::float     := () -> eval(hold(_seqin)(map(args(), float))):

// _sortsum :  nicer _plus notation
_sortsum := proc(s)
              local res;
            begin
              if traperror((res := generate::sortSum(s))) <> 0 then
                FAIL
              else
                res
              end_if
            end_proc:
_plus::_index :=
 proc(s, ind)
   local res, left, right, Nops;
 begin
   res := _sortsum(s);
   Nops := nops(res);
   if res <> FAIL then
     case type(ind)
       of "_range" do
         left := op(ind, 1);
         if left < 0 then
           left := left + Nops + 1;
         end_if;

         right := op(ind, 2);
         if right < 0 then
           right := right + Nops + 1;
         end_if;
         return(_plus(op(res, left..right)));

       of DOM_INT do
         if ind < 0 then
           ind := ind + Nops + 1;
         end_if;
         return(op(res, ind));

       otherwise
         FAIL
     end_case
   else
     FAIL
   end_if;
 end_proc:
_mult::_index :=
 proc(p, ind)
   local res, left, right, Nops;
 begin
   res := Content(p);
   if res::dom::typString(res) = "divide" then
     case ind
       of 1 do
         return(expr(op(res, 1)));
       of 2 do
         return(expr(op(res, 2)));
       otherwise
         FAIL
     end_case;
   else
     // no fraction
    Nops := nops(res);
    case type(ind)
       of "_range" do
         left := op(ind, 1);
         if left < 0 then
           left := left + Nops + 1;
         end_if;

         right := op(ind, 2);
         if right < 0 then
           right := right + Nops + 1;
         end_if;
         return(_mult(map(op(res, left..right), expr)));

       of DOM_INT do
         if ind < 0 then
           ind := ind + Nops + 1;
         end_if;
         return(expr(op(res, ind)));

   otherwise
         FAIL
     end_case

   end_if
 end_proc:


// typesetting library
Content  := loadproc(Content, pathname("OUTPUT"), "Content"):
MathContent  := loadproc(MathContent, pathname("OUTPUT"), "MathContent"):
MathXMLContent  := loadproc(MathXMLContent, pathname("OUTPUT"), "MathXMLContent"):
ContentLayout  := loadproc(ContentLayout, pathname("OUTPUT"), "ContentLayout"):
// only needed for typesetting ASCII fallback
_lastContentOutput := FAIL:

//--------------------------------------------------------------------
// Content methods of kernel function.  They have to be defined before
// the content library is loaded, otherwise we have problems with
// freeze and hold in Typesetting output!!
// only needed for output library
//--------------------------------------------------------------------

_mult_intern := funcenv(_mult_intern, subsop(extop(_mult, 2),
         2=output::Priority::Mult-1,
         3=" ",
		   4="_mult_intern"
		   )):

_mult_intern::type := "_mult_intern":
_mult_intern::Content :=
    loadproc(_mult_intern::Content, pathname("OUTPUT", "EXPR"), "mult"):

old_assign::Content := stdlib::genOutFunc("Cassign", 2):
sysassign::Content  := stdlib::genOutFunc("Cassign", 2):
old_assign::TeX     :=
  (arg1, arg2, arg3) -> generate::TeXoperator(" := ", arg3, output::Priority::Stmt,
                                              op(arg2)):
_assign::TeX     :=
  (arg1, arg2, arg3) -> generate::TeXoperator(" := ", arg3, output::Priority::Stmt,
                                              op(arg2)):



// special case: the arguments are flattened!
_exprseq::Content    := Out -> Out::Capply(Out::Cexprseq, map(args(2..args(0)), Out)):

_div::Content        := stdlib::genOutFunc("Cquotient", 2):
_union::Content      := stdlib::genOutFunc("Cunion", 2, infinity):
_intersect::Content  := stdlib::genOutFunc("Cintersect", 2, infinity):
_minus::Content      := stdlib::genOutFunc("Csetdiff", 2):
_concat::Content     := stdlib::genOutFunc("Cconcat", 2, infinity):

_equal::Content      := stdlib::genOutFunc("Ceq", 2, infinity):
_fconcat::Content    := stdlib::genOutFunc("Ccompose", 2, infinity):
_leequal::Content    := stdlib::genOutFunc("Cleq", 2, infinity):
_less::Content       := stdlib::genOutFunc("Clt", 2, infinity):
_or::Content         := stdlib::genOutFunc("Cor", 2, infinity):
_and::Content        := stdlib::genOutFunc("Cand", 2, infinity):
_range::Content      := stdlib::genOutFunc("Crange", 2):
_unequal::Content    := stdlib::genOutFunc("Cneq", 2, infinity):

_not::Content        := stdlib::genOutFunc("Cnot", 1):
_negate::Content     := stdlib::genOutFunc("Cminus", 1):
_invert::Content     := (Out, data) -> if nops(data) = 1 then
                                         Out::Capply(Out::Cdivide,
                                                     Out(1),Out(op(data))):
                                       else
                                         Out::stdFunc(data)
                                       end_if:
_divide::Content     := stdlib::genOutFunc("Cdivide", 2):
norm::Content        := (Out, data) -> if nops(data) = 0 or nops(data) > 2 then
                                         Out::stdFunc(data)
                                       else
                                         Out::Cnorm(op(map([op(data)], Out))):
                                       end_if:

_index::Content      := (Out, data) -> if nops(data) <= 1 then
                                         Out::stdFunc(data)
                                       else
                                         Out::Cci(
                                           Out::Cindex(Out(op(data, 1)),
                                             Out(op(data, 2..nops(data))))):
                                       end_if:
diff::Content     :=
 proc(Out, data)
   name diff::Content;
   local i, bvar, neubvar, varList, myexp, res;
 begin
   myexp := 0;
   if nops(data) <= 1 then
     return(Out::stdFunc(data))
   end_if;
   neubvar := op(data, nops(data));
   if testtype(neubvar, "_seqgen") then
     bvar := op(neubvar, 1);
     if nops(neubvar) = 1 then
       myexp := myexp + op(neubvar, [1, 2]) - op(neubvar, [1, 1]) + 1;
     elif nops(neubvar) = 2 then
       myexp := myexp + op(neubvar, 2);
     elif nops(neubvar) = 3 then
       myexp := myexp + op(neubvar, [3, 2]) - op(neubvar, [3, 1]) + 1;
     else
       error("illegal argument");
     end_if;
   else
     bvar := neubvar;
     myexp := myexp + 1;
   end_if;
   i := nops(data);
   while 2 < i and
     (op(data, [i - 1, 1]) = bvar or
      testtype(op(data, i - 1), "_index") and op(data, i - 1) = bvar)  do
     neubvar := op(data, i - 1);
     if testtype(neubvar, "_seqgen") then
       if nops(neubvar) = 1 then
         myexp := myexp + op(neubvar, [1, 2]) - op(neubvar, [1, 1]) + 1;
       elif nops(neubvar) = 2 then
         myexp := myexp + op(neubvar, 2);
       elif nops(neubvar) = 3 then
         myexp := myexp + op(neubvar, [3, 2]) - op(neubvar, [3, 1]) + 1;
       else
         error("illegal argument");
       end_if;
     else
       myexp := myexp + 1;
     end_if;
     i := i - 1;
   end_while;
   varList := op(data, 2..i - 1);
   if varList = null() then
     res := Out(op(data, 1));
   else
     res := Out(hold(diff)(op(data, 1), varList));
   end_if;
   if i <> nops(data) or myexp <> 1 then
     Out::Capply(Out::Cpartialdiff,
                 Out::Cbvar(Out(bvar), Out::Cdegree(Out(myexp))), res);
   else
     Out::Capply(Out::Cpartialdiff, Out::Cbvar(Out(bvar)), res);
   end_if;
 end_proc:
_mult::Content    := loadproc(_mult::Content, pathname("OUTPUT", "EXPR"), "mult"):
_mult_intern_2 := funcenv(_mult_intern_2,
                          builtin(1100, output::Priority::Mult,
                                  "*", "_mult_intern_2")):
_mult::expr2text  := proc(m)
                       local sig, num, den, res;
                     begin
                       if traperror(([sig, num, den] := generate::splitProduct(m))) <> 0 then
                         return(FAIL);
                       end_if;
                       if nops(num) = 1 then
                         if nops(den) <> 0 then
                           res := op(num);
                         else
                           return(FAIL)
                         end_if;
                       else
                         res := hold(_mult_intern_2)(op(num));
                       end_if;
                       if nops(den) > 0 then
                         if nops(den) = 1 then
                           res := hold(_divide)(res, op(den));
                         else
                           res := hold(_divide)(res,
                                                hold(_mult_intern_2)(op(den)));
                         end_if;
                       end_if;
                       if sig = -1 then
                         res := hold(_negate)(res)
                       end_if;
                       res;
                     end_proc;
_plus::Content    := proc(Out, data)
                       local sorted;
                     begin
                       if nops(data) <= 1 then
                         return(Out::stdFunc(data))
                       end_if;
                       sorted := [op(generate::sortSum(data))];
                       Out::Capply(Out::Cplus, op(map(sorted, Out))):
                       end_proc:
_subtract::Content:= (Out, data) -> if nops(data) <> 2 then
                                      return(Out::stdFunc(data))
                                    else
                                      Out::Capply(Out::Cminus,
                                                  Out(op(data,1)),
                                                  Out(op(data,2))):
                                    end_if:
                                    
_power::Content   := loadproc(_power::Content, pathname("OUTPUT", "EXPR"), "power"):
_seqgen::Content  := stdlib::genOutFunc("Cseqgen", 1, 3):
_seqstep::Content := stdlib::genOutFunc("Cseqstep", 2, 4):
_seqin::Content   := stdlib::genOutFunc("Cseqin", 3):

slot::Content     := proc(Out, data)
                       local x, key;
                     begin
                       if nops(data) <> 2 then
                         return(Out::stdFunc(data))
                       end_if;
                       key := op(data, 2);
                       if domtype(key) = DOM_STRING and
                          traperror((x := text2expr(key))) = 0 and
                          nops(data) = 2 then
                         Out::Capply(Out::Cslot, Out(op(data,1)), Out(x))
                       else
                         Out::stdFunc(data)
                       end_if:
                     end_proc:

DOM_ARRAY::Content_ :=
   loadproc(DOM_ARRAY::Content_, pathname("OUTPUT", "DOMAINS"), "array"):
DOM_ARRAY::Content :=
   loadproc(DOM_ARRAY::Content, pathname("OUTPUT", "DOMAINS"), "array"):
DOM_HFARRAY::Content :=
   loadproc(DOM_HFARRAY::Content, pathname("OUTPUT", "DOMAINS"), "array"):
DOM_BOOL::Content:= proc(Out, x)
                    begin
                      if x = TRUE then
                        Out::Ctrue()
                      elif x = FALSE then
                        Out::Cfalse()
                      else
                        Out::Cunknown()
                      end_if;
                    end_proc:
DOM_COMPLEX::Content:=
   loadproc(DOM_COMPLEX::Content, pathname("OUTPUT", "DOMAINS"), "complex"):
DOM_DOMAIN::Content:=
  proc(Out, x)
    local domName;
  begin
    if x::Name <> FAIL then
      domName :=  Out(x::Name)
    else
      domName :=  Out(x::key)
    end_if:
    if domtype(domName) = DOM_STRING then
      domName := hold(``).domName
    end;
    domName
  end:
DOM_EXEC::Content:=      (Out, x) ->
                         if domtype(op(x,4)) = DOM_STRING then
                           Out(text2expr(op(x,4)))
                         elif domtype(op(x,3)) = DOM_STRING then
                           Out(text2expr(op(x,3)))
                         else
                           Out(hold(builtin)(op(x)))
                         end_if;
DOM_EXPR::Content:=
   loadproc(DOM_EXPR::Content, pathname("OUTPUT", "DOMAINS"), "expr"):
DOM_FAIL::Content:= (Out, x) -> Out::Cci(FAIL):
DOM_FLOAT::Content    := proc(Out, x)
                           local exponent, hasSign, strX, res, oldTrailing;
                           save DIGITS;
                         begin
                           if x = RD_NAN then
                             return(Out::Cnotanumber())
                           end;
                           
                           // conversion to string respects all the output settings
                           oldTrailing := Pref::trailingZeroes(TRUE);
                           strX := expr2text(x);
                           Pref::trailingZeroes(oldTrailing);
                           if strX[1] = "-" then
                             hasSign := TRUE;
                             strX := strX[2..-1];
                           else
                             hasSign := FALSE;
                           end_if;
                           
                           // converting a string to float conserves the number of digits,
                           // if it is more. Ensure it is:
                           DIGITS := 2;
                           
                           if Pref::floatFormat() = "f" then
                             res := Out::Ccn(text2expr(strX));
                           elif strmatch(strX, "e") then
                             [x, exponent] := strmatch(strX, "(.*)e(.*)", All)[1][2..3];
                             exponent := text2expr(exponent);
                             res := Out::Capply(Out::Ctimes,
                               Out::Ccn(text2expr(x)),
                               Out::Capply(Out::Cpower,
                                 Out(10), Out(exponent)));
                           else
                             res := Out::Ccn(text2expr(strX));
                           end_if;
                           
                           if hasSign then
                             res := Out::Capply(Out::Cminus, res);
                           end_if;
                           res;
                         end_proc:
DOM_FUNC_ENV::Content:=
   loadproc(DOM_FUNC_ENV::Content, pathname("OUTPUT", "DOMAINS"), "funcenv"):
DOM_IDENT::Content:= (Out, x) -> Out::Cci(x):
DOM_INT::Content      := (Out, x) ->
                         if x < 0 then
                           Out(hold(_negate)(-x))
                         else
                           Out::Ccn(["type"="integer"], x)
                         end_if:
DOM_NIL::Content      := (Out, x) -> Out::Cci(NIL):
DOM_NULL::Content     := proc(Out, x) begin Out::Cci(hold(`&NULL;`)) end:
DOM_POLY::Content:=
   loadproc(DOM_POLY::Content, pathname("OUTPUT", "DOMAINS"), "poly"):
DOM_PROC::Content     :=
  proc(Out, data)
    save _varStack_;
    local procSubs, doIt, formals;
  begin
    if contains({op(data,3)}, hold(arrow)) then
      procSubs :=
      proc(x)
        name procSubs;
        save _varStack_;
        local i;
      begin
        if hold(_varStack_) = _varStack_ then
          _varStack_ := [[op(x,[1,i]) $ i=1..nops(op(x,1))]];
          if op(x, 12) <> NIL and op(x, 12) <> FAIL then
            _varStack_ := _varStack_.[[op(x, [12, 1, 1])]]
          end_if;
        else
          _varStack_ := [[op(x,[1,i]) $ i=1..nops(op(x,1))]]._varStack_;
        end_if;
        if domtype(op(x, 4)) = DOM_EXPR then
          subsop(subsop(x, 4 = map(op(x, 4), doIt, Unsimplified),
                        Unsimplified),
                 [4, 0] = doIt(op(x, [4,0])), Unsimplified)
        else
          subsop(x, 4 = map(op(x, 4), doIt, Unsimplified), Unsimplified)
        end:
      end_proc:
      doIt :=
      proc(x)
        name doIt;
        save _varStack_;
        local i, j, subsList;
      begin
        subsList := [];
        if domtype(_varStack_) = DOM_LIST then
          for i from 1 to nops(_varStack_) do
            for j from 1 to nops(_varStack_[i]) do
              subsList := subsList.[DOM_VAR(i-1,j+1) = _varStack_[i][j]]
            end_for;
          end_for;
        end_if;
        misc::maprec(x,
                     {DOM_VAR} = (x -> subs(x, subsList, Unsimplified)),
                     {DOM_PROC} = procSubs,
                     Unsimplified):
      end:

      data := doIt(data);
      formals := op(data,1);
      if formals = NIL then
        formals := hold(`()`);
      end_if;
      Out::Clambda(Out::Cbvar(Out(formals)), Out(op(data,4)));
    else
      Out(hold(``).DOM_PROC::print(data)):
    end_if
  end_proc:
DOM_VAR::Content      := (Out, x) -> if [op(x)] = [0, 0] then
                                       Out(hold(``)."procname")
                                     elif [op(x)] = [0, 1] then
                                       Out(hold(``)."dom")
                                     else
                                       Out(hold(DOM_VAR)(op(x)))
                                     end_if:
DOM_RAT::Content      := (Out, x) ->
                         if x < 0 then
                           Out(hold(_negate)(-x)):
                        else
                           Out::Ccn(["type"="rational"],
                                    op(x,1), Out::Csep(), op(x,2)):
                         end_if:
DOM_LIST::Content     := proc(Out, x) local i;
                         begin
                           Out::Clist(Out(op(x,i)) $ i=1..nops(x))
                         end_proc:
DOM_SET::Content      := proc(Out, x : DOM_SET)
                           local i, s;
                         begin
                           s := DOM_SET::sort(x);
                           Out::Cset(Out(s[i]) $ i = 1..nops(s))
                         end_proc:
DOM_STRING::Content   := (Out, x) -> x:
DOM_TABLE::Content    :=
proc(Out, tbl)
  local i, res;
begin
  tbl := sort([op(tbl)],
              (x,y)->if traperror((res := bool(x[1]<y[1]))) <> 0 then
                       if traperror((res := sysorder([x[1]], [y[1]]))) <> 0 then
                         // e.g. table((1,2)=x1, 2=x2)
                         FALSE
                       else
                         res
                       end
                     else
                       res
                     end);
  Out::Ctable(
		 (Out(op(tbl, [i,1])), Out(op(tbl, [i, 2])))
                         $ i = 1..nops(tbl)
                 )
end_proc:


// ------------------------------------------------------------------
//  Code generation [see 'lib/GENERATER/CF.mu']
// ------------------------------------------------------------------
// define '_assign::CF' slot
old_assign::CF:= (e, p, t, opts) -> generate::_assign__CF(e, p, t, opts);


// ------------------------------------------------------------------
//  Define integral transforms for kernel functions
// ------------------------------------------------------------------
path:= pathname("TRANS","LAPLACE");
_mult::"transform::laplace"   := loadproc(_mult::"transform::laplace",
path, "L_mult"):
_mult::"transform::invlaplace":= loadproc(_mult::"transform::invlaplace",
                                          path, "IL_mult"):
_power::"transform::laplace"  := loadproc(_power::"transform::laplace",
                                          path, "L_power"):
_power::"transform::invlaplace"  := loadproc(_power::"transform::invlaplace",
                                          path, "IL_power"):
diff::"transform::laplace"    := loadproc(diff::"transform::laplace",
                                          path, "L_diff"):

// wurden bisher mit 'domains' geladen:
path:= pathname("DOMAINS", "CONSTR");
_constructor:=	      loadproc(_constructor,        path, "constr");
DomainConstructor:=   loadproc(DomainConstructor,   path, "DomCons");
Category:=            loadproc(Category,            path, "Category");
CategoryConstructor:= loadproc(CategoryConstructor, path, "CatCons");
Axiom:=               loadproc(Axiom,               path, "Axiom");
AxiomConstructor:=    loadproc(AxiomConstructor,    path, "AxCons");

//--------------------------------------------------------------------
//        Methods for basic data types
//--------------------------------------------------------------------
path:= pathname("INTLIB");
DOM_POLY::int:= loadproc( DOM_POLY::int, path, "DOM_POLY" ):

_intersect():=universe:
_minus::_intersect := proc() begin solvelib::solve_intersect( args() ); end_proc:
_union::_intersect := proc() begin solvelib::solve_intersect( args() ); end_proc:
_minus::_union := proc() begin solvelib::solve_union( args() ); end_proc:
_intersect::_union := proc() begin solvelib::solve_union( args() ); end_proc:
/* not possible earlier because stdlib must be loaded first */

//--------------------------------------------------------------------
// path definitions for kernel secure mode
//--------------------------------------------------------------------

//if _pref(SecureKernel) then // andi, 12.10.2000 (future feature)
//    WRITEPATH:="/tmp":
//end_if:
//--------------------------------------------------------------------
// initialize modules
//--------------------------------------------------------------------
if loadmod() = TRUE then
  path     := pathname("MODULE");
  shell    := loadproc(shell,    path, "shell");
  stdmod   := loadproc(stdmod,   path, "stdmod");
  hfa      := loadproc(hfa,      path, "hfa");
  stdlib::uspensky
           := loadproc(stdlib::uspensky, path, "uspensky");
//  util     := loadproc(util,     path, "util");
  stdlib::vcam
           := loadproc(stdlib::vcam,     path, "vcam");
  xmlprint := loadproc(xmlprint, path, "xmlprint");

  if sysname() = "UNIX" then
//  cdebug := loadproc(cdebug,   path, "cdebug");
  end_if:
end_if:


// replacement for the internal system command;
system :=
proc()
  local res;
begin
  if traperror((res := shell::system(args()))) <> 0 then
    return(1)
  else
    print(Unquoted, res);
    return(0)
  end
end:

//--------------------------------------------------------------------
// wrapper called from kernel for write in text mode
//--------------------------------------------------------------------

writeTextWrapper :=
() -> misc::maprec(text2expr(expr2text(args())), {DOM_IDENT} =
                   (x -> if contains({DOM_DOMAIN}, domtype(eval(x))) or
                            contains({hold(array), hold(hfarray),
                                      hold(table)}, x) then
                           x
                         else
                           hold(hold)(x)
                         end),
                   Unsimplified):

//--------------------------------------------------------------------
// Initialize global variables
//--------------------------------------------------------------------
` saved values for prog::remember-functions ` := table():

//--------------------------------------------------------------------
// Remember protected identifiers
//--------------------------------------------------------------------

stdlib::PROTECTED :=
    (stdlib::anames(3)
      minus indexval(stdlib, "ENVIRONMENT_VARIABLES")
      minus indexval(stdlib, "SYSTEM_CONSTANTS")
      minus aux_globals
      minus {hold(_mod)})
      union indexval(stdlib, "LIBRARY_CONSTANTS")
      union indexval(stdlib, "OPTIONS")
      union {PROPERTIES, PI, EULER, CATALAN}:

map(indexval(stdlib, "OPTIONS"), protected, Error);
stdlib::SYSPROTECTED := stdlib::PROTECTED;

map((stdlib::anames(3)
      minus indexval(stdlib, "ENVIRONMENT_VARIABLES")
      minus indexval(stdlib, "SYSTEM_CONSTANTS")
      minus aux_globals
      minus {hold(_mod)})
      union indexval(stdlib, "LIBRARY_CONSTANTS")
      union {PROPERTIES, PI, EULER, CATALAN},
    protected, Error);

 // set callOnExit Procedure, which deletes temporary files
_pref(hold(CallOnExit)=[()->map(stdlib::TempFiles, x -> stdlib::gprof(NIL, x))]):


end_proc(); //------------------------- end of anonymous aux proc

sysassign( _assign, old_assign ):
sysdelete( old_assign ):

//--------------------------------------------------------------------
// Init random generator
//--------------------------------------------------------------------
SEED:= 1:

//--------------------------------------------------------------------
// Versionsnummern von Kern und Library vergleichen.
//--------------------------------------------------------------------

KernelVersion := _pref(hold(Kernel)):
if(KernelVersion[1..3] <> version()) then
    fprint(Unquoted, 0,
          "Warning: Kernel and library release number differ!\n".
          "Kernel : ".expr2text(KernelVersion[1..3])."\n".
          "Library: ".expr2text(version())."\n"
    );
end_if;
if nops(KernelVersion) < 4 or KernelVersion[4] = "" then
    fprint(Unquoted, 0,
          "Warning: This kernel has no BuildNumber!  BuildNumber ".stdlib::MinKernelBuildNumber." expected.");
else
  KernelVersion := op(strmatch(KernelVersion[4], "^[0-9]*", All)):
  if text2expr(KernelVersion) < stdlib::MinKernelBuildNumber then
  //  stdlib::MinKernelBuildNumber is set in LIBFILES/stdlib.mu
    fprint(Unquoted, 0,
          "Warning: This library expects a kernel with a minimal BuildNumber of ".stdlib::MinKernelBuildNumber.".");
  end_if;
end_if;
delete KernelVersion;
): // trick to suppress any output

//- the end ----------------------------------------------------------
