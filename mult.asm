; multiply_6_7.asm
; computes 6 * 7 using repeated addition

        ldc 0x1000
        a2sp
        adj -1
        call main
        adj 1
        ldc result
        ldnl 0      ; A = 42
        HALT

main:
        adj -4
        stl 1       ; save return address

        ; multiplicand = 6
        ldc 6
        stl 2       ; [SP+2] = multiplicand

        ; multiplier = 7 (count)
        ldc 7
        stl 3       ; [SP+3] = count

        ; result = 0
        ldc 0
        stl 4       ; [SP+4] = result

loop:
        ldl 3       ; load count
        brz done    ; if count == 0 → stop

        ; result = result + multiplicand
        ldl 4       ; A = result
        ldl 2       ; B := result, A := multiplicand
        add         ; A = result + multiplicand
        stl 4       ; store result

        ; count--
        ldl 3
        adc -1
        stl 3

        br loop

done:
        ; store result in memory
        ldl 4
        ldc result
        stnl 0

        ldl 1
        adj 4
        return

result: data 0