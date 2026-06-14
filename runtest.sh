#!/usr/bin/env bash
set -euo pipefail

# ...existing code...

# 工作目录
cd /home/ubuntu/competition/HSP/2025/checker

# 确保 example 目录存在
mkdir -p example

# 生成输入（可选传入参数：N L），会在 example/ 下写 infile.txt 和 outfile.txt（outfile 可被后续程序覆盖）
# 用法示例： ./runtest.sh          -> 使用 gen_input.py 默认参数
#            ./runtest.sh 6 200     -> 传递 N=6 L=200 给 gen_input.py
python3 example/gen_input.py "$@" || true

# 输入/输出文件路径
IN=example/infile.txt
OUT=example/outfile.txt

# 编译 mycode.cpp
g++ -std=c++17 -O2 -pipe mycode.cpp -o mycode
g++ -std=c++17 -O2 -pipe opticode.cpp -o opticode
# 运行 mycode，输入为 example/infile.txt，输出写入 example/outfile.txt（会覆盖 gen_input.py 可能生成的 outfile）
/usr/bin/time -v -p ./mycode < "$IN" > "$OUT" 2> example/debug.txt
/usr/bin/time -v -p ./opticode < "$IN" > "example/outfile_opticode.txt" 2> example/opti_debug.txt
# 运行已编译好的 checker（按用户提供的用法）
if [ -x ./checker ]; then
  ./checker "$IN" "$OUT" "$OUT"
  ./checker "$IN" "example/outfile_opticode.txt" "example/outfile_opticode.txt"
else
  echo "错误：checker 可执行文件未找到或不可执行（当前目录: $(pwd)）" >&2
  exit 2
fi
