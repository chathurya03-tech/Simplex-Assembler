; Triangle Numbers (Correct & Safe)

        ldc 0x1000
        a2sp
        adj -3

        ldc count
        ldnl 0
        stl 0          ; count

        ldc 0
        stl 1          ; i = 0

loop:
        ldl 1
        ldl 0
        sub
        brlz compute
        br done

compute:
        ; sum = 0
        ldc 0
        stl 2

        ; j = 1
        ldc 1
        stl 3

inner:
        ldl 3
        ldl 1
        sub
        brlz add_step
        brz add_step
        br store

add_step:
        ldl 2
        ldl 3
        add
        stl 2          ; sum += j

        ldl 3
        adc 1
        stl 3          ; j++

        br inner

store:
        ; store result[i]
        ldc result
        ldl 1
        add            ; address = result + i

        ldl 2          ; value = sum
        stnl 0

        ; i++
        ldl 1
        adc 1
        stl 1

        br loop

done:
        HALT

; Data
count:  data 10
result: data 0