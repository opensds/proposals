export LOG_DIR=/opt/codeDex/log

sh /home/icp/plugins/CodeDEX/coverity.sh 1>$LOG_DIR/coverity.log 2>$LOG_DIR/coverity.err
