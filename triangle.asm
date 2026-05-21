;Triangle.asm
;Name : E V Sai Chathurya
;Roll No. : 2401CS84
;Declaration of Authorship : This program is my own work and has not been copied from any source
   
    ;Triangle numbers using recursion
    ;Computes triangle numbers for values 0..count - 1
    ;Stores results in result array

    ldc 0x1000 ; load code address
    a2sp       ; move A to SP
    adj -1     ; reserve space on stack 
    ldc result 
    stl 0      ; store result address on stack
    ldc count 
    ldnl 0     ; load value from memory address(A+offset)
    call main  ; call main function
    adj 1 
    HALT       ; stop program
; 
main: adj -3 
      stl 1 ; save return address
      stl 2 ; save parameter
      ldc 0 ; zero accumulator (load 0)
      stl 0 ; initialize loop counter
loop: adj -1 
      ldl 3 ; load value from stack (SP+offset) into A
      stl 0 
      ldl 1 
      call triangle
      adj 1 
      ldl 3 
      stnl 0; store B into memory address (A+offset)
      ldl 3 
      adc 1 ; add constant to A 
      stl 3 ; i++
      ldl 0 ; load counter 
      adc 1 
      stl 0 ; store counter
      ldl 0 
      ldl 2 
      sub 
      brlz loop ; branch if A < 0
      ldl 1     ; get return address 
      adj 3 
      return    ; return to caller
; 
triangle:adj -3 ; locals for recursion: prev values, temp
      stl 1 
      stl 2 
      ldc 1 
      shl 
      ldl 3 
      sub 
      brlz skip ; if condition met , jump to skip
      ldl 3 
      ldl 2 
      sub 
      stl 2 
skip: ldl 2 
      brz one ; if zero, base case - return 1
      ldl 3 
      adc -1 
      stl 0 
      adj -1 
      ldl 1 
      stl 0 
      ldl 3 
      adc -1 
      call triangle ; recursive call
      ldl 1 
      stl 0 
      stl 1 
      ldl 3 
      call triangle 
      adj 1 
      ldl 0 
      add 
      ldl 1 
      adj 3 
      return 
one:  ldc 1 ; base case result
      ldl 1 
      adj 3 
      return 
; 
count: data 10 
result: data 0 