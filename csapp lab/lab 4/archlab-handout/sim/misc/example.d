
examples.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <sum_list>:
   0:	55                   	push   %rbp
   1:	48 89 e5             	mov    %rsp,%rbp
   4:	48 89 7d e8          	mov    %rdi,-0x18(%rbp)
   8:	48 c7 45 f8 00 00 00 	movq   $0x0,-0x8(%rbp)
   f:	00 
  10:	eb 17                	jmp    29 <sum_list+0x29>
  12:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
  16:	48 8b 00             	mov    (%rax),%rax
  19:	48 01 45 f8          	add    %rax,-0x8(%rbp)
  1d:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
  21:	48 8b 40 08          	mov    0x8(%rax),%rax
  25:	48 89 45 e8          	mov    %rax,-0x18(%rbp)
  29:	48 83 7d e8 00       	cmpq   $0x0,-0x18(%rbp)
  2e:	75 e2                	jne    12 <sum_list+0x12>
  30:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
  34:	5d                   	pop    %rbp
  35:	c3                   	retq   

0000000000000036 <rsum_list>:
  36:	55                   	push   %rbp
  37:	48 89 e5             	mov    %rsp,%rbp
  3a:	48 83 ec 20          	sub    $0x20,%rsp
  3e:	48 89 7d e8          	mov    %rdi,-0x18(%rbp)
  42:	48 83 7d e8 00       	cmpq   $0x0,-0x18(%rbp)
  47:	75 07                	jne    50 <rsum_list+0x1a>
  49:	b8 00 00 00 00       	mov    $0x0,%eax
  4e:	eb 2a                	jmp    7a <rsum_list+0x44>
  50:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
  54:	48 8b 00             	mov    (%rax),%rax
  57:	48 89 45 f0          	mov    %rax,-0x10(%rbp)
  5b:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
  5f:	48 8b 40 08          	mov    0x8(%rax),%rax
  63:	48 89 c7             	mov    %rax,%rdi
  66:	e8 00 00 00 00       	callq  6b <rsum_list+0x35>
  6b:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
  6f:	48 8b 55 f0          	mov    -0x10(%rbp),%rdx
  73:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
  77:	48 01 d0             	add    %rdx,%rax
  7a:	c9                   	leaveq 
  7b:	c3                   	retq   

000000000000007c <copy_block>:
  7c:	55                   	push   %rbp
  7d:	48 89 e5             	mov    %rsp,%rbp
  80:	48 89 7d e8          	mov    %rdi,-0x18(%rbp)
  84:	48 89 75 e0          	mov    %rsi,-0x20(%rbp)
  88:	48 89 55 d8          	mov    %rdx,-0x28(%rbp)
  8c:	48 c7 45 f0 00 00 00 	movq   $0x0,-0x10(%rbp)
  93:	00 
  94:	eb 33                	jmp    c9 <copy_block+0x4d>
  96:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
  9a:	48 8d 50 08          	lea    0x8(%rax),%rdx
  9e:	48 89 55 e8          	mov    %rdx,-0x18(%rbp)
  a2:	48 8b 00             	mov    (%rax),%rax
  a5:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
  a9:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
  ad:	48 8d 50 08          	lea    0x8(%rax),%rdx
  b1:	48 89 55 e0          	mov    %rdx,-0x20(%rbp)
  b5:	48 8b 55 f8          	mov    -0x8(%rbp),%rdx
  b9:	48 89 10             	mov    %rdx,(%rax)
  bc:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
  c0:	48 31 45 f0          	xor    %rax,-0x10(%rbp)
  c4:	48 83 6d d8 01       	subq   $0x1,-0x28(%rbp)
  c9:	48 83 7d d8 00       	cmpq   $0x0,-0x28(%rbp)
  ce:	7f c6                	jg     96 <copy_block+0x1a>
  d0:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
  d4:	5d                   	pop    %rbp
  d5:	c3                   	retq   
