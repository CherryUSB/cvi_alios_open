/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#include "app_config.h"
#include "app_main.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <soc.h>
#include <drv/tick.h>
#include <boot.h>
#include <boot_wrapper.h>
#include <yoc/partition.h>
#include <yoc/partition_device.h>
#include <decompress.h>

#define LOAD_DDR_ADRR  0x84000000
#define MAX_YOC_SIZE   0xA00000
#define DECOMP_DST_SIZE (16 << 20)

void boot_load_and_jump(void)
{
    const char *jump_to = "prim";
    unsigned long static_addr;
    unsigned long load_addr;

    enum COMPRESS_TYPE comp_type = COMP_LZ4;
    unsigned long uzip_addr = DECOMP_BUF_ADDR;
    char *str = malloc(10);
    int ret =0;
    int i = 0;
    int yoc_size = 0;
    uint32_t image_size;
    partition_t part;
    partition_info_t *part_info;


#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
    printf("load img & jump to [%s]\n", jump_to);
#endif
    part = partition_open(jump_to);
    part_info = partition_info_get(part);

    static_addr = part_info->start_addr + part_info->base_addr;
    load_addr = part_info->load_addr;
    image_size = part_info->image_size;
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
    printf("load&jump 0x%lx,0x%lx,%d\n", static_addr, load_addr, image_size);
#endif
    if (static_addr != load_addr) {
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
        printf("start copy %d bytes\n", image_size);
        extern uint32_t csi_tick_get_ms(void);
        printf("##cur_ms:%d\n", csi_tick_get_ms());
#endif
    partition_read(part, 0, (void *)uzip_addr, image_size);
    //partition_flash_read(NULL, static_addr, (void *)uzip_addr, image_size);

    if (memcmp((uint8_t *)(uzip_addr), "ULZ4", 4) == 0) {
        while(*(uint8_t *)(uzip_addr + 4 + i) != 0x0a)
            i++;
        memcpy(str,(uint8_t *)(uzip_addr + 4),i);
        yoc_size = atoi(str);
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
        printf("str value: %s yoc_size: %d\n",str,yoc_size);
#endif
        uzip_addr = uzip_addr + 5 + i;  

        if (comp_type) {
            size_t dst_size = DECOMP_DST_SIZE;
            void *dst = (void *)(uintptr_t)load_addr;

            ret = decompress(dst, &dst_size, (void *)uzip_addr, yoc_size,comp_type);
            if (ret < 0) {
            	printf("Failed to decompress (%d/%lu)\n", ret, dst_size);
            }
        }
    }else{
        memcpy((void *)load_addr,(void *)uzip_addr,image_size);
        //if (partition_split_and_copy(part, 0)) {
        //   DBG_PRINT("decompress and copy prim bin failed.\n");
        //   //goto fail;
        //}
    } 



#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
        printf("##cur_ms:%d\n", csi_tick_get_ms());
        printf("all copy over..\n");
#endif
    } else {
        printf("xip...\n");
    }
    partition_close(part);
    csi_dcache_clean_invalid();
    csi_icache_invalid();

    void (*func)(void);
    if (memcmp((uint8_t *)(load_addr + 4), "CSKY", 4) == 0) {
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
        printf("j m\n");
#endif
        func = (void (*)(void))((unsigned long *)load_addr);
    } else {
        func = (void (*)(void))(*(unsigned long *)load_addr);
    }
    printf("j 0x%08lx\n", (unsigned long)(*func));
    printf("##cur_ms:%d\n", csi_tick_get_ms());

    csi_tick_uninit();

    (*func)();
    while(1) {;}
}

void boot_sys_reboot(void)
{
    extern void drv_reboot(void);
    drv_reboot();
}

int main(int argc, char *argv[0])
{
    int ret;

    extern void board_yoc_init(void);
    board_yoc_init();

    ret = boot_main(argc, argv);
    return ret;
}
