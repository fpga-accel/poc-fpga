/*-
 *   BSD LICENSE
 *
 *   Copyright(c)  2017 Huawei Technologies Co., Ltd. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Huawei Technologies Co., Ltd  nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include "pci_rw_tool_func.h"
#include "ul_get_port_status.h"

#define	STR_PARSE_ARG	"s:a:h"

static int parse_arg(int argc, char* argv[]);
static void help();

static unsigned int g_port_id = 0;
static unsigned int g_slot_id = 0;
static unsigned int g_bar2_addr = 0;

int main(int argc, char* argv[]) {
	int ret = 0;
    unsigned int bar2_data = 0;
    
	if (0 != parse_arg(argc, argv)) {
		return -EINVAL;
	}

    ret = pci_port_status_init_env();
    if(ret != 0) {
        printf("%s: pci_port_status_init_env failed(%d)\r\n", __FUNCTION__, ret);
		return ret;
    }

    ret = pci_slot_id_to_port_id(g_slot_id, &g_port_id);
    if(ret != 0) {
        printf("%s: convert_slot_to_port failed(%d)\r\n", __FUNCTION__, ret);
		return ret;
    }
    
    ret = pci_bar2_init_env(g_port_id);
    if (ret != 0) {
        printf("%s: pci_bar2_init_env failed(%d)\r\n", __FUNCTION__, ret);
        return ret;
    }

    (void)pci_bar2_read_regs(g_port_id, &g_bar2_addr, sizeof(g_bar2_addr)/sizeof(unsigned int), &bar2_data);
    printf("addr: 0x%08x, data: 0x%08x\r\n", g_bar2_addr, bar2_data);

    (void)pci_bar2_uninit_env();

    return 0;
}

static int parse_arg(int argc, char* argv[]) {
    char* arg_val = NULL;
    int     ch;
        
    while ((ch=getopt(argc, argv, STR_PARSE_ARG)) != -1) {
        switch (ch) {
            case 's': {
                assert(NULL != optarg);
                arg_val = optarg;
                g_slot_id = strtoul(arg_val, NULL, 0);
                break;
            }
            case 'a': {
                assert(NULL != optarg);
                arg_val = optarg;
                g_bar2_addr = strtoul(arg_val, NULL, 0);
                break;
            }
            case 'h':
            default:
                goto parse_error;
                
        }
    }
       
    return 0;
    
parse_error:
    help();
    return -EINVAL;
}

static void help() {
    printf(
        "-----------------------------------------------------------------------------------\r\n"
        "argument format: [-s slot_id] [-a bar2_addr]\r\n"
        "\tslot_id: the VF's slot id, 0 as default\r\n"
        "\tbar2_addr : the addr of bar2 to write data, 0 as default\r\n"
        "\t-h: print help\r\n"
        "-----------------------------------------------------------------------------------\r\n"
        );
}

