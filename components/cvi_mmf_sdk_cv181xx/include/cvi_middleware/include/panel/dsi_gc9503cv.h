#ifndef _MIPI_TX_PARAM_GC9503CV_H_
#define _MIPI_TX_PARAM_GC9503CV_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static struct combo_dev_cfg_s dev_cfg_gc9503cv_480x800 = {
	.devno = 0,
	.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_CLK, -1, -1},
	.lane_pn_swap = {true, true, true, true, true},
	.output_mode = OUTPUT_MODE_DSI_VIDEO,
	.video_mode = BURST_MODE,
	.output_format = OUT_FORMAT_RGB_24_BIT,
	.sync_info = {
		.vid_hsa_pixels = 50,
		.vid_hbp_pixels = 100,
		.vid_hfp_pixels = 30,
		.vid_hline_pixels = 480,
		.vid_vsa_lines = 2,
		.vid_vbp_lines = 15,
		.vid_vfp_lines = 8,
		.vid_active_lines = 800,
		.vid_vsa_pos_polarity = false,
		.vid_hsa_pos_polarity = true,
	},
	.pixel_clk = 33595,
	.reset_pin = {
		.gpio_num = CVI_GPIOA_15,
		.active = 0
	},
	.power_ct_pin = {
		.gpio_num = CVI_GPIOA_19,
		.active = 1
	},
	.pwm_pin = {
		.gpio_num = CVI_GPIOA_18,
		.active = 1
	}
};

// #define PANEL_BIST_MODE

const struct hs_settle_s hs_timing_cfg_gc9503cv_480x800 = { .prepare = 6, .zero = 32, .trail = 1 };

static CVI_U8 data_gc9503cv_0[] = { 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00 };
static CVI_U8 data_gc9503cv_1[] = { 0xF6, 0x5A, 0x87 };
static CVI_U8 data_gc9503cv_2[] = { 0xC1, 0x3F };
static CVI_U8 data_gc9503cv_3[] = { 0xc2, 0x0E };
static CVI_U8 data_gc9503cv_4[] = { 0xC6, 0xF8 };
static CVI_U8 data_gc9503cv_5[] = { 0xCD, 0x25 };
static CVI_U8 data_gc9503cv_6[] = { 0xC9, 0x10};
static CVI_U8 data_gc9503cv_7[] = { 0xF8, 0x8A};
static CVI_U8 data_gc9503cv_8[] = { 0xAC, 0x45};
static CVI_U8 data_gc9503cv_9[] = { 0xA7, 0x47};
static CVI_U8 data_gc9503cv_10[] = { 0xA0, 0xdd};
static CVI_U8 data_gc9503cv_11[] = { 0x86, 0x99, 0xA3, 0xA3, 0x31 };
static CVI_U8 data_gc9503cv_12[] = { 0xFA, 0x08, 0x08, 0x00, 0x04 };
static CVI_U8 data_gc9503cv_13[] = { 0xA3, 0xee };
static CVI_U8 data_gc9503cv_14[] = { 0xFD, 0x28,0x3C,0x00 };
static CVI_U8 data_gc9503cv_15[] = { 0x71, 0x48 };
static CVI_U8 data_gc9503cv_16[] = { 0x72, 0x48 };
static CVI_U8 data_gc9503cv_17[] = { 0x73, 0x00, 0x44 };
static CVI_U8 data_gc9503cv_18[] = { 0x97, 0xEE };
static CVI_U8 data_gc9503cv_19[] = { 0x83, 0x93 };
static CVI_U8 data_gc9503cv_20[] = { 0x9A, 0x84 };
static CVI_U8 data_gc9503cv_21[] = { 0x9B, 0x49 };
static CVI_U8 data_gc9503cv_22[] = { 0x82, 0x3a, 0x3a};
static CVI_U8 data_gc9503cv_23[] = { 0xB1, 0x10 };
static CVI_U8 data_gc9503cv_24[] = { 0x7A, 0x13, 0x1A };
static CVI_U8 data_gc9503cv_25[] = { 0x7B, 0x13, 0x1A };

