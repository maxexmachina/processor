push 10
pop ax
push 20
pop bx
push 40
pop cx
push ax
push bx
push cx
call discr
push ax
out
hlt
discr:
pop cx
pop bx
pop ax
push bx
push bx
mul
call doshit
push dx
push ax
push cx
mul
mul
sub
pop ax
ret
doshit:
push 2
push 2
add
pop dx
ret
