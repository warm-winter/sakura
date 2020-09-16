分三个阶段

* PartA：编写Y86-64程序（熟悉Y86-64工具）
* PartB：使用新的指令扩展SEQ模拟器
* PartC：优化Y86-64基准测试程序和处理器设计

看书第四（PartA、PartB）、五章（PartC）

### PartA

#### sum.ys

编写代码：单链表值的求和

代码模板

```
		.pos 0					#设置初始地址
		irmovq stack,%rsp		#初始栈
		call main				#main函数
		halt					#结束
		
		.align 8				#8字节对齐
		...定义全局变量
		
main:							#main函数
		...设置函数参数
		call 功能函数名			 #调用
		ret						#返回
		
功能函数名:
		...主要编写的函数体
	
		.pos 0x200
stack:
```

全局变量，文件（writeup.pdf）已经给出了

```
		.align 8				#8字节对齐
ele1:							#初始化的全局性变量
		.quad 0x00a
		.quad ele2
ele2:	
		.quad 0x0b0
		.quad ele3
ele3:
		.quad 0xc00
		.quad 0
```

sum.ys，只有一个参数，是代表的初始地址

所以只要把代表起始地址的标号ele1赋给第一个寄存器`%rdi`即可

```
irmovq ele1,%rdi		#设置参数
```

函数体

```
sum_list:	
		xorq %rax,%rax			#val = 0
		andq %rdi,%rdi			#&后进行标志判断
		je end					#结束
loop:	
		mrmovq (%rdi),%rcx		#ls->val
		addq %rcx,%rax			#val += ls->val
		irmovq $8,%rsi			
		addq %rsi,%rdi			#ls->next;
		mrmovq (%rdi),%rdi		#ls = ls->next;
		andq %rdi,%rdi
		jne loop
		
end:	ret
```

Y86-64没有cmp，比较通过and指令

编译，输出

![image-20200727171703526](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200727171703526.png)



#### rsum.ys

用递归实现sum.ys

主要注意返回值，因为返回值约定存在`%rax`里面，所以最外层调用的时候，将临时作为返回值的寄存器压栈，`ret`前出栈。

```
rsum_list:	
		pushq %rcx
		andq %rdi,%rdi
		je end 				
		mrmovq (%rdi),%rcx	
		irmovq $8,%rbx		
		addq %rbx,%rdi
		mrmovq (%rdi),%rdi	
		call rsum_list
		addq %rcx,%rax
end:	
		popq %rcx
		ret
```

![image-20200727172118666](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200727172118666.png)



#### copy.ys

功能：地址传送，src地址的内容放到dest里面去

两块地址里面的内容以及给出

```
		.align 8
#Source block 
src:
		.quad 0x00a
		.quad 0x0b0
		.quad 0xc00

#Destination block 
dest:
		.quad 0x111
		.quad 0x222
		.quad 0x333
```

`long copy_block(long *src, long *dest, long len)`

三个参数，分别是`src`和`dest`的地址，还有赋值的长度

```
		irmovq src,%rdi
		irmovq dest,%rsi
		irmovq $3,%rbx
```

主要的函数体部分，不能直接将寄存器的值和立即数相加，用传送指令先将立即数给临时寄存器，再add

```
copy_block:	
		xorq %rax,%rax				#long result = 0;
loop:	andq %rbx,%rbx				#while (len > 0) {
		jle end
		mrmovq (%rdi),%rdx			#val = *src
		irmovq $8,%rcx				#src++
		addq %rcx,%rdi
		rmmovq %rdx,(%rsi)			#*dest = val
		addq %rcx,%rsi				#dest++
		xorq %rdx,%rax				#result ^= val;
		irmovq $1,%rcx				#len--;
		subq %rcx,%rbx
		jmp loop
		
end:	ret
```



### PartB

实现iaddq指令，功能是另一个常数值和一个寄存器相加

格式如下：

![image-20200728224749377](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200728224749377.png)

![image-20200729000258760](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200729000258760.png)

然后就可以编辑`seq-full.hcl`文件



#### 取值阶段

`Fetch Stage `

当然是有效指令，希望实现的新增的有效指令

```
#是否是有效指令,新增IIADDQ在末尾
bool instr_valid = icode in 
	{ INOP, IHALT, IRRMOVQ, IIRMOVQ, IRMMOVQ, IMRMOVQ,
	       IOPQ, IJXX, ICALL, IRET, IPUSHQ, IPOPQ,IIADDQ };
```

用到了rB寄存器

```
# Does fetched instruction require a regid byte? 获取的指令需要regid字节吗？
bool need_regids =
	icode in { IRRMOVQ, IOPQ, IPUSHQ, IPOPQ, 
		     IIRMOVQ, IRMMOVQ, IMRMOVQ,IIADDQ };
```

用到了常数，值取出来在valC里面

```
# Does fetched instruction require a constant word? 获取指令需要一个常量字吗？
bool need_valC =
	icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL,IIADDQ };
```



#### 译码阶段

`Decode Stage`

只有B源，且rB用作B源

```
##  What register should be used as the B source? 什么寄存器应该用作B源？
word srcB = [
	icode in { IOPQ, IRMMOVQ, IMRMOVQ,IIADDQ  } : rB;
	icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't need register
];
```

结果放在rB中

```
## What register should be used as the E destination? 什么寄存器应该用作E目的地？
word dstE = [
	icode in { IRRMOVQ } && Cnd : rB;
	icode in { IIRMOVQ, IOPQ,IIADDQ} : rB;
	icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't write any register
];
```



#### 执行阶段

`Execute Stage`

valE = valC + valB

所以输入A是valC，输入B是valB

```
## Select input A to ALU 选择输入A到ALU
word aluA = [
	icode in { IRRMOVQ, IOPQ } : valA;
	icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ,IIADDQ } : valC;
	icode in { ICALL, IPUSHQ } : -8;
	icode in { IRET, IPOPQ } : 8;
	# Other instructions don't need ALU
];
```

```
## Select input B to ALU 选择输入B到ALU
word aluB = [
	icode in { IRMMOVQ, IMRMOVQ, IOPQ, ICALL, 
		      IPUSHQ, IRET, IPOPQ,IIADDQ } : valB;
	icode in { IRRMOVQ, IIRMOVQ } : 0;
	# Other instructions don't need ALU
];
```

```
## Should the condition codes be updated? 是否应更新条件代码？
bool set_cc = icode in { IOPQ,IIADDQ };
```

`1.make asumi.yo`

![image-20200728235432251](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200728235432251.png)

`2.make VERSION=full`

![image-20200728235538899](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200728235538899.png)

`3../ssim -t ../y86-code/asumi.yo`

在TTY模式下，将结果与ISA模拟进行比较

![image-20200728235631467](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200728235631467.png)

`4.winter@winter-ubuntu16:~/csapp/archlab-handout/sim/seq$ cd ../y86-code; make testssim`

![image-20200728235755136](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200728235755136.png)

5.winter@winter-ubuntu16:~/csapp/archlab-handout/sim/y86-code$ cd ../ptest; make SIM=../seq/ssim

![image-20200728235841036](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200728235841036.png)

`6.winter@winter-ubuntu16:~/csapp/archlab-handout/sim/ptest$ cd ../ptest; make SIM=../seq/ssim TFLAGS=-i`

![image-20200728235917244](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200728235917244.png)

编译时，可能遇到tcl not found；tk not found；libtcl not found；libtk not found问题 =>  将Make里面的注释“Comment this out if you don’t have Tcl/Tk.”下的行注释或删除

一个非常简单的part，，，

### PartC

