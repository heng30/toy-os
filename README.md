![0.png](./screenshot/0.png)

这是一个根据[Linux操作系统-构建自己的内核](https://www.bilibili.com/video/BV1VJ41157wq?spm_id_from=333.788.videopod.episodes&vd_source=da23da82658adda9cbdfd045a9e6daf7)课程的学习项目。

### 实现的功能
- 运行在保护模式下，不支持分页功能
- 处理鼠标和键盘输入
- 简单的定时器
- 简单的内存管理
- 简单的图层管理，保证图层及时刷新
- 简单的任务调度
- 支持系统调用
- 类似于fat32的只读文件系统
- 一个输入框，可以输入和删除字符
- 一个简易的终端，支持：`cls, ls, cat, mem, free` 命令
- 支持运行外部命令, 外部命令运行在特权级3, 拥有自己的代码段和数据段. 并且内核能够处理外部程序的异常
- 快捷键`ctrl+k`结束正在运行的外部命令（有概率出现bug, 目前没有找到原因）
- 快捷键`ctrl+q`关闭焦点窗口
- 快捷键`ctrl+tab`切换焦点窗口

### 如何使用该项目？
- 编译软盘镜像：`make`
- 启动`virtualbox`，创建一个虚拟机，虚拟机以软盘启动。软盘加载`build/disk.img`作为启动镜像.

![1.png](./screenshot/1.png)

![1.png](./screenshot/2.png)

### 构建工具
- gcc: `gcc (GCC) 13.3.0`
- objconv: `Object file converter version 2.54 for x86 and x86-64 platforms`
- nasm: `NASM version 2.16.03 compiled on Apr 17 2024`

### 注意事项
- 无法使用函数指针，会运行错误

- 整个结构体赋值会出问题，导致整个程序异常退出。应该是objconv反汇编出问题。
```
struct Foo {
    int a;
    int b;
}

Foo a = { .a = 0, .b = 0 };
Foo b = a; // 此处会出问题，可能导致内核在刚启动就异常退出，而不是代码运行到此处
```

- 条件分支语句太长可能会出异常, 如`if/else if/else`和`swich`语句

- 不要把变量的指针作为获取函数内部状态的参数，反汇编的代码出问题
```
void foo(int *flag) {
    *flag = 1;
}

int flag = 0;
foo(&flag);
assert(flag == 1); // 此处会出错，flag的值依然为0
```

- 关中断和开中断的使用?
    - 在中断函数
    ```
        int eflags = io_load_eflags();
        io_cli(); // 暂时停止接收中断信号
        // 中断代码
        io_store_eflags(eflags); // 恢复接收中断信号
    ```

    - 在非中断函数
    ````
        io_cli();
        // 代码
        io_sti();
    ````

### 参考
- [30天自制操作系统](https://github.com/oddman2017/haribote)
- [Linux操作系统-构建自己的内核](https://www.bilibili.com/video/BV1VJ41157wq?spm_id_from=333.788.videopod.episodes&vd_source=da23da82658adda9cbdfd045a9e6daf7)
- [课程代码github](https://github.com/wycl16514)
- [BookOS](https://github.com/hzcx998/BookOS)
