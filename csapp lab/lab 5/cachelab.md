CSAPP书上第六章关于缓存。

### PartA

用C实现缓存模拟器。

~~正确的操作，ida反汇编一下csim-ref文件，看着伪代码写~~



参考模拟器采用以下命令行参数：

```

Usage: ./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>


-h: info可选的帮助标志，用于打印使用情况信息

-v: 显示跟踪信息的可选详细标志

-s <s>: 设置的索引位数（S = 2^s是设置的数量）

-E <E>: 关联性（每组行数）

-b <b>: 块位数（B = 2^b是块大小）

-t <tracefile>: Name of the valgrind trace to replay要重播的valgrind跟踪的名称
```

##### getopt

上面部分，用`getopt`函数实现

功能：解析命令行选项参数的，但是只能解析短选项: -d 100,不能解析长选项：--prefix

参数：

* argc：main()函数传递过来的参数的个数
* argv：main()函数传递过来的参数的字符串指针数组
* optstring：选项字符串，告知 getopt()可以处理哪个选项以及哪个选项需要参数

optstring详解：

char*optstring = “ab:c::”;

| 字符串           | 意义                      | 格式                     |
| ---------------- | ------------------------- | ------------------------ |
| 单个字符a        | 选项a没有参数             | -a即可，不加参数         |
| 单字符加冒号b:   | 选项b有且必须加参数       | -b 100或-b100,但-b=100错 |
| 单字符加2冒号c:: | 表示选项c可以有，也可以无 | -c200，其它格式错误      |

所以，根据题意，optstring应设置为`"s:E:b:t:vh"`，表明`s`、`E`、`b`、`t`四个参数是必须的，`v`、`h`是可选的。



optarg —— 指向当前选项参数的指针。

对于，`./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace`

当取出参数s时，optarg取出4



##### initCache

创建如下的cache

![image-20200730130159846](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200730130159846.png)

```
cache = (cache_t) malloc(sizeof(cache_set_t)*S);
```

`(cache_set_t)` ：`cache_line_t*`

![image-20200730130534347](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200730130534347.png)

这里初始化，cache每个组有这么一行



```
cache[i] = (cache_line_t*) malloc(sizeof(cache_line_t)*E);
```

为每一组分配E行

![image-20200730130607765](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200730130607765.png)



##### replayTrace

功能：读入文件，根据文件开始，判断内存类型，若"L"、"S",加载一次；"M"，加载两次

* "`"I"`表示指令加载
* `"L"`表示数据加载
* `"S"`表示数据存储
* `"M"`表示数据修改

指令加载不用高速缓存，，，

当为`"M"`（数据修改）时，读取数据，修改数据，需要两次访问内存操作，其余的都一次。



##### accessData

根据地址，在高速缓存中，寻找块，记录击中、未击中和逐出情况。在高速缓存满的情况下，找到使用次数最少的块进行替换，未满，加到空块出。

![image-20200730134437793](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200730134437793.png)

```
    mem_addr_t tag = addr >> (s + b);//标记
    mem_addr_t set = addr << (tag_size) >> (tag_size + b);//组索引
```

`set`先左移t位会消掉标记，然后右移`t+b`，消掉块偏移，最后剩下组索引

![image-20200730150514709](C:\Users\YCNN\AppData\Roaming\Typora\typora-user-images\image-20200730150514709.png)