static CVI_U8 data_gc9503cv_26[] = { 0x6D, 0x1E, 0x00, 0x09, 0x0F, 0x01, 0x1F, 0x1E, 0x1E, 0x1E,
	0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E,
	0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1F, 0x08, 0x10,
	0x0A, 0x00, 0x1E };
static CVI_U8 data_gc9503cv_27[] = {
	0x60, 0x18, 0x08, 0x7A, 0x7A, 0x18, 0x02, 0x7A, 0x7A };
static CVI_U8 data_gc9503cv_28[] = {
	0x63, 0x18, 0x02, 0x7A, 0x7A, 0x18, 0x07 ,0x7A, 0x7A };
static CVI_U8 data_gc9503cv_29[] = {
	0x64, 0x18, 0x07, 0x03, 0x21, 0x03, 0x03, 0x18, 0x06, 0x03, 0x22, 0x03, 0x03, 0x7A, 0x7A, 0x7A, 0x7A };
static CVI_U8 data_gc9503cv_30[] = {
	0x67, 0x18, 0x05, 0x03, 0x23, 0x03, 0x03, 0x18, 0x04, 0x03, 0x24, 0x03, 0x03, 0x7A, 0x7A, 0x7A, 0x7A };
static CVI_U8 data_gc9503cv_31[] = {
	0x69, 0X04, 0X22, 0X14, 0X22, 0X44, 0X22, 0X08 };

static CVI_U8 data_gc9503cv_32[] = { 0xD1, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x72, 0x00, 0x97, 0x00,
	0xB1, 0x00, 0xE1, 0x01, 0x03, 0x01, 0x3D, 0x01, 0x6C, 0x01,
	0xB7, 0x01, 0xF1, 0x02, 0x4F, 0x02, 0x98, 0x02, 0x99, 0x02,
	0xDB, 0x03, 0x25, 0x03, 0x51, 0x03, 0x8D, 0x03, 0xAA, 0x03,
	0xC3, 0x03, 0xD1, 0x03, 0xE4, 0x03, 0xEE, 0x03, 0xF9, 0x03,
	0xFC, 0x03, 0xFF };
static CVI_U8 data_gc9503cv_33[] = { 0xD2, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x72, 0x00, 0x97, 0x00,
	0xB1, 0x00, 0xE1, 0x01, 0x03, 0x01, 0x3D, 0x01, 0x6C, 0x01,
	0xB7, 0x01, 0xF1, 0x02, 0x4F, 0x02, 0x98, 0x02, 0x99, 0x02,
	0xDB, 0x03, 0x25, 0x03, 0x51, 0x03, 0x8D, 0x03, 0xAA, 0x03,
	0xC3, 0x03, 0xD1, 0x03, 0xE4, 0x03, 0xEE, 0x03, 0xF9, 0x03,
	0xFC, 0x03, 0xFF };
static CVI_U8 data_gc9503cv_34[] = { 0xD3, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x72, 0x00, 0x97, 0x00,
	0xB1, 0x00, 0xE1, 0x01, 0x03, 0x01, 0x3D, 0x01, 0x6C, 0x01,
	0xB7, 0x01, 0xF1, 0x02, 0x4F, 0x02, 0x98, 0x02, 0x99, 0x02,
	0xDB, 0x03, 0x25, 0x03, 0x51, 0x03, 0x8D, 0x03, 0xAA, 0x03,
	0xC3, 0x03, 0xD1, 0x03, 0xE4, 0x03, 0xEE, 0x03, 0xF9, 0x03,
	0xFC, 0x03, 0xFF };
static CVI_U8 data_gc9503cv_35[] = { 0xD4, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x72, 0x00, 0x97, 0x00,
	0xB1, 0x00, 0xE1, 0x01, 0x03, 0x01, 0x3D, 0x01, 0x6C, 0x01,
	0xB7, 0x01, 0xF1, 0x02, 0x4F, 0x02, 0x98, 0x02, 0x99, 0x02,
	0xDB, 0x03, 0x25, 0x03, 0x51, 0x03, 0x8D, 0x03, 0xAA, 0x03,
	0xC3, 0x03, 0xD1, 0x03, 0xE4, 0x03, 0xEE, 0x03, 0xF9, 0x03,
	0xFC, 0x03, 0xFF};
