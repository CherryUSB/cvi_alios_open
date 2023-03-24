/*
 * Copyright (C) 2018-2022 Alibaba Group Holding Limited
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdint.h>
#include <soc.h>
#include <board_config.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief  init the board for default: pin mux, etc.
 * re-implement if need.
 * @return
 */
void board_init(void);

void board_bt_init(void);

/**
 * @brief  init the board flash for default: pin mux, etc.
 * re-implement if need.
 * @return
 */
void board_flash_init(void);
#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H__ */

