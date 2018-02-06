export LOG_DIR=/opt/codeDex/log

sh /home/icp/plugins/CodeDEX/klocwork_build.sh 1>$LOG_DIR/klocwork_build.log 2>$LOG_DIR/klocwork_build.err
