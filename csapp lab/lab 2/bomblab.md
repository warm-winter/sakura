---
title: bomblab
date: 2020-07-24 04:05:11
tags: csapp

---

总共六个阶段

任务是，运行时，有六个阶段，每个阶段输入一个字符串，如果输入错误，炸弹爆炸。这次给的文件不全，所以，需要看汇编了解程序。

导出汇编代码：`unix> objdump -d > obj.txt`

### 第一阶段

```
  #main这块关于第一阶段的代码
  400e32:	e8 67 06 00 00       	callq  40149e <read_line>
  400e37:	48 89 c7             	mov    %rax,%rdi						#读入的放入rdi
  400e3a:	e8 a1 00 00 00       	callq  400ee0 <phase_1>
  400e3f:	e8 80 07 00 00       	callq  4015c4 <phase_defused>
  400e44:	bf a8 23 40 00       	mov    $0x4023a8,%edi
  400e49:	e8 c2 fc ff ff       	callq  400b10 <puts@plt>
```

读入字符串，字符串地址存放在`rdi`中，然后呼叫`phase_1`函数，phase_defused和put都是输出提示性字符串

每个阶段这块代码类似，字符串在`rdi`中。不再累赘。

````
#phase_1函数
0000000000400ee0 <phase_1>:
  400ee0:	48 83 ec 08          	sub    $0x8,%rsp
  400ee4:	be 00 24 40 00       	mov    $0x402400,%esi					#把0x402400放入esi
  400ee9:	e8 4a 04 00 00       	callq  401338 <strings_not_equal>
  400eee:	85 c0                	test   %eax,%eax						#执行and操作，如果两个相等，zf=1
  400ef0:	74 05                	je     400ef7 <phase_1+0x17>
  400ef2:	e8 43 05 00 00       	callq  40143a <explode_bomb>
  400ef7:	48 83 c4 08          	add    $0x8,%rsp
  400efb:	c3                   	retq   
````

将0x402400地址存入`esi`，然后调用`strings_not_equal`函数，对返回结果`eax`进行查看，等于0成功，否则，炸弹爆炸

```
#strings_not_equal函数
0000000000401338 <strings_not_equal>:										//相等，返回0，不相等，返回1
  401338:	41 54                	push   %r12
  40133a:	55                   	push   %rbp
  40133b:	53                   	push   %rbx
  40133c:	48 89 fb             	mov    %rdi,%rbx						#读入的
  40133f:	48 89 f5             	mov    %rsi,%rbp						#0x402400
  401342:	e8 d4 ff ff ff       	callq  40131b <string_length>			#把字符串放入rdi
  401347:	41 89 c4             	mov    %eax,%r12d         				#读入的长度放入r12d
  40134a:	48 89 ef             	mov    %rbp,%rdi
  40134d:	e8 c9 ff ff ff       	callq  40131b <string_length>
  401352:	ba 01 00 00 00       	mov    $0x1,%edx				
  401357:	41 39 c4             	cmp    %eax,%r12d						#0x402400的长度与读入的长度cmp
  40135a:	75 3f                	jne    40139b <strings_not_equal+0x63>	#如果不相等，eax=0x1,结束
  
																			#相等要返回长度，所以接下来要找到最前面0的位置(二分)
  40135c:	0f b6 03             	movzbl (%rbx),%eax						#如果相等，eax=读入地址里面的内容
																			#movzbl指令负责拷贝一个字节，并用0填充其目的操作数中的其余各位，这种扩展方式叫“零扩展”。
  40135f:	84 c0                	test   %al,%al							#如果al=0，eax=0，结束
  401361:	74 25                	je     401388 <strings_not_equal+0x50>	#如果al！=0
  401363:	3a 45 00             	cmp    0x0(%rbp),%al					#第一个字节相比				
  401366:	74 0a                	je     401372 <strings_not_equal+0x3a>	#相等，rbx、rbp+1
  401368:	eb 25                	jmp    40138f <strings_not_equal+0x57>	#不相等，eax=0x1，返回
  40136a:	3a 45 00             	cmp    0x0(%rbp),%al					#继续比较下一个字节(rbp+1)
  40136d:	0f 1f 00             	nopl   (%rax)							#nop 是什么都不做的意思（no-operation）。这里填入 nop 是为了让后面的函数对齐到 16 字节处
  401370:	75 24                	jne    401396 <strings_not_equal+0x5e>	#后面的字节不相等，eax=1，返回
  401372:	48 83 c3 01          	add    $0x1,%rbx						#相等，继续加
  401376:	48 83 c5 01          	add    $0x1,%rbp
  40137a:	0f b6 03             	movzbl (%rbx),%eax						#eax=下一个地址里面的内容
  40137d:	84 c0                	test   %al,%al							#继续比较al是否是0
  40137f:	75 e9                	jne    40136a <strings_not_equal+0x32>	#不是，回去
  401381:	ba 00 00 00 00       	mov    $0x0,%edx						#是，eax=0
  401386:	eb 13                	jmp    40139b <strings_not_equal+0x63>
  401388:	ba 00 00 00 00       	mov    $0x0,%edx
  40138d:	eb 0c                	jmp    40139b <strings_not_equal+0x63>
  40138f:	ba 01 00 00 00       	mov    $0x1,%edx
  401394:	eb 05                	jmp    40139b <strings_not_equal+0x63>
  401396:	ba 01 00 00 00       	mov    $0x1,%edx
  40139b:	89 d0                	mov    %edx,%eax
  40139d:	5b                   	pop    %rbx
  40139e:	5d                   	pop    %rbp
  40139f:	41 5c                	pop    %r12
  4013a1:	c3                   	retq   
```

