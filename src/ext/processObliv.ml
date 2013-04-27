
open Pretty
open Printf
open Cil
module E = Errormsg
module H = Hashtbl

let typeEqual = SimplifyTagged.typeEqual

(* Initialized during type-checking *)
let oblivBitType = ref (TVoid [])
let oblivBitPtr  = ref (TVoid [])
let oblivConstBitPtr = ref (TVoid [])

let rec firstSome f l = match l with
| [] -> None
| (x::xs) -> match f x with Some r -> Some r
                          | None -> firstSome f xs


let hasOblivAttr = hasAttribute "obliv"
let addOblivAttr a = if not (hasOblivAttr a) 
                       then addAttribute (Attr("obliv",[])) a
                       else a
let dropOblivAttr a = dropAttribute "obliv" a

let addOblivType t = typeAddAttributes [Attr("obliv",[])] t

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

let isOblivSimple t = match t with
| TInt(_,a) | TFloat(_,a) -> hasOblivAttr a
| _ -> false

let isNonOblivSimple t = match t with
| TInt(_,a) | TFloat(_,a) -> not (hasOblivAttr a)
| _ -> false

(* Pre-flattening them becomes harder after the CFG is already made *)
let rec isOblivBlock b = 
  if hasOblivAttr b.battrs then true
  else match b.bstmts with 
  | [s] -> begin match s.skind with (* only one statement *)
           | Block b2 -> isOblivBlock b2
           | _ -> false
           end
  | _ -> false


let boolType = TInt(IBool,[])

let oblivIntType = addOblivType intType
let oblivCharType = addOblivType charType
let oblivBoolType = addOblivType boolType

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

let oblivBitsSizeOf t = begin match t with
| TInt(IBool,_) -> 1
| _ -> bitsSizeOf t
end

let updateOblivBitType ci = begin
  oblivBitType := TComp(ci,[]);
  oblivBitPtr := TPtr(!oblivBitType,[]);
  oblivConstBitPtr := typeAddAttributes [Attr("const",[])] !oblivBitPtr;
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

let conversionError loc =
  E.s (E.error "%s:%i:cannot convert obliv type to non-obliv" loc.file loc.line)

