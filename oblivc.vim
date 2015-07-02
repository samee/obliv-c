" Adding these lines in .vimrc usually helps with .oc and .oh files

function SetOblivcOptions()
  " Hijack C syntax highlighting and indenting
  set filetype=c
  syn match ocConditional "obliv\s\+if"
  syn keyword cType frozen
  syn match ocOblivType "obliv\(\s\+if\)\@!"
  syn match cStatement "\~obliv"
  hi def link ocConditional Conditional
  hi def link ocOblivType Type
endfunction

autocmd BufNewFile,BufRead *.o[ch] call SetOblivcOptions()
