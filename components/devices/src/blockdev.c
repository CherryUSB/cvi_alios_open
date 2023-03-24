/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dev_internal.h>
#include <devices/impl/blockdev_impl.h>

#define BLOCKDEV_DRIVER(dev)  ((blockdev_driver_t*)(dev->drv))
#define BLOCKDEV_VAILD(dev) do { \
    if (device_valid(dev, "mmc") != 0 && device_valid(dev, "sd") != 0 && device_valid(dev, "usb_mass") != 0) \
        return -1; \
} while(0)

#define BLOCK_DEV_LOCK(dev)                                                   \
    do                                                                        \
    {                                                                         \
        if (!aos_mutex_is_valid(&BLOCKDEV_DRIVER(dev)->dev_mtx))              \
        {                                                                     \
            aos_mutex_new(&BLOCKDEV_DRIVER(dev)->dev_mtx);                    \
            aos_mutex_lock(&BLOCKDEV_DRIVER(dev)->dev_mtx, AOS_WAIT_FOREVER); \
        }                                                                     \
    } while (0)
#define BLOCK_DEV_UNLOCK(dev)                             \
    do                                                    \
    {                                                     \
        aos_mutex_unlock(&BLOCKDEV_DRIVER(dev)->dev_mtx); \
    } while (0)

int rvm_hal_blockdev_read_blks(rvm_dev_t *dev, void *buffer, uint32_t start_block, uint32_t block_cnt)
{
    int ret;

    BLOCKDEV_VAILD(dev);

    BLOCK_DEV_LOCK(dev);
    ret = BLOCKDEV_DRIVER(dev)->read_blks(dev, buffer, start_block, block_cnt);
    BLOCK_DEV_UNLOCK(dev);

    return ret;
}

int rvm_hal_blockdev_write_blks(rvm_dev_t *dev, void *buffer, uint32_t start_block, uint32_t block_cnt)
{
    int ret;

    BLOCKDEV_VAILD(dev);

    BLOCK_DEV_LOCK(dev);
    ret = BLOCKDEV_DRIVER(dev)->write_blks(dev, buffer, start_block, block_cnt);
    BLOCK_DEV_UNLOCK(dev);

    return ret;
}

int rvm_hal_blockdev_erase_blks(rvm_dev_t *dev, uint32_t start_block, uint32_t block_cnt)
{
    int ret;

    BLOCKDEV_VAILD(dev);

    BLOCK_DEV_LOCK(dev);
    ret = BLOCKDEV_DRIVER(dev)->erase_blks(dev, start_block, block_cnt);
    BLOCK_DEV_UNLOCK(dev);

    return ret;
}

#if defined(CONFIG_COMP_SDMMC)
int rvm_hal_blockdev_mmc_select_area(rvm_dev_t *dev, rvm_hal_mmc_access_area_t area)
{
    int ret = 0;
    rvm_hal_mmc_access_area_t def_area;

    BLOCKDEV_VAILD(dev);

    if (!BLOCKDEV_DRIVER(dev)->current_area || !BLOCKDEV_DRIVER(dev)->select_area)
        return 0;

    BLOCK_DEV_LOCK(dev);

    ret = BLOCKDEV_DRIVER(dev)->current_area(dev, &def_area);
    if (ret == 0) {
        if (def_area != area) {
            ret = BLOCKDEV_DRIVER(dev)->select_area(dev, area);
        }
    }
    BLOCK_DEV_UNLOCK(dev);
    return ret;
}
#endif

int rvm_hal_blockdev_get_info(rvm_dev_t *dev, rvm_hal_blockdev_info_t *info)
{
    int ret;

    BLOCKDEV_VAILD(dev);

    BLOCK_DEV_LOCK(dev);
    ret = BLOCKDEV_DRIVER(dev)->get_info(dev, info);
    BLOCK_DEV_UNLOCK(dev);

    return ret;
}