# 智能指针测试项目

这个项目演示了C++中智能指针的使用，特别是 `shared_ptr` 和 `weak_ptr` 的使用。

## 文件说明

- `main.cpp`: 包含智能指针测试代码
- `Makefile`: 用于编译和运行项目
- `README.md`: 项目说明文档

## 编译和运行

### 编译项目
```bash
make
```

### 运行测试
```bash
make run
```

### 清理编译文件
```bash
make clean
```

## 代码说明

这个测试程序演示了：

1. 使用 `std::make_shared` 创建 `shared_ptr`
2. 从 `shared_ptr` 创建 `weak_ptr`
3. 使用 `weak_ptr::lock()` 方法安全地访问对象
4. 当 `shared_ptr` 被释放后，`weak_ptr` 无法再访问对象

## 预期输出

程序运行后应该输出：
```
MyClass::DoSomething() called
对象已被释放
```

这证明了 `weak_ptr` 不会阻止对象的销毁，当所有 `shared_ptr` 都被释放后，对象会被自动销毁。 