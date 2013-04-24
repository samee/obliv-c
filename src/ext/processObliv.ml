
open Pretty
open Printf
open Cil
module E = Errormsg
module H = Hashtbl

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

let oblivIntType = addOblivType intType
let oblivCharType = addOblivType charType
let oblivBoolType = oblivCharType

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
let condSimpleObliv = ref dummyExp
let setLessThan = ref dummyExp
let setLogicalAnd = ref dummyExp
let setLogicalXor = ref dummyExp
let setNotEqual = ref dummyExp
let setEqualTo = ref dummyExp

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
  let compops = 
    [setEqualTo,"__obliv_c__setEqualTo"
    ;setNotEqual,"__obliv_c__setNotEqual"
    ;setLessThan,"__obliv_c__setLessThan"
    ] in
  List.iter (fun(fref,fname) -> 
    let fargTypes = ["dest",!oblivBitPtr,[]
                    ;"op1",!oblivConstBitPtr,[] ;"op2",!oblivConstBitPtr,[]
                    ;"bitcount",!typeOfSizeOf,[]
                    ] in
    let ftype = TFun (TVoid [],Some fargTypes,false,[]) in
    fref := Lval (Var (makeGlobalVar fname ftype),NoOffset)
    ) compops;
  (* TODO roll these up *)
  condAssignKnown := begin
    let fargTypes = [ "dest",!oblivBitPtr,[]; "cond",!oblivConstBitPtr,[]
                    ; "bitcount",intType,[]; "val",widestType,[] ] in
    let ftype = TFun (TVoid [],Some fargTypes,false,[]) in
    Lval (Var (makeGlobalVar "__obliv_c__condAssignKnown" ftype),NoOffset)
  end;
  condSimpleObliv := begin
    let fargTypes = [ "dest",!oblivBitPtr,[]; "src",!oblivConstBitPtr,[]
                    ; "bitcount",intType,[] ] in
    let ftype = TFun (TVoid [], Some fargTypes, false, []) in
    Lval (Var (makeGlobalVar "__obliv_c__copySimpleObliv" ftype),NoOffset)
  end;
  setLogicalAnd := begin
    let fargTypes = ["dest",!oblivBitPtr,[]
                    ;"op1",!oblivConstBitPtr,[];"op2",!oblivConstBitPtr,[]] in
    let ftype = TFun (TVoid[], Some fargTypes, false, []) in
    Lval (Var (makeGlobalVar "__obliv_c__setBitAnd" ftype),NoOffset)
  end;
  setLogicalXor := begin
    let fargTypes = ["dest",!oblivBitPtr,[]
                    ;"op1",!oblivConstBitPtr,[];"op2",!oblivConstBitPtr,[]] in
    let ftype = TFun (TVoid[], Some fargTypes, false, []) in
    Lval (Var (makeGlobalVar "__obliv_c__setBitXor" ftype),NoOffset)
  end
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
      if isOblivSimple (typeOf e1) || isOblivSimple (typeOf e2) then
        let e1 = mkCast e1 (addOblivType (typeOf e1)) in
        let e2 = mkCast e2 (addOblivType (typeOf e2)) in
        let tr = addOblivType t in
        BinOp(op,e1,e2,tr)
      else exp
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


(* Just copy doesn't cut it. Need a single function for op-and-assign *)
let codegenInstr curCond (instr:instr) : instr = 
  let namedOblivTInt tv = match tv with
  | TNamed ({tname = tn},[]) -> 
      begin match tn with
      | "__obliv_c__bool" | "__obliv_c__char" | "__obliv_c__short"
      | "__obliv_c__long" | "__obliv_c__int"  | "__obliv_c__lLong" -> true
      | _ -> false
      end
  | _ -> false
  in
  let simptemp lv = hasAttribute SimplifyTagged.simplifyTempTok 
                      (typeAttrs (typeOfLval lv)) in
  match instr with
  | Set(v,BinOp(op,Lval(e1),Lval(e2),t),loc) 
  (* TODO fold this up *)
      when isOblivSimple t && simptemp v ->
        let size t = 
          if namedOblivTInt t then
            raise (Failure "fix this mess, the types are already named")
          else kinteger !kindOfSizeOf (oblivBitsSizeOf t) in
        begin match op with
        | Lt -> Call(None,!setLessThan,
                  [AddrOf v;AddrOf e1;AddrOf e2;size t],loc)
        | LAnd -> Call(None,!setLogicalAnd,
                  [AddrOf v;AddrOf e1;AddrOf e2;size t],loc)
        | Ne -> Call(None,!setNotEqual,
                  [AddrOf v;AddrOf e1;AddrOf e2;size t],loc)
        | Eq -> Call(None,!setEqualTo,
                  [AddrOf v;AddrOf e1;AddrOf e2;size t],loc)
        | _ -> instr
        end
  | Set(v,CastE(destt,srcx),loc) when isOblivSimple destt -> begin
        let isize = SizeOf (typeOf(Lval v)) in
        let condval = (Var curCond,NoOffset) in
        Call(None,!condAssignKnown,[AddrOf v;AddrOf condval;isize;srcx],loc)
      end
  | _ -> instr


class codegenVisitor (curFunc:fundec) (curCond:varinfo) : cilVisitor = object
  inherit nopCilVisitor
  method vtype t = match t with
  | TInt(k,a) when hasOblivAttr a -> 
      let a2 = dropOblivAttr a in
      ChangeTo (setTypeAttrs (intTargetType k) a2)
  | _ -> DoChildren

  method vstmt s = match s.skind with
  | Instr ilist -> 
      ChangeTo (mkStmt (Instr (List.map (codegenInstr curCond) ilist)))
  | If(c,tb,fb,loc) when isOblivBlock tb ->
      let cv = SimplifyTagged.makeSimplifyTemp curFunc oblivBoolType in
      let ct = SimplifyTagged.makeSimplifyTemp curFunc oblivBoolType in
      let cf = SimplifyTagged.makeSimplifyTemp curFunc oblivBoolType in
      let visitSubBlock cond blk = 
        visitCilBlock (new codegenVisitor curFunc cond) blk in
      let cs = mkStmt (Instr 
        [ codegenInstr curCond (Set (var cv,c,loc))
        ; codegenInstr curCond (Set (var ct,
            BinOp(LAnd,Lval(var cv),Lval(var curCond),oblivBoolType),loc))
        ; codegenInstr curCond (Set (var cf,
            BinOp(Ne  ,Lval(var ct),Lval(var curCond),oblivBoolType),loc))
        ]) in
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

let trueCond = makeGlobalVar "__obliv_c__trueCond" oblivBoolType

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
      (*
      SimplifyTagged.feature.fd_doit f;
      mapGlobals f genFunc;
      visitCilFileSameGlobals (new rmSimplifyVisitor) f
      *)
    );
    fd_post_check = true;
  } 
