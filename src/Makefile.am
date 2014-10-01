bin_PROGRAMS=dfalse
dfalse_SOURCES=main.c

vimsyntaxdir=${HOME}/.vim/syntax
vimsyntax_DATA=vim/syntax/df.vim

vimftdetectdir=${HOME}/.vim/ftdetect
vimftdetect_DATA=vim/ftdetect/df.vim

vimindentdir=${HOME}/.vim/indent
vimindent_DATA=vim/indent/df.vim

.PHONY: test
test:
	 ./dfalse test.df

.PHONY: run
run:
	gdb dfalse
