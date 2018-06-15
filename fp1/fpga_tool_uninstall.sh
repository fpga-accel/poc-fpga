#!/bin/bash

#
#   BSD LICENSE
#
#   Copyright(c)  2017 Huawei Technologies Co., Ltd. All rights reserved.
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#     * Neither the name of Huawei Technologies Co., Ltd  nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

#check authority
sudo -V &> /dev/null
RET=$?
if [ $RET != 0 ]; 
then
    echo "FPGA_TOOL UNISTALL ERROR: sudo is not in path or not installed. Driver installations will fail "
    echo "To unistall drivers please add sudo to your path or install sudo by running \"yum install sudo\" as root "
    exit $RET 
fi

FPGA_TOOL_UNISTALL_DIR=$(pwd)/tools/fpga_tool/build

#unistall fpga_tool
(cd $FPGA_TOOL_UNISTALL_DIR && bash fpga_tool_clean.sh)
RET=$?
if [ $RET != 0 ]; 
then
    echo "FPGA_TOOL UNISTALL ERROR: Unistall fpga tool failed."
    exit $RET
fi

echo "FPGA_TOOL UNISTALL MESSAGE: Unistall completed."


