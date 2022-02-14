" Vim syntax file
" Language: LopAsm

if exists("b:current_syntax")
	finish
endif

set iskeyword+=_,!,@-@

syntax keyword lopasmKeywords nop hlt push drop dup swap isum fsum isub fsub imul fmul idiv fdiv imod fmod shl shr bor band xor bnot lor land lnot igt fgt ilt flt igte fgte ilte flte ieq feq ineq fneq jmp cjmp rjmp crjmp call ret ncall cast @8 @16 @32 @64 !8 !16 !32 !64 f2i i2f

syntax match lopasmLabel "\w\+:"

syntax match lopasmLitInt "[+-]\?\d\+"

syntax match lopasmComment "//.*$"
syntax match lopasmComment "#.*$"
syntax match lopasmComment ";.*$"

syntax region lopasmChar start='\'' skip='\\\'' end='\''

highlight default link lopasmKeywords	Statement
highlight default link lopasmLabel 	Function
highlight default link lopasmLitInt	Constant
highlight default link lopasmComment Comment
highlight default link lopasmChar 	Constant
