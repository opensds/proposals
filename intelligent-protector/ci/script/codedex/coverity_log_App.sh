export LOG_DIR=/opt/codeDex/log

sh /home/icp/plugins/CodeDEX/coverity_app.sh 1>$LOG_DIR/coverity.log 2>$LOG_DIR/coverity.err