通过分析可得，当`rdi`、`rsi`两个寄存器中地址对应的字符串内容，相等时，返回0，否则，返回1

综上：

当输入的字符串的内容和`0x402400`地址中的内容相等，即可通过。

![image-20200722183034318](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200722183034318.png)

Border relations with Canada have never been better.

### 第二阶段

```
0000000000400efc <phase_2>:
  400efc:	55                   	push   %rbp
  400efd:	53                   	push   %rbx
  400efe:	48 83 ec 28          	sub    $0x28,%rsp
  
  400f02:	48 89 e6             	mov    %rsp,%rsi					#当前栈顶备份rsi
  400f05:	e8 52 05 00 00       	callq  40145c <read_six_numbers>	#主要是判断是否输入了六个数
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)					#栈顶数据必须是1
  400f0e:	74 20                	je     400f30 <phase_2+0x34>		#如果相等，rbx=rsp+0x4；rbp=rsp+0x18，否则，爆炸
																		#rbp指向最后一个数字的位置,rbx指向rsp下面一个单元
  400f10:	e8 25 05 00 00       	callq  40143a <explode_bomb>
  400f15:	eb 19                	jmp    400f30 <phase_2+0x34>
  400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax				#eax初始化是rsp栈顶的数据
  400f1a:	01 c0                	add    %eax,%eax					#eax×2
  400f1c:	39 03                	cmp    %eax,(%rbx)					#eax和rbx的值比较
																		#(也就是下一个单元的数据和上一个单元比较)
  400f1e:	74 05                	je     400f25 <phase_2+0x29>		
																		#如果相等，rbx+0x4，否则，爆炸
  400f20:	e8 15 05 00 00       	callq  40143a <explode_bomb>
  400f25:	48 83 c3 04          	add    $0x4,%rbx					rbx和rbp比较，rbx和rbp如果不相等(没走完6个)，继续循环
  400f29:	48 39 eb             	cmp    %rbp,%rbx
  400f2c:	75 e9                	jne    400f17 <phase_2+0x1b>
  400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>		#rbx和rbp比较如果想等，结束
  400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx				#rbx(来判断的指针)，从rsp下面第一个数开始走			
  400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp		
  400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>
  400f3c:	48 83 c4 28          	add    $0x28,%rsp					#六个走完，就结束了
  400f40:	5b                   	pop    %rbx
  400f41:	5d                   	pop    %rbp
  400f42:	c3                   	retq   
```

所以，phase_2函数做的，就是将栈中的前六个单元的数据，相邻两个比较，如果2×上面单元地址 = 下面单元地址，才不会爆炸，又因为第一个是1（esp的值），六个，所以可以得到：

栈内情况如下

 ![image-20200722234727371](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200722234727371.png)



对于`read_six_numbers`函数，主要看一下几行

```
  40148a:	e8 61 f7 ff ff       	callq  400bf0 <__isoc99_sscanf@plt>		#输入h函数
  40148f:	83 f8 05             	cmp    $0x5,%eax					   #eax存放输入的参数个数，说明至少六个才行（eax返回6）
  401492:	7f 05                	jg     401499 <read_six_numbers+0x3d>	#JG： 大于转移指令。
  401494:	e8 a1 ff ff ff       	callq  40143a <explode_bomb>
  401499:	48 83 c4 18          	add    $0x18,%rsp						#小于0x5，顺利退出
  40149d:	c3                   	retq   
```

