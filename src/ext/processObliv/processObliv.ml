
open OblivUtils
open Pretty
open Printf
open Cil
open Feature
module E = Errormsg
module H = Hashtbl

let rec checkOblivType t = match t with
| TVoid a -> if hasOblivAttr a then Some "void" else None
| TInt _ -> None
| TFloat(FFloat, _) -> None
| TFloat(_,a) -> if hasOblivAttr a then Some "unimplemented" else None
| TPtr(t,a) -> if hasOblivAttr a then Some "pointer" else checkOblivType t
| TArray(t,_,a) -> if hasOblivAttr a then Some "array" else checkOblivType t
| TFun(tres,targs,_,_) -> begin match checkOblivType tres with
                          | Some cat -> Some cat
                          | None -> firstSome checkOblivArg (argsToList targs)
                          end
| TNamed(tinfo,a) -> let t = if hasOblivAttr a then addOblivType tinfo.ttype 
                                               else tinfo.ttype in
                     checkOblivType t
| TComp(cinfo,a) -> let ct = if cinfo.cstruct then "struct" else "union" in
                    if hasOblivAttr a then Some ct else None
| TEnum(_,a) -> if hasOblivAttr a then Some "unimplemented" else None
| TBuiltin_va_list a -> if hasOblivAttr a then Some "va_list" else None
and checkOblivArg (_,t,_) = checkOblivType t
;;

let frozen = "frozen"

let boolType = TInt(IBool,[])

let oblivIntType = addOblivType intType
let oblivCharType = addOblivType charType
let oblivBoolType = addOblivType boolType
let constOblivBoolPtrType = 
  TPtr(typeAddAttributes [constAttr] oblivBoolType,[])

let widestType =  let tinfo = { tname = "widest_t"
                              ; ttype = TInt (ILongLong,[])
                              ; treferenced = true
                              } in TNamed (tinfo,[])

let dummyType = TVoid []
let oblivBoolTarget = ref dummyType
let oblivCharTarget = ref dummyType
let oblivIntTarget = ref dummyType
let oblivShortTarget = ref dummyType
let oblivLongTarget = ref dummyType
let oblivLLongTarget = ref dummyType
let oblivFloatTarget = ref dummyType
let oblivDoubleTarget = ref dummyType
let oblivLongDoubleTarget = ref dummyType

(* signed-ness affects code generation (e.g. signed vs unsigned comparison)
 * but not the generated data type *)
let intTargetType k = match k with
| IChar | ISChar | IUChar -> !oblivCharTarget
| IBool -> !oblivBoolTarget
| IInt | IUInt -> !oblivIntTarget
| IShort | IUShort -> !oblivShortTarget
| ILong | IULong -> !oblivLongTarget
| ILongLong | IULongLong -> !oblivLLongTarget
;;

let floatTargetType k = match k with
  | FFloat -> !oblivFloatTarget
  | FDouble -> !oblivDoubleTarget
  | FLongDouble -> !oblivLongDoubleTarget
;;

let dummyExp = Const (CChr 'x')
let condAssignKnown = ref dummyExp
let copySimpleObliv = ref dummyExp

let updateOblivBitType ci = begin
  oblivBitType := TComp(ci,[]);
  oblivBitPtr := TPtr(!oblivBitType,[]);
  cOblivBitPtr := TPtr(typeAddAttributes [constAttr] !oblivBitType,[]);
  oblivConstBitPtr := typeAddAttributes [constAttr] !oblivBitPtr;
  oblivBitsSize := bitsSizeOf !oblivBitType;
  let types = 
    [oblivBoolTarget,"__obliv_c__bool",1
    ;oblivCharTarget,"__obliv_c__char",bitsSizeOf charType
    ;oblivIntTarget,"__obliv_c__int",bitsSizeOf intType
    ;oblivShortTarget,"__obliv_c__short",bitsSizeOf (TInt(IShort,[]))
    ;oblivLongTarget,"__obliv_c__long",bitsSizeOf (TInt(ILong,[]))
    ;oblivLLongTarget,"__obliv_c__lLong",bitsSizeOf (TInt(ILongLong,[]))
    ;oblivFloatTarget,"__obliv_c__float",bitsSizeOf (TFloat(FFloat,[]))
    ] in
  List.iter (fun (tref,tname,bits) ->
    let ti = { tname = tname
             ; ttype = TArray(!oblivBitType,Some (integer bits),[])
             ; treferenced = true } in
    tref := TNamed(ti,[])
    ) types;
  (* TODO roll these up *)
  condAssignKnown := begin
    let fargTypes = [ "dest",!oblivBitPtr,[]; "cond",!oblivConstBitPtr,[]
                    ; "bitcount",intType,[]; "val",widestType,[] ] in
    let ftype = TFun (TVoid [],Some fargTypes,false,[]) in
    Lval (Var (makeGlobalVar "__obliv_c__condAssignKnown" ftype),NoOffset)
  end;
  copySimpleObliv := begin
    let fargTypes = [ "dest",!oblivBitPtr,[]; "src",!oblivConstBitPtr,[]
                    ; "bitcount",intType,[] ] in
    let ftype = TFun (TVoid [], Some fargTypes, false, []) in
    Lval (Var (makeGlobalVar "__obliv_c__copySimpleObliv" ftype),NoOffset)
  end;
end

(* Is t1 -> t2 conversion valid with regard to oblivious-ness *)
(* vtype ensures stupid types do not show up in declarations or casts.
 * So isOblivSimple can assume stupid types do not exist. *)
let invalidOblivConvert t1 t2 = isOblivSimple t1 && isNonOblivSimple t2

