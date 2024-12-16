This is a project for learning os knowledge.

### 注意事项
- 整个结构体赋值会出问题，导致整个程序异常退出。应该是objconv反汇编出问题。
```
struct Foo {
    int a;
    int b;
}

Foo a = { .a = 0, .b = 0 };
Foo b = a; // 此处会出问题，可能导致内核在刚启动就异常退出，而不是代码运行到此处
```

### Build tool version
- `gcc (GCC) 13.3.0`
- objconv: `Object file converter version 2.54 for x86 and x86-64 platforms`

### Reference
- [Linux操作系统-构建自己的内核](https://www.bilibili.com/video/BV1VJ41157wq?spm_id_from=333.788.videopod.episodes&vd_source=da23da82658adda9cbdfd045a9e6daf7)
- [课程代码github](https://github.com/wycl16514)
- [重点课程：从实模式进入保护模式](https://www.bilibili.com/video/BV1VJ41157wq?spm_id_from=333.788.player.switch&vd_source=da23da82658adda9cbdfd045a9e6daf7&p=5)

