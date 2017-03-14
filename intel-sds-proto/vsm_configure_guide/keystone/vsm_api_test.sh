#!/bin/bash

ret=`./get-token`

echo $ret

OLD_IFS="$IFS" 
IFS="-" 
arr=($ret) 
IFS="$OLD_IFS" 

OS_TOKEN=${arr[0]}
TENANT_ID=${arr[1]}

unset http_proxy
unset https_proxy

curl -s \
  -H "X-Auth-Token: $OS_TOKEN" \
  -H "Content-Type: application/json" \
  http://10.239.131.163:8778/v1/$TENANT_ID/$1 ; echo