将参数压栈的操作

```
0000000000400bf0 <__isoc99_sscanf@plt>:
  400bf0:	ff 25 92 24 20 00    	jmpq   *0x202492(%rip)        # 603088 <_GLOBAL_OFFSET_TABLE_+0xa0>
  400bf6:	68 11 00 00 00       	pushq  $0x11
  400bfb:	e9 d0 fe ff ff       	jmpq   400ad0 <_init+0x10>		# jmpq 就是jmp 指令，q表示跳转到64位地址。
```

```
0000000000400ad0 <getenv@plt-0x10>:
  400ad0:	ff 35 1a 25 20 00    	pushq  0x20251a(%rip)        # 602ff0 <_GLOBAL_OFFSET_TABLE_+0x8>
  400ad6:	ff 25 1c 25 20 00    	jmpq   *0x20251c(%rip)        # 602ff8 <_GLOBAL_OFFSET_TABLE_+0x10>
  400adc:	0f 1f 40 00          	nopl   0x0(%rax)
```

应该是0x400ad0做的，，，（猜测）

1 2 4 8 16 32

### 第三阶段

```
0000000000400f43 <phase_3>:
  400f43:	48 83 ec 18          	sub    $0x18,%rsp						
  400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx					#rcx = rsp + 0xc
  400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx					#rdx = rsp + 0x8
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi					#esi = 0x4025cf
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax						#eax = 0x0
  400f5b:	e8 90 fc ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
  400f60:	83 f8 01             	cmp    $0x1,%eax						
  400f63:	7f 05                	jg     400f6a <phase_3+0x27>			#至少要输入两个参数
  400f65:	e8 d0 04 00 00       	callq  40143a <explode_bomb>
  400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)					#rsp + 0x8 与 0x7 做比较，需要小于
  400f6f:	77 3c                	ja     400fad <phase_3+0x6a>			#rsp + 0x8 大于 0x7 就爆炸了
  400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax					#eax = rsp + 0x8
  400f75:	ff 24 c5 70 24 40 00 	jmpq   *0x402470(,%rax,8)				#switch结构
  400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax						#eax = 0xcf 	207
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
  400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax						#eax = 0x2c3		rsp = eax - 0xc
  400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
  400f8a:	b8 00 01 00 00       	mov    $0x100,%eax						#eax = 0x100		256						
  400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>
  400f91:	b8 85 01 00 00       	mov    $0x185,%eax						#eax = 0x185	389
  400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
  400f98:	b8 ce 00 00 00       	mov    $0xce,%eax						#eax = 0xce		206
  400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
  400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax						#eax = 0x2aa	682
  400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
  400fa6:	b8 47 01 00 00       	mov    $0x147,%eax						#eax = 0x147	327
  400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
  400fad:	e8 88 04 00 00       	callq  40143a <explode_bomb>			#爆炸
  400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
  400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
  400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax					#比较eax 和 rsp + 0xc
  400fc2:	74 05                	je     400fc9 <phase_3+0x86>
  400fc4:	e8 71 04 00 00       	callq  40143a <explode_bomb>
  400fc9:	48 83 c4 18          	add    $0x18,%rsp
  400fcd:	c3                   	retq   
```

这个阶段，只需要对该代码有较好的理解即可，主要是对分支（switch语句）的理解。

对于sscanf函数，原型

```
int sscanf(const char* str,const char* format,...);
```

在这里，`esi`里面存放的是format的内容，`[esp+4]`、`[esp+0xc]`里面分别是第一第二个参数

![image-20200723015133883](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200723015133883.png)

![image-20200723020444125](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200723020444125.png)

根据format和这两条语句，可以知道，参数为2。

```
  400f60:	83 f8 01             	cmp    $0x1,%eax						
  400f63:	7f 05                	jg     400f6a <phase_3+0x27>			#至少要输入两个参数		#至少要输入两个参数
```

下面到了重点，也就是switch语句。

```
  400f75:	ff 24 c5 70 24 40 00 	jmpq   *0x402470(,%rax,8)				#switch结构
  400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax						#eax = 0xcf 	207
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
```

```
  400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax	
```

到内存中查看0x402470和0x402478

![image-20200723020928192](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200723020928192.png)

`0x402470`是当rax=0时，`jmpq   *0x402470(,%rax,8)	`调转到地址`0x402470`,当rax=1时，`jmpq   *0x402470(,%rax,8)	`调转到地址`0x400fb9`，一次类推，只要eax属于（0，0xx7），就有它对应的解

