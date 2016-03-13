Installing OCaml findlib
========================

When installing Obliv-C, if the `./configure` command gives you an error about missing findlib, you probably need to follow these steps to install findlib.

Step 1) Install opam: `sudo apt-get install opam m4`

Step 2) Initialize Opam for first use. Use `opam --version` to check Opam version. You can find latest information in the [Opam FAQ](https://opam.ocaml.org/doc/FAQ.html#Gaspopaminitgivesmescreensfullsoferrorsaboutupgrading)

  * If it is 1.2 or higher, you can just run `opam init`
  * If it is 1.1, run `opam init https://opam.ocaml.org/1.1`
 
It will ask you to add a line on your `~/.bashrc`. Say 'y', do *not* just press enter. If you do press enter by mistake, either rerun the `init` command as above, or manually add the following line:

```
. ~/.opam/opam-init/init.sh > /dev/null 2> /dev/null || true
```
 
Or, you can of course just run this command each time you do a `./configure`.

Step 3) Install findlib: `opam install ocamlfind`

At this point, installation of Obliv-C should work if you resume from the `./configure` step of [README.md](README.md). You might have to restart your terminal window or re-execute `~/bashrc` for this. Once again, please contact Samee Zahur at samee@virgnia.edu if you run into problems.
