.section .head
mov $stack_top, %rsp
call kernel_main
ret

.section .stack
.align 16
stack_bottom:
.skip 0x4000 # 16 KiB
stack_top:

