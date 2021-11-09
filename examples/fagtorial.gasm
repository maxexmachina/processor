in

pop ax
push 1
pop bx

call fagtorial

push bx
out
hlt

fagtorial:
	push ax
	push 1
	jbe base
	
	push ax
	push bx
	mul
	pop bx

	push ax
	push 1
	sub
	pop ax

	call fagtorial
	ret

	base:
		ret
