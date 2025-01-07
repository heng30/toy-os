; cli不允许在ring-3中调用，会触发异常
cli ; 关中断
fin:
    jmp fin
