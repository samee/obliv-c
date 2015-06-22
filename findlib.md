Installing OCaml findlib
========================

When installing Obliv-C, if the `./configure` command gives you an error about missing findlib, you probably need to follow these steps to install findlib.

Step 1) Install opam: `sudo apt-get install opam m4`

Step 2) Initialize Opam for first use. Use `opam --version` to check Opam version. You can find latest information in the [Opam FAQ](https://opam.ocaml.org/doc/FAQ.html#Gaspopaminitgivesmescreensfullsoferrorsaboutupgrading)

  * If it is 1.2 or higher, you can just run `opam init`
  * If it is 1.1, run `opam init https://opam.ocaml.org/1.1`

Step 3) Install findlib: `opam install ocamlfind`

At this point, installation of Obliv-C should work if you resume from the `./configure` step of [README.md](README.md). You might have to restart your terminal window or re-execute `~/bashrc` for this. Once again, please contact Samee Zahur at samee@virgnia.edu if you run into problems.
