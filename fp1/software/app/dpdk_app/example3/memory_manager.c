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

#include "memory_manager.h"
#include "fpga_ddr_rw_interface.h"
#include "securec.h"

#include <rte_config.h>
#include <rte_eal.h>
#include <rte_memzone.h>
#include <rte_mempool.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_debug.h>
#include <rte_errno.h>

#include <unistd.h>
#include <fcntl.h>

/* mbuf space, reserve 128byte to avoid write data from HEADROOM */
#define MBUF_SIZE(n)            ((n) + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM)

#define HARD_ACC_OFFSET        (32)
/* mbuff real size */
#define MBUF_SIZE_512K          (MBUF_SIZE(512*1024) + HARD_ACC_OFFSET)
#define MBUF_SIZE_1M            (MBUF_SIZE(1*1024*1024) + HARD_ACC_OFFSET)
#define MBUF_SIZE_2M            (MBUF_SIZE(2*1024*1024) + HARD_ACC_OFFSET)
#define MBUF_SIZE_4M            (MBUF_SIZE(4*1024*1024) + HARD_ACC_OFFSET)
#define MBUF_SIZE_8M            (MBUF_SIZE(8*1024*1024) + HARD_ACC_OFFSET)
#define MBUF_SIZE_16M           (MBUF_SIZE(16*1024*1024) + HARD_ACC_OFFSET)
#define MBUF_SIZE_32M           (MBUF_SIZE(32*1024*1024) + HARD_ACC_OFFSET)
#define MBUF_SIZE_64M           (MBUF_SIZE(64*1024*1024) + HARD_ACC_OFFSET)
#define MBUF_SIZE_256M          (MBUF_SIZE(256*1024*1024) + HARD_ACC_OFFSET)
#define MBUF_SIZE_512M          (MBUF_SIZE(512*1024*1024) + HARD_ACC_OFFSET)

/*PRIV mbuf private head size*/
#define MBUF_PRIV_SIZE          (sizeof(struct rte_pktmbuf_pool_private))
/* offset bytes from mbuff head to data*/
#define MBUF_OFFSET             (sizeof(struct rte_mbuf))

/* mbuf resource global management stru */
static MEM_POOL_CFG_DFX_STRU g_mem_pool_cfg[MAX_MEMPOOL_NUM] = {};
static int g_mem_pool_cfg_init_flag = 0;

