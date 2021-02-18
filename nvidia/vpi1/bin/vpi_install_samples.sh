#!/bin/sh
#
# Copyright (c) 2021, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA Corporation and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA Corporation is strictly prohibited.

PATH=/usr/bin:/bin:/usr/sbin:/sbin;
SAMPLES_SRC=${SAMPLES_SRC:-"/opt/nvidia/vpi1/samples"};

if [ -z "$1" ]; then
    echo "Error: target directory missing";
    echo "Usage: $(basename $0) <target directory>";
    echo "       Will copy NVIDIA VPI 1.0 samples to <target directory>";
    exit 1;
fi

SAMPLES_DIR=$1;
echo $SAMPLES_DIR | grep -q "/$"
if [ $? -ne 0 ]; then
    SAMPLES_DIR=$SAMPLES_DIR"/";
fi
SAMPLES_DIR=$SAMPLES_DIR"NVIDIA_VPI-1.0-samples";

mkdir -p "$SAMPLES_DIR" >/dev/null 2>&1;

if [ -d "$SAMPLES_DIR" -a -w "$SAMPLES_DIR" ]; then
    echo "Copying samples to $SAMPLES_DIR now...";
    cp -R $SAMPLES_SRC/* "$SAMPLES_DIR";
    echo "Finished copying samples.";
else
    echo "Do not have permissions to write to $SAMPLES_DIR";
    exit 1;
fi

exit 0;
