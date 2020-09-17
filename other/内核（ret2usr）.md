适合人群：内核基础为0

## 知识学习

基础知识：

https://ctf-wiki.github.io/ctf-wiki/pwn/linux/kernel/basic_knowledge-zh/

https://xz.aliyun.com/t/7625#toc-10

#### copy_from_user

copy_from_user(void *to, const void __user *from, unsigned long n)

（1）to：将数据拷贝到**内核地址** 

（2）from：需要拷贝数据的**用户地址**

（3）n：拷贝数据的长度（字节）

> 也就是将form用户地址中的数据拷贝到to内核地址中去，拷贝长度是n

#### cpio解压和打包

解压：cpio -idvm < ../initramfs.cpio
打包：find . | cpio -H newc -o > ../initramfs.cpio

### 环境搭建

调试，`startvm.sh`末尾加上

```
-gdb tcp::1234 -S
```

【注：上一行后面还要加上' \ '，不然的话，远程调试可能端口没开，连不上】

![image-20200903111140398](./内核（ret2usr）/image-20200903111140398.png)



## 实践

#### ret2usr（level1）

说明：参考`Linux Kernel Pwn 初探`,主要加上具体的一些细节

##### 查找prepare_kernel_cred和commit_creds的地址

	 $ grep prepare_kernel_cred  /proc/kallsyms 
	 $ grep commit_creds  /proc/kallsyms 
![image-20200903105056015](./内核（ret2usr）/image-20200903105056015.png)

但是直接执行，地址都是0x0，需要root权限。

![image-20200903105223481](./内核（ret2usr）/image-20200903105223481.png)

方法：修改suid

（1）创建一个文件夹

```
$ mkdir myimage
$ cd myimage
```

（2）解压initramfs.cpio文件

```
cpio -idvm < ../initramfs.cpio
```

（3）修改suid

进入etc/init.d/rcS，将1000修改为0

```
level1/myimage$ gedit etc/init.d/rcS
```

![image-20200903110009863](./内核（ret2usr）/image-20200903110009863.png)

（4）重新打包

```
level1/myimage$ find . | cpio -H newc -o > ../initramfs.cpio
level1/myimage$ cd ..
```

（5）再次执行两条命令即可

![image-20200903110203208](./内核（ret2usr）/image-20200903110203208.png)

已经是root权限了

![image-20200903110251793](./内核（ret2usr）/image-20200903110251793.png)

`prepare_kernel_cred`的地址为`0xffffffff810b9d80`

`commit_creds`的地址为`0xffffffff810b99d0`

>疑问解答：此时已经获得root权限了，不就可以直接cat /flag了？
>
>实际题目中，不会把真的flag放文件里给你，而是在远程环境了。但是本地的地址和远程的地址是一样的，所以，可以通过这种方法得到本地的用户权限，找到prepare_kernel_cred和commit_creds的地址，但是没有办法直接获得flag。

##### 查找基地址

以root权限运行，参考上面（修改etc/init.d/rcS）

```
/home/pwn # cat /proc/modules 
baby 16384 0 - Live 0xffffffffc0002000 (POE)
```

##### 调试内核

（1）编辑startvm.sh，端口可以修改

0x7ffd958d7e08

![image-20200905004851358](./内核（ret2usr）/image-20200905004851358.png)

（2）执行`./startvm.sh`

（3）打开新窗口，在`level1`目录下执行`gdb exp`

（4）远程连接，`target remote :1234`

（5）下断点（ida里面的地址+基地址），继续执行

![image-20200905103517562](./内核（ret2usr）/image-20200905103517562.png)

![image-20200905103116021](./内核（ret2usr）/image-20200905103116021.png)

（6）在原来的窗口，执行exp

![image-20200905103146690](./内核（ret2usr）/image-20200905103146690.png)

（7）接着，就可以正常调试了

##### 执行exp步骤

（1）编译exp.c

```
$ gcc exp.c -o exp -w -static -fPIC
```

（2）将initramfs.cpio解压到myimage文件夹

（3）将exp移入myimage文件夹下

```
cp exp myimage/
```

（4）重新打包，执行./startvm.sh

exp已在/目录下了

![image-20200903111831663](./内核（ret2usr）/image-20200903111831663.png)

##### 疑惑的0x88和0x80

一开始，说将`0x100`的用户数据拷贝到内核栈上，高度只有`0x88`，后面又说实际上缓冲区距离`rbp`是`0x80`，有点迷。

一开始的0x88，指的是初始化的时候。

但是程序初始化的时候，有个压栈操作，所以少减了一个8

![image-20200903112746033](./内核（ret2usr）/image-20200903112746033.png)

##### 程序执行流程

init_module是内核加载模块的时候调用的

开始的时候（内核加载模块）：调用init_module

中间的时候：

我们调用ioctl调用了sub_0函数

ioctl的参数就是sub_0的参数

![image-20200905120014834](./内核（ret2usr）/image-20200905120014834.png)

` return (signed int)copy_from_user(&v4, v2, 256LL);`

我们通过buf覆盖栈上的返回地址，执行v2中copy过去的templine函数，获得shell

结束的时候：cleanup_module