int memory_manager_init( MEM_POOL_CFG_STRU * mem_pool_cfg )
{
    struct rte_mempool* mem_pool;

    char pool_name[10] = { 0 };
    unsigned int buff_size = 0;

    unsigned int idx = 0;
    int ret = 0;

    if(NULL == mem_pool_cfg) {
        printf("[%s] line:%d [%s], mem_pool_cfg is null\n", __FILE__, __LINE__, __FUNCTION__);
        return -1;
    }

    /* init mempool management structure */
    if (!g_mem_pool_cfg_init_flag)
    {
        /* only call once */
        for ( idx = 0 ; idx < MAX_MEMPOOL_NUM ; idx++ )
        {     
            g_mem_pool_cfg[idx].buff_num = 0;
            g_mem_pool_cfg[idx].buff_size = 0;
            g_mem_pool_cfg[idx].flag_created = 0;
            g_mem_pool_cfg[idx].mem_pool = NULL;
            (void)memset_s(g_mem_pool_cfg[idx].name, MPOOL_NAME_SIZE, 0x0, MPOOL_NAME_SIZE);

            ret = pthread_mutex_init(&g_mem_pool_cfg[idx].alloc_mutex, NULL);
            if (0 != ret)
            {
                printf("[%s][Line%d] pthread_mutex_init alloc_mutex failed, ret = %d.\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
        }
        g_mem_pool_cfg_init_flag = 1;
    }

    switch ( mem_pool_cfg->buf_type )
    {
        case MBUF_512K :
            {
                (void)strcpy_s(pool_name, MPOOL_NAME_SIZE, "512K");
                buff_size = MBUF_SIZE_512K;
            }
            break;
        case MBUF_1M :
            {
                (void)strcpy_s(pool_name, MPOOL_NAME_SIZE, "1M");
                buff_size = MBUF_SIZE_1M;
            }
            break;
        case MBUF_2M :
            {
                (void)strcpy_s(pool_name, MPOOL_NAME_SIZE, "2M");
                buff_size = MBUF_SIZE_2M;
            }
            break;
        case MBUF_4M :
            {
                (void)strcpy_s(pool_name, MPOOL_NAME_SIZE, "4M");
                buff_size = MBUF_SIZE_4M;
            }
            break;
        case MBUF_8M :
            {
                (void)strcpy_s(pool_name, MPOOL_NAME_SIZE, "8M");
                buff_size = MBUF_SIZE_8M;
            }
            break;
        case MBUF_16M :
            {
                (void)strcpy_s(pool_name, MPOOL_NAME_SIZE, "16M");
                buff_size = MBUF_SIZE_16M;
            }
            break;
        case MBUF_32M :
            {
                (void)strcpy_s(pool_name, MPOOL_NAME_SIZE, "32M");
                buff_size = MBUF_SIZE_32M;
            }
            break;
        case MBUF_64M :
            {
                (void)strcpy_s(pool_name, MPOOL_NAME_SIZE, "64M");
                buff_size = MBUF_SIZE_64M;
            }
            break;
        case MBUF_256M :
            {
                (void)strcpy_s(pool_name, MPOOL_NAME_SIZE, "256M");
                buff_size = MBUF_SIZE_256M;
            }
            break;
        case MBUF_512M :
            {
                (void)strcpy_s(pool_name, MPOOL_NAME_SIZE, "512M");
                buff_size = MBUF_SIZE_512M;
            }
            break;
        default:
            {
                printf("[%s] line:%d [%s], buf_type[%d] is not supported\n", __FILE__, __LINE__, __FUNCTION__, mem_pool_cfg->buf_type);
                return -2;
            }
    }

    mem_pool = rte_mempool_lookup(pool_name);
    if ( NULL == mem_pool )
    {
        printf("[%s] line:%d [%s], rte_mempool_lookup (name:%s)\n", __FILE__, __LINE__, __FUNCTION__, pool_name);
        mem_pool = rte_mempool_create(pool_name, mem_pool_cfg->buf_num, buff_size,
                            0, MBUF_PRIV_SIZE, rte_pktmbuf_pool_init,
                            NULL, rte_pktmbuf_init, NULL, 0, MEMPOOL_F_SP_PUT | MEMPOOL_F_SC_GET);
        if ( NULL == mem_pool )
        {
            printf("[%s] line:%d [%s], rte_mempool_create failed\n", __FILE__, __LINE__, __FUNCTION__);
            return -3;
        }
    }

    g_mem_pool_cfg[mem_pool_cfg->buf_type].buff_num = mem_pool_cfg->buf_num;
    g_mem_pool_cfg[mem_pool_cfg->buf_type].free_num = mem_pool_cfg->buf_num;
    g_mem_pool_cfg[mem_pool_cfg->buf_type].used_num = 0;
    (void)memcpy_s(g_mem_pool_cfg[mem_pool_cfg->buf_type].name, MPOOL_NAME_SIZE, pool_name, sizeof(pool_name));
    g_mem_pool_cfg[mem_pool_cfg->buf_type].buff_size = buff_size - sizeof(struct rte_mbuf) - RTE_PKTMBUF_HEADROOM;
    g_mem_pool_cfg[mem_pool_cfg->buf_type].flag_created = 1;
    g_mem_pool_cfg[mem_pool_cfg->buf_type].mem_pool = mem_pool;
    g_mem_pool_cfg[mem_pool_cfg->buf_type].phy2virt_offset = mem_pool->elt_va_start - mem_pool->elt_pa[0];

    return 0;
}

void* memory_manager_alloc_bulk(unsigned int buff_size)
{
    unsigned int idx;
    struct rte_mempool* valid_mpool;
    struct rte_mbuf * alloc_mbuff;
    void *buff_vaddr;

    if ( 0 == buff_size )
    {
        printf("[%s] line:%d [%s], buff_size is 0\n", __FILE__, __LINE__, __FUNCTION__);
        return NULL;
    }

    valid_mpool = NULL;

    /* find suitable mempool */
    for ( idx = 0 ; idx < MAX_MEMPOOL_NUM ; idx++ )
    {
        if (( 1 == g_mem_pool_cfg[idx].flag_created ) && (g_mem_pool_cfg[idx].buff_size >= buff_size))
        {
            valid_mpool = g_mem_pool_cfg[idx].mem_pool;
            break;
        }
    }

    if ( NULL == valid_mpool )
    {
        printf("[%s] line:%d [%s], no supported mem pool for size[%d]\n", __FILE__, __LINE__, __FUNCTION__, buff_size);
        return NULL;
    }

    /* alloc bulk from mempool */
    (void)pthread_mutex_lock(&g_mem_pool_cfg[idx].alloc_mutex);
    
    alloc_mbuff = rte_pktmbuf_alloc(valid_mpool);
    if ( NULL == alloc_mbuff)
    {
		(void)pthread_mutex_unlock(&g_mem_pool_cfg[idx].alloc_mutex);
        //printf("[%s] line:%d [%s], rte_pktmbuf_alloc failed\n", __FILE__, __LINE__, __FUNCTION__);
        return NULL;
    }

    /* dataaddr skew more RTE_PKTMBUF_HEADROOM space*/
    g_mem_pool_cfg[idx].used_num++;
    g_mem_pool_cfg[idx].free_num--;

    (void)pthread_mutex_unlock(&g_mem_pool_cfg[idx].alloc_mutex);
    
    buff_vaddr = (void*)(rte_pktmbuf_mtod(alloc_mbuff, char*) + RTE_PKTMBUF_HEADROOM + HARD_ACC_OFFSET);

    return buff_vaddr;
}

int memory_manager_free_bulk( void* buff_vaddr)
{
    struct rte_mbuf *m;
    struct rte_mbuf *start;
    unsigned int data_off;
    unsigned int i;

    if ( NULL == buff_vaddr )
    {
        printf("[%s] line:%d [%s], buff_vaddr is 0\n", __FILE__, __LINE__, __FUNCTION__);
        return -1;
    }

    /* find which mempool it belong to */
    for(i = 0; i < MAX_MEMPOOL_NUM; i++)
    {
        if(0 == g_mem_pool_cfg[i].flag_created)
            continue;

        start = (struct rte_mbuf *)((unsigned long long)g_mem_pool_cfg[i].mem_pool->elt_va_start + g_mem_pool_cfg[i].mem_pool->header_size);
        if((unsigned long long)buff_vaddr >= ((unsigned long long)rte_pktmbuf_mtod(start, void*) + RTE_PKTMBUF_HEADROOM)
            && (unsigned long long)buff_vaddr <= (unsigned long long)g_mem_pool_cfg[i].mem_pool->elt_va_end)
        {
            break;
        }
    }

    if(MAX_MEMPOOL_NUM == i)
    {
        printf("[%s] line:%d [%s], Not find Pool\n", __FILE__, __LINE__, __FUNCTION__);
        return -2;
    }

    data_off = (unsigned long long)rte_pktmbuf_mtod(start, void*) + RTE_PKTMBUF_HEADROOM
            - (g_mem_pool_cfg[i].mem_pool->elt_va_start + g_mem_pool_cfg[i].mem_pool->header_size);

    /* find out mbuff virth addr */
    m = (struct rte_mbuf *)((unsigned long long)buff_vaddr - HARD_ACC_OFFSET - data_off);

    /* alloc bulk from mempool */
    (void)pthread_mutex_lock(&g_mem_pool_cfg[i].alloc_mutex);
    
    rte_pktmbuf_free(m);
    g_mem_pool_cfg[i].used_num--;
    g_mem_pool_cfg[i].free_num++;

    (void)pthread_mutex_unlock(&g_mem_pool_cfg[i].alloc_mutex);

    return 0;
}

int memory_manager_v2p( void* buff_vaddr, unsigned long long* buff_paddr )
{
    unsigned int i;
    unsigned int mem_pool_index = 0;
    struct rte_mbuf *start;
    
    if ( NULL == buff_vaddr )
    {
        printf("[%s] line:%d [%s], buff_vaddr is 0\n", __FILE__, __LINE__, __FUNCTION__);
        return -1;
    }

    if ( NULL == buff_paddr )
    {
        printf("[%s] line:%d [%s], buff_paddr is 0\n", __FILE__, __LINE__, __FUNCTION__);
        return -2;
    }

    /*find out which mempool it belongs to*/

    /*| rte_mempool | private_data | header | rte_mbuf | headroom | data | rte_mbuf | headroom | data |........
                                   |<-mem_pool->elt_va_start
                                                       |<-mem_pool->elt_va_start + sizeof(rte_mbuf) + mem_pool->head_size*/
    for(i = 0; i < MAX_MEMPOOL_NUM; i++)
    {
        if(0 == g_mem_pool_cfg[i].flag_created)
            continue;

        start = (struct rte_mbuf *)((unsigned long long)g_mem_pool_cfg[i].mem_pool->elt_va_start + g_mem_pool_cfg[i].mem_pool->header_size);
        if((unsigned long long)buff_vaddr >= ((unsigned long long)rte_pktmbuf_mtod(start, void*) + RTE_PKTMBUF_HEADROOM)
            && (unsigned long long)buff_vaddr <= g_mem_pool_cfg[i].mem_pool->elt_va_end)
        {
            mem_pool_index = i;
            break;
        }
    }

    if(MAX_MEMPOOL_NUM == i)
    {
        printf("[%s] line:%d [%s], Not find Pool\n", __FILE__, __LINE__, __FUNCTION__);
        return -3;
    }

    *(unsigned long long *)buff_paddr = (unsigned long long)buff_vaddr - (unsigned long long)g_mem_pool_cfg[mem_pool_index].phy2virt_offset;

    return 0;
}

int memory_manager_p2v( void* buff_paddr, unsigned long long* buff_vaddr )
{
    unsigned int i;
    unsigned int mem_pool_index = 0;
    unsigned int data_offset;
    struct rte_mbuf *start;
    
    if ( NULL == buff_paddr )
    {
        printf("[%s] line:%d [%s], buff_paddr is 0\n", __FILE__, __LINE__, __FUNCTION__);
        return -1;
    }

    if ( NULL == buff_vaddr )
    {
        printf("[%s] line:%d [%s], buff_vaddr is 0\n", __FILE__, __LINE__, __FUNCTION__);
        return -2;
    }

    /*find out which mempool it belongs to*/
    /*| rte_mempool | private_data | header | rte_mbuf | headroom | data | rte_mbuf | data |........
                                   |<-mem_pool->elt_pa[0]
                                                       |<-mem_pool->elt_pa[0] + sizeof(rte_mbuf) + mem_pool->head_size*/
    for(i = 0; i < MAX_MEMPOOL_NUM; i++)
    {
        if(0 == g_mem_pool_cfg[i].flag_created)
            continue;

        start = (struct rte_mbuf *)((unsigned long long)g_mem_pool_cfg[i].mem_pool->elt_va_start + g_mem_pool_cfg[i].mem_pool->header_size);
        data_offset = (unsigned long long)rte_pktmbuf_mtod(start, void*) + RTE_PKTMBUF_HEADROOM
            - (g_mem_pool_cfg[i].mem_pool->elt_va_start + g_mem_pool_cfg[i].mem_pool->header_size);
        
        if((unsigned long long)buff_paddr >= (g_mem_pool_cfg[i].mem_pool->elt_pa[0]
            + g_mem_pool_cfg[i].mem_pool->header_size + data_offset)
            && (unsigned long long)buff_paddr <= (g_mem_pool_cfg[i].mem_pool->elt_pa[0] 
            + (g_mem_pool_cfg[i].mem_pool->elt_va_end - g_mem_pool_cfg[i].mem_pool->elt_va_start)))
        {
            mem_pool_index = i;
            break;
        }
    }

    if(MAX_MEMPOOL_NUM == i)
    {
        printf("[%s] line:%d [%s], Not find Pool\n", __FILE__, __LINE__, __FUNCTION__);
        return -3;
    }

    *(unsigned long long *)buff_vaddr = (unsigned long long)buff_paddr + g_mem_pool_cfg[mem_pool_index].phy2virt_offset;

    return 0;
}
void info_collect_mem_manager(void)
{
    unsigned int idx;

    printf("====================== pool statistics ======================\n");
    printf("    id          buff_size     buff_num      pool_name     used_num     remain_num\n");

    for ( idx = 0 ; idx < MAX_MEMPOOL_NUM ; idx++ )
    {
        if ( g_mem_pool_cfg[idx].flag_created )
        {
            printf("    [%2d]       [0x%08x]     [%d]       [%s]          [%d]          [%d]\n",
                idx, g_mem_pool_cfg[idx].buff_size, g_mem_pool_cfg[idx].buff_num, g_mem_pool_cfg[idx].name,
                g_mem_pool_cfg[idx].used_num, g_mem_pool_cfg[idx].free_num);
        }
    }

    return;
}