let frozenConversionError loc =
  E.s (E.error "%s:%i:cannot convert a frozen pointer to a non-frozen one"
      loc.file loc.line)

let oblivConversionError loc =
  E.s (E.error "%s:%i:cannot convert obliv type to non-obliv" loc.file loc.line)

let isImplicitCastResult t = hasAttribute "implicitCast" (typeAttrs t)

(* Remember that obliv overrides frozen *)
let isFrozenQualified t = let a = typeAttrs t in
                          hasAttribute frozen a && not (hasOblivAttr a)

let isFrozenPtr e = match unrollType e with
| TPtr(t,_) -> isFrozenQualified t
| _ -> false

let isNonFrozenPtr e = match unrollType e with
| TPtr(t,_) -> not (isFrozenQualified t)
| _ -> false

let invalidFrozenPtrConvert t1 t2 = isFrozenPtr t1 && isNonFrozenPtr t2

(* Implicit casts were inserted automatically before we inferred the proper
 * frozen qualifiers in a scope-sensitive way. So we might need to 
 * bypass those *)
let rec invalidFrozenPtrAsgn e1 t2 = match e1 with
| CastE(t1,e1') when isImplicitCastResult t1 -> invalidFrozenPtrAsgn e1' t2
| _ -> invalidFrozenPtrConvert (typeOf e1) t2

(* Used for ChangeDoChildrenPost *)
type 't visitorResponse = 't -> ('t * ('t -> 't)) ;;

let chainVisitorResponse 
  (a : 't visitorResponse) (b : 't visitorResponse) : 't visitorResponse =
    begin fun x -> 
      let (x1,post1) = a x in
      let (x2,post2) = b x1 in
      (x2, compose post1 post2)
    end

let wrapPostProcessor
  (x1:'t) (post1 : ('t->'t)) (vb : 't->'t visitAction) : 't visitAction =
    match vb x1 with
    | SkipChildren -> post1 x1; SkipChildren
    | DoChildren   -> ChangeDoChildrenPost(x1,post1)
    | ChangeTo x2  -> ChangeTo(post1 x2)
    | ChangeDoChildrenPost(x2,post2) -> 
        ChangeDoChildrenPost(x2, compose post1 post2)

(* Allows any cilVisitor to track the current 'obliv-depth'. To use this, simply
 * wrap vblock and vfunc with the corresponding methods here. then #curDepth
 * will give you the current depth, while varDepth will give you the current
 * effective depth of the variable's declaration. See other visitors below for
 * examples *)
class depthTracker = object(self)
  val currentOblivDepth = ref 0
  val currentRootDepth  = ref 0
  (* Should always satisfy varDepth v <= curDepth() for all v in scope *)
  method varDepth vinfo =
    (* This 'if' clause shouldn't be necessary, but some temporaries
     * (possibly created from cabs2cil) cause Not_found without it *)
    if !currentOblivDepth = !currentRootDepth then self#curDepth()
    else try
      let decldepth = Hashtbl.find vidOblivDepth vinfo.vid in
      max 0 (decldepth - !currentRootDepth)
      with Not_found -> !currentOblivDepth (* assume temp XXX *)
  (* Should never be negative *)
  method curDepth () = !currentOblivDepth - !currentRootDepth
  method wrapVBlock childVisitor b = 
    if isOblivBlock b then (
      incr currentOblivDepth;
      let undo x = decr currentOblivDepth; x in
      wrapPostProcessor b undo childVisitor
    )else if isRipOblivBlock b then
      let c = !currentRootDepth in
      let undo x = currentRootDepth := c; x in
      currentRootDepth := !currentOblivDepth;
      wrapPostProcessor b undo childVisitor
    else childVisitor b
  method defaultVBlock b = self#wrapVBlock (fun _ -> DoChildren) b
  method wrapVFunc childVisitor fundec = 
    let vi = fundec.svar in
    let o = isFunctionType vi.vtype && hasOblivAttr (typeAttrs vi.vtype) in
    if o then (
      incr currentOblivDepth;
      let undo x = decr currentOblivDepth; x in
      wrapPostProcessor fundec undo childVisitor
    )else childVisitor fundec
  method defaultVFunc fundec = self#wrapVFunc (fun _ -> DoChildren) fundec
end

let rec dropFrozenAttr (t:typ) = match unrollType t with
| TPtr(t,a) -> TPtr(dropFrozenAttr t,dropAttribute frozen a)
| TArray(t,exp,a) -> TArray(dropFrozenAttr t,exp,dropAttribute frozen a)
| t -> typeRemoveAttributes [frozen] t

(* Generally the purpose is typechecking, but also initializes globals in
 * OblivUtils: oblivBitType, oblivBitPtr, oblivConstBitPtr *)
class typeCheckVisitor = object(self)
  inherit nopCilVisitor
  val dt = new depthTracker

  val funcOblivness = Hashtbl.create 100

  method isFuncObliv vinfo = 
    if vinfo.vglob && isFunctionType vinfo.vtype 
      then try Hashtbl.find funcOblivness vinfo.vname
           with Not_found -> false
      else raise (Invalid_argument 
        "typeCheckVisitor#isFuncObliv expects a global function")

  (* Adds a 'frozen' qualification to type if necessary *)
  method effectiveVarType vinfo = 
    (* Looks like a hack. Think of types a little better *)
    if dt#curDepth() = 0 then dropFrozenAttr vinfo.vtype
    else if dt#curDepth() = dt#varDepth vinfo then vinfo.vtype
    else typeAddAttributes [Attr(frozen,[])] vinfo.vtype

  (* Counting up on obliv-if and ~obliv blocks  *)
  method vblock = dt#defaultVBlock

  method vtype vtype = match checkOblivType vtype with
    | None -> DoChildren
    | Some cat -> if cat = "unimplemented" 
                    then E.s (E.unimp "obliv double/long double/enum")
                    else E.s (E.error "%s:%i: %s cannot be obliv-type" 
                              !currentLoc.file !currentLoc.line cat)

  method vinst instr = ChangeDoChildrenPost ([instr], List.map (
    fun instr -> match instr with
      | Set (lv,exp,loc) -> 
          if dt#curDepth()>0 && isFrozenQualified (typeOfLval lv) then
            E.s (E.error "%s:%i:cannot assign to frozen qualified lvalue"
                         loc.file loc.line)
          else if dt#curDepth()>0 && invalidFrozenPtrAsgn exp (typeOfLval lv) then
            frozenConversionError loc
          else if invalidOblivConvert (typeOf exp) (typeOfLval lv) then
            oblivConversionError loc
          else instr
      | Call (lvopt,f,args,loc) ->
          begin match typeOf f with
          | TFun (tr,targsPack,isVarg,a) -> begin
              let targs = argsToList targsPack in
              let rec matchArgs a b = match a,b with
              | _,[] -> ()
              | [],_ -> E.s (E.error "%s:%i:too few arguments to function"
                              loc.file loc.line)
              | a::al, (_,b,_)::bl -> 
                  if invalidOblivConvert (typeOf a) b then 
                    oblivConversionError loc
                  else if dt#curDepth()>0 && invalidFrozenPtrAsgn a b then
                    frozenConversionError loc
                  else matchArgs al bl
              in
              matchArgs args targs;
              match lvopt with 
              | Some lv -> 
                  if invalidOblivConvert tr (typeOfLval lv) then
                    oblivConversionError loc
                  else instr
              | None -> instr
            end
          | _ -> E.s (E.error "%s:%i:trying to call non-function" loc.file
                        loc.line)
          end
      | _ -> instr
      ))

  method vstmt s = ChangeDoChildrenPost (s, fun s -> match s.skind with
    | If(e,tb,fb,l) -> 
        begin
          let e' = match unrollType (typeOf e) with
            | TInt(IBool,_) -> e
            | t ->
                let boolType = TInt(IBool,[]) in
                let mybool = if hasOblivAttr(typeAttrs t)
                      then addOblivType boolType
                      else boolType
                in CastE(mybool,e)
          in
          let s' = { s with skind = If(e',tb,fb,l) } in
          if isOblivSimple (typeOf e') then
            if isOblivBlock tb then s'
            else E.s (E.error
              "%s:%i:Cannot use obliv-type expression as a condition" l.file
              l.line)
          else s'
        end
    | _ -> s
  )

  (* obliv-related typechecking *)
  method vexprObliv e = (e, fun exp -> match exp with
  | UnOp (op,e,t) -> 
      let t2 = match op with LNot -> boolType | _ -> t in
      if isOblivSimple (typeOf e) then
        let tr = addOblivType t2 in UnOp(op,e,tr)
      else exp
  | BinOp(op,e1,e2,t) ->
      let t2 = match op with
      | Lt | Gt | Le | Ge | Eq | Ne | LAnd | LOr -> boolType
      | _ -> t
      in
      if isOblivSimple (typeOf e1) || isOblivSimple (typeOf e2) then
        match op with 
        | PlusPI | MinusPI | IndexPI -> 
            E.s (E.error "%a: obliv types cannot be used in pointer arithmetic"
                  d_loc !currentLoc)
        | Shiftlt | Shiftrt ->
            if isOblivSimple (typeOf e2) then
              E.s (E.error "%a obliv types cannot be used as shift amounts"
                    d_loc !currentLoc)
            else BinOp(op,e1,e2,addOblivType t2)
        | _ ->
            let e1 = mkCast e1 (addOblivType (typeOf e1)) in
            let e2 = mkCast e2 (addOblivType (typeOf e2)) in
            let tr = addOblivType t2 in
            BinOp(op,e1,e2,tr)
      else exp
  | CastE (t,e) when typeSig t = typeSig (typeOf e) -> e
  | CastE (t,e) when isImplicitCastResult t ->
      let st = typeOf e in
      if isOblivSimple st && not (isOblivSimple t) then
        let isint = function TInt _ -> true | _ -> false in
        let isfloat = function TFloat _ -> true | _ -> false in
        if (isint st && isint t) || (isfloat st && isfloat t) 
          then CastE(addOblivType t,e)
        else
        begin
        ignore (Pretty.printf "Expr: %a\n" d_exp e);
        E.s (E.error "%a: Cannot convert obliv type '%a' to non-obliv '%a'"
          d_loc !currentLoc d_type st 
            d_type (typeRemoveAttributes ["implicitCast"] t))
        end
      else CastE(t,e)
  | _ -> exp
  )

  (* frozen-related typechecking *)
  method vexprFrozen e = (e, fun exp -> match exp with
  (* Lval types do not have to change frozen since we hooked in from
   * featureDescr. This may be a problem since we can never accurately
   * obtain frozen qualifications of Lval() expressions outside of
   * typeCheckVisitor, since we need to know what "level" the expression
   * appears in. Hopefully, we won't have to know frozen after typechecking
   * is done. *)
  | BinOp(op,e1,e2,t) ->
      let t = match op with (* Propagate frozen qualifiers *)
              | PlusPI | IndexPI | MinusPI -> typeOf e1
              | _ -> t
      in BinOp(op,e1,e2,t)
  | CastE (t,e2) when isImplicitCastResult t && isFrozenPtr (typeOf e2) ->
      begin match t with 
      | TPtr(t2,a) -> 
          let t' = TPtr(typeAddAttributes [Attr(frozen,[])] t2,a) in
          CastE (t',e2)
      | _ -> e
      end
  | _ -> exp
  )
  method vexpr e 
    = let (e',post) = chainVisitorResponse self#vexprFrozen self#vexprObliv e in
      ChangeDoChildrenPost (e',post)

  (* incr/decr currentOblivDepth on obliv functions *)
  method vfunc = dt#defaultVFunc

  method vglob v = begin match v with 
  | GCompTag(ci,loc) when ci.cname = "OblivBit" ->  
      updateOblivBitType ci; DoChildren
  | GVarDecl (vi,loc) | GFun ({svar=vi},loc) when isFunctionType vi.vtype ->
      let no = hasOblivAttr (typeAttrs vi.vtype) in
      begin try let oo = Hashtbl.find funcOblivness vi.vname in
            if oo <> no 
              then E.s (E.error "Function %s was previously declared with \
                          different obliviousness" vi.vname)
            with Not_found -> Hashtbl.add funcOblivness vi.vname no
      end; 
      DoChildren
  | _ -> DoChildren
  end
end

(* I should be disallowing ~obliv(en) inside non-obliv blocks TODO *)
class controlCheckVisitor = object(self)
  inherit nopCilVisitor
  val breakOk = ref false
  val contOk = ref false
  val returnOk = ref true

  method getRestorer() = 
    let oldbreak,oldcont,oldret = !breakOk,!contOk,!returnOk in
    (fun() -> breakOk:=oldbreak; contOk:=oldcont; returnOk:=oldret)

  method vstmt s = let restore = self#getRestorer() in begin
    match s.skind with
    | If(c,tb,fb,loc) -> if isOblivBlock tb then
          (breakOk:=false; contOk:=false; returnOk:=false)
    | Loop _ -> breakOk:=true; contOk:=true
    | Switch _ -> breakOk:=true
    | TryFinally _ | TryExcept _ -> 
        E.s (E.error "%a: Obliv-C does not support exceptions"
          d_loc !currentLoc)
    | Break loc -> if not !breakOk then
                      E.s (E.error "%a: unexpected break" d_loc loc)
    | Continue loc -> if not !contOk then
                        E.s (E.error "%a: unexpected continue" d_loc loc)
    | Return (_,loc) -> if not !returnOk then
                          E.s (E.error "%a: unexpected return" d_loc loc)
    | _ -> (); (* Yeah, I'm not handling Goto *)
    ;
    ChangeDoChildrenPost(s,fun s -> restore(); s)
  end

  method vglob v = begin match v with
  | GFun ({svar=vi},loc) when isFunctionType vi.vtype ->
      (breakOk:=true; contOk:=true; returnOk:=true; DoChildren)
  | _ -> DoChildren
  end

  val dt = new depthTracker
  method vfunc = dt#defaultVFunc
  method vblock = dt#defaultVBlock

  (* I need to check this for (p+5)(x,y) TODO *)
  method vinst instr = ChangeDoChildrenPost ([instr], List.map (
    fun instr -> 
      if dt#curDepth () = 0 then instr else begin match instr with
      | Call (lvopt,f,args,loc) -> 
          begin match unrollType (typeOf f) with
          | TFun (tr,targsPack,isVarg,a) ->
            if hasOblivAttr a then instr
            else 
              E.s (E.error "%a: cannot invoke non-obliv function in obliv \
              scope\n" d_loc !currentLoc);
          | _ -> instr
          end
      | _ -> instr
      end
  ))
end

let voidFunc name argTypes =
  let ftype = TFun(TVoid [], Some argTypes,false,[]) in
  Lval(Var(makeGlobalVar name ftype),NoOffset)

let cmpLtFuncs = ("__obliv_c__setLessThanUnsigned"
                 ,"__obliv_c__setLessThanSigned")
let cmpLeFuncs = ("__obliv_c__setLessOrEqualUnsigned"
                 ,"__obliv_c__setLessOrEqualSigned")


let setComparison fname dest s1 s2 loc = 
  let optype = typeOfLval s1 in
  let coptype = typeAddAttributes [constAttr] optype in
  let fargTypes = ["dest",TPtr(oblivBoolType,[]),[]
                  ;"s1",TPtr(coptype,[]),[];"s2",TPtr(coptype,[]),[]
                  ;"bitcount",!typeOfSizeOf,[]
                  ] in
  let func = voidFunc fname fargTypes in
  Call(None,func,[AddrOf dest; AddrOf s1; AddrOf s2
                 ;xoBitsSizeOf optype],loc)

let setShift fname (dest:lval) (src:lval) (amt:lval) loc =
  let optype = typeOfLval src in
  let coptype = typeAddAttributes [constAttr] optype in
  let fargTypes = ["dest",TPtr(optype,[]),[]
                  ;"src",TPtr(coptype,[]),[]
                  ;"bitcount",!typeOfSizeOf,[]
                  ;"shiftAmount",TInt(IUInt,[]),[]
  ] in
  let func = voidFunc fname fargTypes in
  Call(None,func,[AddrOf dest; AddrOf src; xoBitsSizeOf optype; Lval amt],loc)

(* Same as setComparison, but picks the right function name
 * based on signedness of operators *)
let setComparisonUS fnames dest s1 s2 loc = 
  let optype = typeOfLval s1 in
  let fname = match unrollType optype with
  | TInt(k,_) -> if isSigned k then snd fnames else fst fnames
  | _ -> E.s (E.error "Cannot operate on obliv values of type %a" d_type optype)
  in
  setComparison fname dest s1 s2 loc

let setLogicalOp fname dest s1 s2 loc = 
  let fargTypes = ["dest",!oblivBitPtr,[]
                  ;"s1",!cOblivBitPtr,[]; "s2",!cOblivBitPtr,[]
                  ] in
  let func = voidFunc fname fargTypes in
  Call(None,func,[CastE(!oblivBitPtr,AddrOf dest)
                 ;CastE(!cOblivBitPtr,AddrOf s1)
                 ;CastE(!cOblivBitPtr,AddrOf s2)],loc)

let setBitwiseOp fname dest s1 s2 loc =
  setComparison fname dest s1 s2 loc
  (* The only difference is return value type, which is not a problem 
   * as long as we are casting everything to and from void* *)

(* Same comments as in setBitwiseOp *)
let setArith fname dest s1 s2 loc = setBitwiseOp fname dest s1 s2 loc

let setUnop fname dest s loc = 
  let fargTypes = ["dest",!oblivBitPtr,[]
                  ;"s",!cOblivBitPtr,[]
                  ;"bitcount",!typeOfSizeOf,[]
  ] in
  let func = voidFunc fname fargTypes in
  Call(None,func,[CastE(!oblivBitPtr,AddrOf dest);CastE(!cOblivBitPtr,AddrOf s)
                 ;xoBitsSizeOf (typeOfLval s)],loc)

let setIntExtend fname dv dk sv sk loc = 
  let fargTypes = ["dest",TPtr(TVoid [],[]),[]
                  ;"dsize",!typeOfSizeOf,[]
                  ;"src",TPtr(TVoid [constAttr], []),[]
                  ;"ssize",!typeOfSizeOf,[]
  ] in
  let func = voidFunc fname fargTypes in
  Call(None,func,[ mkAddrOf dv; xoBitsSizeOf (TInt(dk,[]))
                 ; mkAddrOf sv; xoBitsSizeOf (TInt(sk,[]))
                 ],loc)

let setKnownInt v k x loc = 
  let fargTypes = ["dest",TPtr(typeOfLval v,[]),[]
                  ;"bitcount",!typeOfSizeOf,[]
                  ;"value",widestType,[]
                  ] in
  let func = voidFunc "__obliv_c__setSignedKnown" fargTypes in
  Call(None,func,[ AddrOf v; xoBitsSizeOf (TInt(k,[]))
                 ; CastE(widestType,CastE(TInt(k,[]),x))
                 ],loc)

let condSetKnownInt c v k x loc = 
  let fargTypes = ["cond",TPtr(oblivBoolType,[]),[]
                  ;"dest",TPtr(typeOfLval v,[]),[]
                  ;"size",!typeOfSizeOf,[]
                  ;"val",widestType,[]
                  ] in
  let func = voidFunc "__obliv_c__condAssignKnown" fargTypes in
  Call(None,func,[ mkAddrOf c; mkAddrOf v; xoBitsSizeOf (TInt(k,[]))
                 ; CastE(widestType,CastE(TInt(k,[]),x))
                 ],loc)

let setKnownFloat v k x loc =
  let fargTypes = ["dest",TPtr(typeOfLval v,[]),[]
                ;"bitcount",!typeOfSizeOf,[]
                ;"value",widestType,[]
                ] in
  let func = voidFunc "__obliv_c__setFloatKnown" fargTypes in
  Call(None,func,[ AddrOf v; xoBitsSizeOf (TFloat(k,[]))
               ; CastE(widestType,CastE(TFloat(k,[]),x))
               ],loc)

let condSetKnownFloat c v k x loc =
  let fargTypes = ["cond",TPtr(oblivBoolType,[]),[]
                  ;"dest",TPtr(typeOfLval v,[]),[]
                  ;"size",!typeOfSizeOf,[]
                  ;"val",widestType,[]
                  ] in
  let func = voidFunc "__obliv_c__condAssignKnownF" fargTypes in
  Call(None,func,[ mkAddrOf c; mkAddrOf v; xoBitsSizeOf (TFloat(k,[]))
                 ; CastE(widestType,CastE(TFloat(k,[]),x))
                 ],loc)

let setIfThenElse dest c ts fs loc = 
  let fargTypes = ["dest",TPtr(TVoid [],[]),[]
                  ;"tsrc",TPtr(TVoid [constAttr],[]),[]
                  ;"fsrc",TPtr(TVoid [constAttr],[]),[]
                  ;"size",!typeOfSizeOf,[]
                  ;"cond",TPtr(TVoid [constAttr],[]),[]
  ] in
  let func = voidFunc "__obliv_c__ifThenElse" fargTypes in
  let args = [ mkAddrOf dest; mkAddrOf ts; mkAddrOf fs
             ; xoBitsSizeOf (typeOfLval dest)
             ; mkAddrOf c ] in
  Call(None,func,args,loc)

let condAssign c dest src loc = 
  let fargTypes = ["cond",TPtr(TVoid [constAttr],[]),[]
                  ;"dest",TPtr(TVoid [],[]),[]
                  ;"src" ,TPtr(TVoid [constAttr],[]),[]
                  ;"size",!typeOfSizeOf,[]
  ] in
  let func = voidFunc "__obliv_c__condAssign" fargTypes in
  let args = [ mkAddrOf c; mkAddrOf dest; mkAddrOf src
             ; xoBitsSizeOf (typeOfLval dest) ] in
  Call(None,func,args,loc)

let zeroSet (dest:varinfo) loc = 
  let fargTypes = ["s",TPtr(TVoid [],[]),[]
                  ;"c",TInt(IInt,[]),[]
                  ;"n",!typeOfSizeOf,[]
  ] in
  let voidptr = TPtr(TVoid [],[]) in
  let ftype = TFun(voidptr, Some fargTypes,false,[]) in
  let func = var(makeGlobalVar "memset" ftype) in
  let args = [ mkCast (mkAddrOf (var dest)) voidptr
             ; kinteger IInt 0; SizeOf(dest.vtype) ] in
  Call(None,Lval(func),args,loc)


let trueCond = var (makeGlobalVar "__obliv_c__trueCond" oblivBoolType)

(* Codegen, when conditions don't matter *)
let rec codegenUncondInstr (instr:instr) : instr = match instr with
| Set(v,UnOp(op,Lval e,t),loc) -> 
    begin match unrollType t with
    | TInt(kind,a) when hasOblivAttr a ->
        begin match unrollType (typeOf (Lval e)) with
        | TFloat(kind,a) when hasOblivAttr a ->
          begin match op with
          | LNot -> E.s (E.error
                    "Unimplemented. Please compare directly with 0.")
          | _ -> E.s (E.error "Unexpected operator %a" d_unop op)
          end
        | _ ->
          begin match op with
          | Neg  -> setUnop "__obliv_c__setNeg" v e loc
          | BNot -> setUnop "__obliv_c__setBitwiseNot" v e loc
          | LNot -> setUnop "__obliv_c__setLogicalNot" v e loc
          end
        end
    | TFloat(kind, a) when hasOblivAttr a ->
        begin match op with
        | Neg -> setUnop "__obliv_c__setNegF" v e loc
        | _ -> E.s (E.error "Unexpected error. %s"
                            ("!(float exp) or ~(float exp) should not " ^
                            "produce float expression"))
        end
    | _ -> instr
    end
| Set(v,BinOp(op,Lval e1,Lval e2,t),loc) ->
    begin match unrollType t with
    | TInt(IBool,a) when hasOblivAttr a &&
                         isOblivFloat (typeOf (Lval e1)) &&
                         isOblivFloat (typeOf (Lval e2)) ->
        begin match op with
        | Ne -> setComparison "__obliv_c__setNotEqualF" v e1 e2 loc
        | Eq -> setComparison "__obliv_c__setEqualToF"  v e1 e2 loc
        | Lt -> setComparison "__obliv_c__setLessThanF" v e1 e2 loc
        | Gt -> setComparison "__obliv_c__setLessThanF" v e2 e1 loc
        | Le -> setComparison "__obliv_c__setLessThanEqF" v e1 e2 loc
        | Ge -> setComparison "__obliv_c__setLessThanEqF" v e2 e1 loc
        | _ -> E.s (E.error "Unexpected operator %a between obliv floats"
                            d_binop op)
        end
    | TInt(kind,a) when hasOblivAttr a ->
        begin match op with
        | PlusA  -> setArith "__obliv_c__setPlainAdd" v e1 e2 loc
        | MinusA -> setArith "__obliv_c__setPlainSub" v e1 e2 loc
        | Mult   -> setArith "__obliv_c__setMul" v e1 e2 loc
        | Shiftlt-> setShift "__obliv_c__setLShift" v e1 e2 loc
        | Ne -> setComparison "__obliv_c__setNotEqual" v e1 e2 loc
        | Eq -> setComparison "__obliv_c__setEqualTo"  v e1 e2 loc
        | Lt -> setComparisonUS cmpLtFuncs v e1 e2 loc
        | Gt -> setComparisonUS cmpLtFuncs v e2 e1 loc
        | Le -> setComparisonUS cmpLeFuncs v e1 e2 loc
        | Ge -> setComparisonUS cmpLeFuncs v e2 e1 loc
        | BAnd -> setBitwiseOp "__obliv_c__setBitwiseAnd" v e1 e2 loc
        | BXor -> setBitwiseOp "__obliv_c__setBitwiseXor" v e1 e2 loc
        | BOr  -> setBitwiseOp "__obliv_c__setBitwiseOr" v e1 e2 loc
        | LAnd -> setLogicalOp "__obliv_c__setBitAnd" v e1 e2 loc
        | LOr  -> setLogicalOp "__obliv_c__setBitOr"  v e1 e2 loc
        | _ when isSigned kind ->
            begin match op with
            | Shiftrt -> setShift "__obliv_c__setRShiftSigned" v e1 e2 loc
            | Div -> setArith "__obliv_c__setDivSigned" v e1 e2 loc
            | Mod -> setArith "__obliv_c__setModSigned" v e1 e2 loc
            | _ -> instr
            end
        | _ -> 
            begin match op with
            | Shiftrt -> setShift "__obliv_c__setRShiftUnsigned" v e1 e2 loc
            | Div -> setArith "__obliv_c__setDivUnsigned" v e1 e2 loc
            | Mod -> setArith "__obliv_c__setModUnsigned" v e1 e2 loc
            | _ -> instr
            end
        end
    | TFloat(kind, a) when hasOblivAttr a ->
        let opError s =
          E.s (E.error "invalid 'obliv float' operand to binary %s" s) in
        begin match op with
        | PlusA  -> setArith "__obliv_c__setPlainAddF" v e1 e2 loc
        | MinusA -> setArith "__obliv_c__setPlainSubF" v e1 e2 loc
        | Mult   -> setArith "__obliv_c__setMulF" v e1 e2 loc
        | Div    -> setArith "__obliv_c__setDivF" v e1 e2 loc
        | _ -> instr
        end
    | _ -> instr
    end
(* Promotion from non-obliv data to obliv *)
| Set(v,CastE(dt,x),loc) when isOblivSimple dt
                           && not (isOblivSimple (typeOf x)) ->
    begin match unrollType dt with 
    | TInt(k,_) -> setKnownInt v k x loc
    | TFloat(k, _) -> setKnownFloat v k x loc
    | _ -> instr
    end
| Set(_,CastE(dt, x), loc) when isOblivSimple dt
                             && isOblivSimple (typeOf x)
                             && isOblivInt dt <> isOblivInt (typeOf x) ->
    E.s (E.error "Unimplemented: conversion between obliv int and obliv float")
| Set(dv,CastE(dt,Lval sv),loc) when isOblivInt dt ->
    begin match unrollType dt,unrollType (typeOfLval sv) with
    | TInt(dk,da), TInt(sk,sa) when hasOblivAttr sa ->
        if isSigned sk then
          setIntExtend "__obliv_c__setSignExtend" dv dk sv sk loc
        else setIntExtend "__obliv_c__setZeroExtend" dv dk sv sk loc
    | _ -> instr
    end
| Set(dv,CastE(dt,Lval sv),loc) when isOblivFloat dt -> instr
| Set(v,CastE(t,x),loc) when isOblivSimple t ->
    codegenUncondInstr (Set(v,CastE(unrollType t,x),loc))
| Call(lvo,exp,args,loc) when isOblivFunc (typeOf exp) ->
    Call(lvo,exp,mkAddrOf trueCond::args,loc)
| _ -> instr

let isTaggedTemp lv = hasAttribute SimplifyTagged.simplifyTempTok
                        (typeAttrs (typeOfLval lv))

(* Ok, I really should stop adding more parameters here 
 * curCond : generated instructions should only have an effect if this is true
 * tmpVar: creates a new temporary var in current function
 * isDeepVar: checks if a given varinfo is declared at current block scope (vs.
 *              some outer scope)
 * instr: the instruction to be compiled *)
let rec codegenInstr curCond tmpVar isDeepVar (instr:instr) : instr list = 
  let setUsingTmp v x loc = 
    let nv = var (tmpVar (typeOfLval v)) in
    let ilist = [Set(nv,x,loc); Set(v,Lval nv,loc)] in
    mapcat (codegenInstr curCond tmpVar isDeepVar) ilist
  in
  if curCond == trueCond then [codegenUncondInstr instr]
  else match instr with 
  | Set(v,_,_) when isTaggedTemp v -> [codegenUncondInstr instr]
  | Set((Var(v),NoOffset),_,_) when isDeepVar v -> 
      [codegenUncondInstr instr]
  | Set(v,Lval(v2),loc) when isOblivSimple (typeOfLval v) -> 
      if isOblivSimple (typeOfLval v2) then
        [condAssign curCond v v2 loc]
      else setUsingTmp v (Lval v2) loc
  | Set(v,(BinOp(_,_,_,t) as x),loc) when isOblivSimple t -> setUsingTmp v x loc
  | Set(v,(CastE(t,x) as xf),loc)
      when (isOblivInt t || isOblivFloat t) && isOblivSimple (typeOfLval v) ->
      if isOblivSimple (typeOf x) then setUsingTmp v xf loc
      else begin match unrollType t with
           | TInt(k,_) -> [condSetKnownInt curCond v k x loc]
           | TFloat(k, a) -> [condSetKnownFloat curCond v k x loc]
           | _ -> [instr]
           end
  | Call(lvo,exp,args,loc) when isOblivFunc (typeOf exp) ->
      let callinstr lvo' = Call(lvo',exp,mkAddrOf curCond::args,loc) in
      begin match lvo with
      | None -> [callinstr None]
      | Some lv -> let nv = var (tmpVar (typeOfLval lv)) in
                   callinstr (Some nv) ::
                     codegenInstr curCond tmpVar isDeepVar (Set(lv,Lval nv,loc))
      end
  | _ -> [instr]


(* Traverses the whole function to check this *)
let hasOblivBlocks f = begin
  let vis = object
    inherit nopCilVisitor
    val foundObliv = ref false
    method vblock b = begin
      if isOblivBlock b then foundObliv:=true;
      if !foundObliv then SkipChildren else DoChildren
    end
    method found = !foundObliv
  end in
  ignore (visitCilFunction (vis :> cilVisitor) f);
  vis#found
end

let zeroOutEnable = ref false

class codegenVisitor (curFunc : fundec) (dt:depthTracker) (curCond : lval) 
  : cilVisitor = object(self)
  inherit nopCilVisitor
  method vstmt s = 
    let tmpVar ?name t = SimplifyTagged.makeSimplifyTemp ?name curFunc t in
    let isDeepVar v = dt#curDepth() = dt#varDepth v in
    match s.skind with
    | Instr ilist -> 
        let nestedGen = codegenInstr curCond tmpVar isDeepVar in
        ChangeTo (mkStmt (Instr (mapcat nestedGen ilist)))
    | If(c,tb,fb,loc) when isOblivBlock tb ->
        let cond = "__obliv_c__cond" in
        let cv = var (tmpVar ~name:cond oblivBoolType) in
        let ct = var (tmpVar ~name:cond oblivBoolType) in
        let cf = var (tmpVar ~name:cond oblivBoolType) in
        let visitSubBlock cond blk = 
          visitCilBlock (new codegenVisitor curFunc dt cond) blk in
        let cs = mkStmt (Instr (List.map codegenUncondInstr
          [ Set (cv,c,loc)
          ; Set (ct,BinOp(LAnd,Lval cv
                              ,Lval curCond,oblivBoolType),loc)
          ; Set (cf,BinOp(Ne  ,Lval ct
                              ,Lval curCond,oblivBoolType),loc)
          ])) in
        let ts = mkStmt (Block (visitSubBlock ct tb)) in
        let fs = mkStmt (Block (visitSubBlock cf fb)) in
        ChangeTo (mkStmt (Block {battrs=[]; bstmts=[cs;ts;fs]}))
    (* Filter out the trivial cases *)
    | Return(None,_) | Return(Some(Lval _),_) -> DoChildren
    (* and then the unsimplified return *)
    | Return(Some x,l) when isOblivSimple (typeOf x) -> 
        let rv = var (tmpVar (typeOf x)) in
        let set = mkStmt (Instr [codegenUncondInstr (Set (rv,x,l))]) in
        let ret = mkStmt (Return (Some (Lval rv),l)) in
        ChangeTo (mkStmt (Block {battrs=[]; bstmts=[set;ret]}))
    | _ -> DoChildren

  method vblock = dt#wrapVBlock begin fun b ->
    if isOblivBlock b then 
      ChangeDoChildrenPost ( { b with battrs = dropAttribute "obliv" b.battrs }
                           , fun x -> x)
    else match isRipObliv curFunc b with
    | NotRipObliv -> DoChildren
    | RipOblivWithVar vi ->
        let asg = mkStmt (Instr [Set (var vi,Lval curCond,!currentLoc)]) in
        let b' = dropRipOblivAttr b in
        let b'' = {b' with bstmts = asg :: b'.bstmts} in
        ChangeTo (visitCilBlock (new codegenVisitor curFunc dt trueCond) b'')
    | RipOblivNoVar ->
        let b' = dropRipOblivAttr b in
        ChangeTo (visitCilBlock (new codegenVisitor curFunc dt trueCond) b')
  end
  method vfunc = dt#wrapVFunc begin fun f ->
    if !zeroOutEnable then begin
      (* if isOblivFunc f.svar.vtype || hasOblivBlocks f then begin *)
        (* This really needs to be done only for obliv types
         * But I am too lazy to traverse through structs *)
      let nonbasic t = match unrollType t with
        | TPtr _ -> false
        | TFloat _ when not (isOblivSimple t) -> false
        | TFun _ -> false
        | TInt _ when not (isOblivSimple t) -> false
        | _ -> true
      in
      let nontemp = List.filter (fun v -> 
              nonbasic v.vtype
           && not (isTaggedTemp (var v)))
                                f.slocals in
      self#queueInstr (List.map (fun v -> zeroSet v !currentLoc) nontemp);
      (* end; *)
    end;
    DoChildren
  end
end

let genFunc g = match g with
| GFun(f,loc) ->
    (* Using ~insert:false here. This makes it easier to uniformly treat
     * direct and pointer function calls. They are later added as formals in
     * typeFixVisitor *)
    let makeFormal f' 
      = mkMem (Lval(var(makeLocalVar f' ~insert:false "__obliv_c__en"
                          constOblivBoolPtrType))) NoOffset in
    let isofun = isOblivFunc f.svar.vtype in
    let c = if isofun then makeFormal f else trueCond in
    let cv = new codegenVisitor f (new depthTracker) c in
    GFun(visitCilFunction cv f,loc)
| GVar(vi,_,_) ->
    if vi.vname = "__obliv_c__enable_zero_out" then
      zeroOutEnable := true; 
    g
| _ -> g

(* If this gets too slow, merge it with codegen *)
class rmSimplifyVisitor = object
  inherit nopCilVisitor
  method vattr at = match at with 
  | Attr(s,[]) when s = SimplifyTagged.simplifyTempTok -> ChangeTo []
  | _ -> DoChildren
end

class typeFixVisitor wasObliv : cilVisitor = object(self)
  inherit nopCilVisitor
  method vtype t = 
    let t' = typeRemoveAttributes ["implicitCast";frozen] t in
    ChangeDoChildrenPost (t', fun t -> match unrollType t with
    | TInt(k,a) when hasOblivAttr a -> 
        let a2 = dropOblivAttr a in
        setTypeAttrs (intTargetType k) a2
    | TFloat(k, a) when hasOblivAttr a ->
        let a2 = dropOblivAttr a in
        setTypeAttrs (floatTargetType k) a2
    | TFun(tres,argso,vargs,a) when isOblivFunc t -> 
        let entarget = visitCilType (self :> cilVisitor) constOblivBoolPtrType 
        in
        let args = ("__obliv_c__en",entarget,[]) :: argsToList argso in
        let a' = dropOblivAttr a in
        TFun(tres,Some args,vargs,a')
    | _ -> t
    )

  method vglob g = match g with
  | GFun(f,loc) when wasObliv f.svar ->
      ignore(makeFormalVar f ~where:"^" "__obliv_c__en" constOblivBoolPtrType);
      DoChildren
  | _ -> DoChildren
end

let feature =
  { fd_name = "processObliv";
    fd_enabled = false;
    fd_description = "handles obliv keyword";
    fd_extraopt = [];
    fd_doit = 
    (function (f: file) -> 
      let tcVisitor = new typeCheckVisitor in
      let oldtov = !typeOfVinfo in
      typeOfVinfo := tcVisitor#effectiveVarType;
      visitCilFileSameGlobals (tcVisitor :> cilVisitor) f;
      typeOfVinfo := oldtov;
      let isObliv = tcVisitor#isFuncObliv in
  
      visitCilFileSameGlobals (new controlCheckVisitor :> cilVisitor) f;

      SimplifyTagged.feature.fd_doit f; (* Note: this can screw up type equality
                                                 checks *)
      (* types are often visited before the functions, so it is often easier
       * to transform functions and types in separate phases *)
      mapGlobals f genFunc;
      (* might merge these two *)
      visitCilFileSameGlobals (new rmSimplifyVisitor) f;
      visitCilFileSameGlobals (new typeFixVisitor isObliv) f
    );
    fd_post_check = true;
  } 

let () = Feature.register feature
