
open OblivUtils
open Pretty
open Printf
open Cil
module E = Errormsg
module H = Hashtbl

let rec checkOblivType t = match t with
| TVoid a -> if hasOblivAttr a then Some "void" else None
| TInt _ -> None
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

let dummyExp = Const (CChr 'x')
let condAssignKnown = ref dummyExp
let copySimpleObliv = ref dummyExp

let updateOblivBitType ci = begin
  oblivBitType := TComp(ci,[]);
  oblivBitPtr := TPtr(!oblivBitType,[]);
  oblivConstBitPtr := typeAddAttributes [constAttr] !oblivBitPtr;
  let types = 
    [oblivBoolTarget,"__obliv_c__bool",1
    ;oblivCharTarget,"__obliv_c__char",bitsSizeOf charType
    ;oblivIntTarget,"__obliv_c__int",bitsSizeOf intType
    ;oblivShortTarget,"__obliv_c__short",bitsSizeOf (TInt(IShort,[]))
    ;oblivLongTarget,"__obliv_c__long",bitsSizeOf (TInt(ILong,[]))
    ;oblivLLongTarget,"__obliv_c__lLong",bitsSizeOf (TInt(ILongLong,[]))
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
                          hasAttribute "frozen" a && not (hasOblivAttr a)

let isFrozenPtr e = match e with
| TPtr(t,_) -> isFrozenQualified t
| _ -> false

let isNonFrozenPtr e = match e with
| TPtr(t,_) -> not (isFrozenQualified t)
| _ -> false

let invalidFrozenPtrConvert t1 t2 = isFrozenPtr t1 && isNonFrozenPtr t2

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
    else
      let decldepth = Hashtbl.find vidOblivDepth vinfo.vid in
      max 0 (decldepth - !currentRootDepth)
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
    if dt#curDepth() = dt#varDepth vinfo then vinfo.vtype
    else typeAddAttributes [Attr("frozen",[])] vinfo.vtype

  (* Counting up on obliv-if and ~obliv blocks  *)
  method vblock = dt#defaultVBlock

  method vtype vtype = match checkOblivType vtype with
    | None -> DoChildren
    | Some cat -> if cat = "unimplemented" 
                    then E.s (E.unimp "obliv float/double/enum")
                    else E.s (E.error "%s cannot be obliv-type" cat)

  method vinst instr = ChangeDoChildrenPost ([instr], List.map (
    fun instr -> match instr with
      | Set (lv,exp,loc) -> 
          if isFrozenQualified (typeOfLval lv) then
            E.s (E.error "%s:%i:cannot assign to frozen qualified lvalue"
                         loc.file loc.line)
          else if invalidFrozenPtrConvert (typeOf exp) (typeOfLval lv) then
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
                  else if invalidFrozenPtrConvert (typeOf a) b then
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

  (* TODO check if break/continue goes through obliv if *)
  method vstmt s = ChangeDoChildrenPost (s, fun s -> match s.skind with
    | If(e,tb,fb,l) -> 
        if isOblivSimple (typeOf e) then
          if isOblivBlock tb then s
          else E.s (E.error 
            "%s:%i:Cannot use obliv-type expression as a condition" l.file
            l.line)
        else s
    | _ -> s
  )

  (* obliv-related typechecking *)
  method vexprObliv e = (e, fun exp -> match exp with
  | UnOp (op,e,t) -> if isOblivSimple (typeOf e) then
                       let tr = addOblivType t in UnOp(op,e,tr)
                     else exp
  | BinOp(op,e1,e2,t) ->
      let t2 = match op with
      | Lt | Gt | Le | Ge | Eq | Ne | LAnd | LOr -> boolType
      | _ -> t
      in
      if isOblivSimple (typeOf e1) || isOblivSimple (typeOf e2) then
        let e1 = mkCast e1 (addOblivType (typeOf e1)) in
        let e2 = mkCast e2 (addOblivType (typeOf e2)) in
        let tr = addOblivType t2 in
        BinOp(op,e1,e2,tr)
      else exp
  | CastE (t,e) when t = typeOf e -> e
  | CastE (t,e) when isImplicitCastResult t ->
      let st = typeOf e in
      if isOblivSimple st && not (isOblivSimple t) then
        E.s (E.error "%a: Cannot convert obliv type '%a' to non-obliv '%a'"
          d_loc !currentLoc d_type st 
            d_type (typeRemoveAttributes ["implicitCast"] t))
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
          let t' = TPtr(typeAddAttributes [Attr("frozen",[])] t2,a) in
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

(* Same as setComparison, but picks the right function name
 * based on signedness of operators *)
let setComparisonUS fnames dest s1 s2 loc = 
  let optype = typeOfLval s1 in
  let fname = match optype with
  | TInt(k,_) -> if isSigned k then snd fnames else fst fnames
  | _ -> E.s (E.error "Cannot operate on obliv values of type %a" d_type optype)
  in
  setComparison fname dest s1 s2 loc

let setLogicalOp fname dest s1 s2 loc = 
  let cOblivBitPtr = TPtr(typeAddAttributes [constAttr] !oblivBitType,[]) in
  let fargTypes = ["dest",!oblivBitPtr,[]
                  ;"s1",cOblivBitPtr,[]; "s2",cOblivBitPtr,[]
                  ] in
  let func = voidFunc fname fargTypes in
  Call(None,func,[CastE(!oblivBitPtr,AddrOf dest)
                 ;CastE(cOblivBitPtr,AddrOf s1)
                 ;CastE(cOblivBitPtr,AddrOf s2)],loc)

let setBitwiseOp fname dest s1 s2 loc =
  setComparison fname dest s1 s2 loc
  (* The only difference is return value type, which is not a problem 
   * as long as we are casting everything to and from void* *)

(* Same comments as in setBitwiseOp *)
let setArith fname dest s1 s2 loc = setBitwiseOp fname dest s1 s2 loc

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
  Call(None,func,[ AddrOf v; xoBitsSizeOf (typeOf x)
                 ; CastE(widestType,CastE(TInt(k,[]),x))
                 ],loc)

let condSetKnownInt c v k x loc = 
  let fargTypes = ["cond",TPtr(oblivBoolType,[]),[]
                  ;"dest",TPtr(typeOfLval v,[]),[]
                  ;"size",!typeOfSizeOf,[]
                  ;"val",widestType,[]
                  ] in
  let func = voidFunc "__obliv_c__condAssignKnown" fargTypes in
  Call(None,func,[ mkAddrOf c; mkAddrOf v; xoBitsSizeOf (typeOf x)
                 ; CastE(widestType,CastE(TInt(k,[]),x))
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
let codegenUncondInstr (instr:instr) : instr = match instr with
| Set(v,BinOp(op,Lval e1,Lval e2,t),loc) when isOblivSimple t ->
    begin match op with
    | PlusA -> setArith "__obliv_c__setPlainAdd" v e1 e2 loc
    | MinusA -> setArith "__obliv_c__setPlainSub" v e1 e2 loc
    | Lt -> setComparisonUS cmpLtFuncs v e1 e2 loc
    | Gt -> setComparisonUS cmpLtFuncs v e2 e1 loc
    | Le -> setComparisonUS cmpLeFuncs v e1 e2 loc
    | Ge -> setComparisonUS cmpLeFuncs v e2 e1 loc
    | Ne -> setComparison "__obliv_c__setNotEqual" v e1 e2 loc
    | Eq -> setComparison "__obliv_c__setEqualTo"  v e1 e2 loc
    | BAnd -> setBitwiseOp "__obliv_c__setBitwiseAnd" v e1 e2 loc
    | BXor -> setBitwiseOp "__obliv_c__setBitwiseXor" v e1 e2 loc
    | BOr  -> setBitwiseOp "__obliv_c__setBitwiseOr" v e1 e2 loc
    | LAnd -> setLogicalOp "__obliv_c__setBitAnd" v e1 e2 loc
    | LOr  -> setLogicalOp "__obliv_c__setBitOr"  v e1 e2 loc
    | _ -> instr
    end
| Set(v,CastE(TInt(k,a) as dt,x),loc) 
    when isOblivSimple dt && not (isOblivSimple (typeOf x)) ->
      setKnownInt v k x loc
| Set(dv,CastE(TInt(dk,da),Lval sv),loc) when hasOblivAttr da ->
    begin match typeOfLval sv with
    | TInt(sk,sa) when hasOblivAttr sa ->
        if isSigned sk then
          setIntExtend "__obliv_c__setSignExtend" dv dk sv sk loc
        else setIntExtend "__obliv_c__setZeroExtend" dv dk sv sk loc
    | _ -> instr
    end
| Call(lvo,exp,args,loc) when isOblivFunc (typeOf exp) ->
    Call(lvo,exp,mkAddrOf trueCond::args,loc)
| _ -> instr

let isTaggedTemp lv = hasAttribute SimplifyTagged.simplifyTempTok
                        (typeAttrs (typeOfLval lv))

(* Ok, I really should stop adding more parameters here 
 * curCond : generated instructions should only have an effect if this is true
 * tmpVar: creates a new temporary var in current function
 * isDeepVar: checks if a given varinfo is declared at current block scope (vs.
 *              some outer scope
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
        [setIfThenElse v curCond v2 v loc]
      else setUsingTmp v (Lval v2) loc
  | Set(v,(BinOp(_,_,_,t) as x),loc) when isOblivSimple t -> setUsingTmp v x loc
  | Set(v,(CastE(TInt(k,a),x) as xf),loc) when isOblivSimple (typeOfLval v) ->
      if isOblivSimple (typeOf x) then setUsingTmp v xf loc
      else [condSetKnownInt curCond v k x loc]
  | Call(lvo,exp,args,loc) when isOblivFunc (typeOf exp) ->
      [Call(lvo,exp,mkAddrOf curCond::args,loc)]
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

class codegenVisitor (curFunc : fundec) (dt:depthTracker) (curCond : lval) 
  : cilVisitor = object(self)
  inherit nopCilVisitor

  method vstmt s = 
    let tmpVar t = SimplifyTagged.makeSimplifyTemp curFunc t in
    let isDeepVar v = dt#curDepth() = dt#varDepth v in
    match s.skind with
    | Instr ilist -> 
        let nestedGen = codegenInstr curCond tmpVar isDeepVar in
        ChangeTo (mkStmt (Instr (mapcat nestedGen ilist)))
    | If(c,tb,fb,loc) when isOblivBlock tb ->
        let cv = var (tmpVar oblivBoolType) in
        let ct = var (tmpVar oblivBoolType) in
        let cf = var (tmpVar oblivBoolType) in
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
    else match ripOblivEnVar curFunc b with
    | Some vi -> 
        let asg = mkStmt (Instr [Set (var vi,Lval curCond,!currentLoc)]) in
        let b' = { bstmts = asg :: b.bstmts
                 ; battrs = dropAttribute "~obliv" b.battrs } in 
        ChangeTo (visitCilBlock (new codegenVisitor curFunc dt trueCond) b')
    | None -> DoChildren
  end
  method vfunc = dt#wrapVFunc begin fun f ->
    if isOblivFunc f.svar.vtype || hasOblivBlocks f then begin
      (* This really needs to be done only for obliv types
       * But I am too lazy to traverse through structs *)
      let nontemp = List.filter (fun v -> not (isTaggedTemp (var v)))
                                f.slocals in
      self#queueInstr (List.map (fun v -> zeroSet v !currentLoc) nontemp);
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
  method vtype t = let t' = typeRemoveAttributes ["implicitCast";"frozen"] t in
    ChangeDoChildrenPost (t', fun t -> match t with
  | TInt(k,a) when hasOblivAttr a -> 
      let a2 = dropOblivAttr a in
      setTypeAttrs (intTargetType k) a2
  | TFun(tres,argso,vargs,a) when isOblivFunc t -> 
      let entarget = visitCilType (self :> cilVisitor) constOblivBoolPtrType in
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

let feature : featureDescr =
  { fd_name = "processObliv";
    fd_enabled = ref false;
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
