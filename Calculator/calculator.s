#
# Usage: ./calculator <op> <arg1> <arg2>
#

# Make `main` accessible outside of this module
.global main

# Start of the code section
.text

# int main(int argc, char argv[][])
main:
  # Function prologue
  enter $0, $0

  # Variable mappings:
  # op -> %r12
  # arg1 -> %r13
  # arg2 -> %r14
  movq 8(%rsi), %r12  # op = argv[1]
  movq 16(%rsi), %r13 # arg1 = argv[2]
  movq 24(%rsi), %r14 # arg2 = argv[3]
  

  # Hint: Convert 1st operand to long int
  movq %r13, %rdi
  call atol
#note - atol moves value into rax, so move it back
  movq %rax, %r13

  # Hint: Convert 2nd operand to long int
  movq %r14, %rdi
  call atol
  movq %rax, %r14

  # Hint: Copy the first char of op into an 8-bit register
  # i.e., op_char = op[0] - something like mov 0(%r12), ???

  mov 0(%r12), %al  


  cmp $'+, %al 
  je addition
  jne step2

step2:
  cmp $'-, %al
  je subtraction
  jne step3

step3:
  cmp $'*, %al
  je multiplication
  jne step4

step4:
  cmp $'/, %al
  je division
  jne step5

step5:
  je exit
 

  # if (op_char == '+') {
  #   ...
  # }
  # else if (op_char == '-') {
  #  ...
  # }
  # ...
  # else {
  #   // print error
  #   // return 1 from main
  # }

#error message display
exit:
movq $error, %rdi
mov $1, %al
call printf
leave 
ret

#error message display for when user tries to divide by 0
zero:
movq $divbyzero, %rdi
mov $1, %al
call printf
leave 
ret

addition:
  addq %r13, %r14
  movq $format, %rdi
  movq %r14, %rsi
  mov $0, %al
  call printf
  leave
  ret

subtraction:
  sub %r14, %r13
  movq $format, %rdi
  movq %r13, %rsi
  mov $0, %al
  call printf
  leave
  ret

multiplication:
  imulq %r13, %r14
  movq $format, %rdi
  movq %r14, %rsi
  mov $0, %al
  call printf
  leave
  ret

division:
  cmp $0, %r14
  je zero
  movq %r13, %rax
  movq %r14, %rbx
  movq %rbx, %rcx
  cqo
  idivq %rbx
  movq $format, %rdi
  movq %rax, %rsi
  mov $0, %al
  call printf
  leave
  ret

  # Function epilogue
  leave
  ret


# Start of the data section

.data


format: 
  .asciz "%ld\n"

error:
  .asciz "Unknown operation\n"
 
divbyzero:
  .asciz "Cannot divide by 0\n!"
