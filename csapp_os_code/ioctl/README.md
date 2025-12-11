**隐藏在 libc 中的设备查询**: libc (musl libc) 会根据是否输出到 tty 控制缓冲行为；glibc 则是使用 fstat。功能的增加势必带来了操作系统和应用程序的复杂性。
