" Vim syntax file
" Language: LopAsm

if exists("b:current_syntax")
	finish
endif

syntax keyword lopasmKeywords nop hlt push drop dup swap sum sub mul div mod shl shr bor band xor bnot lor land lnot gt lt gte lte eq neq jmp cjmp rjmp crjmp call ret dump putc

syntax match lopasmLabel "\w\+:"

syntax match lopasmLitInt "[+-]\?\d\+"

syntax match lopasmComment "//.*$"
syntax match lopasmComment "#.*$"
syntax match lopasmComment ";.*$"

" TODO: this does not support the character literal '\''
syntax region lopasmChar start='\'' end='\''

highlight default link lopasmKeywords	Statement
highlight default link lopasmLabel 	Function
highlight default link lopasmLitInt	Constant
highlight default link lopasmComment Comment
highlight default link lopasmChar 	Constant