* 0	0xcf			207
* 1    0x137         311
* 2    0x2c3          707
* 3    0x100          256
* 4    0x185          389
* 5    0xce            206
* 6    0x2aa          682
* 7    0x147          327

共有八组解，都可以使用

另外，如果不想奋斗看汇编，其实，可以ida里面看伪代码，分分钟秒杀

![image-20200723021624745](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200723021624745.png)



### 第四阶段

读懂程序，就可以简单绕过了

```
000000000040100c <phase_4>:
  40100c:	48 83 ec 18          	sub    $0x18,%rsp						
  401010:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
  401015:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
  40101a:	be cf 25 40 00       	mov    $0x4025cf,%esi
  40101f:	b8 00 00 00 00       	mov    $0x0,%eax
  401024:	e8 c7 fb ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
  401029:	83 f8 02             	cmp    $0x2,%eax						#参数必须是两个
  40102c:	75 07                	jne    401035 <phase_4+0x29>
  40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)					#第一个参数必须小于等于0xe
  401033:	76 05                	jbe    40103a <phase_4+0x2e>			
  401035:	e8 00 04 00 00       	callq  40143a <explode_bomb>
  40103a:	ba 0e 00 00 00       	mov    $0xe,%edx						#edx = 0xe
  40103f:	be 00 00 00 00       	mov    $0x0,%esi						#esi = 0x0
  401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi					#edi = rsp+0x8
  401048:	e8 81 ff ff ff       	callq  400fce <func4>					#func4
  40104d:	85 c0                	test   %eax,%eax						#eax必须等于0
  40104f:	75 07                	jne    401058 <phase_4+0x4c>			
  401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)					#rsp + 0xc 要等于 0x0
  401056:	74 05                	je     40105d <phase_4+0x51>
  401058:	e8 dd 03 00 00       	callq  40143a <explode_bomb>
  40105d:	48 83 c4 18          	add    $0x18,%rsp
  401061:	c3                   	retq   
```

这个函数中，得到的消息有，参数必须是两个，其中，第一个不确定，在经过`fun4函数`后，`eax`返回值要是0，第二个参数必须是0。

在`fun4`之前，`edx = 0xe`、`esi = 0x0`、`edi = rsp+0x8`,是fun4的三个参数

```
0000000000400fce <func4>:
  400fce:	48 83 ec 08          	sub    $0x8,%rsp
  400fd2:	89 d0                	mov    %edx,%eax		#eax = edx = 0xe
  400fd4:	29 f0                	sub    %esi,%eax		#eax = eax - esi(0x0)
  400fd6:	89 c1                	mov    %eax,%ecx		#ecx = eax
  400fd8:	c1 e9 1f             	shr    $0x1f,%ecx		#ecx左移5位 => ecx = 0
  400fdb:	01 c8                	add    %ecx,%eax		#eax = eax + ecx
  400fdd:	d1 f8                	sar    %eax				#右移1位	eax = 7
  400fdf:	8d 0c 30             	lea    (%rax,%rsi,1),%ecx	#ecx = rax + rsi * 1 7
  400fe2:	39 f9                	cmp    %edi,%ecx			#edi(第一个参数)与ecx比较
  400fe4:	7e 0c                	jle    400ff2 <func4+0x24>	#如果edi <= ecx
  400fe6:	8d 51 ff             	lea    -0x1(%rcx),%edx		#edi > ecx，edx = rcx -1
  400fe9:	e8 e0 ff ff ff       	callq  400fce <func4>		#递归
  400fee:	01 c0                	add    %eax,%eax
  400ff0:	eb 15                	jmp    401007 <func4+0x39>
  400ff2:	b8 00 00 00 00       	mov    $0x0,%eax  		#如果ecx小于等于edi，eax = 0
  400ff7:	39 f9                	cmp    %edi,%ecx		#如果edi >= ecx#结束
  400ff9:	7d 0c                	jge    401007 <func4+0x39>
  400ffb:	8d 71 01             	lea    0x1(%rcx),%esi			#esi = rcx + 1
  400ffe:	e8 cb ff ff ff       	callq  400fce <func4>			#递归
  401003:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax	#eax = rax + rax + 1
  401007:	48 83 c4 08          	add    $0x8,%rsp				#结束
  40100b:	c3                   	retq   
```

在执行两次判断之前，`eax = 7`，接下来是将输入的第一个参数和`eax`，做比较，第一次如果小于等于，则得到想要的`eax=0`，再一次比较，如果大于等于，就结束了，所以，相等，即可顺利通过，也就是第一个参数是7.

所以，第四阶段 7 0



