;factorial.asm
;Name : E V Sai Chathurya
;Roll No. : 2401CS84
;Declaration of Authorship : This program is my own work and has not been copied from any source

        ldc 0x1000 ; initialise stack pointer
        a2sp
        adj -1
        call main
        adj 1
        ldc result ; A = address of result
        ldnl 0     ; A = N!
        HALT       ; Expected: A = 120

main:
        adj -5
        stl 1 ; [SP+1] = return address

        ; i = 2  (start multiplying from 2; result already = 1)
        ldc 2
        stl 2 ; [SP+2] = i

        ; result = 1
        ldc 1
        stl 3 ; [SP+3] = result

; outer loop: for i = 2 to N 
outer:
        ; exit when i > N, i.e. when i - N > 0
        ; compute i - N; if > 0, done
        ldc     N
        ldnl    0               ; A = N
        stl     0               ; slot0 = N  (tmp)
        ldl     2               ; B := N, A := i
        ldl     0               ; B := i, A := N
        sub                     ; A = i - N
        brlz    do_mul          ; i < N  -> multiply
        brz     do_mul          ; i == N -> multiply (do last step)
        br      outer_done      ; i > N  -> finished

do_mul:
        ; multiply: result = result * i 
        ; inner loop: sum = 0; count = i; while count>0: sum+=result; count--

        ; sum = 0
        ldc     0
        stl     5               ; [SP+5] = sum

        ; count = i
        ldl     2               ; A = i
        stl     4               ; [SP+4] = count

inner:
        ldl     4               ; A = count
        brz     mul_done        ; count == 0 -> done

        ; sum += result
        ldl     5               ; A = sum
        ldl     3               ; B := sum, A := result
        add                     ; A = sum + result
        stl     5               ; sum = sum + result

        ; count--
        ldl     4
        adc     -1
        stl     4               ; count--

        br      inner

mul_done:
        ; result = sum
        ldl     5               ; A = sum  (= old result * i)
        stl     3               ; result = sum

        ; i++
        ldl     2
        adc     1
        stl     2               ; i++

        br      outer

outer_done:
        ; store final result into memory
        ldl     3               ; A = result  (load value first)
        ldc     result          ; B := result, A := &result
        stnl    0               ; mem[result] = N!

        ldl     1               ; A = return address
        adj     5
        return

N:      data    3
result: data    0