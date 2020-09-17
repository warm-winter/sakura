
1.概念上理解所有的内核保护机制


CISCN2017 babydriver.ppt   uaf写，bypass smep
pwn core.ppt               栈溢出，bypass kalsr和 canary。
rop_linux_kernel_pwn.pdf   全局变量越界，fn(), 栈转移+rop，bypass smep ，工程：kernel_rop
四种方法： 如果绕过smep，kalsr，smap
1. 堆风水实现任意地址写
2. ptmx覆盖结构体，rop+gadget
3. 阻塞copy_frome_user，userfaultd + ptmx覆盖结构体
4.cred 写入+ 阻塞userfaultd 

vdso
ret2dir



