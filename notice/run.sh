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

if [ -x ${basedir}/bin/notice ]; then
	echo 'start run notice...'
	${basedir}/bin/notice -listenAddr :5000 -domain sc.ftqq.com -token 123456 > /dev/null 2>&1 &
	echo ''
fi