### 第五阶段

```
  401067:	48 89 fb             	mov    %rdi,%rbx				#rbx = rdi(读入的数据)
  40106a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax			#rax = canary
  401071:	00 00 
  401073:	48 89 44 24 18       	mov    %rax,0x18(%rsp)			#rsp+0x18 = rax = canary
  401078:	31 c0                	xor    %eax,%eax				#eax = 0
  40107a:	e8 9c 02 00 00       	callq  40131b <string_length>	#求长度
  40107f:	83 f8 06             	cmp    $0x6,%eax				
  401082:	74 4e                	je     4010d2 <phase_5+0x70>	#长度必须等于6
```

要求输入的字符串长度必须是6

```
  401089:	eb 47                	jmp    4010d2 <phase_5+0x70>	#eax = 0
  40108b:	0f b6 0c 03          	movzbl (%rbx,%rax,1),%ecx		#ecx = rbx + rax × 1
  40108f:	88 0c 24             	mov    %cl,(%rsp)				#(rsp) = cl(ecx低16位)ecx
  401092:	48 8b 14 24          	mov    (%rsp),%rdx				#rdx = (rsp) = cl(ecx低16位)
  401096:	83 e2 0f             	and    $0xf,%edx				#edx & 0xf(只要edx的低四位)
  401099:	0f b6 92 b0 24 40 00 	movzbl 0x4024b0(%rdx),%edx		#edx = rdx + 0x4024b0
								0x4024b0 <array.3449>:	"maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"
  4010a0:	88 54 04 10          	mov    %dl,0x10(%rsp,%rax,1)	#rsp + rax × 1 + 0x10 = dl(edx的低4位)
  4010a4:	48 83 c0 01          	add    $0x1,%rax				#rax + 1
  4010a8:	48 83 f8 06          	cmp    $0x6,%rax				#rax与6比较
  4010ac:	75 dd                	jne    40108b <phase_5+0x29>
```

进行了一顿操作，将`rax`（偏移从0到6） + `rbx`（输入的）的低4位，在字符串（`"maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"`）中找到对应的字符，放入`rsp + rax × 1 + 0x10`内地址对应的区域，一直放了6个字符

```
  4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi			#esi = 0x40245e "flyers"

  4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi			#rdi = (rsp + 0x10)
  4010bd:	e8 76 02 00 00       	callq  401338 <strings_not_equal> #rdi和rsi
  4010c2:	85 c0                	test   %eax,%eax				
  4010c4:	74 13                	je     4010d9 <phase_5+0x77>	#字符串必须一样
```

然后将那个地址的字符，放入`rdi`中，进行字符串比较，要求和内存单元`0x40245e`里面的一样，也就是字符串`"flyers"`

综上，所以，我们需要的偏移，在对应上面的字符串`maduiersnfotvby`上找，是9、15、14、5、6、7，也就是我们输入的六个可见字符，他们的低4位需要对应以上数，对应ascii码表，可以区数字部分的，0对应0x30，刚好。

![image-20200723212445995](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200723212445995.png)

所以，也就是9?>567



### 第六阶段

由于最后一个有点复杂，直接在ida里进行分析

主要分为五个小节

第一小节，判断输入数据的合法性

![image-20200724034848651](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200724034848651.png)

结论：需要输入的数据[1,6]，且两两互不相等



第二小节，对输入的数据进行小小变换操作

![image-20200724035019549](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200724035019549.png)

结论：原来输入的数据，变成了`7-原数据`

第三小节，将node链表根据输入重新排列到新的地方

![image-20200724035051039](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200724035051039.png)

因为语句`v6 = (int *)*((_QWORD *)v6 + 1); `中， `+1`通过转换加的是qword数据，根据下面图可以，`+1`后便是下一个node的地址。

因为每次取出输入变换的数据`v8`，取链上node，一直取，直到走了`v8`步才停下，赋给了`v17..`这里的数据

![image-20200724035203891](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200724035203891.png)

第五小节，判断链上的数据，是否递减排列

![image-20200724035304948](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200724035304948.png)

node上数据

* 1 0x14c
* 2 0x0a8
* 3 0x39c
* 4 0x2b3
* 5 0x1dd
* 6 0x1bb

所以，从大到小排列：3 4 5 6 1 2

7 - 原数据 = 现数据

原数据 = 7 - 现数据

结果：4 3 2 1 6 5



Border relations with Canada have never been better.

1 2 4 8 16 32

0 207

7 0

9?>567

4 3 2 1 6 5

![image-20200724040224131](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200724040224131.png)