#!/bin/bash

# 代码质量检查脚本
# 用于检查和改进代码质量

set -e

# 颜色定义
RED='\033[31m'
GREEN='\033[32m'
YELLOW='\033[33m'
BLUE='\033[34m'
NC='\033[0m'

echo -e "${BLUE}🔍 IPC 项目代码质量检查${NC}"
echo "==============================="

# 1. 编译检查
echo -e "\n${YELLOW}1. 编译检查${NC}"
echo "-------------"
if make clean && make all; then
    echo -e "${GREEN}✅ 编译检查通过${NC}"
else
    echo -e "${RED}❌ 编译检查失败${NC}"
    exit 1
fi

# 2. 静态代码分析
echo -e "\n${YELLOW}2. 静态代码分析${NC}"
echo "----------------"
if command -v cppcheck >/dev/null 2>&1; then
    echo "运行 cppcheck..."
    cppcheck --enable=all --std=c99 --suppress=missingIncludeSystem \
             --suppress=unusedFunction --suppress=unmatchedSuppression \
             *.c 2>&1 | tee cppcheck.log
    
    if [ -s cppcheck.log ]; then
        echo -e "${YELLOW}⚠️  发现一些静态分析问题，请查看 cppcheck.log${NC}"
    else
        echo -e "${GREEN}✅ 静态代码分析通过${NC}"
    fi
else
    echo -e "${YELLOW}⚠️  cppcheck 未安装，跳过静态分析${NC}"
    echo "安装命令: sudo apt-get install cppcheck"
fi

# 3. 代码风格检查
echo -e "\n${YELLOW}3. 代码风格检查${NC}"
echo "----------------"
echo "检查基本代码风格..."

# 检查缩进一致性
echo "• 检查缩进一致性..."
for file in *.c; do
    if [ -f "$file" ]; then
        # 检查是否混用了空格和制表符
        if grep -P '\t' "$file" >/dev/null && grep -P '^    ' "$file" >/dev/null; then
            echo -e "${YELLOW}  ⚠️  $file 混用了制表符和空格${NC}"
        fi
    fi
done

# 检查行长度
echo "• 检查行长度..."
for file in *.c; do
    if [ -f "$file" ]; then
        long_lines=$(awk 'length > 100 {print NR ": " $0}' "$file")
        if [ -n "$long_lines" ]; then
            echo -e "${YELLOW}  ⚠️  $file 有超过100字符的行:${NC}"
            echo "$long_lines" | head -5
        fi
    fi
done

echo -e "${GREEN}✅ 代码风格检查完成${NC}"

# 4. 内存检查
echo -e "\n${YELLOW}4. 内存检查${NC}"
echo "------------"
if command -v valgrind >/dev/null 2>&1; then
    echo "运行 valgrind 内存检查..."
    
    for prog in pipe_demo msgqueue_demo sharedmem_demo semaphore_demo socket_demo; do
        if [ -x "./$prog" ]; then
            echo "检查 $prog..."
            timeout 10 valgrind --leak-check=full --error-exitcode=1 \
                     --log-file="valgrind_$prog.log" \
                     ./"$prog" < /dev/null > /dev/null 2>&1 || true
            
            if grep -q "ERROR SUMMARY: 0 errors" "valgrind_$prog.log" 2>/dev/null; then
                echo -e "  ${GREEN}✅ $prog 内存检查通过${NC}"
                rm -f "valgrind_$prog.log"
            else
                echo -e "  ${YELLOW}⚠️  $prog 有内存问题，查看 valgrind_$prog.log${NC}"
            fi
        fi
    done
else
    echo -e "${YELLOW}⚠️  valgrind 未安装，跳过内存检查${NC}"
    echo "安装命令: sudo apt-get install valgrind"
fi

# 5. 功能测试
echo -e "\n${YELLOW}5. 功能测试${NC}"
echo "------------"
echo "运行基本功能测试..."
if ./run_tests.sh > test_results.log 2>&1; then
    echo -e "${GREEN}✅ 功能测试通过${NC}"
    rm -f test_results.log
else
    echo -e "${YELLOW}⚠️  功能测试有问题，查看 test_results.log${NC}"
fi

# 6. 文档检查
echo -e "\n${YELLOW}6. 文档检查${NC}"
echo "------------"
echo "检查文档完整性..."

# 检查 README
if [ -f "README.md" ]; then
    echo -e "${GREEN}✅ README.md 存在${NC}"
    
    # 检查 README 内容
    if grep -q "编译" README.md && grep -q "运行" README.md; then
        echo -e "${GREEN}✅ README 包含编译和运行说明${NC}"
    else
        echo -e "${YELLOW}⚠️  README 缺少编译或运行说明${NC}"
    fi
else
    echo -e "${RED}❌ README.md 不存在${NC}"
fi

# 检查代码注释
echo "检查代码注释..."
for file in *.c; do
    if [ -f "$file" ]; then
        comment_lines=$(grep -c '/\*\|//\|#' "$file" || true)
        total_lines=$(wc -l < "$file")
        comment_ratio=$((comment_lines * 100 / total_lines))
        
        if [ $comment_ratio -lt 10 ]; then
            echo -e "${YELLOW}  ⚠️  $file 注释比例较低 ($comment_ratio%)${NC}"
        else
            echo -e "${GREEN}  ✅ $file 注释充足 ($comment_ratio%)${NC}"
        fi
    fi
done

# 7. 安全检查
echo -e "\n${YELLOW}7. 安全检查${NC}"
echo "------------"
echo "检查潜在的安全问题..."

# 检查危险函数的使用
dangerous_functions=("strcpy" "strcat" "sprintf" "gets")
for file in *.c; do
    if [ -f "$file" ]; then
        for func in "${dangerous_functions[@]}"; do
            if grep -q "$func" "$file"; then
                echo -e "${YELLOW}  ⚠️  $file 使用了潜在危险函数: $func${NC}"
            fi
        done
    fi
done

# 检查缓冲区溢出风险
echo "检查缓冲区大小..."
for file in *.c; do
    if [ -f "$file" ]; then
        # 检查是否有合适的缓冲区大小定义
        if grep -q "#define.*SIZE" "$file"; then
            echo -e "${GREEN}  ✅ $file 定义了缓冲区大小常量${NC}"
        fi
    fi
done

# 8. 性能检查
echo -e "\n${YELLOW}8. 性能检查${NC}"
echo "------------"
echo "检查性能相关问题..."

# 检查是否有不必要的系统调用
for file in *.c; do
    if [ -f "$file" ]; then
        # 检查是否在循环中调用了系统调用
        if grep -A5 -B5 "for\|while" "$file" | grep -q "malloc\|free\|printf"; then
            echo -e "${YELLOW}  ⚠️  $file 可能在循环中有性能问题${NC}"
        fi
    fi
done

# 总结
echo -e "\n${BLUE}📊 检查总结${NC}"
echo "============"
echo -e "${GREEN}✅ 编译检查通过${NC}"
echo -e "${GREEN}✅ 代码风格检查完成${NC}"
echo -e "${GREEN}✅ 文档检查完成${NC}"
echo -e "${GREEN}✅ 安全检查完成${NC}"
echo -e "${GREEN}✅ 性能检查完成${NC}"

echo -e "\n${BLUE}🎉 代码质量检查完成！${NC}"
echo "如有警告，请查看相应的日志文件。"

# 清理临时文件
rm -f cppcheck.log
