in
pop dx
call sqrt

push ex 
dmp
out
hlt

sqrt:
    push 0
    pop fx
    push dx
    pop gx

	push fx
	push gx
	jbe next

    next:
        push fx
        push gx
        add
        push 2
        div
        pop hx

        push hx
        push hx
		mul
        push dx
        je end_loop

        push hx
        push hx
		mul
        push dx
        jb left

		push hx
		push 1
		sub
		pop gx

		next_check:
			push fx
			push gx
			jbe next

		push ex
		push ex
		mul
		push dx
		je stop

    decimal:
        push 0.1
        pop fx

        push 0
        pop gx

        decimal_next:
            push gx
            push 1
            jae stop

			while_next:
            push ex
            push ex
			mul
            push dx
            ja while_stop

            push fx
            push ex
            add
            pop ex
			jmp while_next

            while_stop:
                push ex
                push fx 
                sub
                push ex

                push fx
                push 10
                div
                pop fx
                jmp decimal_next

            stop:
                ret
        
        left:
            push hx
            push 1
            add
            pop fx
            push hx
            pop ex
			jmp next_check 

        end_loop:
            push hx
            pop ex

			push ex
			push ex
			mul
			push dx
			je stop

            jmp decimal
