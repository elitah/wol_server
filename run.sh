#!/bin/bash

if [ -z ${0} ]; then
  exit
fi

basedir=`readlink -f ${0}`

if [ -z ${basedir} ]; then
  exit
fi

basedir=`dirname ${basedir}`

if [ -z ${basedir} ]; then
  exit
fi

echo ''
echo ''
echo ''
echo 'execute path: '${basedir}
echo ''

if [ -z $1 ]; then
  echo -n -e "\033[31;1m是否添加启动项(y): \033[0m"

  read -n 1 -t 30 capture_key

  if [ ! -z ${capture_key} ] && ([ 'Yx' = ${capture_key}'x' ] || [ 'yx' = ${capture_key}'x' ]); then
    envdir='/etc'
    echo ''
    echo ''
    echo ''
    echo -e "\033[31;1m正在安装启动项\033[0m\n"
    echo '#!/bin/sh -e' > ${envdir}/rc.local
    if [ 0 -ne ${?} ]; then
      echo -e "\033[31;1m安装失败\033[0m"
      exit
    fi
    echo '#' >> ${envdir}/rc.local
    echo '# rc.local' >> ${envdir}/rc.local
    echo '#' >> ${envdir}/rc.local
    echo '# This script is executed at the end of each multiuser runlevel.' >> ${envdir}/rc.local
    echo '# Make sure that the script will "exit 0" on success or any other' >> ${envdir}/rc.local
    echo '# value on error.' >> ${envdir}/rc.local
    echo '#' >> ${envdir}/rc.local
    echo '# In order to enable or disable this script just change the execution' >> ${envdir}/rc.local
    echo '# bits.' >> ${envdir}/rc.local
    echo '#' >> ${envdir}/rc.local
    echo '# By default this script does nothing.' >> ${envdir}/rc.local
    echo '' >> ${envdir}/rc.local
    echo "${basedir}/run.sh start > /dev/null 2>&1 &" >> ${envdir}/rc.local
    echo '' >> ${envdir}/rc.local
    echo 'exit 0' >> ${envdir}/rc.local
    chmod 0755 ${envdir}/rc.local
  fi
fi

echo ''
echo ''
echo ''

while [ -x ${basedir}/wol_server ]; do
  ${basedir}/wol_server -f -p 4000
  sleep 1
done
