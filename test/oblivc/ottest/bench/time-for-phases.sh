#!/bin/bash
# TODO explain --remote-host/--remote-path

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_PATH="$(cd "$DIR/../../../.." && pwd)"
REMOTE_PATH="$PROJECT_PATH"
REMOTE_HOST="localhost"
LOCAL_HOST="localhost"
LOCAL_PORT=1234

OTSRC="src/ext/oblivc/ot.c"
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
  elif [ $1 = "--thread-never" ]; then
    sed -i 's/#define OT_THREAD_THRESHOLD .*$/#define OT_THREAD_THRESHOLD 0x7fffffff/' $PROJECT_PATH/$OTSRC
  fi
  shift
done

echo $REMOTE_HOST:$REMOTE_PATH
cd "$PROJECT_PATH/$BENCHDIR"
port=$LOCAL_PORT
for macro_suffix in BASE_OT EXTENSION VALIDATION PAYLOAD; do
  # Change thread count
  sed -i "1i #define PHASE_TIME_UPTO_$macro_suffix" $PROJECT_PATH/$OTSRC
  # Build project
  ( cd "$PROJECT_PATH" && $BUILDCMD )

  # Build remote project
  if [ $REMOTE_HOST != "localhost" ]; then
    scp $PROJECT_PATH/$OTSRC $REMOTE_HOST:$REMOTE_PATH/$OTSRC
    ssh $REMOTE_HOST "cd $REMOTE_PATH && $BUILDCMD"
  fi

  # Compile benchmark program
  $OBLIVCC -O3 $PROJECT_PATH/$BENCHSRC -o $BENCHBIN
  ssh $REMOTE_HOST "cd $REMOTE_PATH/$BENCHDIR && $REMOTE_PATH/bin/oblivcc -O3 $REMOTE_PATH/$BENCHSRC -o $BENCHBIN"
  for ottype in M Q; do
    for ((run=0; run<5; run++)); do
      ./$BENCHBIN $port -- $ottype 5000000 &
      sleep 0.3
      echo -n "$port $ottype $tcount" >> $0.log
      ssh $REMOTE_HOST time $REMOTE_PATH/$BENCHDIR/$BENCHBIN $port $LOCAL_HOST $ottype 5000000 &>> $0.log
      port=$((port+1))
    done
  done
  sed -i "/#define PHASE_TIME_UPTO_$macro_suffix/d" $PROJECT_PATH/$OTSRC
done

# Restore source file
git checkout HEAD $PROJECT_PATH/$OTSRC
scp $PROJECT_PATH/$OTSRC $REMOTE_HOST:$REMOTE_PATH/$OTSRC
