

//--------------------------------------------------------------------
// MuPAD - Systeminitialisierungsdatei
//--------------------------------------------------------------------

(// trick to suppress all output

//--------------------------------------------------------------------
// Initialize domains for buit-in types
//--------------------------------------------------------------------

old_assign := _assign;
protected(hold(_assign), None);
sysassign( _assign, sysassign );

sysdelete( Dpoly ):   // use polylib::Dpoly 

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

// -------- DOM_BOOL --------
DOM_BOOL:= domtype(TRUE);
DOM_BOOL::new:= proc() name new; begin args() end_proc;
DOM_BOOL::new_extelement:= DOM_ARRAY::new_extelement;

// -------- DOM_COMPLEX --------
DOM_COMPLEX:= domtype(I);
DOM_COMPLEX::D := 0 ;
DOM_COMPLEX::new:= proc() name new; begin args(1)+I*args(2) end_proc;
DOM_COMPLEX::new_extelement:= DOM_ARRAY::new_extelement;
DOM_COMPLEX::_index := (x, ind) -> if contains({1,2}, ind) then
                                     op(x, ind)
                                   else
                                     FAIL
                                   end_if:
 
// -------- DOM_DOMAIN --------
DOM_DOMAIN:= domtype(newDomain("DOM_DOMAIN"));
DOM_DOMAIN::func_call:= _domainfunccall ;
DOM_DOMAIN::new:= newDomain;
DOM_DOMAIN::new_extelement:= DOM_ARRAY::new_extelement;

// -------- DOM_EXEC --------
DOM_EXEC:= domtype(op(_assign, 1));
DOM_EXEC::new:= builtin;
DOM_EXEC::new_extelement:= DOM_ARRAY::new_extelement;

// -------- DOM_EXPR --------
DOM_EXPR:= domtype(XX+1);
DOM_EXPR::testtype:= proc(x,T) name testtype; option noDebug; 
begin
    if domtype(T) = DOM_EXPR then
        if domtype(x) <> DOM_EXPR then
            FALSE
        elif op(x, 0) <> op(T, 0) then
            FALSE
        elif nops(x) <> nops(T) then
            FALSE
        elif nops(x) = 0 then
            TRUE
        elif nops(x) = 1 then
            testtype(op(x, 1), op(T, 1))
        else
            testtype(op(x), Type::Product(op(T)))
        end_if
    else
        case T
        of domtype(x) do
        //of DOM_STRING do
        //of DOM_POLY do
            return( TRUE )
        otherwise
            return( FAIL )
        end_case
    end_if
end_proc;
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
    local T;
begin
    T:= x::dom;
    case T
    of DOM_STRING do
        return(expr2text(x))
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
    local T;
begin
    T:= x::dom;
    case T
    of DOM_STRING do
        return(expr2text(x))
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
        return(expr2text(x))
    of DOM_FLOAT do
        return(float(x))
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
    local T, p;
begin
    T:= x::dom;
    case T
    of DOM_EXPR do
        if traperror( (p:= poly(x) ) ) <> 0 then
            return( FAIL )
        else
            // t could be FAIL
            return( p )
        end_if
    of DOM_LIST do
        p:= [op(indets(x))];
        if traperror((p:= poly(x,p))) <> 0 then
            return( FAIL )
        else
            return( p )
        end_if
    of DOM_STRING do
        return(expr2text(x))
    otherwise
        return(T::convert_to(x,DOM_POLY))
    end_case
end_proc:

// -------- DOM_RPOLY -------
DOM_RPOLY:= domtype(rpoly(XX));
DOM_RPOLY::new:= rpoly;
DOM_RPOLY::new_extelement:= DOM_ARRAY::new_extelement;

// -------- DOM_PROC --------
DOM_PROC:= domtype(proc() begin end_proc);
DOM_PROC::new:= _procdef;
DOM_PROC::print :=
proc(p)
  local Name, param, str;
  option noDebug;
begin
  if contains({op(p,  3)}, hold(arrow)) then
    str := stringlib::collapseWhitespace(expr2text(p));
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

// -------- DOM_VAR --------
DOM_VAR:= domtype(op(proc(x) begin x end_proc, 4));
DOM_VAR::new:= proc(d,i) name new; begin 
    subsop(op(proc(x) begin x end_proc, 4), 1=d, 2=i, Unsimplified);
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
        return(expr2text(x))
    of DOM_FLOAT do
        return(float(x))
    otherwise
        return(T::convert_to(x,DOM_RAT))
    end_case
end_proc:
DOM_RAT::_index := (x, ind) -> if contains({1,2}, ind) then
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
        return( [op(x)] )
    of DOM_POLY do
        return(poly2list(x))
    of DOM_STRING do
        return(expr2text(x))
    otherwise
        return(T::convert_to(x,DOM_LIST))
    end_case
end_proc:

// -------- DOM_SET --------
DOM_SET:= domtype({});

// -------- DOM_STRING --------
DOM_STRING:= domtype("");
DOM_STRING::testtype:= proc(x,T) name testtype; option noDebug; local Op0;
begin
  if domtype(T) = DOM_STRING then
    if domtype(x) = DOM_EXPR then
      Op0 := eval(op(x, 0));
      bool(type(x) = T 
           or (T = "function" and domtype(Op0) = DOM_FUNC_ENV
               // std functional output iff
               //     output func is no DOM_EXEC (int, sum, abs, ...)
               // OR  is a DOM_EXEC for functional output
               and _lazy_or(domtype(op(Op0, 2)) <> DOM_EXEC or op(Op0, [2,1]) = 1101)))
    else
        FALSE
    end_if;
  elif domtype(x) = T then
    TRUE
  else
    FAIL
  end_if
end_proc;
DOM_STRING::coerce:= x -> expr2text(x):
DOM_STRING::_less := proc(x:DOM_STRING, y:DOM_STRING) name _less; begin
  bool(sort([x, y])=[x, y] and x <> y)
end_proc;
DOM_STRING::_leequal := proc(x:DOM_STRING, y:DOM_STRING) name _leequal; begin
  bool(sort([x, y])=[x, y])
end_proc;
DOM_STRING::new:= DOM_BOOL::new;
DOM_STRING::new_extelement:= DOM_ARRAY::new_extelement;
// for the internal Print methods
DOM_STRING::Print := id:

// -------- DOM_TABLE --------
DOM_TABLE:= domtype(table());
DOM_TABLE::func_call:= DOM_ARRAY::func_call;
DOM_TABLE::new:= table;
DOM_TABLE::new_extelement:= DOM_ARRAY::new_extelement;
DOM_TABLE::coerce:= proc(x)
    local T;
begin
    T:= x::dom;
    case T
    of DOM_STRING do
        return(expr2text(x))
    otherwise
        return(T::convert_to(x,DOM_TABLE))
    end_case
end_proc:
  
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
  (Out, data) -> if _mod = modp then
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
  (Out, data) -> if _mod = mods then
                   Out::Capply(Out::Crem, map(op(data), Out))
                 else
                   Out::stdFunc(data)
                 end:
mods::type := "_mod":
_mod := modp:

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
	   save l;
	   eval(hold(_delete)(l));
	   r := context(subs(r, substs));
	   if hastype(r, DOM_VAR) then
	     error("right hand side contains references to local variables");
	   end_if;
	   fp::unapply((r), (l));
	 end_proc:

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
_union::float     := () -> hold(_union)    (op(map([args()], float))):
_intersect::float := () -> hold(_intersect)(op(map([args()], float))):

//--------------------------------------------------------------------
// Define shortcuts for basic arithmetic operators                    
//--------------------------------------------------------------------

`+`  := hold(_plus):
`-`  := hold(_negate):
`*`  := hold(_mult):
`/`  := hold(_divide):
`^`  := hold(_power):
`**` := hold(_power):
`=`  := hold(_equal):
`<=` := hold(_leequal):
`<>` := hold(_unequal):
`<`  := hold(_less):
`>`  := (a,b) -> a>b:
`>=` := (a,b) -> a>=b:
`==>`:= hold(_implies):
`|`  := hold(evalAt):


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

//--------------------------------------------------------------------
// Erzeugung des stdlib-Domains (wird von loadproc benoetigt)	     
// The 'stdlib' library domain serves only as name space for utilies  
//--------------------------------------------------------------------

stdlib:= newDomain("stdlib");
stdlib::info:= "Library 'stdlib': the basic functionality of MuPAD":

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
      of "_less" do  
       op(a,0)(op(a, 1)+op(b, 1), op(a, 2)+op(b,2));
       break
      of "_unequal" do
       FAIL;
       break;
      of DOM_SET do
      of DOM_LIST do
       map(b, x->a+x);
       break;
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
      otherwise
       op(a,0)(op(a, 1)+b, op(a, 2)+b);
   end_case
  end_proc:

// (A_1 union ... union A_n)+B = (A_1+B) union ... (A_n+B)
_union::_plus    := (a, b) -> map(a, _plus, b):

_intersect::_plus := () -> hold(_plus)(args()):
_minus::_plus     := () -> hold(_plus)(args()):
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
      otherwise
       if is(b=0) = FALSE then
         op(a,0)(op(a, 1)*b, op(a, 2)*b);
       else
         hold(_mult)(args());
       end_if;
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

 _intersect::_mult := () -> hold(_mult)(args()):
_minus::_mult      := () -> hold(_mult)(args()):
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
_union::_power      := () -> hold(_power)(args()):
_intersect::_power  := () -> hold(_power)(args()):
_minus::_power      := () -> hold(_power)(args()):
_and::__power       := () -> hold(_power)(args()):
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

//--------------------------------------------------------------------------
// Kern-Funktionen sichern (werden in specfunc bzw. polylib umdefiniert)    
//--------------------------------------------------------------------------

specfunc:= newDomain("specfunc");
specfunc::info:= "Library 'specfunc': elementary and special functions";

specfunc::sin:= sin;
specfunc::cos:= cos;
specfunc::tan:= tan;
specfunc::arcsin:= arcsin;
specfunc::arccos:= arccos;
specfunc::arctan:= arctan;
specfunc::sinh:= sinh;
specfunc::cosh:= cosh;
specfunc::tanh:= tanh;
specfunc::arcsinh:= arcsinh;
specfunc::arccosh:= arccosh;
specfunc::arctanh:= arctanh;
specfunc::exp:= exp;
specfunc::ln:= ln;
specfunc::sqrt:= sqrt;
specfunc::gamma:= gamma;
specfunc::igamma:= igamma;
specfunc::zeta:= zeta;
specfunc::erfc:= erfc;
specfunc::sign:= sign;
specfunc::abs:= abs;
specfunc::fact:= fact;
specfunc::psi:=psi;

specfunc::lambertW := lambertW;
delete lambertW;
specfunc::Ei:= funcenv(builtin(1001, 1325, "Ei", NIL),
		    builtin(1101, 0, NIL, "Ei" ),
		    NIL );
delete eint;
specfunc::Ci:=Ci;
specfunc::Si:=Si;
specfunc::floor:= floor;
specfunc::ceil:= ceil;
specfunc::frac:= frac;
specfunc::round:= round;
specfunc::trunc:= trunc;
// We need to give PARI's ibeta another name,
// because ibeta is reserved for a symbolic 
// library routine (to be implemented in the future)
// Presently, the numerical library wrapper specfunc::ibeta
// calls the PARI routine specfunc::ibetaPARI
specfunc::ibetaPARI:= ibeta;
delete ibeta;

// keep polynomial kernel functions in std lib
stdlib::lterm:=lterm;
stdlib::lmonomial:=lmonomial;
stdlib::lcoeff:=lcoeff;
stdlib::tcoeff:=tcoeff;
stdlib::nthterm:=nthterm;
stdlib::nthcoeff:=nthcoeff;
stdlib::nthmonomial:=nthmonomial;
stdlib::degreevec:=degreevec;


stdlib::powermod:= powermod;
sysdelete(powermod):
stdlib::ithprime:= ithprime;
sysdelete(ithprime):
stdlib::max:= max;
stdlib::min:= min;
stdlib::anames:= anames;

stdlib::expr:= expr:
delete expr:

stdlib::exp1 := exp1:
delete exp1:

stdlib::ifactor:= ifactor: // keep the kernel function 'ifactor'
sysdelete( ifactor ):      // use stdlib::ifactor
stdlib::frandom:= frandom;
sysdelete(frandom);

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

stdlib::hasfloat := `stdlib::hasfloat`:
sysdelete(`stdlib::hasfloat`):

stdlib::interval := interval:
sysdelete(interval):

// the lib-readbytes searches the READPATH, so move the
// Kernel-ffunction to stdlib::
stdlib::readbytes:= readbytes: // keep the kernel function 'readbytes'
sysdelete( readbytes ):      // use stdlib::ifactor


// free global namespace
stdlib::gprof := gprof:
delete gprof:
stdlib::gcov := gcov:
delete gcov:

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

// warning for usage of obsolete table
stdlib::LoadlibSTATE := newDomain(stdlib::LoadlibSTATE):
stdlib::LoadlibSTATE::set_index :=
      proc() name stdlib::LoadlibState;
      begin
        warning("stdlib::LoadlibState is obsolete");
      end_proc:
stdlib::LoadlibState := new(stdlib::LoadlibSTATE):

// use library patch archive first if one exists 

filedescr := fopen(lib_path.path_sep."plib.tar");
if filedescr <> FAIL then
    fclose(filedescr);
    delete filedescr;
    LIBPATH := lib_path.path_sep."plib.tar#plib", LIBPATH;
    sysread("STDLIB".stdlib::PathSep."patchlev");
else
    patchlevel:= proc() name patchlevel; begin 0 end_proc;
end_if;

sysread("STDLIB".stdlib::PathSep."alias");
sysread("STDLIB".stdlib::PathSep."unalias");
sysread("STDLIB".stdlib::PathSep."pathname");
if strmatch(_pref(hold(UserOptions)), "\*ANALYZE\*") then
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
RGB      := loadproc(RGB,       path, "RGB");
Series   := loadproc(Series,    path, "Series");
plot     := loadproc(plot,      path, "plot");
polylib  := loadproc(polylib,   path, "polylib");
Pref     := loadproc(Pref,      path, "Pref");
property := loadproc(property,  path, "property");
logic    := loadproc(logic,     path, "logic");
Type     := loadproc(Type,      path, "Type");
adt      := loadproc(adt,       path, "adt");
combinat := loadproc(combinat,  path, "combinat");
crypto   := loadproc(crypto,    path, "crypto");
detools  := loadproc(detools,   path, "detools");
domains  := loadproc(domains,   path, "domains");
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
operators:= loadproc(operators, path, "operators");
orthpoly := loadproc(orthpoly,  path, "orthpoly");
output   := loadproc(output,    path, "output");
prog     := loadproc(prog,      path, "prog");
rec      := loadproc(rec,       path, "rec");
Simplify := loadproc(Simplify,  path, "simplify");
Rule     := loadproc(Rule,      path, "Rule");
solvelib := loadproc(solvelib,  path, "solvelib");
stats    := loadproc(stats,     path, "stats");
stringlib:= loadproc(stringlib, path, "stringlib");
student  := loadproc(student,   path, "student");
transform:= loadproc(transform, path, "transform");
unit     := loadproc(unit,      path, "unit");

//////////////////////////////////////////////////////////////////////
//       initialize basic sets 
//////////////////////////////////////////////////////////////////////

Z_:= loadproc(Z_, path,"solvelib"):
Q_:= loadproc(Q_, path,"solvelib"):
R_:= loadproc(R_, path,"solvelib"):
C_:= loadproc(C_, path,"solvelib"):


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
ContentLayout  := loadproc(ContentLayout, pathname("OUTPUT"), "ContentLayout"):
// only needed for typesetting ASCII fallback
_lastContentOutput := FAIL:
 
//--------------------------------------------------------------------
// Content methods of kernel function.  They have to be defined before
// the content library is loaded, otherwise we have problems with
// freeze and hold in Typesetting output!!
// only needed for output library                                     
//--------------------------------------------------------------------

_mult_intern := funcenv(_mult_intern):
_mult_intern::type := "_mult_intern":
_mult_intern::Content :=
    loadproc(_mult_intern::Content, pathname("OUTPUT", "EXPR"), "mult"):

old_assign::Content := (Out, data) ->
                       Out::Capply(Out::Cassign, op(map([op(data)],Out))):

_exprseq::Content    := Out -> Out::Cexprseq(map(args(2..args(0)), Out)):
_div::Content        := (Out, data) -> Out::Capply(Out::Cquotient, map(op(data), Out)):
_union::Content      := (Out, data) -> Out::Capply(Out::Cunion, map(op(data), Out)):
_intersect ::Content := (Out, data) -> Out::Capply(Out::Cintersect, map(op(data), Out)):
_minus ::Content := (Out, data) -> Out::Capply(Out::Csetdiff, map(op(data), Out)):
_concat::Content     := (Out, data) -> Out::Capply(Out::Cconcat, map(op(data), Out)):

_equal::Content      := (Out, data) -> Out::Capply(Out::Ceq, map(op(data), Out)):
_fconcat::Content    := (Out, data) -> Out::Capply(Out::Ccompose, map(op(data), Out)):
_leequal::Content    := (Out, data) -> Out::Capply(Out::Cleq, map(op(data), Out)):
_less::Content       := (Out, data) -> Out::Capply(Out::Clt, map(op(data), Out)):
_or::Content         := (Out, data) -> Out::Capply(Out::Cor, map(op(data), Out)):
_and::Content        := (Out, data) -> Out::Capply(Out::Cand, map(op(data), Out)):
_range::Content      := (Out, data) -> Out::Capply(Out::Crange, map(op(data), Out)):
_unequal::Content    := (Out, data) -> Out::Capply(Out::Cneq, map(op(data), Out)):

_not::Content        := (Out, data) -> Out::Capply(Out::Cnot, Out(op(data))):
_negate::Content     := (Out, data) -> Out::Capply(Out::Cminus, Out(op(data,1))):
_invert::Content     := (Out, data) -> Out::Capply(Out::Cdivide, Out(1),Out(op(data))):
_divide::Content     := (Out, data) -> Out::Capply(Out::Cdivide, Out(op(data,1)), Out(op(data,2))):
norm::Content        := (Out, data) -> Out::Capply(Out::Cnorm, map(op(data), Out)):


_index::Content   := (Out, data) -> Out::Cci(Out::Cindex(Out(op(data, 1)),
                                              Out(op(data, 2..nops(data))))):
diff::Content     := proc(Out, data) 
                       local i, bvar, varList;
                     begin
                         if nops(data) = 1 then return(Out(op(data))) end:

                         bvar := op(data, nops(data)); i := nops(data);
                         while i > 2 and op(data, i-1) = bvar do
                           i := i-1;
                         end_while; 
                         varList := op(data, 2..i-1);
                         if i = nops(data) then
                            Out::Capply(Out::Cpartialdiff, 
                                        Out::Cbvar(Out(bvar)),
                                        Out(hold(diff)(op(data, 1), varList)) 
                                        ):
                         else
                            Out::Capply(Out::Cpartialdiff, 
                                        Out::Cbvar(Out(bvar), Out::Cdegree(nops(data)-i+1)),
                                        Out(hold(diff)(op(data, 1), varList))
                                        ):
                         end_if;
                     end_proc:
_mult::Content    := loadproc(_mult::Content, pathname("OUTPUT", "EXPR"), "mult"):
_plus::Content    := proc(Out, data)
                       local sorted;
                     begin
                       sorted := [op(generate::sortSum(data))];
                       sorted := map(sorted, x -> if generate::isNeg(x) then
                                                    hold(_mult)(generate::negate(x), -1)
                                                  else
                                                    x
                                                  end);
                       Out::Capply(Out::Cplus, op(map(sorted, Out))):
                       end_proc:
_subtract::Content:= (Out, data) -> Out::Capply(Out::Cminus,
                                                Out(op(data,1)),
                                                Out(op(data,2))):
_power::Content   := loadproc(_power::Content, pathname("OUTPUT", "EXPR"), "power"):
_seqgen::Content  := (Out, data) -> error("not yet implemented"):
_seqin::Content  := (Out, data) -> error("not yet implemented"):
_seqstep::Content  := (Out, data) -> error("not yet implemented"):
slot::Content     := proc(Out, data) 
                       local x, key;
                     begin
                       key := op(data, 2);
                       if domtype(key) = DOM_STRING and
                          traperror((x := text2expr(key))) = 0 and
                          nops(data) = 2 then
                         Out::Capply(Out::Cslot, Out(op(data,1)), x)
                       else
                         Out(hold(slot)(op(data)))
                       end_if:
                     end_proc:

DOM_ARRAY::Content :=
   loadproc(DOM_ARRAY::Content, pathname("OUTPUT", "DOMAINS"), "array"):
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
                           local exponent, hasSign, signLessX, res;
                         begin
                           if x = RD_NAN then
                             return(Out::Cnotanumber())
                           end;
                           // reduce number of valid digits to
                           // current value of DIGITS
                           x := x*float(1); 
                           hasSign := if x<0 then TRUE else FALSE end;
                           signLessX := abs(x);
                           if signLessX <> 0.0 and
                              (signLessX > 10^DIGITS or 
                               signLessX < 10^(-DIGITS)) then
                             exponent := floor(ln(signLessX)/ln(10.0));
                             res := Out(hold(_mult)(signLessX*10^(-exponent), 
                                                    hold(_power)(10, exponent)));
                             if hasSign then
                               return(Out::Capply(Out::Cminus, res))
                             else
                               return(res)
                             end_if;
                           else
                             if hasSign then
                               Out(hold(_negate)(-x)):
                             else
                               Out::Ccn(x):
                             end_if;
                           end_if:
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
    local procSubs, doIt;
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
      Out::Capply(Out::Clambda, 
                  Out::Cbvar(Out(op(data,1))),
                  Out(op(data,4))
                     );
    else
      Out(hold(``).DOM_PROC::print(data)):
    end_if
  end_proc:
DOM_VAR::Content      := () -> error("not yet implemented"):
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
DOM_TABLE::Content    := proc(Out, tbl)
  local i;
begin
    Out::Ctable(
		 (Out(op(tbl, [i,1])), Out(op(tbl, [i, 2])))
                         $ i = 1..nops(tbl)
                 )
end_proc:


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
/* not possible earlier because stdlib must be loaded first */


//--------------------------------------------------------------------
// Warnung bei unbekannten globalen Variablen			     
//--------------------------------------------------------------------

_check_global:= proc(v, p) option hold, noDebug; local s; begin
    if contains({hold(ERRORLEVEL),hold(PRINTLEVEL),hold(EVAL_STMT),
                 hold(LIB_PATH), hold(READ_PATH), hold(WRITE_PATH), 
                 hold(PRETTY_PRINT)}, v)
    then s:= "Obselete environment variable '".v."' used"
    elif v = hold(HISTORY)
    then s:= "HISTORY may no longer be changed for procedures"
    elif contains({hold(fun),hold(func),hold(newpurefunc),
        hold(newfuncarg)}, v)
    then s:= "Pure functions no longer available [".v."]"
//  Bleibt dieser Zweig drin, so wird die ueberfluessige Warnung:
//
//     >> proc(y) begin (x->x^2)(y) end
//     Warning: Unknown global identifier 'x'
//
//  ezeugt. 
/*  elif protected(v) <> hold(Error) and
    not contains(indexval(stdlib, "ENVIRONMENT_VARIABLES"), v) and
    not contains(indexval(stdlib, "SYSTEM_CONSTANTS"), v) and
    not contains(indexval(stdlib, "LIBRARY_CONSTANTS"), v) and
    not contains(indexval(stdlib, "OPTIONS"), v) and
    (_parser_config() = null() or not contains(_parser_config()[3],v)) and
    not contains(indexval(Type, "interface"),v)
    then s:= "Unknown global identifier '".v."'" 
*/
    else return(null())
    end_if;
    if p <> NIL then
        s:= s." in procedure '".expr2text(p)."'"
    end;
    warning(s);
end_proc:

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


//--------------------------------------------------------------------
// Remember protected identifiers				     
//--------------------------------------------------------------------

package("scilab", Quiet):

 stdlib::PROTECTED := 
    (stdlib::anames(3)
      minus indexval(stdlib, "ENVIRONMENT_VARIABLES")
      minus indexval(stdlib, "SYSTEM_CONSTANTS")
      minus aux_globals)
      union indexval(stdlib, "LIBRARY_CONSTANTS")
      union indexval(stdlib, "OPTIONS")
      union {bna}:

map((stdlib::anames(3)
      minus indexval(stdlib, "ENVIRONMENT_VARIABLES")
      minus indexval(stdlib, "SYSTEM_CONSTANTS")
      minus aux_globals)
      union indexval(stdlib, "LIBRARY_CONSTANTS")
      union {bna},
    protected, Error);

map(indexval(stdlib, "OPTIONS"), protected, Warning);
stdlib::SYSPROTECTED := stdlib::PROTECTED;

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

if( _pref(hold(Kernel)) <> version() ) then
    print( Unquoted, 
           "Warning: Kernel and library release number differ!\n".
           "Kernel : ".expr2text(_pref(hold(Kernel)))."\n".
           "Library: ".expr2text(version())."\n"
    );
end_if;

): // trick to suppress any output

//- the end ----------------------------------------------------------
