在编译单元测试的时候请使用
cd ${AGENT_ROOT}/test/build
source env.sh
test_make.sh

如果编译单元测试的时候需要生成覆盖率相关的信息
在gent_llt_coverage.sh会去执行一次所有的测试用例（可能会花比较长的时间），
如果你只想执行自己模块的测试用例请修改gen_llt_coverage.sh（里面写好了详细的说明）
cd ${AGENT_ROOT}/test/build
source env.sh
test_make.sh coverage
gen_llt_coverage.sh

生成的可执行文件请放入到${AGENT_ROOT}/test/bin下

在${AGENT_ROOT}/test/build/makefile里面完成了主要的编译工作。
每个人的模块生成一个可执行文件，生成的可执行文件请放入到${AGENT_ROOT}/test/bin下
命名不重复即可（Agent+模块名字+Test，例如AgentCommonTest）	