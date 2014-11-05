/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef ILI9341_H
#define ILI9341_H

// ILI9341 commands
#define ILI9341_CMD_NOP                 0x00    /**< No operation.*/
#define ILI9341_CMD_RESET               0x01    /**< Software reset.*/
#define ILI9341_GET_ID_INFO             0x04    /**< Get ID information.*/
#define ILI9341_GET_STATUS              0x09    /**< Get status.*/
#define ILI9341_GET_PWR_MODE            0x0A    /**< Get power mode.*/
#define ILI9341_GET_MADCTL              0x0B    /**< Get MADCTL.*/
#define ILI9341_GET_PIX_FMT             0x0C    /**< Get pixel format.*/
#define ILI9341_GET_IMG_FMT             0x0D    /**< Get image format.*/
#define ILI9341_GET_SIG_MODE            0x0E    /**< Get signal mode.*/
#define ILI9341_GET_SELF_DIAG           0x0F    /**< Get self-diagnostics.*/
#define ILI9341_CMD_SLEEP_ON            0x10    /**< Enter sleep mode.*/
#define ILI9341_CMD_SLEEP_OFF           0x11    /**< Exist sleep mode.*/
#define ILI9341_CMD_PARTIAL_ON          0x12    /**< Enter partial mode.*/
#define ILI9341_CMD_PARTIAL_OFF         0x13    /**< Exit partial mode.*/
#define ILI9341_CMD_INVERT_ON           0x20    /**< Enter inverted mode.*/
#define ILI9341_CMD_INVERT_OFF          0x21    /**< Exit inverted mode.*/
#define ILI9341_SET_GAMMA               0x26    /**< Set gamma params.*/
#define ILI9341_CMD_DISPLAY_OFF         0x28    /**< Disable display.*/
#define ILI9341_CMD_DISPLAY_ON          0x29    /**< Enable display.*/
#define ILI9341_SET_COL_ADDR            0x2A    /**< Set column address.*/
#define ILI9341_SET_PAGE_ADDR           0x2B    /**< Set page address.*/
#define ILI9341_SET_MEM                 0x2C    /**< Set memory.*/
#define ILI9341_SET_COLOR               0x2D    /**< Set color.*/
#define ILI9341_GET_MEM                 0x2E    /**< Get memory.*/
#define ILI9341_SET_PARTIAL_AREA        0x30    /**< Set partial area.*/
#define ILI9341_SET_VSCROLL             0x33    /**< Set vertical scroll def.*/
#define ILI9341_CMD_TEARING_ON          0x34    /**< Tearing line enabled.*/
#define ILI9341_CMD_TEARING_OFF         0x35    /**< Tearing line disabled.*/
#define ILI9341_SET_MEM_ACS_CTL         0x36    /**< Set mem access ctl.*/
#define ILI9341_SET_VSCROLL_ADDR        0x37    /**< Set vscroll start addr.*/
#define ILI9341_CMD_IDLE_OFF            0x38    /**< Exit idle mode.*/
#define ILI9341_CMD_IDLE_ON             0x39    /**< Enter idle mode.*/
#define ILI9341_SET_PIX_FMT             0x3A    /**< Set pixel format.*/
#define ILI9341_SET_MEM_CONT            0x3C    /**< Set memory continue.*/
#define ILI9341_GET_MEM_CONT            0x3E    /**< Get memory continue.*/
#define ILI9341_SET_TEAR_SCANLINE       0x44    /**< Set tearing scanline.*/
#define ILI9341_GET_TEAR_SCANLINE       0x45    /**< Get tearing scanline.*/
#define ILI9341_SET_BRIGHTNESS          0x51    /**< Set brightness.*/
#define ILI9341_GET_BRIGHTNESS          0x52    /**< Get brightness.*/
#define ILI9341_SET_DISPLAY_CTL         0x53    /**< Set display ctl.*/
#define ILI9341_GET_DISPLAY_CTL         0x54    /**< Get display ctl.*/
#define ILI9341_SET_CABC                0x55    /**< Set CABC.*/
#define ILI9341_GET_CABC                0x56    /**< Get CABC.*/
#define ILI9341_SET_CABC_MIN            0x5E    /**< Set CABC min.*/
#define ILI9341_GET_CABC_MIN            0x5F    /**< Set CABC max.*/
#define ILI9341_GET_ID1                 0xDA    /**< Get ID1.*/
#define ILI9341_GET_ID2                 0xDB    /**< Get ID2.*/
#define ILI9341_GET_ID3                 0xDC    /**< Get ID3.*/