class typeCheckVisitor = object
  inherit nopCilVisitor
  method vtype vtype = match checkOblivType vtype with
    | None -> DoChildren
    | Some cat -> if cat = "unimplemented" 
                    then E.s (E.unimp "obliv float/double/enum")
                    else E.s (E.error "%s cannot be obliv-type" cat)

  method vinst instr = ChangeDoChildrenPost ([instr], List.map (
    fun instr -> match instr with
      | Set (lv,exp,loc) -> 
          if invalidOblivConvert (typeOf exp) (typeOfLval lv) then
            conversionError loc
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
                  if invalidOblivConvert (typeOf a) b then conversionError loc
                  else matchArgs al bl
              in
              matchArgs args targs;
              match lvopt with 
              | Some lv -> 
                  if invalidOblivConvert tr (typeOfLval lv) then
                    conversionError loc
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

  method vexpr e = ChangeDoChildrenPost (e, fun exp -> match exp with
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
  | CastE (t,e) -> if isOblivSimple (typeOf e) && not (isOblivSimple t) 
                     then mkCast e (addOblivType t)
                     else exp
  | _ -> exp
  )

  method vglob v = begin match v with 
  | GCompTag(ci,loc) when ci.cname = "OblivBit" ->  
      updateOblivBitType ci; DoChildren
  | _ -> DoChildren
  end
end

let constAttr = Attr("const",[])
let voidFunc name argTypes =
  let ftype = TFun(TVoid [], Some argTypes,false,[]) in
  Lval(Var(makeGlobalVar name ftype),NoOffset)


let setComparison fname dest s1 s2 loc = 
  let optype = typeOfLval s1 in
  let coptype = typeAddAttributes [constAttr] optype in
  let fargTypes = ["dest",TPtr(oblivBoolType,[]),[]
                  ;"s1",TPtr(coptype,[]),[];"s2",TPtr(coptype,[]),[]
                  ;"bitcount",!typeOfSizeOf,[]
                  ] in
  let func = voidFunc fname fargTypes in
  Call(None,func,[AddrOf dest; AddrOf s1; AddrOf s2
                 ;kinteger !kindOfSizeOf (oblivBitsSizeOf optype)],loc)

let setLogicalOp fname dest s1 s2 loc = 
  (*
  let cbool = typeAddAttributes [constAttr] oblivBoolType in *)
  let cOblivBitPtr = TPtr(typeAddAttributes [Attr("const",[])] 
                                            !oblivBitType,[]) in
  let fargTypes = ["dest",!oblivBitPtr,[]
                  ;"s1",cOblivBitPtr,[];"s2",cOblivBitPtr,[]
                  ;"bitcount",!typeOfSizeOf,[]
                  ] in
  let func = voidFunc fname fargTypes in
  Call(None,func,[CastE(!oblivBitPtr,AddrOf dest)
                 ;CastE(cOblivBitPtr,AddrOf s1)
                 ;CastE(cOblivBitPtr,AddrOf s2)],loc)

let setKnownInt v k x loc = 
  let fargTypes = ["dest",TPtr(typeOfLval v,[]),[]
                  ;"bitcount",!typeOfSizeOf,[]
                  ;"value",widestType,[]
                  ] in
  let func = voidFunc "__obliv_c__setSignedKnown" fargTypes in
  Call(None,func,[ AddrOf v; kinteger !kindOfSizeOf (oblivBitsSizeOf (typeOf x))
                 ; CastE(widestType,CastE(TInt(k,[]),x))
                 ],loc)

let xoBitsSizeOf t = kinteger !kindOfSizeOf (oblivBitsSizeOf t)

let condSetKnownInt c v k x loc = 
  let fargTypes = ["cond",TPtr(oblivBoolType,[]),[]
                  ;"dest",TPtr(typeOfLval v,[]),[]
                  ;"size",!typeOfSizeOf,[]
                  ;"val",widestType,[]
                  ] in
  let func = voidFunc "__obliv_c__condAssignKnown" fargTypes in
  Call(None,func,[ AddrOf (var c); AddrOf v; xoBitsSizeOf (typeOf x)
                 ; CastE(widestType,CastE(TInt(k,[]),x))
                 ],loc)

let trueCond = makeGlobalVar "__obliv_c__trueCond" oblivBoolType

(* Codegen, when conditions don't matter *)
let codegenUncondInstr (instr:instr) : instr = match instr with
| Set(v,BinOp(op,Lval e1,Lval e2,t),loc) when isOblivSimple t ->
    begin match op with
    | Lt -> setComparison "__obliv_c__setLessThan" v e1 e2 loc
    | Ne -> setComparison "__obliv_c__setNotEqual" v e1 e2 loc
    | Eq -> setComparison "__obliv_c__setEqualTo"  v e1 e2 loc
    | LAnd -> setLogicalOp "__obliv_c__setBitAnd" v e1 e2 loc
    | _ -> instr
    end
| Set(v,CastE(TInt(k,a) as dt,x),loc) when isOblivSimple dt -> 
    if not (isOblivSimple (typeOf x)) then
      setKnownInt v k x loc
    else instr
      (* TODO
    (* (* Sign/zero extend already truncates in the library *)
    else if source is wider then
      truncate
      *)
    (* Think harder about extending laws *)
    else if dest is signed then
      setSignExtend
    else setZeroExtend
    *)
| _ -> instr


(* Just copy doesn't cut it. Need a single function for op-and-assign *)
let codegenInstr curCond (instr:instr) : instr = 
  let simptemp lv = hasAttribute SimplifyTagged.simplifyTempTok 
                      (typeAttrs (typeOfLval lv)) in
  if curCond == trueCond then codegenUncondInstr instr
  else match instr with 
  | Set(v,_,_) when simptemp v -> codegenUncondInstr instr
  | Set(v,CastE(TInt(k,a),x),loc) when isOblivSimple (typeOfLval v) ->
      if isOblivSimple (typeOf x) then
        instr (* TODO *)
      else condSetKnownInt curCond v k x loc
  | _ -> instr

class typeFixVisitor : cilVisitor = object
  inherit nopCilVisitor
  method vtype t = match t with
  | TInt(k,a) when hasOblivAttr a -> 
      let a2 = dropOblivAttr a in
      ChangeTo (setTypeAttrs (intTargetType k) a2)
  | _ -> DoChildren
end

class codegenVisitor (curFunc:fundec) (curCond:varinfo) : cilVisitor = object
  inherit nopCilVisitor

  method vstmt s = match s.skind with
  | Instr ilist -> 
      ChangeTo (mkStmt (Instr (List.map (codegenInstr curCond) ilist)))
  | If(c,tb,fb,loc) when isOblivBlock tb ->
      let cv = SimplifyTagged.makeSimplifyTemp curFunc oblivBoolType in
      let ct = SimplifyTagged.makeSimplifyTemp curFunc oblivBoolType in
      let cf = SimplifyTagged.makeSimplifyTemp curFunc oblivBoolType in
      let visitSubBlock cond blk = 
        visitCilBlock (new codegenVisitor curFunc cond) blk in
      let cs = mkStmt (Instr (List.map (codegenInstr trueCond)
        [ Set (var cv,c,loc)
        ; Set (var ct,BinOp(LAnd,Lval(var cv)
                                ,Lval(var curCond),oblivBoolType),loc)
        ; Set (var cf,BinOp(Ne  ,Lval(var ct)
                                ,Lval(var curCond),oblivBoolType),loc)
        ])) in
      let ts = mkStmt (Block (visitSubBlock ct tb)) in
      let fs = mkStmt (Block (visitSubBlock cf fb)) in
      ChangeTo (mkStmt (Block {battrs=[]; bstmts=[cs;ts;fs]}))
  | _ -> DoChildren

  method vblock b = if isOblivBlock b
    then ChangeDoChildrenPost 
          ( { b with battrs = dropAttribute "obliv" b.battrs }
          , fun x -> x)
    else DoChildren
end

let genFunc g = match g with
| GFun(f,loc) -> GFun(visitCilFunction (new codegenVisitor f trueCond) f,loc)
| _ -> g
;;

(* If this gets too slow, merge it with codegen *)
class rmSimplifyVisitor = object
  inherit nopCilVisitor
  method vattr at = match at with 
  | Attr(s,[]) when s = SimplifyTagged.simplifyTempTok -> ChangeTo []
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
      visitCilFileSameGlobals tcVisitor f;
      SimplifyTagged.feature.fd_doit f; (* Note: this can screw up type equality
                                                 checks *)
      mapGlobals f genFunc;
      (* might merge these two *)
      visitCilFileSameGlobals (new rmSimplifyVisitor) f;
      visitCilFileSameGlobals (new typeFixVisitor) f
    );
    fd_post_check = true;
  } 
