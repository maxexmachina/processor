in
pop dx
call sqrt

push fx
dmp
out
hlt

sqrt:
	push 1
	pop fx

	next:
		push fx
		push fx
		mul
		push dx
		sub
		abs
		push 1
		jbe stop

		push fx
		push fx
		mul
		push dx

		jb right
		push fx
		push fx
		mul
		push dx
		ja left

	stop:
		ret

	right:
		push fx
		push 2
		mul
		pop fx
		jmp next

	left:
		push fx
		push 2
		div
		pop fx
		jmp next
