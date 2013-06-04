open Cil

let mapcat f l = 
  let rec aux acc l = match l with
  | [] -> acc
  | x::xs -> aux (List.rev_append (f x) acc) xs
  in
  List.rev (aux [] l)

let constAttr = Attr("const",[])

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

let isOblivFunc t = match t with
| TFun(_,_,_,a) -> hasOblivAttr a
| _ -> false

let isOblivSimple t = match t with
| TInt(_,a) | TFloat(_,a) -> hasOblivAttr a
| _ -> false

let rec isOblivSimpleOrArray t = match t with
| TInt(_,a) | TFloat(_,a) -> hasOblivAttr a
| TArray(t,_,_) -> isOblivSimpleOrArray t
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


let oblivBitsSizeOf t = begin match t with
| TInt(IBool,_) -> 1
| _ -> bitsSizeOf t
end

let xoBitsSizeOf t = kinteger !kindOfSizeOf (oblivBitsSizeOf t)

