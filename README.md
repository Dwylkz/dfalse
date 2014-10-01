## FALSE interpretor
### description
"""
FALSE[1][http://strlen.com/false/false.txt] programming language implement purely written in C
without buffer flush operation and stack pick operation
as my keyboard has no that two funny keys.
vim syntax hightlight and indent is inside.
"""
### install
"""
./configure ...
make install
"""
### usage
"""
<usage> = "dfalse" <src-with-df-suffix>
"""
### demo
> src.df:
"""
{ simple echo }
"hello echo>"
[^$1_=~][,]#%
"""
> excute like this
"""
dfalse src.df

hello echo>hehe
hehe
"""
> explicit error probe message
"""
{ simple echo }
"hello echo>"
[^$1_=][,#
"""
"""
dfalse src.df
missing matched ]
3:8: from here
3:8: [^$1_=~][
3:8:        ^
3:8: from here
3:8: [^$1_=][
3:8:        ^
interpret failed
pop function
hello echo>
"""
