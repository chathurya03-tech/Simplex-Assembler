;fibonacci.asm
;Name : E V Sai Chathurya
;Roll No. : 2401CS84
;Declaration of Authorship : This program is my own work and has not been copied from any source

        ldc 0x1000 ; set up stack
        a2sp
        adj -1
        call main  ; call main function
        adj 1
        ldc result ; A = address of result
        ldnl 0     ; A = fib(N)
        HALT       ; stop program, Expected: A = 55

main:
        adj -4
        stl 1 ; [SP+1] = return address

        ; counter = N - 1
        ldc N
        ldnl 0 ; A = N
        adc -1 ; A = N - 1
        stl 2  ; store counter

        ; a = 0
        ldc 0
        stl 3 ; [SP+3] = a

        ; b = 1
        ldc 1
        stl 4 ; [SP+4] = b

loop:
        ldl 2 ; A = counter
        brz done ; stop if counter = 0

        ; compute next fibonacci value
        ldl 4 ; load b
        ldl 3 ; load a
        add   ; a+b
        stl 4 ; new b
        stl 3 ; new a

        ; counter
        ldl 2 ; A = counter
        adc -1
        stl 2 ; counter--

        br loop ; repeat loop

done:
        ; store result
        ldl 4 ; load final fibonacci value
        ldc result 
        stnl 0 ; mem[result] = b

        ldl 1 ; load return address
        adj 4 ; restore stack
        return

N: data 10 ; input value
result: data 0 ; stores fib(N)