// ILI9341 extended commands
#define ILI9341_SET_RGB_IF_SIG_CTL      0xB0    /**< RGB IF signal ctl.*/
#define ILI9341_SET_FRAME_CTL_NORMAL    0xB1    /**< Set frame ctl (normal).*/
#define ILI9341_SET_FRAME_CTL_IDLE      0xB2    /**< Set frame ctl (idle).*/
#define ILI9341_SET_FRAME_CTL_PARTIAL   0xB3    /**< Set frame ctl (partial).*/
#define ILI9341_SET_INVERSION_CTL       0xB4    /**< Set inversion ctl.*/
#define ILI9341_SET_BLANKING_PORCH_CTL  0xB5    /**< Set blanking porch ctl.*/
#define ILI9341_SET_FUNCTION_CTL        0xB6    /**< Set function ctl.*/
#define ILI9341_SET_ENTRY_MODE          0xB7    /**< Set entry mode.*/
#define ILI9341_SET_LIGHT_CTL_1         0xB8    /**< Set backlight ctl 1.*/
#define ILI9341_SET_LIGHT_CTL_2         0xB9    /**< Set backlight ctl 2.*/
#define ILI9341_SET_LIGHT_CTL_3         0xBA    /**< Set backlight ctl 3.*/
#define ILI9341_SET_LIGHT_CTL_4         0xBB    /**< Set backlight ctl 4.*/
#define ILI9341_SET_LIGHT_CTL_5         0xBC    /**< Set backlight ctl 5.*/
#define ILI9341_SET_LIGHT_CTL_7         0xBE    /**< Set backlight ctl 7.*/
#define ILI9341_SET_LIGHT_CTL_8         0xBF    /**< Set backlight ctl 8.*/
#define ILI9341_SET_POWER_CTL_1         0xC0    /**< Set power ctl 1.*/
#define ILI9341_SET_POWER_CTL_2         0xC1    /**< Set power ctl 2.*/
#define ILI9341_SET_VCOM_CTL_1          0xC5    /**< Set VCOM ctl 1.*/
#define ILI9341_SET_VCOM_CTL_2          0xC6    /**< Set VCOM ctl 2.*/
#define ILI9341_SET_NVMEM               0xD0    /**< Set NVMEM data.*/
#define ILI9341_GET_NVMEM_KEY           0xD1    /**< Get NVMEM protect key.*/
#define ILI9341_GET_NVMEM_STATUS        0xD2    /**< Get NVMEM status.*/
#define ILI9341_GET_ID4                 0xD3    /**< Get ID4.*/
#define ILI9341_SET_PGAMMA              0xE0    /**< Set positive gamma.*/
#define ILI9341_SET_NGAMMA              0xE1    /**< Set negative gamma.*/
#define ILI9341_SET_DGAMMA_CTL_1        0xE2    /**< Set digital gamma ctl 1.*/
#define ILI9341_SET_DGAMMA_CTL_2        0xE3    /**< Set digital gamma ctl 2.*/
#define ILI9341_SET_IF_CTL              0xF6    /**< Set interface control.*/

// ILI9341 interface modes
#define ILI9341_IM_3LSI_1               0x5     /**< 3-line serial, mode 1.*/
#define ILI9341_IM_3LSI_2               0xD     /**< 3-line serial, mode 2.*/
#define ILI9341_IM_4LSI_1               0x6     /**< 4-line serial, mode 1.*/
#define ILI9341_IM_4LSI_2               0xE     /**< 4-line serial, mode 2.*/

// ILI9341 command params (little endian)

#pragma pack(push, 1)

typedef union {
  struct ILI9341ParamBits_GET_ID_INFO {
    uint8_t reserved_;
    uint8_t ID1;
    uint8_t ID2;
    uint8_t ID3;
  } bits;
  uint8_t   bytes[4];
} ILI9341Params_GET_ID_INFO;

typedef union {
  struct ILI9341ParamBits_GET_STATUS {
    unsigned  _reserved_1       : 5; /* D[ 4: 0] */
    unsigned  tearing_mode      : 1; /* D[ 5] */
    unsigned  gamma_curve       : 3; /* D[ 8: 6] */
    unsigned  tearing           : 1; /* D[ 9] */
    unsigned  display           : 1; /* D[10] */
    unsigned  all_on            : 1; /* D[11] */
    unsigned  all_off           : 1; /* D[12] */
    unsigned  invert            : 1; /* D[13] */
    unsigned  _reserved_2       : 1; /* D[14] */
    unsigned  vscroll           : 1; /* D[15] */
    unsigned  normal            : 1; /* D[16] */
    unsigned  sleep             : 1; /* D[17] */
    unsigned  partial           : 1; /* D[18] */
    unsigned  idle              : 1; /* D[19] */
    unsigned  pixel_format      : 3; /* D[22:20] */
    unsigned  _reserved_3       : 2; /* D[24:23] */
    unsigned  hrefr_rtl_nltr    : 1; /* D[25] */
    unsigned  bgr_nrgb          : 1; /* D[26] */
    unsigned  vrefr_btt_nttb    : 1; /* D[27] */
    unsigned  transpose         : 1; /* D[28] */
    unsigned  coladr_rtl_nltr   : 1; /* D[29] */
    unsigned  rowadr_btt_nttb   : 1; /* D[30] */
    unsigned  booster           : 1; /* D[31] */
  } bits;
  uint8_t   bytes[4];
} ILI9341Params_GET_STATUS;

