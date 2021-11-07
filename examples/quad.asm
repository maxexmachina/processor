in
pop ax
in
pop bx
in
pop cx

call quad

push ax
out
push bx
out
push cx
out
hlt

quad:
	push ax
	push 0
	je a_zero

	push cx
	push 0
	je c_zero

	call discr
	
	push dx
	push 0
	jb neg_discr

	push dx
	push 0
	je zero_discr

	call sqrt

	push bx
	push -1
	mul
	push dx
	sub
	push 2
	push ax
	mul
	div

	push bx
	push -1
	mul
	push dx
	add
	push 2
	push ax
	mul
	div

	pop cx
	pop bx

	push 2
	pop ax
	ret

	neg_discr:
		push 0
		pop ax
		ret

	zero_discr:
		push bx
		push -1
		mul
		push 2
		push a
		mul
		div

		push 1
		pop ax
		pop bx
		ret
	
	a_zero:
		push bx
		push 0
		je b_zero

		push 1
		pop ax
	
		push bx
		pop dx
		push cx
		pop ex
		call lin

		push dx
		pop bx
		ret
	
	b_zero:
		push cx
		push 0
		je inf_roots

	inf_roots:
		push 3
		pop ax
		ret

	c_zero:
		push ax
		pop dx
		push bx
		pop ex
		call lin

		push dx
		push 0
		je c_lin_zero

		push 2
		pop ax
		push dx
		pop bx
		push 0
		pop cx
		ret

		c_lin_zero:
			push 1
			pop ax
			push dx
			pop bx
			ret

lin:
	push ex 
	push -1
	mul
	push dx
	div
	pop dx
	ret

discr:
	pop cx
	pop bx
	pop ax

	push bx
	push bx
	mul

	push 4 
 	push ax
	push cx
	mul
	mul
	sub

	pop dx 
	ret

sqrt:
	push 1
	pop fx

	next:
		push fx
		push fx
		mul
		push dx
		sub
		push 0.01
		jbe stop

		push fx
		push fx
		mul
		push dx

		ja right
		jmp left

	right:
		push fx
		push 2
		mul
		jmp next

	left:
		push fx
		push 2
		div
		jmp next
	
	stop:
		ret
