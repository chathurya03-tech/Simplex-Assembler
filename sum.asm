;sum.asm
;Name : E V Sai Chathurya
;Roll No. : 2401CS84
;Declaration of Authorship : This program is my own work and has not been copied from any source

;Sum of elements of the array
        ldc 0x1000 ; initialize stack
        a2sp
        adj -1
        call main  ; call main function    
        adj 1
        HALT       ; stop program

main:
        adj -3 ; create stack frame
        stl 1  ; save return address

        ldc 0
        stl 2  ; i = 0

        ldc 0
        stl 3  ; sum = 0

loop:
        ; Exit when i >= N
        ldc N
        ldnl 0 ; A = N
        stl 0 
        ldl 2  ; A = i
        ldl 0 
        sub    ; A = i - N
        brlz loop_body ; continue if i < N
        br loop_done

loop_body:
        ; Load array[i]
        ldc array
        ldl 2  ; B = base, A = i
        add    ; A = &array[i]
        ldnl 0 ; A = array[i]

        ; sum += array[i]
        ldl 3 ; B = array[i],  A = sum
        add   ; A = sum + array[i]
        stl 3 ; store new sum

        ; i++
        ldl 2
        adc 1
        stl 2
        br  loop ; repeat loop

loop_done:
        ; Store sum into result
        ldl 3      ; A = sum  (value first) 
        ldc result ; B = sum,  A = &result
        stnl 0     ; mem[result] = sum  

        ldl 1 ; load return address
        adj 3 ; restore stack
        return

; Data
N:      data    5 ; no. of elements
array:  data    10
        data    20
        data    30
        data    40
        data    50 ; array values
result: data    0 ; stores final sum of elements of the array