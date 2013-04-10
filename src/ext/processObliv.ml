
open Pretty
open Cil
module E = Errormsg
module H = Hashtbl

let hasOblivAttr = List.exists (function Attr("obliv",_) -> true | _ -> false);;

(* TODO check obliv array length (variable) in decl *)

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

let feature : featureDescr =
  { fd_name = "processObliv";
    fd_enabled = ref false;
    fd_description = "handles obliv keyword";
    fd_extraopt = [];
    fd_doit = 
    (function (f: file) -> 
      let lwVisitor = new typeCheckVisitor in
      visitCilFileSameGlobals lwVisitor f);
    fd_post_check = true;
  } 
