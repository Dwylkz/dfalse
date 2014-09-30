## FALSE interpretor
### description
[FALSE](http://strlen.com/false/false.txt) programming language implement purely written in C
without buffer flush operation and stack pick operation
as my keyboard has no that two funny keys.
vim syntax hightlight and indent is inside.
### install
```bash
./configure ...
make install
```
### usage
```bash
<usage> = "dfalse" <src-with-df-suffix>
```
### demo
#### src.df:
```false
{ simple echo }
"hello echo>"
[^$1_=~][,]#%
```
#### execute like 
```bash
dfalse src.df
hello echo>hehe
hehe
```
#### explicit error message
#### src.df
```false
{ simple echo }
"hello echo>"
[^$1_=][,#
```
#### again execute it
```bash
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
```
