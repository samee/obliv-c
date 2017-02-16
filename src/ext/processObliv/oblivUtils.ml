open Cil
module E = Errormsg

let compose f g x = f (g x)

let mapcat f l = 
  let rec aux acc l = match l with
  | [] -> acc
  | x::xs -> aux (List.rev_append (f x) acc) xs
  in
  List.rev (aux [] l)

let constAttr = Attr("const",[])

let rec firstSome f l = match l with
| [] -> None
| (x::xs) -> match f x with Some r -> Some r
                          | None -> firstSome f xs


(* Initialized during type-checking *)
let oblivBitType = ref (TVoid [])
let oblivBitPtr  = ref (TVoid [])
let cOblivBitPtr  = ref (TVoid [constAttr])
let oblivConstBitPtr = ref (TVoid [])


type isRipOblivResult = NotRipObliv | RipOblivNoVar | RipOblivWithVar of varinfo

(* Returns the name of the new local variable for condition if true *)
let rec isRipObliv (f:fundec) (b:block) : isRipOblivResult =
  let x = filterAttributes "~obliv" b.battrs in
  match x with
  | [Attr(_,[AStr vname])] -> 
      if vname = "" then RipOblivNoVar
      else
        let t = addOblivType (TInt(IBool,[])) in
        (* The variable was already inserted in cabs2cil, but we only saved its
         * name, not a varinfo *)
        RipOblivWithVar (makeLocalVar f ~insert:false vname t)
  | _::_ -> E.s (E.error "directly nested ~obliv blocks")
  | [] -> match b.bstmts with
          | [s] -> begin match s.skind with
                   | Block b2 -> isRipObliv f b2
                   | _ -> NotRipObliv
                   end
          | _ -> NotRipObliv

let rec dropRipOblivAttr (b:block) : block =
  let attr = dropAttribute "~obliv" b.battrs in
  let stmts = match b.bstmts with
  | [s] -> begin match s.skind with
           | Block b2 -> [{s with skind = Block (dropRipOblivAttr b2)}]
           | _ -> b.bstmts
           end
  | _ -> b.bstmts
  in
  { battrs = attr; bstmts = stmts }

(* Grep on oblivBitsSizeOf and fix everything FIXME *)
let oblivBitsSizeOf t = begin match t with
| TInt(IBool,_) -> 1
| _ -> bitsSizeOf t
end

let xoBitsSizeOf t =kinteger !kindOfSizeOf (oblivBitsSizeOf (unoblivType t))

