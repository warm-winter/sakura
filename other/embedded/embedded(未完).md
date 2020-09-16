## embedded

### 反汇编

mips架构的题，ida没法反编译成伪代码，用反编译mips的工具`ghidra`，安装包在文件夹里。windows下运行`ghidraRun.bat`,linux下执行`ghidraRun`,emm,需要有jdk的环境。



### 传文件到qemu

将linux下的embedded_heap文件传入qemu：

linux：sudo ifconfig tap0 12.0.0.2

qemu：ifconfig eth0 12.0.0.1 

遇到设备不存在，ifconfig，在开头看网卡名字



ping通

linux：python -m SimpleHTTPServer

qemu：wget  12.0.0.2:8000/embedded_heap

![image-20200831110650474](C:\Users\YCNN\Desktop\embedded\embedded_img\image-20200831110650474.png)

### 启动

将两个so.0文件放到/lib文件下（注意：是根目录，，，）

```
$ mv libuClibc-0.9.33.2.so libc.so.0
$ mv ld-uClibc-0.9.33.2.so ld-uClibc.so.0
```

![image-20200831191612015](C:\Users\YCNN\Desktop\embedded\embedded_img\image-20200831191612015.png)

qemu-mips  -L ./ ./embedded_heap

也可以直接`./embedded_heap`

【可以使用的原因可能是因为binutils的存在用编译器自动转换了】

**注意**

还要修改lib文件下链接文件的权限

![image-20200831191636914](C:\Users\YCNN\Desktop\embedded\embedded_img\image-20200831191636914.png)





### 参考

https://e3pem.github.io/2019/08/26/0ctf-2019/embedded_heap/

https://zybuluo.com/H4l0/note/1633971



---

### 交叉编译环境

[交叉编译](https://baike.baidu.com/item/交叉编译/10916911)（cross-compilation）是指，在某个主机平台上（比如PC上）用[交叉编译器](https://baike.baidu.com/item/交叉编译器/5125452)编译出可在其他平台上（比如ARM上）运行的代码的过程。