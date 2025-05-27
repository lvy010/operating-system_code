# Operating Systems: Three Easy Pieces 读书笔记

## 简介
这个仓库包含了在阅读《Operating Systems: Three Easy Pieces》时所做的笔记。

## 目录结构
- `/notes`: 包含了所有的读书笔记。
- `/code`: 一些简单的代码示例，用于演示书中提到的概念。
- `/resources`: 学习资源链接。

## 笔记内容概览

### 第一部分：虚拟化（Virtualization）
1. **Introduction**
   - 操作系统的基本概念和目标。
2. **Abstraction: The Basic Idea**
   - 抽象的概念及其在操作系统中的重要性。
3. **Processes**
   - 进程的概念、创建、调度及管理。
4. **Process API**
   - 如何使用API进行进程控制。
5. **CPU Scheduling**
   - CPU调度策略和算法。
6. **Beyond Single-Core**
   - 多核环境下的进程管理。

### 第二部分：并发（Concurrency）
1. **Introduction to Concurrency**
   - 并发的基础知识。
2. **Thread API**
   - 线程的概念及其应用。
3. **Locks**
   - 锁机制及其使用。
4. **Condition Variables**
   - 条件变量的作用和实现。
5. **Semaphores**
   - 信号量的原理和应用场景。
6. **Monitors**
   - 监视器的概念和用法。
7. **Case Study: Copy-on-Write Fork**
   - 一个实际案例研究。

### 第三部分：持久性（Persistence）
1. **Introduction to Persistence**
   - 数据持久性的基础。
2. **Files**
   - 文件系统的基本概念。
3. **File System Implementation**
   - 文件系统的实现细节。
4. **Crash Consistency: FSCK and Journaling**
   - 崩溃一致性及其解决方案。
5. **Log-Structured File Systems**
   - 日志结构文件系统的介绍。
6. **Data Integrity and Security**
   - 数据完整性和安全性。

## 如何贡献
如果你有改进意见或希望贡献自己的笔记，请遵循以下步骤：
1. Fork这个仓库。
2. 创建一个新的分支 (`git checkout -b feature-new-feature`)。
3. 提交你的更改 (`git commit -am 'Add some feature'`)。
4. 推送到分支 (`git push origin feature-new-feature`)。
5. 打开一个Pull Request。

## 许可证
本项目采用MIT许可证。详情请参阅[LICENSE](LICENSE)文件。

## 联系方式
如果有任何疑问或建议，可以通过[电子邮件](17338770572@163.com)联系我~