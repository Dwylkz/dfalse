" Author: dwylkz
" Email: xellosgeteternal@gmail.com
" Description: dfalse hightlight indent

sy match Operator /[-+*\/_=&|~\$%\\@^.,]/
sy match Constant /[0-9]\+\|'./
sy match Identifier /[a-z]/

sy region String start=/"/hs=s+1 end=/"/he=e-1 skip=/\\"/
sy region Ignore fold start=/{/ end=/}/ 
sy region Ignore transparent fold start=/\[/ end=/\]/
