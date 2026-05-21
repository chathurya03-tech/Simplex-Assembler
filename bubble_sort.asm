;bubble_sort.asm
;Name : E V Sai Chathurya
;Roll No. : 2401CS84
;Declaration of Authorship : This program is my own work and has not been copied from any source
      
        ldc 0x1000
        a2sp ; initialize stack pointer
        adj -1
        call sort ; call sort function
        adj 1
        HALT ; stop program

sort:
        adj -7 ; create stack frame
        stl 1 ; save return address
        ldc 0 ; i = 0 (outer loop counter)
        stl 2 

;outer loop
outer:
        ; Exit when i >= N-1.
        ; Compute i - (N-1); exit if >= 0.

        ldc N
        ldnl 0 ; load N
        adc -1 ; compute N-1
        stl 0
        ldl 2 
        ldl 0 ; B = i, A = N-1
        sub ; A = i - (N-1)
        brlz outer_body  ; continue if i < N-1 
        br sort_done

outer_body:
        ldc 0
        stl 3 ; j = 0 (inner loop counter)

; inner loop
inner:
        ; limit = N-1-i.  Exit when j >= limit.
        ; Check: j - limit = j + i - (N-1)
        ; If >= 0 exit.

        ldc N
        ldnl 0 
        adc -1
        stl 0 
        ldl 2 
        ldl 0 
        sub 
        ldl 3 
        add ; check j < (N-1-i)  
        brlz inner_body  
        br next_outer ; go to next outer iteration

inner_body:
        ldc array
        ldl 3 
        add 
        stl 5 ; store address of array[j]

        ldc array
        ldl 3
        adc 1 ; A = j+1
        add ; A = &array[j+1]
        stl 6 ; store address of array[j+1]

        ldl 5 
        ldnl 0 
        stl 4 ; save array[j]

        ; Load array[j+1]
        ldl 6 ; A = &array[j+1]
        ldnl 0 ; A = array[j+1]

        ldl 4 ; B := array[j+1],  A := array[j]
        sub ; A = B - A = array[j+1] - array[j]

        brlz do_swap ; swap if array[j] > array[j+1]
        br no_swap

do_swap:
        ldl 6 
        ldnl 0 
        ldl 5 
        stnl 0 ; array[j] = array[j+1]  

        ldl 4 
        ldl 6 
        stnl 0 ; array[j+1] = array[j]  

no_swap:
        ldl 3
        adc 1 ; j++
        stl 3
        br inner

next_outer:
        ldl 2
        adc 1 ; i++
        stl 2
        br outer

sort_done:
        ldl 1 ; A = return address
        adj 7 ; restore stack
        return

; Data segment
N: data 8 ; no. of elements
array:  data 64
        data 25
        data 12
        data 87
        data 3
        data 45
        data 78
        data 19 ; unsorted array