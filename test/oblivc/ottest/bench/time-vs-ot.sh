#!/bin/bash
# No params: leave source as is
# --thread-git: restore ot.c before running script
# --thread-always/--thread-never: always/never executes OTs in multiple thread
# TODO explain --remote-host/--remote-path

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_PATH="$(cd "$DIR/../../../.." && pwd)"
REMOTE_PATH="$PROJECT_PATH"
REMOTE_HOST="localhost"
LOCAL_HOST="localhost"

OTSRC="src/ext/oblivc/ot.c"
BENCHDIR="test/oblivc/ottest/"
BENCHSRC="$BENCHDIR/ottime.c"
BENCHBIN="ottime"
OBLIVCC="$PROJECT_PATH/bin/oblivcc"
BUILDCMD="make cilly oblivruntime RELEASELIB=1 NATIVECAML=1"

while [ $# -ge 1 ]; do
  if [ $1 = "--thread-git" ]; then
    git checkout HEAD $PROJECT_PATH/$OTSRC
  elif [ $1 = "--thread-always" ]; then
    sed -i 's/#define OT_THREAD_THRESHOLD .*$/#define OT_THREAD_THRESHOLD 0/' $PROJECT_PATH/$OTSRC
  elif [ $1 = "--thread-never" ]; then
    sed -i 's/#define OT_THREAD_THRESHOLD .*$/#define OT_THREAD_THRESHOLD 0x7fffffff/' $PROJECT_PATH/$OTSRC
  elif [[ $1 = "--remote-host="* ]]; then 
    REMOTE_HOST=${1#--remote-host=}
  elif [[ $1 = "--remote-path="* ]]; then
    REMOTE_PATH=${1#--remote-path=}
  elif [[ $1 = "--local-host="* ]]; then
    LOCAL_HOST=${1#--local-host=}
  fi
  shift
done

echo $REMOTE_HOST:$REMOTE_PATH
# Build project
( cd "$PROJECT_PATH" && $BUILDCMD )

# Okay I need to
# Make changes locally, build.
# Make changes on remote, build remote (if remote!=localhost)
# compile ottime.c, locally and remotely
# Run in loop

# Build remote project
if [ $REMOTE_HOST != "localhost" ]; then
  scp $PROJECT_PATH/$OTSRC $REMOTE_HOST:$REMOTE_PATH/$OTSRC
  ssh $REMOTE_HOST "cd $REMOTE_PATH && $BUILDCMD"
fi

cd "$PROJECT_PATH/$BENCHDIR"
# Compilie benchmark program locally
$OBLIVCC -O3 $PROJECT_PATH/$BENCHSRC -o $BENCHBIN
ssh $REMOTE_HOST "cd $REMOTE_PATH/$BENCHDIR && $REMOTE_PATH/bin/oblivcc -O3 $REMOTE_PATH/$BENCHSRC -o $BENCHBIN -DREMOTEHOST='\"$LOCAL_HOST\"'"
for ottype in H M P; do
  for ((otcount=1000000; $otcount<=5000000; otcount=1+$otcount)); do
    for ((port=1234; $port<1234+6; port=$port+1)); do
      ./$BENCHBIN $port 1 $ottype $otcount &
      sleep 0.1
      ssh $REMOTEHOST $REMOTE_PATH/$BENCHDIR/BENCHBIN $port 2 $ottype $otcount
    done
  done
done
