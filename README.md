# DoggyOS

It's a tiny operation system on floppy for PC. It's the Advanced OS course project done in my first year graduate in 2008 by 2 guys.

##

压缩包包括DoggyOS源码（doggy）和报告（DoggyOS）。

doggy中包含完整软盘镜像DOGGY.IMG，一些辅助运行的脚本，可在gcc下编译的源代码和makefile（gcc-3.3.6下通过）。

代码基于于渊的《自己动手写操作系统》开发，并且包含其部分源码。

##

运行OS的方法有多种，可如下：
1. 直接把DOGGY.IMG刻录到软盘上（不能把DOGGY.IMG当成文件拷到软盘中，复制包括软盘引导扇区），从软盘启动运行，CPU需为X86体系架构。
2. 用模拟器或虚拟机运行。

- 安装qemu，修改qemu_doggy.bat中qemu的目录，双击运行。推荐。
- 安装VmWare，VirtualPC，加载DOGGY.IMG运行。
- 安装Bochs（Bochs-2.3.5），双击bochsrc.bxrc运行。内存初始化较慢，需等一段时间。
- ...

编译内核，需到linux下，安装好gcc（gcc-3.3.6）和nasm，在doggy目录下输入make image编译并复制到镜像DOGGY.IMG中。

##

运行Doggy之后，Alt+F1，Alt+F2，Alt+F3在TTY1，TTY2，TTY3下切换，每个TTY可登录一个用户。目前内置的用户名有root，doggy，beggar。登录后可运行ls，ll，ps，kill，lu，who，quit，clean这些Shell内置命令，也可运行测试可执行程序pa，pb，pc，pd。用户登录之后输入命令按回车就可以了。

## 
应用程序开发：
Doggy提供一些API支持应用程序开发，包括系统调用和一些通用函数，基中系统调用主要包括get_ticks，write，exec，_exit，kill，get_all_files，malloc，free。理论上支持所有用上面API实现的用户程序，gcc编译，即可在Doggy上运行。具体方法可参数可执行程序pa，pb，pc，pd的开发。

2008-01-23 01:21