typedef union {
  struct ILI9341ParamBits_GET_PWR_MODE {
    unsigned  _reserved_1       : 2; /* D[1:0] */
    unsigned  display           : 1; /* D[2] */
    unsigned  normal            : 1; /* D[3] */
    unsigned  sleep             : 1; /* D[4] */
    unsigned  partial           : 1; /* D[5] */
    unsigned  idle              : 1; /* D[6] */
    unsigned  booster           : 1; /* D[7] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_GET_PWR_MODE;

typedef union {
  struct ILI9341ParamBits_GET_MADCTL {
    unsigned  _reserved_1       : 2; /* D[1:0] */
    unsigned  refr_rtl_nltr     : 1; /* D[2] */
    unsigned  bgr_nrgb          : 1; /* D[3] */
    unsigned  refr_btt_nttb     : 1; /* D[4] */
    unsigned  invert            : 1; /* D[5] */
    unsigned  rtl_nltr          : 1; /* D[6] */
    unsigned  btt_nttb          : 1; /* D[7] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_GET_MADCTL;

typedef union {
  struct ILI9341ParamBits_GET_PIX_FMT {
    unsigned  DBI               : 3; /* D[2:0] */
    unsigned  _reserved_1       : 1; /* D[3] */
    unsigned  DPI               : 3; /* D[6:4] */
    unsigned  RIM               : 1; /* D[7] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_GET_PIX_FMT;

typedef union {
  struct ILI9341ParamBits_GET_IMG_FMT {
    unsigned  gamma_curve       : 3; /* D[2:0] */
    unsigned  _reserved_1       : 5; /* D[7:3] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_GET_IMG_FMT;

typedef union {
  struct ILI9341ParamBits_GET_SIG_MODE {
    unsigned  _reserved_1       : 2; /* D[1:0] */
    unsigned  data_enable       : 1; /* D[2] */
    unsigned  pixel_clock       : 1; /* D[3] */
    unsigned  vsync             : 1; /* D[4] */
    unsigned  hsync             : 1; /* D[5] */
    unsigned  tearing_mode      : 1; /* D[6] */
    unsigned  tearing           : 1; /* D[7] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_GET_SIG_MODE;

typedef union {
  struct ILI9341ParamBits_GET_SELF_DIAG {
    unsigned  _reserved_1       : 6; /* D[5:0] */
    unsigned  func_err          : 1; /* D[6] */
    unsigned  reg_err           : 1; /* D[7] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_GET_SELF_DIAG;

typedef union {
  struct ILI9341ParamBits_SET_GAMMA {
    uint8_t   gamma_curve;      /* D[7:0] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_SET_GAMMA;

typedef union {
  struct ILI9341ParamBits_SET_COL_ADDR {
    uint8_t   SC_15_8;          /* D[ 7: 0] */
    uint8_t   SC_7_0;           /* D[15: 8] */
    uint8_t   EC_15_8;          /* D[23:16] */
    uint8_t   EC_7_0;           /* D[31:24] */
  } bits;
  uint8_t   bytes[4];
} ILI9341Params_SET_COL_ADDR;

typedef union {
  struct ILI9341ParamBits_SET_PAGE_ADDR {
    uint8_t   SP_15_8;          /* D[ 7: 0] */
    uint8_t   SP_7_0;           /* D[15: 8] */
    uint8_t   EP_15_8;          /* D[23:16] */
    uint8_t   EP_7_0;           /* D[31:24] */
  } bits;
  uint8_t   bytes[4];
} ILI9341Params_SET_PAGE_ADDR;

typedef union {
  struct ILI9341ParamBits_SET_PARTIAL_AREA {
    uint8_t   SR_15_8;          /* D[ 7: 0] */
    uint8_t   SR_7_0;           /* D[15: 8] */
    uint8_t   ER_15_8;          /* D[23:16] */
    uint8_t   ER_7_0;           /* D[31:24] */
  } bits;
  uint8_t   bytes[4];
} ILI9341Params_SET_PARTIAL_AREA;

typedef union {
  struct ILI9341ParamBits_SET_VSCROLL {
    uint8_t   TFA_15_8;         /* D[ 7: 0] */
    uint8_t   TFA_7_0;          /* D[15: 8] */
    uint8_t   VSA_15_8;         /* D[23:16] */
    uint8_t   VSA_7_0;          /* D[31:24] */
    uint8_t   BFA_15_8;         /* D[39:32] */
    uint8_t   BFA_7_0;          /* D[47:40] */
  } bits;
  uint8_t   bytes[6];
} ILI9341Params_SET_VSCROLL;

