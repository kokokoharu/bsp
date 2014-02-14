#!/bin/sh

# Arguments:
# 1: slam folder
#        belief, state, lp, control
# 2: timesteps

SLAM_TYPE=$1
TIMESTEPS=$2


BSP_DIR=$(sed s:/"bsp"/.*:/"bsp": <<< $(pwd))
SLAM_DIR=" $( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DIR=$SLAM_DIR/$SLAM_TYPE

MPC_DIR=$DIR/mpc
if [ $SLAM_TYPE = "belief" ]; then
MPC_FILE_NAME="${SLAM_TYPE}PenaltyMPC"
else
MPC_FILE_NAME="${SLAM_TYPE}MPC"
fi

CPP_TIMESTEP_DEF="#define TIMESTEPS"

cd $SLAM_DIR

# deal with FORCES files
MPC_C_FILE="${MPC_DIR}/${MPC_FILE_NAME}${TIMESTEPS}.c"
MPC_H_FILE="${MPC_DIR}/${MPC_FILE_NAME}${TIMESTEPS}.h"

if [ ! -f $MPC_C_FILE ] || [ ! -f $MPC_H_FILE ]; then
    echo "mpc files do not exist. generating with matlab..."
    cd $DIR
    if [ $SLAM_TYPE = "belief" ]; then MATLAB_FILE_NAME="${SLAM_TYPE}_penalty"
    else MATLAB_FILE_NAME="${SLAM_TYPE}"; fi
    
    matlab -nodisplay -nosplash -nodesktop -r "slam_${MATLAB_FILE_NAME}_mpc_gen(${TIMESTEPS}); exit"
    cd $SLAM_DIR
fi

echo "Copying ${MPC_FILE_NAME}${TIMESTEPS}.c/h to ${MPC_FILE_NAME}.c/h"
cp ${MPC_C_FILE} ${DIR}/${MPC_FILE_NAME}".c"
cp ${MPC_H_FILE} ${DIR}/${MPC_FILE_NAME}".h"

# modify slam.h
echo "replacing TIMESTEPS definition with new TIMESTEPS for slam.h in ${SLAM_DIR}"
H_WRITE="${SLAM_DIR}/slam.h"
sed -i "s/^${CPP_TIMESTEP_DEF}.*/${CPP_TIMESTEP_DEF} ${TIMESTEPS}/" $H_WRITE

# make casadi files
# move casadi files over if different
if [ $SLAM_TYPE = "state" ]; then
    CASADI_SLAM_DIR="${BSP_DIR}/casadi/slam"
    echo "Making casadi files"
    make -C ${CASADI_SLAM_DIR} T=${TIMESTEPS}

    CASADI_FILES=${CASADI_SLAM_DIR}/*.c
    for CASADI_FILE in $CASADI_FILES
    do
	CASADI_FILE_NAME=$(basename $CASADI_FILE)
	SLAM_CASADI_FILE=${SLAM_DIR}/state/${CASADI_FILE_NAME}

	if [ ! -f $SLAM_CASADI_FILE ]; then
	    echo "casadi file ${CASADI_FILE_NAME} does not exit, copying over"
	    cp $CASADI_FILE $SLAM_CASADI_FILE
	fi

	if [ $(diff $CASADI_FILE $SLAM_CASADI_FILE | wc -w) -gt 0 ]; then
	    echo "casadi file ${CASADI_FILE_NAME} differs, copying new one over"
	    cp $CASADI_FILE $SLAM_CASADI_FILE
	fi
   done
fi