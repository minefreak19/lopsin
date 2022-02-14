" Vim syntax file
" Language: LopAsm

if exists("b:current_syntax")
	finish
endif

set iskeyword+=_,!,@-@

syntax keyword lopasmKeywords nop hlt push drop dup swap sum sub mul div mod shl shr bor band xor bnot lor land lnot gt lt gte lte eq neq jmp cjmp rjmp crjmp call ret ncall cast @8 @16 @32 @64 !8 !16 !32 !64

syntax match lopasmLabel "\w\+:"

syntax match lopasmLitInt "[+-]\?\d\+"

syntax match lopasmComment "//.*$"
syntax match lopasmComment "#.*$"
syntax match lopasmComment ";.*$"

" TODO(#1): vim syntax does not support the character literal '\''
syntax region lopasmChar start='\'' skip='\\\'' end='\''

highlight default link lopasmKeywords	Statement
highlight default link lopasmLabel 	Function
highlight default link lopasmLitInt	Constant
highlight default link lopasmComment Comment
highlight default link lopasmChar 	Constant
