#!/bin/bash
# No params: leave source as is
# TODO explain --remote-host/--remote-path

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_PATH="$(cd "$DIR/../../../.." && pwd)"
REMOTE_PATH="$PROJECT_PATH"
REMOTE_HOST="localhost"
LOCAL_HOST="localhost"
LOCAL_PORT=1234

OBITSSRC="src/ext/oblivc/obliv_bits.c"
BENCHDIR="test/oblivc/ottest/"
BENCHSRC="$BENCHDIR/ottime.c"
BENCHBIN="ottime"
OBLIVCC="$PROJECT_PATH/bin/oblivcc"
BUILDCMD="make cilly oblivruntime RELEASELIB=1 NATIVECAML=1"

while [ $# -ge 1 ]; do
  if [[ $1 = "--remote-host="* ]]; then 
    REMOTE_HOST=${1#--remote-host=}
  elif [[ $1 = "--remote-path="* ]]; then
    REMOTE_PATH=${1#--remote-path=}
  elif [[ $1 = "--local-host="* ]]; then
    LOCAL_HOST=${1#--local-host=}
  elif [[ $1 = "--local-port-init="* ]]; then
    LOCAL_PORT=${1#--local-port-init=}
  fi
  shift
done

sed -i 's|//#define PROFILE_NETWORK|#define PROFILE_NETWORK|' $PROJECT_PATH/$OBITSSRC

# Build project
( cd "$PROJECT_PATH" && $BUILDCMD )

# Build remote project
if [ $REMOTE_HOST != "localhost" ]; then
  scp $PROJECT_PATH/$OBITSRC $REMOTE_HOST:$REMOTE_PATH/$OBITSRC
  ssh $REMOTE_HOST "cd $REMOTE_PATH && $BUILDCMD"
fi

cd "$PROJECT_PATH/$BENCHDIR"
# Compilie benchmark program locally
$OBLIVCC -O3 $PROJECT_PATH/$BENCHSRC -o $BENCHBIN
ssh $REMOTE_HOST "cd $REMOTE_PATH/$BENCHDIR && $REMOTE_PATH/bin/oblivcc -O3 $REMOTE_PATH/$BENCHSRC -o $BENCHBIN -DREMOTEHOST='\"$LOCAL_HOST\"'"
port=$LOCAL_PORT
for ottype in H M P; do
  for ((otcount=1000000; otcount<=5000000; otcount=1000000+$otcount)); do
    echo $otcount $ottype
    ./$BENCHBIN $port 1 $ottype $otcount 2>/dev/null &
    sleep 0.1
    ssh $REMOTE_HOST $REMOTE_PATH/$BENCHDIR/$BENCHBIN $port 2 $ottype $otcount
    port=$(($port+1))
  done
done

sed -i 's|#define PROFILE_NETWORK|//#define PROFILE_NETWORK|' $PROJECT_PATH/$OBITSSRC
