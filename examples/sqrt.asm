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

    next:
        push fx
        push gx
        add
        push 2
        div
        pop hx

        push hx
        push hx
        push dx
        je end_loop

        end_loop:
            push hx
            pop ex
            jmp decimal

        push hx
        push hx
        push dx
        jb left

        push hx
        push 1
        sub
        pop gx

        push fx
        push gx
        jbe next

    decimal:
        hlt
        push 0.1
        pop fx

        push 0
        pop gx

        decimal_next:
            push gx
            push 3
            jae stop

            push ex
            push ex
            push dx
            ja while_stop

            push fx
            push ex
            add
            pop ex

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
