主要包含一些测试用例程序

### Note
- 不能复制字符串常量到栈上局部内存中，因为字符串常量在代码段中，而栈上的内存在数据段中。两者的起始位置不一致。如果真要复制可以借助系统调用进行