static CVI_U8 data_gc9503cv_36[] = { 0xD5, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x72, 0x00, 0x97, 0x00,
	0xB1, 0x00, 0xE1, 0x01, 0x03, 0x01, 0x3D, 0x01, 0x6C, 0x01,
	0xB7, 0x01, 0xF1, 0x02, 0x4F, 0x02, 0x98, 0x02, 0x99, 0x02,
	0xDB, 0x03, 0x25, 0x03, 0x51, 0x03, 0x8D, 0x03, 0xAA, 0x03,
	0xC3, 0x03, 0xD1, 0x03, 0xE4, 0x03, 0xEE, 0x03, 0xF9, 0x03,
	0xFC, 0x03, 0xFF};
static CVI_U8 data_gc9503cv_37[] = { 0xD6, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x72, 0x00, 0x97, 0x00,
	0xB1, 0x00, 0xE1, 0x01, 0x03, 0x01, 0x3D, 0x01, 0x6C, 0x01,
	0xB7, 0x01, 0xF1, 0x02, 0x4F, 0x02, 0x98, 0x02, 0x99, 0x02,
	0xDB, 0x03, 0x25, 0x03, 0x51, 0x03, 0x8D, 0x03, 0xAA, 0x03,
	0xC3, 0x03, 0xD1, 0x03, 0xE4, 0x03, 0xEE, 0x03, 0xF9, 0x03,
	0xFC, 0x03, 0xFF};
static CVI_U8 data_gc9503cv_38[] = { 0x11 };
static CVI_U8 data_gc9503cv_39[] = { 0x29 };

const struct dsc_instr dsi_init_cmds_gc9503cv_480x800[] = {
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_0), .data = data_gc9503cv_0 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_1), .data = data_gc9503cv_1 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_2), .data = data_gc9503cv_2 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_3), .data = data_gc9503cv_3 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_4), .data = data_gc9503cv_4 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_5), .data = data_gc9503cv_5 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_6), .data = data_gc9503cv_6 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_7), .data = data_gc9503cv_7 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_8), .data = data_gc9503cv_8 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_9), .data = data_gc9503cv_9 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_10), .data = data_gc9503cv_10 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_11), .data = data_gc9503cv_11 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_12), .data = data_gc9503cv_12 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_13), .data = data_gc9503cv_13 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_14), .data = data_gc9503cv_14 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_15), .data = data_gc9503cv_15 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_16), .data = data_gc9503cv_16 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_17), .data = data_gc9503cv_17 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_18), .data = data_gc9503cv_18 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_19), .data = data_gc9503cv_19 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_20), .data = data_gc9503cv_20 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_21), .data = data_gc9503cv_21 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_22), .data = data_gc9503cv_22 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_gc9503cv_23), .data = data_gc9503cv_23 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_24), .data = data_gc9503cv_24 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_25), .data = data_gc9503cv_25 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_26), .data = data_gc9503cv_26 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_27), .data = data_gc9503cv_27 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_28), .data = data_gc9503cv_28 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_29), .data = data_gc9503cv_29 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_30), .data = data_gc9503cv_30 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_31), .data = data_gc9503cv_31 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_32), .data = data_gc9503cv_32 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_33), .data = data_gc9503cv_33 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_34), .data = data_gc9503cv_34 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_35), .data = data_gc9503cv_35 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_36), .data = data_gc9503cv_36 },
	{.delay = 0, .data_type = 0x39, .size = sizeof(data_gc9503cv_37), .data = data_gc9503cv_37 },
	{.delay = 200, .data_type = 0x05, .size = sizeof(data_gc9503cv_38), .data = data_gc9503cv_38 },
	{.delay = 0, .data_type = 0x05, .size = sizeof(data_gc9503cv_39), .data = data_gc9503cv_39 },
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#else
#error "MIPI_TX_PARAM multi-delcaration!!"
#endif // _MIPI_TX_PARAM_GC9503CV_H_
