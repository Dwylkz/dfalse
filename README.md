## [FALSE](http://strlen.com/false/false.txt)
### description
> FALSE programming language implement purely written in C
using functional programming skill
without buffer flush operation.
vim syntax hightlight and indent is inside.

### install
> ```bash
./configure ...
make install
```

### usage
> ```bash
<usage> = "dfalse" <src-with-df-suffix>
```

### demo
> src.df:
```false
{ simple echo }
"hello echo>"
[^$1_=~][,]#%
```
execute like 
```bash
dfalse src.df
hello echo>hehe
hehe
```
stack trace  error message
src.df
```false
{ simple echo }
"hello echo>"
[^$1_=][,#
```
again execute it
```bash
dfalse src.df
missing matched ]
3:8: from here
3:8: [^$1_=][
3:8:        ^
3:8: from here
3:8: [^$1_=][
3:8:        ^
interpret failed
pop:
function start
3:2: from here
3:2: [^
3:2:  ^
function end
hello echo>
```
