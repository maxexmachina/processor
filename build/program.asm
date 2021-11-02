push 20
push 5
pop ax
pop [ax + 5]
push [10]
pop ax
push 40
pop bx
push 10
pop cx
push bx
push bx
mul
push 4
push ax
push cx
mul
mul
sub
out
hlt
