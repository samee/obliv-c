
open Pretty
open Printf
open Cil
module E = Errormsg
module H = Hashtbl

let hasOblivAttr = List.exists (function Attr("obliv",_) -> true | _ -> false);;

let rec firstSome f l = match l with
| [] -> None
| (x::xs) -> match f x with Some r -> Some r
                         | None -> firstSome f xs


let hasOblivAttr = hasAttribute "obliv"
let addOblivAttr a = if not (hasOblivAttr a) 
                       then addAttribute (Attr("obliv",[])) a
                       else a

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

(* Is t1 -> t2 conversion valid with regard to oblivious-ness *)
(* vtype ensures stupid types do not show up in declarations or casts.
 * So isOblivSimple can assume stupid types do not exist. *)
let invalidOblivConvert t1 t2 = isOblivSimple t1 && isNonOblivSimple t2

let conversionError () = E.s (E.error "cannot convert obliv type to non-obliv")

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
            conversionError()
          else instr
      | Call (lvopt,f,args,loc) ->
          begin match typeOf f with
          | TFun (tr,targsPack,isVarg,a) -> begin
              let targs = argsToList targsPack in
              let rec matchArgs a b = match a,b with
              | _,[] -> ()
              | [],_ -> E.s (E.error "too few arguments to function")
              | a::al, (_,b,_)::bl -> 
                  if invalidOblivConvert (typeOf a) b then conversionError()
                  else matchArgs al bl
              in
              matchArgs args targs;
              match lvopt with 
              | Some lv -> 
                  if invalidOblivConvert tr (typeOfLval lv) then
                    conversionError()
                  else instr
              | None -> instr
            end
          | _ -> E.s (E.error "trying to call non-function")
          end
      | _ -> instr
      ))

  (* TODO check if break/continue goes through obliv if *)
  method vstmt s = ChangeDoChildrenPost (s, fun s -> match s.skind with
    | If(e,tb,fb,l) -> 
        if isOblivSimple (typeOf e) then
          if isOblivBlock tb then s
          else
            E.s (E.error "Cannot use obliv-type expression as a condition")
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
end

let oblivIntType = addOblivType intType
let oblivCharType = addOblivType charType
let oblivBoolType = oblivCharType

(* FIXME these functions should use custom structs, not obliv int *)
let condAssignKnown = 
  let fargTypes = ["dest",TPtr(oblivIntType,[]),[]
                  ;"cond",TPtr(oblivBoolType,[]),[]
                  ;"bitcount",intType,[]
                  ;"val",TInt(ILongLong,[]),[]
                  ] in
  let ftype = TFun (TVoid [],Some fargTypes,false,[]) in
  let fvar = makeGlobalVar "__obliv_c__condAssignKnown" ftype in
  Lval (Var fvar,NoOffset)

let setLessThan = 
  let fargTypes = ["dest",TPtr(oblivBoolType,[]),[]
                  ;"op1",TPtr(oblivIntType,[]),[]
                  ;"op2",TPtr(oblivIntType,[]),[]
                  ;"bitcount",intType,[]
                  ] in
  let ftype = TFun (TVoid [], Some fargTypes, false, []) in
  let fvar = makeGlobalVar "__obliv_c__setLessThan" ftype in
  Lval (Var fvar,NoOffset)

let setLogicalAnd = 
  let fargTypes = ["dest",TPtr(oblivBoolType,[]),[]
                  ;"op1",TPtr(oblivBoolType,[]),[]
                  ;"op2",TPtr(oblivBoolType,[]),[]
                  ] in
  let ftype = TFun (TVoid [], Some fargTypes, false, []) in
  let fvar = makeGlobalVar "__obliv_c__setLogicalAnd" ftype in
  Lval (Var fvar,NoOffset)

let setLogicalXor = 
  let fargTypes = ["dest",TPtr(oblivBoolType,[]),[]
                  ;"op1",TPtr(oblivBoolType,[]),[]
                  ;"op2",TPtr(oblivBoolType,[]),[]
                  ] in
  let ftype = TFun (TVoid [], Some fargTypes, false, []) in
  let fvar = makeGlobalVar "__obliv_c__setLogicalXor" ftype in
  Lval (Var fvar,NoOffset)

class codegenVisitor (curFunc:fundec) (curCond:varinfo) : cilVisitor = object
  inherit nopCilVisitor
  method vstmt s = match s.skind with
  | Instr ilist -> 
      let f instr = match instr with
      | Set(v,CastE(destt,srcx),loc) when isOblivSimple destt -> begin
            let isize = SizeOf (typeOf(Lval v)) in
            let condval = (Var curCond,NoOffset) in
            Call(None,condAssignKnown,[AddrOf v;AddrOf condval;isize;srcx],loc)
          end
      | _ -> instr
      in
      ChangeTo (mkStmt (Instr (List.map f ilist)))
  | If(c,tb,fb,loc) when isOblivBlock tb ->
      let cv = makeTempVar curFunc oblivBoolType in
      let ct = makeTempVar curFunc oblivBoolType in
      let cf = makeTempVar curFunc oblivBoolType in
      let addrOfV v = mkAddrOf (var v) in
      let boolsize = SizeOf oblivBoolType in
      let visitSubBlock cond blk = 
        visitCilBlock (new codegenVisitor curFunc cond) blk in
      let cs = mkStmt (Instr 
        [ Set (var cv,c,loc) (* oblivify TODO *)
        ; Call(None,setLogicalAnd,[addrOfV ct;addrOfV cv;addrOfV curCond
                                  ;boolsize],loc)
        ; Call(None,setLogicalXor,[addrOfV cf;addrOfV ct;addrOfV curCond
                                  ;boolsize],loc)
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

let feature : featureDescr =
  { fd_name = "processObliv";
    fd_enabled = ref false;
    fd_description = "handles obliv keyword";
    fd_extraopt = [];
    fd_doit = 
    (function (f: file) -> 
      let tcVisitor = new typeCheckVisitor in
      visitCilFileSameGlobals tcVisitor f;
      mapGlobals f genFunc
    );
    fd_post_check = true;
  } 