typedef union {
  struct ILI9341ParamBits_CMD_TEARING_ON {
    unsigned  M                 : 1; /* D[0] */
    unsigned  _reserved_1       : 7; /* D[7:1] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_CMD_TEARING_ON;

typedef union {
  struct ILI9341ParamBits_SET_MEM_ACS_CTL {
    unsigned  _reserved_1       : 2; /* D[1:0] */
    unsigned  MH                : 1; /* D[2] */
    unsigned  BGR               : 1; /* D[3] */
    unsigned  ML                : 1; /* D[4] */
    unsigned  MV                : 1; /* D[5] */
    unsigned  MX                : 1; /* D[6] */
    unsigned  MY                : 1; /* D[7] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_SET_MEM_ACS_CTL;

typedef union {
  struct ILI9341ParamBits_SET_VSCROLL_ADDR {
    uint8_t   VSP_15_8;         /* D[ 7: 0] */
    uint8_t   VSP_7_0;          /* D[15: 8] */
  } bits;
  uint8_t   bytes[2];
} ILI9341Params_SET_VSCROLL_ADDR;

typedef union {
  struct ILI9341ParamBits_SET_PIX_FMT {
    unsigned  DBI               : 3; /* D[2:0] */
    unsigned  _reserved_1       : 1; /* D[3] */
    unsigned  DPI               : 3; /* D[4:6] */
    unsigned  _reserved_2       : 1; /* D[7] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_SET_PIX_FMT;

typedef union {
  struct ILI9341ParamBits_SET_TEAR_SCANLINE {
    uint8_t   STS_8;            /* D[ 7: 0] */
    uint8_t   STS_7_0;          /* D[15: 8] */
  } bits;
  uint8_t   bytes[4];
} ILI9341Params_SET_TEAR_SCANLINE;

typedef union {
  struct ILI9341ParamBits_GET_TEAR_SCANLINE {
    uint8_t   GTS_9_8;          /* D[ 7: 0] */
    uint8_t   GTS_7_0;          /* D[15: 8] */
  } bits;
  uint8_t   bytes[2];
} ILI9341Params_GET_TEAR_SCANLINE;

typedef union {
  struct ILI9341ParamBits_SET_BRIGHTNESS {
    uint8_t   DBV;              /* D[7:0] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_SET_BRIGHTNESS;

typedef union {
  struct ILI9341ParamBits_GET_BRIGHTNESS {
    uint8_t   DBV;              /* D[7:0] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_GET_BRIGHTNESS;

typedef union {
  struct ILI9341ParamBits_SET_DISPLAY_CTL {
    unsigned  _reserved_1       : 2; /* D[1:0] */
    unsigned  BL                : 1; /* D[2] */
    unsigned  DD                : 1; /* D[3] */
    unsigned  _reserved_2       : 1; /* D[4] */
    unsigned  BCTRL             : 1; /* D[5] */
    unsigned  _reserved_3       : 1; /* D[7:6] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_SET_DISPLAY_CTL;

typedef union {
  struct ILI9341ParamBits_GET_DISPLAY_CTL {
    unsigned  _reserved_1       : 2; /* D[1:0] */
    unsigned  BL                : 1; /* D[2] */
    unsigned  DD                : 1; /* D[3] */
    unsigned  _reserved_2       : 1; /* D[4] */
    unsigned  BCTRL             : 1; /* D[5] */
    unsigned  _reserved_3       : 1; /* D[7:6] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_GET_DISPLAY_CTL;

typedef union {
  struct ILI9341ParamBits_SET_CABC {
    unsigned  C                 : 2; /* D[1:0] */
    unsigned  _reserved_1       : 6; /* D[7:2] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_SET_CABC;

typedef union {
  struct ILI9341ParamBits_GET_CABC {
    unsigned  C                 : 2; /* D[1:0] */
    unsigned  _reserved_1       : 6; /* D[7:2] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_GET_CABC;

typedef union {
  struct ILI9341ParamBits_SET_CABC_MIN {
    uint8_t   CMB;              /* D[7:0] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_SET_CABC_MIN;

typedef union {
  struct ILI9341ParamBits_GET_CABC_MIN {
    uint8_t   CMB;              /* D[7:0] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_GET_CABC_MIN;

#if 0 /* TODO: Extended command structs.*/

typedef union {
  struct ILI9341ParamBits {
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_;

typedef union {
  struct ILI9341ParamBits {
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
    unsigned    : 1; /* D[] */
  } bits;
  uint8_t   bytes[1];
} ILI9341Params_;

#endif /*0*/

#pragma pack(pop)

#endif /* ILI9341_H */
