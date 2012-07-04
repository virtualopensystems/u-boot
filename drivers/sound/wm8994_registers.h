/*
 * wm8994_registers.h -- Register definitions for WM8994
 * This file is taken from kernel and used as it is
 *
 * Copyright 2009 Wolfson Microelectronics PLC.
 *
 * Author: Mark Brown <broonie@opensource.wolfsonmicro.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#ifndef __MFD_WM8994_REGISTERS_H__
#define __MFD_WM8994_REGISTERS_H__

/*
 * Register values.
 */
#define WM8994_SOFTWARE_RESET                   0x00
#define WM8994_POWER_MANAGEMENT_1               0x01
#define WM8994_POWER_MANAGEMENT_2               0x02
#define WM8994_POWER_MANAGEMENT_3               0x03
#define WM8994_POWER_MANAGEMENT_4               0x04
#define WM8994_POWER_MANAGEMENT_5               0x05
#define WM8994_POWER_MANAGEMENT_6               0x06
#define WM8994_INPUT_MIXER_1                    0x15
#define WM8994_LEFT_LINE_INPUT_1_2_VOLUME       0x18
#define WM8994_LEFT_LINE_INPUT_3_4_VOLUME       0x19
#define WM8994_RIGHT_LINE_INPUT_1_2_VOLUME      0x1A
#define WM8994_RIGHT_LINE_INPUT_3_4_VOLUME      0x1B
#define WM8994_LEFT_OUTPUT_VOLUME               0x1C
#define WM8994_RIGHT_OUTPUT_VOLUME              0x1D
#define WM8994_LINE_OUTPUTS_VOLUME              0x1E
#define WM8994_HPOUT2_VOLUME                    0x1F
#define WM8994_LEFT_OPGA_VOLUME                 0x20
#define WM8994_RIGHT_OPGA_VOLUME                0x21
#define WM8994_SPKMIXL_ATTENUATION              0x22
#define WM8994_SPKMIXR_ATTENUATION              0x23
#define WM8994_SPKOUT_MIXERS                    0x24
#define WM8994_CLASSD                           0x25
#define WM8994_SPEAKER_VOLUME_LEFT              0x26
#define WM8994_SPEAKER_VOLUME_RIGHT             0x27
#define WM8994_INPUT_MIXER_2                    0x28
#define WM8994_INPUT_MIXER_3                    0x29
#define WM8994_INPUT_MIXER_4                    0x2A
#define WM8994_INPUT_MIXER_5                    0x2B
#define WM8994_INPUT_MIXER_6                    0x2C
#define WM8994_OUTPUT_MIXER_1                   0x2D
#define WM8994_OUTPUT_MIXER_2                   0x2E
#define WM8994_OUTPUT_MIXER_3                   0x2F
#define WM8994_OUTPUT_MIXER_4                   0x30
#define WM8994_OUTPUT_MIXER_5                   0x31
#define WM8994_OUTPUT_MIXER_6                   0x32
#define WM8994_HPOUT2_MIXER                     0x33
#define WM8994_LINE_MIXER_1                     0x34
#define WM8994_LINE_MIXER_2                     0x35
#define WM8994_SPEAKER_MIXER                    0x36
#define WM8994_ADDITIONAL_CONTROL               0x37
#define WM8994_ANTIPOP_1                        0x38
#define WM8994_ANTIPOP_2                        0x39
#define WM8994_MICBIAS                          0x3A
#define WM8994_LDO_1                            0x3B
#define WM8994_LDO_2                            0x3C
#define WM8958_MICBIAS1                         0x3D
#define WM8958_MICBIAS2                         0x3E
#define WM8994_CHARGE_PUMP_1                    0x4C
#define WM8958_CHARGE_PUMP_2                    0x4D
#define WM8994_CLASS_W_1                        0x51
#define WM8994_DC_SERVO_1                       0x54
#define WM8994_DC_SERVO_2                       0x55
#define WM8994_DC_SERVO_4                       0x57
#define WM8994_DC_SERVO_READBACK                0x58
#define WM8994_DC_SERVO_4E                      0x59
#define WM8994_ANALOGUE_HP_1                    0x60
#define WM8958_MIC_DETECT_1                     0xD0
#define WM8958_MIC_DETECT_2                     0xD1
#define WM8958_MIC_DETECT_3                     0xD2
#define WM8994_CHIP_REVISION                    0x100
#define WM8994_CONTROL_INTERFACE                0x101
#define WM8994_WRITE_SEQUENCER_CTRL_1           0x110
#define WM8994_WRITE_SEQUENCER_CTRL_2           0x111
#define WM8994_AIF1_CLOCKING_1                  0x200
#define WM8994_AIF1_CLOCKING_2                  0x201
#define WM8994_AIF2_CLOCKING_1                  0x204
#define WM8994_AIF2_CLOCKING_2                  0x205
#define WM8994_CLOCKING_1                       0x208
#define WM8994_CLOCKING_2                       0x209
#define WM8994_AIF1_RATE                        0x210
#define WM8994_AIF2_RATE                        0x211
#define WM8994_RATE_STATUS                      0x212
#define WM8994_FLL1_CONTROL_1                   0x220
#define WM8994_FLL1_CONTROL_2                   0x221
#define WM8994_FLL1_CONTROL_3                   0x222
#define WM8994_FLL1_CONTROL_4                   0x223
#define WM8994_FLL1_CONTROL_5                   0x224
#define WM8958_FLL1_EFS_1                       0x226
#define WM8958_FLL1_EFS_2                       0x227
#define WM8994_FLL2_CONTROL_1                   0x240
#define WM8994_FLL2_CONTROL_2                   0x241
#define WM8994_FLL2_CONTROL_3                   0x242
#define WM8994_FLL2_CONTROL_4                   0x243
#define WM8994_FLL2_CONTROL_5                   0x244
#define WM8958_FLL2_EFS_1                       0x246
#define WM8958_FLL2_EFS_2                       0x247
#define WM8994_AIF1_CONTROL_1                   0x300
#define WM8994_AIF1_CONTROL_2                   0x301
#define WM8994_AIF1_MASTER_SLAVE                0x302
#define WM8994_AIF1_BCLK                        0x303
#define WM8994_AIF1ADC_LRCLK                    0x304
#define WM8994_AIF1DAC_LRCLK                    0x305
#define WM8994_AIF1DAC_DATA                     0x306
#define WM8994_AIF1ADC_DATA                     0x307
#define WM8994_AIF2_CONTROL_1                   0x310
#define WM8994_AIF2_CONTROL_2                   0x311
#define WM8994_AIF2_MASTER_SLAVE                0x312
#define WM8994_AIF2_BCLK                        0x313
#define WM8994_AIF2ADC_LRCLK                    0x314
#define WM8994_AIF2DAC_LRCLK                    0x315
#define WM8994_AIF2DAC_DATA                     0x316
#define WM8994_AIF2ADC_DATA                     0x317
#define WM1811_AIF2TX_CONTROL                   0x318
#define WM8958_AIF3_CONTROL_1                   0x320
#define WM8958_AIF3_CONTROL_2                   0x321
#define WM8958_AIF3DAC_DATA                     0x322
#define WM8958_AIF3ADC_DATA                     0x323
#define WM8994_AIF1_ADC1_LEFT_VOLUME            0x400
#define WM8994_AIF1_ADC1_RIGHT_VOLUME           0x401
#define WM8994_AIF1_DAC1_LEFT_VOLUME            0x402
#define WM8994_AIF1_DAC1_RIGHT_VOLUME           0x403
#define WM8994_AIF1_ADC2_LEFT_VOLUME            0x404
#define WM8994_AIF1_ADC2_RIGHT_VOLUME           0x405
#define WM8994_AIF1_DAC2_LEFT_VOLUME            0x406
#define WM8994_AIF1_DAC2_RIGHT_VOLUME           0x407
#define WM8994_AIF1_ADC1_FILTERS                0x410
#define WM8994_AIF1_ADC2_FILTERS                0x411
#define WM8994_AIF1_DAC1_FILTERS_1              0x420
#define WM8994_AIF1_DAC1_FILTERS_2              0x421
#define WM8994_AIF1_DAC2_FILTERS_1              0x422
#define WM8994_AIF1_DAC2_FILTERS_2              0x423
#define WM8958_AIF1_DAC1_NOISE_GATE             0x430
#define WM8958_AIF1_DAC2_NOISE_GATE             0x431
#define WM8994_AIF1_DRC1_1                      0x440
#define WM8994_AIF1_DRC1_2                      0x441
#define WM8994_AIF1_DRC1_3                      0x442
#define WM8994_AIF1_DRC1_4                      0x443
#define WM8994_AIF1_DRC1_5                      0x444
#define WM8994_AIF1_DRC2_1                      0x450
#define WM8994_AIF1_DRC2_2                      0x451
#define WM8994_AIF1_DRC2_3                      0x452
#define WM8994_AIF1_DRC2_4                      0x453
#define WM8994_AIF1_DRC2_5                      0x454
#define WM8994_AIF1_DAC1_EQ_GAINS_1             0x480
#define WM8994_AIF1_DAC1_EQ_GAINS_2             0x481
#define WM8994_AIF1_DAC1_EQ_BAND_1_A            0x482
#define WM8994_AIF1_DAC1_EQ_BAND_1_B            0x483
#define WM8994_AIF1_DAC1_EQ_BAND_1_PG           0x484
#define WM8994_AIF1_DAC1_EQ_BAND_2_A            0x485
#define WM8994_AIF1_DAC1_EQ_BAND_2_B            0x486
#define WM8994_AIF1_DAC1_EQ_BAND_2_C            0x487
#define WM8994_AIF1_DAC1_EQ_BAND_2_PG           0x488
#define WM8994_AIF1_DAC1_EQ_BAND_3_A            0x489
#define WM8994_AIF1_DAC1_EQ_BAND_3_B            0x48A
#define WM8994_AIF1_DAC1_EQ_BAND_3_C            0x48B
#define WM8994_AIF1_DAC1_EQ_BAND_3_PG           0x48C
#define WM8994_AIF1_DAC1_EQ_BAND_4_A            0x48D
#define WM8994_AIF1_DAC1_EQ_BAND_4_B            0x48E
#define WM8994_AIF1_DAC1_EQ_BAND_4_C            0x48F
#define WM8994_AIF1_DAC1_EQ_BAND_4_PG           0x490
#define WM8994_AIF1_DAC1_EQ_BAND_5_A            0x491
#define WM8994_AIF1_DAC1_EQ_BAND_5_B            0x492
#define WM8994_AIF1_DAC1_EQ_BAND_5_PG           0x493
#define WM8994_AIF1_DAC1_EQ_BAND_1_C            0x494
#define WM8994_AIF1_DAC2_EQ_GAINS_1             0x4A0
#define WM8994_AIF1_DAC2_EQ_GAINS_2             0x4A1
#define WM8994_AIF1_DAC2_EQ_BAND_1_A            0x4A2
#define WM8994_AIF1_DAC2_EQ_BAND_1_B            0x4A3
#define WM8994_AIF1_DAC2_EQ_BAND_1_PG           0x4A4
#define WM8994_AIF1_DAC2_EQ_BAND_2_A            0x4A5
#define WM8994_AIF1_DAC2_EQ_BAND_2_B            0x4A6
#define WM8994_AIF1_DAC2_EQ_BAND_2_C            0x4A7
#define WM8994_AIF1_DAC2_EQ_BAND_2_PG           0x4A8
#define WM8994_AIF1_DAC2_EQ_BAND_3_A            0x4A9
#define WM8994_AIF1_DAC2_EQ_BAND_3_B            0x4AA
#define WM8994_AIF1_DAC2_EQ_BAND_3_C            0x4AB
#define WM8994_AIF1_DAC2_EQ_BAND_3_PG           0x4AC
#define WM8994_AIF1_DAC2_EQ_BAND_4_A            0x4AD
#define WM8994_AIF1_DAC2_EQ_BAND_4_B            0x4AE
#define WM8994_AIF1_DAC2_EQ_BAND_4_C            0x4AF
#define WM8994_AIF1_DAC2_EQ_BAND_4_PG           0x4B0
#define WM8994_AIF1_DAC2_EQ_BAND_5_A            0x4B1
#define WM8994_AIF1_DAC2_EQ_BAND_5_B            0x4B2
#define WM8994_AIF1_DAC2_EQ_BAND_5_PG           0x4B3
#define WM8994_AIF1_DAC2_EQ_BAND_1_C            0x4B4
#define WM8994_AIF2_ADC_LEFT_VOLUME             0x500
#define WM8994_AIF2_ADC_RIGHT_VOLUME            0x501
#define WM8994_AIF2_DAC_LEFT_VOLUME             0x502
#define WM8994_AIF2_DAC_RIGHT_VOLUME            0x503
#define WM8994_AIF2_ADC_FILTERS                 0x510
#define WM8994_AIF2_DAC_FILTERS_1               0x520
#define WM8994_AIF2_DAC_FILTERS_2               0x521
#define WM8958_AIF2_DAC_NOISE_GATE              0x530
#define WM8994_AIF2_DRC_1                       0x540
#define WM8994_AIF2_DRC_2                       0x541
#define WM8994_AIF2_DRC_3                       0x542
#define WM8994_AIF2_DRC_4                       0x543
#define WM8994_AIF2_DRC_5                       0x544
#define WM8994_AIF2_EQ_GAINS_1                  0x580
#define WM8994_AIF2_EQ_GAINS_2                  0x581
#define WM8994_AIF2_EQ_BAND_1_A                 0x582
#define WM8994_AIF2_EQ_BAND_1_B                 0x583
#define WM8994_AIF2_EQ_BAND_1_PG                0x584
#define WM8994_AIF2_EQ_BAND_2_A                 0x585
#define WM8994_AIF2_EQ_BAND_2_B                 0x586
#define WM8994_AIF2_EQ_BAND_2_C                 0x587
#define WM8994_AIF2_EQ_BAND_2_PG                0x588
#define WM8994_AIF2_EQ_BAND_3_A                 0x589
#define WM8994_AIF2_EQ_BAND_3_B                 0x58A
#define WM8994_AIF2_EQ_BAND_3_C                 0x58B
#define WM8994_AIF2_EQ_BAND_3_PG                0x58C
#define WM8994_AIF2_EQ_BAND_4_A                 0x58D
#define WM8994_AIF2_EQ_BAND_4_B                 0x58E
#define WM8994_AIF2_EQ_BAND_4_C                 0x58F
#define WM8994_AIF2_EQ_BAND_4_PG                0x590
#define WM8994_AIF2_EQ_BAND_5_A                 0x591
#define WM8994_AIF2_EQ_BAND_5_B                 0x592
#define WM8994_AIF2_EQ_BAND_5_PG                0x593
#define WM8994_AIF2_EQ_BAND_1_C                 0x594
#define WM8994_DAC1_MIXER_VOLUMES               0x600
#define WM8994_DAC1_LEFT_MIXER_ROUTING          0x601
#define WM8994_DAC1_RIGHT_MIXER_ROUTING         0x602
#define WM8994_DAC2_MIXER_VOLUMES               0x603
#define WM8994_DAC2_LEFT_MIXER_ROUTING          0x604
#define WM8994_DAC2_RIGHT_MIXER_ROUTING         0x605
#define WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING     0x606
#define WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING    0x607
#define WM8994_AIF1_ADC2_LEFT_MIXER_ROUTING     0x608
#define WM8994_AIF1_ADC2_RIGHT_MIXER_ROUTING    0x609
#define WM8994_DAC1_LEFT_VOLUME                 0x610
#define WM8994_DAC1_RIGHT_VOLUME                0x611
#define WM8994_DAC2_LEFT_VOLUME                 0x612
#define WM8994_DAC2_RIGHT_VOLUME                0x613
#define WM8994_DAC_SOFTMUTE                     0x614
#define WM8994_OVERSAMPLING                     0x620
#define WM8994_SIDETONE                         0x621
#define WM8994_GPIO_1                           0x700
#define WM8994_GPIO_2                           0x701
#define WM8994_GPIO_3                           0x702
#define WM8994_GPIO_4                           0x703
#define WM8994_GPIO_5                           0x704
#define WM8994_GPIO_6                           0x705
#define WM1811_JACKDET_CTRL                     0x705
#define WM8994_GPIO_7                           0x706
#define WM8994_GPIO_8                           0x707
#define WM8994_GPIO_9                           0x708
#define WM8994_GPIO_10                          0x709
#define WM8994_GPIO_11                          0x70A
#define WM8994_PULL_CONTROL_1                   0x720
#define WM8994_PULL_CONTROL_2                   0x721
#define WM8994_REGISTER_COUNT                   736
#define WM8994_MAX_REGISTER                     0x31FF
#define WM8994_MAX_CACHED_REGISTER              0x749

/*
 * Field Definitions.
 */

/*
 * R0 (0x00) - Software Reset
 */
#define WM8994_SW_RESET_MASK                    0xFFFF  /* SW_RESET - [15:0] */
#define WM8994_SW_RESET_SHIFT                        0  /* SW_RESET - [15:0] */
#define WM8994_SW_RESET_WIDTH                       16  /* SW_RESET - [15:0] */
#define WM8994_SW_RESET                              1  /* SW_RESET */
/*
 * R1 (0x01) - Power Management (1)
 */
#define WM8994_SPKOUTR_ENA                      0x2000  /* SPKOUTR_ENA */
#define WM8994_SPKOUTR_ENA_MASK                 0x2000  /* SPKOUTR_ENA */
#define WM8994_SPKOUTR_ENA_SHIFT                    13  /* SPKOUTR_ENA */
#define WM8994_SPKOUTR_ENA_WIDTH                     1  /* SPKOUTR_ENA */
#define WM8994_SPKOUTL_ENA                      0x1000  /* SPKOUTL_ENA */
#define WM8994_SPKOUTL_ENA_MASK                 0x1000  /* SPKOUTL_ENA */
#define WM8994_SPKOUTL_ENA_SHIFT                    12  /* SPKOUTL_ENA */
#define WM8994_SPKOUTL_ENA_WIDTH                     1  /* SPKOUTL_ENA */
#define WM8994_HPOUT2_ENA                       0x0800  /* HPOUT2_ENA */
#define WM8994_HPOUT2_ENA_MASK                  0x0800  /* HPOUT2_ENA */
#define WM8994_HPOUT2_ENA_SHIFT                     11  /* HPOUT2_ENA */
#define WM8994_HPOUT2_ENA_WIDTH                      1  /* HPOUT2_ENA */
#define WM8994_HPOUT1L_ENA                      0x0200  /* HPOUT1L_ENA */
#define WM8994_HPOUT1L_ENA_MASK                 0x0200  /* HPOUT1L_ENA */
#define WM8994_HPOUT1L_ENA_SHIFT                     9  /* HPOUT1L_ENA */
#define WM8994_HPOUT1L_ENA_WIDTH                     1  /* HPOUT1L_ENA */
#define WM8994_HPOUT1R_ENA                      0x0100  /* HPOUT1R_ENA */
#define WM8994_HPOUT1R_ENA_MASK                 0x0100  /* HPOUT1R_ENA */
#define WM8994_HPOUT1R_ENA_SHIFT                     8  /* HPOUT1R_ENA */
#define WM8994_HPOUT1R_ENA_WIDTH                     1  /* HPOUT1R_ENA */
#define WM8994_MICB2_ENA                        0x0020  /* MICB2_ENA */
#define WM8994_MICB2_ENA_MASK                   0x0020  /* MICB2_ENA */
#define WM8994_MICB2_ENA_SHIFT                       5  /* MICB2_ENA */
#define WM8994_MICB2_ENA_WIDTH                       1  /* MICB2_ENA */
#define WM8994_MICB1_ENA                        0x0010  /* MICB1_ENA */
#define WM8994_MICB1_ENA_MASK                   0x0010  /* MICB1_ENA */
#define WM8994_MICB1_ENA_SHIFT                       4  /* MICB1_ENA */
#define WM8994_MICB1_ENA_WIDTH                       1  /* MICB1_ENA */
#define WM8994_VMID_SEL_MASK                    0x0006  /* VMID_SEL - [2:1] */
#define WM8994_VMID_SEL_SHIFT                        1  /* VMID_SEL - [2:1] */
#define WM8994_VMID_SEL_WIDTH                        2  /* VMID_SEL - [2:1] */
#define WM8994_BIAS_ENA                         0x0001  /* BIAS_ENA */
#define WM8994_BIAS_ENA_MASK                    0x0001  /* BIAS_ENA */
#define WM8994_BIAS_ENA_SHIFT                        0  /* BIAS_ENA */
#define WM8994_BIAS_ENA_WIDTH                        1  /* BIAS_ENA */

/*
 * R2 (0x02) - Power Management (2)
 */
#define WM8994_TSHUT_ENA                        0x4000  /* TSHUT_ENA */
#define WM8994_TSHUT_ENA_MASK                   0x4000  /* TSHUT_ENA */
#define WM8994_TSHUT_ENA_SHIFT                      14  /* TSHUT_ENA */
#define WM8994_TSHUT_ENA_WIDTH                       1  /* TSHUT_ENA */
#define WM8994_TSHUT_OPDIS                      0x2000  /* TSHUT_OPDIS */
#define WM8994_TSHUT_OPDIS_MASK                 0x2000  /* TSHUT_OPDIS */
#define WM8994_TSHUT_OPDIS_SHIFT                    13  /* TSHUT_OPDIS */
#define WM8994_TSHUT_OPDIS_WIDTH                     1  /* TSHUT_OPDIS */
#define WM8994_OPCLK_ENA                        0x0800  /* OPCLK_ENA */
#define WM8994_OPCLK_ENA_MASK                   0x0800  /* OPCLK_ENA */
#define WM8994_OPCLK_ENA_SHIFT                      11  /* OPCLK_ENA */
#define WM8994_OPCLK_ENA_WIDTH                       1  /* OPCLK_ENA */
#define WM8994_MIXINL_ENA                       0x0200  /* MIXINL_ENA */
#define WM8994_MIXINL_ENA_MASK                  0x0200  /* MIXINL_ENA */
#define WM8994_MIXINL_ENA_SHIFT                      9  /* MIXINL_ENA */
#define WM8994_MIXINL_ENA_WIDTH                      1  /* MIXINL_ENA */
#define WM8994_MIXINR_ENA                       0x0100  /* MIXINR_ENA */
#define WM8994_MIXINR_ENA_MASK                  0x0100  /* MIXINR_ENA */
#define WM8994_MIXINR_ENA_SHIFT                      8  /* MIXINR_ENA */
#define WM8994_MIXINR_ENA_WIDTH                      1  /* MIXINR_ENA */
#define WM8994_IN2L_ENA                         0x0080  /* IN2L_ENA */
#define WM8994_IN2L_ENA_MASK                    0x0080  /* IN2L_ENA */
#define WM8994_IN2L_ENA_SHIFT                        7  /* IN2L_ENA */
#define WM8994_IN2L_ENA_WIDTH                        1  /* IN2L_ENA */
#define WM8994_IN1L_ENA                         0x0040  /* IN1L_ENA */
#define WM8994_IN1L_ENA_MASK                    0x0040  /* IN1L_ENA */
#define WM8994_IN1L_ENA_SHIFT                        6  /* IN1L_ENA */
#define WM8994_IN1L_ENA_WIDTH                        1  /* IN1L_ENA */
#define WM8994_IN2R_ENA                         0x0020  /* IN2R_ENA */
#define WM8994_IN2R_ENA_MASK                    0x0020  /* IN2R_ENA */
#define WM8994_IN2R_ENA_SHIFT                        5  /* IN2R_ENA */
#define WM8994_IN2R_ENA_WIDTH                        1  /* IN2R_ENA */
#define WM8994_IN1R_ENA                         0x0010  /* IN1R_ENA */
#define WM8994_IN1R_ENA_MASK                    0x0010  /* IN1R_ENA */
#define WM8994_IN1R_ENA_SHIFT                        4  /* IN1R_ENA */
#define WM8994_IN1R_ENA_WIDTH                        1  /* IN1R_ENA */

/*
 * R3 (0x03) - Power Management (3)
 */
#define WM8994_LINEOUT1N_ENA                    0x2000  /* LINEOUT1N_ENA */
#define WM8994_LINEOUT1N_ENA_MASK               0x2000  /* LINEOUT1N_ENA */
#define WM8994_LINEOUT1N_ENA_SHIFT                  13  /* LINEOUT1N_ENA */
#define WM8994_LINEOUT1N_ENA_WIDTH                   1  /* LINEOUT1N_ENA */
#define WM8994_LINEOUT1P_ENA                    0x1000  /* LINEOUT1P_ENA */
#define WM8994_LINEOUT1P_ENA_MASK               0x1000  /* LINEOUT1P_ENA */
#define WM8994_LINEOUT1P_ENA_SHIFT                  12  /* LINEOUT1P_ENA */
#define WM8994_LINEOUT1P_ENA_WIDTH                   1  /* LINEOUT1P_ENA */
#define WM8994_LINEOUT2N_ENA                    0x0800  /* LINEOUT2N_ENA */
#define WM8994_LINEOUT2N_ENA_MASK               0x0800  /* LINEOUT2N_ENA */
#define WM8994_LINEOUT2N_ENA_SHIFT                  11  /* LINEOUT2N_ENA */
#define WM8994_LINEOUT2N_ENA_WIDTH                   1  /* LINEOUT2N_ENA */
#define WM8994_LINEOUT2P_ENA                    0x0400  /* LINEOUT2P_ENA */
#define WM8994_LINEOUT2P_ENA_MASK               0x0400  /* LINEOUT2P_ENA */
#define WM8994_LINEOUT2P_ENA_SHIFT                  10  /* LINEOUT2P_ENA */
#define WM8994_LINEOUT2P_ENA_WIDTH                   1  /* LINEOUT2P_ENA */
#define WM8994_SPKRVOL_ENA                      0x0200  /* SPKRVOL_ENA */
#define WM8994_SPKRVOL_ENA_MASK                 0x0200  /* SPKRVOL_ENA */
#define WM8994_SPKRVOL_ENA_SHIFT                     9  /* SPKRVOL_ENA */
#define WM8994_SPKRVOL_ENA_WIDTH                     1  /* SPKRVOL_ENA */
#define WM8994_SPKLVOL_ENA                      0x0100  /* SPKLVOL_ENA */
#define WM8994_SPKLVOL_ENA_MASK                 0x0100  /* SPKLVOL_ENA */
#define WM8994_SPKLVOL_ENA_SHIFT                     8  /* SPKLVOL_ENA */
#define WM8994_SPKLVOL_ENA_WIDTH                     1  /* SPKLVOL_ENA */
#define WM8994_MIXOUTLVOL_ENA                   0x0080  /* MIXOUTLVOL_ENA */
#define WM8994_MIXOUTLVOL_ENA_MASK              0x0080  /* MIXOUTLVOL_ENA */
#define WM8994_MIXOUTLVOL_ENA_SHIFT                  7  /* MIXOUTLVOL_ENA */
#define WM8994_MIXOUTLVOL_ENA_WIDTH                  1  /* MIXOUTLVOL_ENA */
#define WM8994_MIXOUTRVOL_ENA                   0x0040  /* MIXOUTRVOL_ENA */
#define WM8994_MIXOUTRVOL_ENA_MASK              0x0040  /* MIXOUTRVOL_ENA */
#define WM8994_MIXOUTRVOL_ENA_SHIFT                  6  /* MIXOUTRVOL_ENA */
#define WM8994_MIXOUTRVOL_ENA_WIDTH                  1  /* MIXOUTRVOL_ENA */
#define WM8994_MIXOUTL_ENA                      0x0020  /* MIXOUTL_ENA */
#define WM8994_MIXOUTL_ENA_MASK                 0x0020  /* MIXOUTL_ENA */
#define WM8994_MIXOUTL_ENA_SHIFT                     5  /* MIXOUTL_ENA */
#define WM8994_MIXOUTL_ENA_WIDTH                     1  /* MIXOUTL_ENA */
#define WM8994_MIXOUTR_ENA                      0x0010  /* MIXOUTR_ENA */
#define WM8994_MIXOUTR_ENA_MASK                 0x0010  /* MIXOUTR_ENA */
#define WM8994_MIXOUTR_ENA_SHIFT                     4  /* MIXOUTR_ENA */
#define WM8994_MIXOUTR_ENA_WIDTH                     1  /* MIXOUTR_ENA */

/*
 * R4 (0x04) - Power Management (4)
 */
#define WM8994_AIF2ADCL_ENA                     0x2000  /* AIF2ADCL_ENA */
#define WM8994_AIF2ADCL_ENA_MASK                0x2000  /* AIF2ADCL_ENA */
#define WM8994_AIF2ADCL_ENA_SHIFT                   13  /* AIF2ADCL_ENA */
#define WM8994_AIF2ADCL_ENA_WIDTH                    1  /* AIF2ADCL_ENA */
#define WM8994_AIF2ADCR_ENA                     0x1000  /* AIF2ADCR_ENA */
#define WM8994_AIF2ADCR_ENA_MASK                0x1000  /* AIF2ADCR_ENA */
#define WM8994_AIF2ADCR_ENA_SHIFT                   12  /* AIF2ADCR_ENA */
#define WM8994_AIF2ADCR_ENA_WIDTH                    1  /* AIF2ADCR_ENA */
#define WM8994_AIF1ADC2L_ENA                    0x0800  /* AIF1ADC2L_ENA */
#define WM8994_AIF1ADC2L_ENA_MASK               0x0800  /* AIF1ADC2L_ENA */
#define WM8994_AIF1ADC2L_ENA_SHIFT                  11  /* AIF1ADC2L_ENA */
#define WM8994_AIF1ADC2L_ENA_WIDTH                   1  /* AIF1ADC2L_ENA */
#define WM8994_AIF1ADC2R_ENA                    0x0400  /* AIF1ADC2R_ENA */
#define WM8994_AIF1ADC2R_ENA_MASK               0x0400  /* AIF1ADC2R_ENA */
#define WM8994_AIF1ADC2R_ENA_SHIFT                  10  /* AIF1ADC2R_ENA */
#define WM8994_AIF1ADC2R_ENA_WIDTH                   1  /* AIF1ADC2R_ENA */
#define WM8994_AIF1ADC1L_ENA                    0x0200  /* AIF1ADC1L_ENA */
#define WM8994_AIF1ADC1L_ENA_MASK               0x0200  /* AIF1ADC1L_ENA */
#define WM8994_AIF1ADC1L_ENA_SHIFT                   9  /* AIF1ADC1L_ENA */
#define WM8994_AIF1ADC1L_ENA_WIDTH                   1  /* AIF1ADC1L_ENA */
#define WM8994_AIF1ADC1R_ENA                    0x0100  /* AIF1ADC1R_ENA */
#define WM8994_AIF1ADC1R_ENA_MASK               0x0100  /* AIF1ADC1R_ENA */
#define WM8994_AIF1ADC1R_ENA_SHIFT                   8  /* AIF1ADC1R_ENA */
#define WM8994_AIF1ADC1R_ENA_WIDTH                   1  /* AIF1ADC1R_ENA */
#define WM8994_DMIC2L_ENA                       0x0020  /* DMIC2L_ENA */
#define WM8994_DMIC2L_ENA_MASK                  0x0020  /* DMIC2L_ENA */
#define WM8994_DMIC2L_ENA_SHIFT                      5  /* DMIC2L_ENA */
#define WM8994_DMIC2L_ENA_WIDTH                      1  /* DMIC2L_ENA */
#define WM8994_DMIC2R_ENA                       0x0010  /* DMIC2R_ENA */
#define WM8994_DMIC2R_ENA_MASK                  0x0010  /* DMIC2R_ENA */
#define WM8994_DMIC2R_ENA_SHIFT                      4  /* DMIC2R_ENA */
#define WM8994_DMIC2R_ENA_WIDTH                      1  /* DMIC2R_ENA */
#define WM8994_DMIC1L_ENA                       0x0008  /* DMIC1L_ENA */
#define WM8994_DMIC1L_ENA_MASK                  0x0008  /* DMIC1L_ENA */
#define WM8994_DMIC1L_ENA_SHIFT                      3  /* DMIC1L_ENA */
#define WM8994_DMIC1L_ENA_WIDTH                      1  /* DMIC1L_ENA */
#define WM8994_DMIC1R_ENA                       0x0004  /* DMIC1R_ENA */
#define WM8994_DMIC1R_ENA_MASK                  0x0004  /* DMIC1R_ENA */
#define WM8994_DMIC1R_ENA_SHIFT                      2  /* DMIC1R_ENA */
#define WM8994_DMIC1R_ENA_WIDTH                      1  /* DMIC1R_ENA */
#define WM8994_ADCL_ENA                         0x0002  /* ADCL_ENA */
#define WM8994_ADCL_ENA_MASK                    0x0002  /* ADCL_ENA */
#define WM8994_ADCL_ENA_SHIFT                        1  /* ADCL_ENA */
#define WM8994_ADCL_ENA_WIDTH                        1  /* ADCL_ENA */
#define WM8994_ADCR_ENA                         0x0001  /* ADCR_ENA */
#define WM8994_ADCR_ENA_MASK                    0x0001  /* ADCR_ENA */
#define WM8994_ADCR_ENA_SHIFT                        0  /* ADCR_ENA */
#define WM8994_ADCR_ENA_WIDTH                        1  /* ADCR_ENA */

/*
 * R5 (0x05) - Power Management (5)
 */
#define WM8994_AIF2DACL_ENA                     0x2000  /* AIF2DACL_ENA */
#define WM8994_AIF2DACL_ENA_MASK                0x2000  /* AIF2DACL_ENA */
#define WM8994_AIF2DACL_ENA_SHIFT                   13  /* AIF2DACL_ENA */
#define WM8994_AIF2DACL_ENA_WIDTH                    1  /* AIF2DACL_ENA */
#define WM8994_AIF2DACR_ENA                     0x1000  /* AIF2DACR_ENA */
#define WM8994_AIF2DACR_ENA_MASK                0x1000  /* AIF2DACR_ENA */
#define WM8994_AIF2DACR_ENA_SHIFT                   12  /* AIF2DACR_ENA */
#define WM8994_AIF2DACR_ENA_WIDTH                    1  /* AIF2DACR_ENA */
#define WM8994_AIF1DAC2L_ENA                    0x0800  /* AIF1DAC2L_ENA */
#define WM8994_AIF1DAC2L_ENA_MASK               0x0800  /* AIF1DAC2L_ENA */
#define WM8994_AIF1DAC2L_ENA_SHIFT                  11  /* AIF1DAC2L_ENA */
#define WM8994_AIF1DAC2L_ENA_WIDTH                   1  /* AIF1DAC2L_ENA */
#define WM8994_AIF1DAC2R_ENA                    0x0400  /* AIF1DAC2R_ENA */
#define WM8994_AIF1DAC2R_ENA_MASK               0x0400  /* AIF1DAC2R_ENA */
#define WM8994_AIF1DAC2R_ENA_SHIFT                  10  /* AIF1DAC2R_ENA */
#define WM8994_AIF1DAC2R_ENA_WIDTH                   1  /* AIF1DAC2R_ENA */
#define WM8994_AIF1DAC1L_ENA                    0x0200  /* AIF1DAC1L_ENA */
#define WM8994_AIF1DAC1L_ENA_MASK               0x0200  /* AIF1DAC1L_ENA */
#define WM8994_AIF1DAC1L_ENA_SHIFT                   9  /* AIF1DAC1L_ENA */
#define WM8994_AIF1DAC1L_ENA_WIDTH                   1  /* AIF1DAC1L_ENA */
#define WM8994_AIF1DAC1R_ENA                    0x0100  /* AIF1DAC1R_ENA */
#define WM8994_AIF1DAC1R_ENA_MASK               0x0100  /* AIF1DAC1R_ENA */
#define WM8994_AIF1DAC1R_ENA_SHIFT                   8  /* AIF1DAC1R_ENA */
#define WM8994_AIF1DAC1R_ENA_WIDTH                   1  /* AIF1DAC1R_ENA */
#define WM8994_DAC2L_ENA                        0x0008  /* DAC2L_ENA */
#define WM8994_DAC2L_ENA_MASK                   0x0008  /* DAC2L_ENA */
#define WM8994_DAC2L_ENA_SHIFT                       3  /* DAC2L_ENA */
#define WM8994_DAC2L_ENA_WIDTH                       1  /* DAC2L_ENA */
#define WM8994_DAC2R_ENA                        0x0004  /* DAC2R_ENA */
#define WM8994_DAC2R_ENA_MASK                   0x0004  /* DAC2R_ENA */
#define WM8994_DAC2R_ENA_SHIFT                       2  /* DAC2R_ENA */
#define WM8994_DAC2R_ENA_WIDTH                       1  /* DAC2R_ENA */
#define WM8994_DAC1L_ENA                        0x0002  /* DAC1L_ENA */
#define WM8994_DAC1L_ENA_MASK                   0x0002  /* DAC1L_ENA */
#define WM8994_DAC1L_ENA_SHIFT                       1  /* DAC1L_ENA */
#define WM8994_DAC1L_ENA_WIDTH                       1  /* DAC1L_ENA */
#define WM8994_DAC1R_ENA                        0x0001  /* DAC1R_ENA */
#define WM8994_DAC1R_ENA_MASK                   0x0001  /* DAC1R_ENA */
#define WM8994_DAC1R_ENA_SHIFT                       0  /* DAC1R_ENA */
#define WM8994_DAC1R_ENA_WIDTH                       1  /* DAC1R_ENA */

/*
 * R6 (0x06) - Power Management (6)
 */
#define WM8958_AIF3ADC_SRC_MASK             0x0600  /* AIF3ADC_SRC - [10:9] */
#define WM8958_AIF3ADC_SRC_SHIFT                 9  /* AIF3ADC_SRC - [10:9] */
#define WM8958_AIF3ADC_SRC_WIDTH                 2  /* AIF3ADC_SRC - [10:9] */
#define WM8958_AIF2DAC_SRC_MASK             0x0180  /* AIF2DAC_SRC - [8:7] */
#define WM8958_AIF2DAC_SRC_SHIFT                 7  /* AIF2DAC_SRC - [8:7] */
#define WM8958_AIF2DAC_SRC_WIDTH                 2  /* AIF2DAC_SRC - [8:7] */
#define WM8994_AIF3_TRI                     0x0020  /* AIF3_TRI */
#define WM8994_AIF3_TRI_MASK                0x0020  /* AIF3_TRI */
#define WM8994_AIF3_TRI_SHIFT                    5  /* AIF3_TRI */
#define WM8994_AIF3_TRI_WIDTH                    1  /* AIF3_TRI */
#define WM8994_AIF3_ADCDAT_SRC_MASK         0x0018  /* AIF3_ADCDAT_SRC - [4:3] */
#define WM8994_AIF3_ADCDAT_SRC_SHIFT             3  /* AIF3_ADCDAT_SRC - [4:3] */
#define WM8994_AIF3_ADCDAT_SRC_WIDTH             2  /* AIF3_ADCDAT_SRC - [4:3] */
#define WM8994_AIF2_ADCDAT_SRC              0x0004  /* AIF2_ADCDAT_SRC */
#define WM8994_AIF2_ADCDAT_SRC_MASK         0x0004  /* AIF2_ADCDAT_SRC */
#define WM8994_AIF2_ADCDAT_SRC_SHIFT             2  /* AIF2_ADCDAT_SRC */
#define WM8994_AIF2_ADCDAT_SRC_WIDTH             1  /* AIF2_ADCDAT_SRC */
#define WM8994_AIF2_DACDAT_SRC              0x0002  /* AIF2_DACDAT_SRC */
#define WM8994_AIF2_DACDAT_SRC_MASK         0x0002  /* AIF2_DACDAT_SRC */
#define WM8994_AIF2_DACDAT_SRC_SHIFT             1  /* AIF2_DACDAT_SRC */
#define WM8994_AIF2_DACDAT_SRC_WIDTH             1  /* AIF2_DACDAT_SRC */
#define WM8994_AIF1_DACDAT_SRC              0x0001  /* AIF1_DACDAT_SRC */
#define WM8994_AIF1_DACDAT_SRC_MASK         0x0001  /* AIF1_DACDAT_SRC */
#define WM8994_AIF1_DACDAT_SRC_SHIFT             0  /* AIF1_DACDAT_SRC */
#define WM8994_AIF1_DACDAT_SRC_WIDTH             1  /* AIF1_DACDAT_SRC */

/*
 * R28 (0x1C) - Left Output Volume
 */
#define WM8994_HPOUT1_VU                    0x0100  /* HPOUT1_VU */
#define WM8994_HPOUT1_VU_MASK               0x0100  /* HPOUT1_VU */
#define WM8994_HPOUT1_VU_SHIFT                   8  /* HPOUT1_VU */
#define WM8994_HPOUT1_VU_WIDTH                   1  /* HPOUT1_VU */
#define WM8994_HPOUT1L_ZC                   0x0080  /* HPOUT1L_ZC */
#define WM8994_HPOUT1L_ZC_MASK              0x0080  /* HPOUT1L_ZC */
#define WM8994_HPOUT1L_ZC_SHIFT                  7  /* HPOUT1L_ZC */
#define WM8994_HPOUT1L_ZC_WIDTH                  1  /* HPOUT1L_ZC */
#define WM8994_HPOUT1L_MUTE_N               0x0040  /* HPOUT1L_MUTE_N */
#define WM8994_HPOUT1L_MUTE_N_MASK          0x0040  /* HPOUT1L_MUTE_N */
#define WM8994_HPOUT1L_MUTE_N_SHIFT              6  /* HPOUT1L_MUTE_N */
#define WM8994_HPOUT1L_MUTE_N_WIDTH              1  /* HPOUT1L_MUTE_N */
#define WM8994_HPOUT1L_VOL_MASK             0x003F  /* HPOUT1L_VOL - [5:0] */
#define WM8994_HPOUT1L_VOL_SHIFT                 0  /* HPOUT1L_VOL - [5:0] */
#define WM8994_HPOUT1L_VOL_WIDTH                 6  /* HPOUT1L_VOL - [5:0] */

/*
 * R29 (0x1D) - Right Output Volume
 */
#define WM8994_HPOUT1_VU                    0x0100  /* HPOUT1_VU */
#define WM8994_HPOUT1_VU_MASK               0x0100  /* HPOUT1_VU */
#define WM8994_HPOUT1_VU_SHIFT                   8  /* HPOUT1_VU */
#define WM8994_HPOUT1_VU_WIDTH                   1  /* HPOUT1_VU */
#define WM8994_HPOUT1R_ZC                   0x0080  /* HPOUT1R_ZC */
#define WM8994_HPOUT1R_ZC_MASK              0x0080  /* HPOUT1R_ZC */
#define WM8994_HPOUT1R_ZC_SHIFT                  7  /* HPOUT1R_ZC */
#define WM8994_HPOUT1R_ZC_WIDTH                  1  /* HPOUT1R_ZC */
#define WM8994_HPOUT1R_MUTE_N               0x0040  /* HPOUT1R_MUTE_N */
#define WM8994_HPOUT1R_MUTE_N_MASK          0x0040  /* HPOUT1R_MUTE_N */
#define WM8994_HPOUT1R_MUTE_N_SHIFT              6  /* HPOUT1R_MUTE_N */
#define WM8994_HPOUT1R_MUTE_N_WIDTH              1  /* HPOUT1R_MUTE_N */
#define WM8994_HPOUT1R_VOL_MASK             0x003F  /* HPOUT1R_VOL - [5:0] */
#define WM8994_HPOUT1R_VOL_SHIFT                 0  /* HPOUT1R_VOL - [5:0] */
#define WM8994_HPOUT1R_VOL_WIDTH                 6  /* HPOUT1R_VOL - [5:0] */

/*
 * R30 (0x1E) - Line Outputs Volume
 */
#define WM8994_LINEOUT1N_MUTE                   0x0040  /* LINEOUT1N_MUTE */
#define WM8994_LINEOUT1N_MUTE_MASK              0x0040  /* LINEOUT1N_MUTE */
#define WM8994_LINEOUT1N_MUTE_SHIFT                  6  /* LINEOUT1N_MUTE */
#define WM8994_LINEOUT1N_MUTE_WIDTH                  1  /* LINEOUT1N_MUTE */
#define WM8994_LINEOUT1P_MUTE                   0x0020  /* LINEOUT1P_MUTE */
#define WM8994_LINEOUT1P_MUTE_MASK              0x0020  /* LINEOUT1P_MUTE */
#define WM8994_LINEOUT1P_MUTE_SHIFT                  5  /* LINEOUT1P_MUTE */
#define WM8994_LINEOUT1P_MUTE_WIDTH                  1  /* LINEOUT1P_MUTE */
#define WM8994_LINEOUT1_VOL                     0x0010  /* LINEOUT1_VOL */
#define WM8994_LINEOUT1_VOL_MASK                0x0010  /* LINEOUT1_VOL */
#define WM8994_LINEOUT1_VOL_SHIFT                    4  /* LINEOUT1_VOL */
#define WM8994_LINEOUT1_VOL_WIDTH                    1  /* LINEOUT1_VOL */
#define WM8994_LINEOUT2N_MUTE                   0x0004  /* LINEOUT2N_MUTE */
#define WM8994_LINEOUT2N_MUTE_MASK              0x0004  /* LINEOUT2N_MUTE */
#define WM8994_LINEOUT2N_MUTE_SHIFT                  2  /* LINEOUT2N_MUTE */
#define WM8994_LINEOUT2N_MUTE_WIDTH                  1  /* LINEOUT2N_MUTE */
#define WM8994_LINEOUT2P_MUTE                   0x0002  /* LINEOUT2P_MUTE */
#define WM8994_LINEOUT2P_MUTE_MASK              0x0002  /* LINEOUT2P_MUTE */
#define WM8994_LINEOUT2P_MUTE_SHIFT                  1  /* LINEOUT2P_MUTE */
#define WM8994_LINEOUT2P_MUTE_WIDTH                  1  /* LINEOUT2P_MUTE */
#define WM8994_LINEOUT2_VOL                     0x0001  /* LINEOUT2_VOL */
#define WM8994_LINEOUT2_VOL_MASK                0x0001  /* LINEOUT2_VOL */
#define WM8994_LINEOUT2_VOL_SHIFT                    0  /* LINEOUT2_VOL */
#define WM8994_LINEOUT2_VOL_WIDTH                    1  /* LINEOUT2_VOL */

/*
 * R31 (0x1F) - HPOUT2 Volume
 */
#define WM8994_HPOUT2_MUTE                      0x0020  /* HPOUT2_MUTE */
#define WM8994_HPOUT2_MUTE_MASK                 0x0020  /* HPOUT2_MUTE */
#define WM8994_HPOUT2_MUTE_SHIFT                     5  /* HPOUT2_MUTE */
#define WM8994_HPOUT2_MUTE_WIDTH                     1  /* HPOUT2_MUTE */
#define WM8994_HPOUT2_VOL                       0x0010  /* HPOUT2_VOL */
#define WM8994_HPOUT2_VOL_MASK                  0x0010  /* HPOUT2_VOL */
#define WM8994_HPOUT2_VOL_SHIFT                      4  /* HPOUT2_VOL */
#define WM8994_HPOUT2_VOL_WIDTH                      1  /* HPOUT2_VOL */

/*
 * R32 (0x20) - Left OPGA Volume
 */
#define WM8994_MIXOUT_VU                        0x0100  /* MIXOUT_VU */
#define WM8994_MIXOUT_VU_MASK                   0x0100  /* MIXOUT_VU */
#define WM8994_MIXOUT_VU_SHIFT                       8  /* MIXOUT_VU */
#define WM8994_MIXOUT_VU_WIDTH                       1  /* MIXOUT_VU */
#define WM8994_MIXOUTL_ZC                       0x0080  /* MIXOUTL_ZC */
#define WM8994_MIXOUTL_ZC_MASK                  0x0080  /* MIXOUTL_ZC */
#define WM8994_MIXOUTL_ZC_SHIFT                      7  /* MIXOUTL_ZC */
#define WM8994_MIXOUTL_ZC_WIDTH                      1  /* MIXOUTL_ZC */
#define WM8994_MIXOUTL_MUTE_N                   0x0040  /* MIXOUTL_MUTE_N */
#define WM8994_MIXOUTL_MUTE_N_MASK              0x0040  /* MIXOUTL_MUTE_N */
#define WM8994_MIXOUTL_MUTE_N_SHIFT                  6  /* MIXOUTL_MUTE_N */
#define WM8994_MIXOUTL_MUTE_N_WIDTH                  1  /* MIXOUTL_MUTE_N */
#define WM8994_MIXOUTL_VOL_MASK             0x003F  /* MIXOUTL_VOL - [5:0] */
#define WM8994_MIXOUTL_VOL_SHIFT                 0  /* MIXOUTL_VOL - [5:0] */
#define WM8994_MIXOUTL_VOL_WIDTH                 6  /* MIXOUTL_VOL - [5:0] */

/*
 * R33 (0x21) - Right OPGA Volume
 */
#define WM8994_MIXOUT_VU                        0x0100  /* MIXOUT_VU */
#define WM8994_MIXOUT_VU_MASK                   0x0100  /* MIXOUT_VU */
#define WM8994_MIXOUT_VU_SHIFT                       8  /* MIXOUT_VU */
#define WM8994_MIXOUT_VU_WIDTH                       1  /* MIXOUT_VU */
#define WM8994_MIXOUTR_ZC                       0x0080  /* MIXOUTR_ZC */
#define WM8994_MIXOUTR_ZC_MASK                  0x0080  /* MIXOUTR_ZC */
#define WM8994_MIXOUTR_ZC_SHIFT                      7  /* MIXOUTR_ZC */
#define WM8994_MIXOUTR_ZC_WIDTH                      1  /* MIXOUTR_ZC */
#define WM8994_MIXOUTR_MUTE_N                   0x0040  /* MIXOUTR_MUTE_N */
#define WM8994_MIXOUTR_MUTE_N_MASK              0x0040  /* MIXOUTR_MUTE_N */
#define WM8994_MIXOUTR_MUTE_N_SHIFT                  6  /* MIXOUTR_MUTE_N */
#define WM8994_MIXOUTR_MUTE_N_WIDTH                  1  /* MIXOUTR_MUTE_N */
#define WM8994_MIXOUTR_VOL_MASK             0x003F  /* MIXOUTR_VOL - [5:0] */
#define WM8994_MIXOUTR_VOL_SHIFT                 0  /* MIXOUTR_VOL - [5:0] */
#define WM8994_MIXOUTR_VOL_WIDTH                 6  /* MIXOUTR_VOL - [5:0] */

/*
 * R45 (0x2D) - Output Mixer (1)
 */
#define WM8994_DAC1L_TO_HPOUT1L                 0x0100  /* DAC1L_TO_HPOUT1L */
#define WM8994_DAC1L_TO_HPOUT1L_MASK            0x0100  /* DAC1L_TO_HPOUT1L */
#define WM8994_DAC1L_TO_HPOUT1L_SHIFT                8  /* DAC1L_TO_HPOUT1L */
#define WM8994_DAC1L_TO_HPOUT1L_WIDTH                1  /* DAC1L_TO_HPOUT1L */
#define WM8994_MIXINR_TO_MIXOUTL                0x0080  /* MIXINR_TO_MIXOUTL */
#define WM8994_MIXINR_TO_MIXOUTL_MASK           0x0080  /* MIXINR_TO_MIXOUTL */
#define WM8994_MIXINR_TO_MIXOUTL_SHIFT               7  /* MIXINR_TO_MIXOUTL */
#define WM8994_MIXINR_TO_MIXOUTL_WIDTH               1  /* MIXINR_TO_MIXOUTL */
#define WM8994_MIXINL_TO_MIXOUTL                0x0040  /* MIXINL_TO_MIXOUTL */
#define WM8994_MIXINL_TO_MIXOUTL_MASK           0x0040  /* MIXINL_TO_MIXOUTL */
#define WM8994_MIXINL_TO_MIXOUTL_SHIFT               6  /* MIXINL_TO_MIXOUTL */
#define WM8994_MIXINL_TO_MIXOUTL_WIDTH               1  /* MIXINL_TO_MIXOUTL */
#define WM8994_IN2RN_TO_MIXOUTL                 0x0020  /* IN2RN_TO_MIXOUTL */
#define WM8994_IN2RN_TO_MIXOUTL_MASK            0x0020  /* IN2RN_TO_MIXOUTL */
#define WM8994_IN2RN_TO_MIXOUTL_SHIFT                5  /* IN2RN_TO_MIXOUTL */
#define WM8994_IN2RN_TO_MIXOUTL_WIDTH                1  /* IN2RN_TO_MIXOUTL */
#define WM8994_IN2LN_TO_MIXOUTL                 0x0010  /* IN2LN_TO_MIXOUTL */
#define WM8994_IN2LN_TO_MIXOUTL_MASK            0x0010  /* IN2LN_TO_MIXOUTL */
#define WM8994_IN2LN_TO_MIXOUTL_SHIFT                4  /* IN2LN_TO_MIXOUTL */
#define WM8994_IN2LN_TO_MIXOUTL_WIDTH                1  /* IN2LN_TO_MIXOUTL */
#define WM8994_IN1R_TO_MIXOUTL                  0x0008  /* IN1R_TO_MIXOUTL */
#define WM8994_IN1R_TO_MIXOUTL_MASK             0x0008  /* IN1R_TO_MIXOUTL */
#define WM8994_IN1R_TO_MIXOUTL_SHIFT                 3  /* IN1R_TO_MIXOUTL */
#define WM8994_IN1R_TO_MIXOUTL_WIDTH                 1  /* IN1R_TO_MIXOUTL */
#define WM8994_IN1L_TO_MIXOUTL                  0x0004  /* IN1L_TO_MIXOUTL */
#define WM8994_IN1L_TO_MIXOUTL_MASK             0x0004  /* IN1L_TO_MIXOUTL */
#define WM8994_IN1L_TO_MIXOUTL_SHIFT                 2  /* IN1L_TO_MIXOUTL */
#define WM8994_IN1L_TO_MIXOUTL_WIDTH                 1  /* IN1L_TO_MIXOUTL */
#define WM8994_IN2LP_TO_MIXOUTL                 0x0002  /* IN2LP_TO_MIXOUTL */
#define WM8994_IN2LP_TO_MIXOUTL_MASK            0x0002  /* IN2LP_TO_MIXOUTL */
#define WM8994_IN2LP_TO_MIXOUTL_SHIFT                1  /* IN2LP_TO_MIXOUTL */
#define WM8994_IN2LP_TO_MIXOUTL_WIDTH                1  /* IN2LP_TO_MIXOUTL */
#define WM8994_DAC1L_TO_MIXOUTL                 0x0001  /* DAC1L_TO_MIXOUTL */
#define WM8994_DAC1L_TO_MIXOUTL_MASK            0x0001  /* DAC1L_TO_MIXOUTL */
#define WM8994_DAC1L_TO_MIXOUTL_SHIFT                0  /* DAC1L_TO_MIXOUTL */
#define WM8994_DAC1L_TO_MIXOUTL_WIDTH                1  /* DAC1L_TO_MIXOUTL */

/*
 * R46 (0x2E) - Output Mixer (2)
 */
#define WM8994_DAC1R_TO_HPOUT1R                 0x0100  /* DAC1R_TO_HPOUT1R */
#define WM8994_DAC1R_TO_HPOUT1R_MASK            0x0100  /* DAC1R_TO_HPOUT1R */
#define WM8994_DAC1R_TO_HPOUT1R_SHIFT                8  /* DAC1R_TO_HPOUT1R */
#define WM8994_DAC1R_TO_HPOUT1R_WIDTH                1  /* DAC1R_TO_HPOUT1R */
#define WM8994_MIXINL_TO_MIXOUTR                0x0080  /* MIXINL_TO_MIXOUTR */
#define WM8994_MIXINL_TO_MIXOUTR_MASK           0x0080  /* MIXINL_TO_MIXOUTR */
#define WM8994_MIXINL_TO_MIXOUTR_SHIFT               7  /* MIXINL_TO_MIXOUTR */
#define WM8994_MIXINL_TO_MIXOUTR_WIDTH               1  /* MIXINL_TO_MIXOUTR */
#define WM8994_MIXINR_TO_MIXOUTR                0x0040  /* MIXINR_TO_MIXOUTR */
#define WM8994_MIXINR_TO_MIXOUTR_MASK           0x0040  /* MIXINR_TO_MIXOUTR */
#define WM8994_MIXINR_TO_MIXOUTR_SHIFT               6  /* MIXINR_TO_MIXOUTR */
#define WM8994_MIXINR_TO_MIXOUTR_WIDTH               1  /* MIXINR_TO_MIXOUTR */
#define WM8994_IN2LN_TO_MIXOUTR                 0x0020  /* IN2LN_TO_MIXOUTR */
#define WM8994_IN2LN_TO_MIXOUTR_MASK            0x0020  /* IN2LN_TO_MIXOUTR */
#define WM8994_IN2LN_TO_MIXOUTR_SHIFT                5  /* IN2LN_TO_MIXOUTR */
#define WM8994_IN2LN_TO_MIXOUTR_WIDTH                1  /* IN2LN_TO_MIXOUTR */
#define WM8994_IN2RN_TO_MIXOUTR                 0x0010  /* IN2RN_TO_MIXOUTR */
#define WM8994_IN2RN_TO_MIXOUTR_MASK            0x0010  /* IN2RN_TO_MIXOUTR */
#define WM8994_IN2RN_TO_MIXOUTR_SHIFT                4  /* IN2RN_TO_MIXOUTR */
#define WM8994_IN2RN_TO_MIXOUTR_WIDTH                1  /* IN2RN_TO_MIXOUTR */
#define WM8994_IN1L_TO_MIXOUTR                  0x0008  /* IN1L_TO_MIXOUTR */
#define WM8994_IN1L_TO_MIXOUTR_MASK             0x0008  /* IN1L_TO_MIXOUTR */
#define WM8994_IN1L_TO_MIXOUTR_SHIFT                 3  /* IN1L_TO_MIXOUTR */
#define WM8994_IN1L_TO_MIXOUTR_WIDTH                 1  /* IN1L_TO_MIXOUTR */
#define WM8994_IN1R_TO_MIXOUTR                  0x0004  /* IN1R_TO_MIXOUTR */
#define WM8994_IN1R_TO_MIXOUTR_MASK             0x0004  /* IN1R_TO_MIXOUTR */
#define WM8994_IN1R_TO_MIXOUTR_SHIFT                 2  /* IN1R_TO_MIXOUTR */
#define WM8994_IN1R_TO_MIXOUTR_WIDTH                 1  /* IN1R_TO_MIXOUTR */
#define WM8994_IN2RP_TO_MIXOUTR                 0x0002  /* IN2RP_TO_MIXOUTR */
#define WM8994_IN2RP_TO_MIXOUTR_MASK            0x0002  /* IN2RP_TO_MIXOUTR */
#define WM8994_IN2RP_TO_MIXOUTR_SHIFT                1  /* IN2RP_TO_MIXOUTR */
#define WM8994_IN2RP_TO_MIXOUTR_WIDTH                1  /* IN2RP_TO_MIXOUTR */
#define WM8994_DAC1R_TO_MIXOUTR                 0x0001  /* DAC1R_TO_MIXOUTR */
#define WM8994_DAC1R_TO_MIXOUTR_MASK            0x0001  /* DAC1R_TO_MIXOUTR */
#define WM8994_DAC1R_TO_MIXOUTR_SHIFT                0  /* DAC1R_TO_MIXOUTR */
#define WM8994_DAC1R_TO_MIXOUTR_WIDTH                1  /* DAC1R_TO_MIXOUTR */

/*
 * R47 (0x2F) - Output Mixer (3)
 */
#define WM8994_IN2LP_MIXOUTL_VOL_MASK           0x0E00  /* IN2LP_MIXOUTL_VOL - [11:9] */
#define WM8994_IN2LP_MIXOUTL_VOL_SHIFT               9  /* IN2LP_MIXOUTL_VOL - [11:9] */
#define WM8994_IN2LP_MIXOUTL_VOL_WIDTH               3  /* IN2LP_MIXOUTL_VOL - [11:9] */
#define WM8994_IN2LN_MIXOUTL_VOL_MASK           0x01C0  /* IN2LN_MIXOUTL_VOL - [8:6] */
#define WM8994_IN2LN_MIXOUTL_VOL_SHIFT               6  /* IN2LN_MIXOUTL_VOL - [8:6] */
#define WM8994_IN2LN_MIXOUTL_VOL_WIDTH               3  /* IN2LN_MIXOUTL_VOL - [8:6] */
#define WM8994_IN1R_MIXOUTL_VOL_MASK            0x0038  /* IN1R_MIXOUTL_VOL - [5:3] */
#define WM8994_IN1R_MIXOUTL_VOL_SHIFT                3  /* IN1R_MIXOUTL_VOL - [5:3] */
#define WM8994_IN1R_MIXOUTL_VOL_WIDTH                3  /* IN1R_MIXOUTL_VOL - [5:3] */
#define WM8994_IN1L_MIXOUTL_VOL_MASK            0x0007  /* IN1L_MIXOUTL_VOL - [2:0] */
#define WM8994_IN1L_MIXOUTL_VOL_SHIFT                0  /* IN1L_MIXOUTL_VOL - [2:0] */
#define WM8994_IN1L_MIXOUTL_VOL_WIDTH                3  /* IN1L_MIXOUTL_VOL - [2:0] */

/*
 * R48 (0x30) - Output Mixer (4)
 */
#define WM8994_IN2RP_MIXOUTR_VOL_MASK           0x0E00  /* IN2RP_MIXOUTR_VOL - [11:9] */
#define WM8994_IN2RP_MIXOUTR_VOL_SHIFT               9  /* IN2RP_MIXOUTR_VOL - [11:9] */
#define WM8994_IN2RP_MIXOUTR_VOL_WIDTH               3  /* IN2RP_MIXOUTR_VOL - [11:9] */
#define WM8994_IN2RN_MIXOUTR_VOL_MASK           0x01C0  /* IN2RN_MIXOUTR_VOL - [8:6] */
#define WM8994_IN2RN_MIXOUTR_VOL_SHIFT               6  /* IN2RN_MIXOUTR_VOL - [8:6] */
#define WM8994_IN2RN_MIXOUTR_VOL_WIDTH               3  /* IN2RN_MIXOUTR_VOL - [8:6] */
#define WM8994_IN1L_MIXOUTR_VOL_MASK            0x0038  /* IN1L_MIXOUTR_VOL - [5:3] */
#define WM8994_IN1L_MIXOUTR_VOL_SHIFT                3  /* IN1L_MIXOUTR_VOL - [5:3] */
#define WM8994_IN1L_MIXOUTR_VOL_WIDTH                3  /* IN1L_MIXOUTR_VOL - [5:3] */
#define WM8994_IN1R_MIXOUTR_VOL_MASK            0x0007  /* IN1R_MIXOUTR_VOL - [2:0] */
#define WM8994_IN1R_MIXOUTR_VOL_SHIFT                0  /* IN1R_MIXOUTR_VOL - [2:0] */
#define WM8994_IN1R_MIXOUTR_VOL_WIDTH                3  /* IN1R_MIXOUTR_VOL - [2:0] */

/*
 * R49 (0x31) - Output Mixer (5)
 */
#define WM8994_DAC1L_MIXOUTL_VOL_MASK           0x0E00  /* DAC1L_MIXOUTL_VOL - [11:9] */
#define WM8994_DAC1L_MIXOUTL_VOL_SHIFT               9  /* DAC1L_MIXOUTL_VOL - [11:9] */
#define WM8994_DAC1L_MIXOUTL_VOL_WIDTH               3  /* DAC1L_MIXOUTL_VOL - [11:9] */
#define WM8994_IN2RN_MIXOUTL_VOL_MASK           0x01C0  /* IN2RN_MIXOUTL_VOL - [8:6] */
#define WM8994_IN2RN_MIXOUTL_VOL_SHIFT               6  /* IN2RN_MIXOUTL_VOL - [8:6] */
#define WM8994_IN2RN_MIXOUTL_VOL_WIDTH               3  /* IN2RN_MIXOUTL_VOL - [8:6] */
#define WM8994_MIXINR_MIXOUTL_VOL_MASK          0x0038  /* MIXINR_MIXOUTL_VOL - [5:3] */
#define WM8994_MIXINR_MIXOUTL_VOL_SHIFT              3  /* MIXINR_MIXOUTL_VOL - [5:3] */
#define WM8994_MIXINR_MIXOUTL_VOL_WIDTH              3  /* MIXINR_MIXOUTL_VOL - [5:3] */
#define WM8994_MIXINL_MIXOUTL_VOL_MASK          0x0007  /* MIXINL_MIXOUTL_VOL - [2:0] */
#define WM8994_MIXINL_MIXOUTL_VOL_SHIFT              0  /* MIXINL_MIXOUTL_VOL - [2:0] */
#define WM8994_MIXINL_MIXOUTL_VOL_WIDTH              3  /* MIXINL_MIXOUTL_VOL - [2:0] */

/*
 * R50 (0x32) - Output Mixer (6)
 */
#define WM8994_DAC1R_MIXOUTR_VOL_MASK           0x0E00  /* DAC1R_MIXOUTR_VOL - [11:9] */
#define WM8994_DAC1R_MIXOUTR_VOL_SHIFT               9  /* DAC1R_MIXOUTR_VOL - [11:9] */
#define WM8994_DAC1R_MIXOUTR_VOL_WIDTH               3  /* DAC1R_MIXOUTR_VOL - [11:9] */
#define WM8994_IN2LN_MIXOUTR_VOL_MASK           0x01C0  /* IN2LN_MIXOUTR_VOL - [8:6] */
#define WM8994_IN2LN_MIXOUTR_VOL_SHIFT               6  /* IN2LN_MIXOUTR_VOL - [8:6] */
#define WM8994_IN2LN_MIXOUTR_VOL_WIDTH               3  /* IN2LN_MIXOUTR_VOL - [8:6] */
#define WM8994_MIXINL_MIXOUTR_VOL_MASK          0x0038  /* MIXINL_MIXOUTR_VOL - [5:3] */
#define WM8994_MIXINL_MIXOUTR_VOL_SHIFT              3  /* MIXINL_MIXOUTR_VOL - [5:3] */
#define WM8994_MIXINL_MIXOUTR_VOL_WIDTH              3  /* MIXINL_MIXOUTR_VOL - [5:3] */
#define WM8994_MIXINR_MIXOUTR_VOL_MASK          0x0007  /* MIXINR_MIXOUTR_VOL - [2:0] */
#define WM8994_MIXINR_MIXOUTR_VOL_SHIFT              0  /* MIXINR_MIXOUTR_VOL - [2:0] */
#define WM8994_MIXINR_MIXOUTR_VOL_WIDTH              3  /* MIXINR_MIXOUTR_VOL - [2:0] */

/*
 * R59 (0x3B) - LDO 1
 */
#define WM8994_LDO1_VSEL_MASK                   0x000E  /* LDO1_VSEL - [3:1] */
#define WM8994_LDO1_VSEL_SHIFT                       1  /* LDO1_VSEL - [3:1] */
#define WM8994_LDO1_VSEL_WIDTH                       3  /* LDO1_VSEL - [3:1] */
#define WM8994_LDO1_DISCH                       0x0001  /* LDO1_DISCH */
#define WM8994_LDO1_DISCH_MASK                  0x0001  /* LDO1_DISCH */
#define WM8994_LDO1_DISCH_SHIFT                      0  /* LDO1_DISCH */
#define WM8994_LDO1_DISCH_WIDTH                      1  /* LDO1_DISCH */

/*
 * R60 (0x3C) - LDO 2
 */
#define WM8994_LDO2_VSEL_MASK                   0x0006  /* LDO2_VSEL - [2:1] */
#define WM8994_LDO2_VSEL_SHIFT                       1  /* LDO2_VSEL - [2:1] */
#define WM8994_LDO2_VSEL_WIDTH                       2  /* LDO2_VSEL - [2:1] */
#define WM8994_LDO2_DISCH                       0x0001  /* LDO2_DISCH */
#define WM8994_LDO2_DISCH_MASK                  0x0001  /* LDO2_DISCH */
#define WM8994_LDO2_DISCH_SHIFT                      0  /* LDO2_DISCH */
#define WM8994_LDO2_DISCH_WIDTH                      1  /* LDO2_DISCH */

/*
 * R76 (0x4C) - Charge Pump (1)
 */
#define WM8994_CP_ENA                           0x8000  /* CP_ENA */
#define WM8994_CP_ENA_MASK                      0x8000  /* CP_ENA */
#define WM8994_CP_ENA_SHIFT                         15  /* CP_ENA */
#define WM8994_CP_ENA_WIDTH                          1  /* CP_ENA */

/*
 * R77 (0x4D) - Charge Pump (2)
 */
#define WM8958_CP_DISCH                         0x8000  /* CP_DISCH */
#define WM8958_CP_DISCH_MASK                    0x8000  /* CP_DISCH */
#define WM8958_CP_DISCH_SHIFT                       15  /* CP_DISCH */
#define WM8958_CP_DISCH_WIDTH                        1  /* CP_DISCH */

/*
 * R84 (0x54) - DC Servo (1)
 */
#define WM8994_DCS_TRIG_SINGLE_1                0x2000  /* DCS_TRIG_SINGLE_1 */
#define WM8994_DCS_TRIG_SINGLE_1_MASK           0x2000  /* DCS_TRIG_SINGLE_1 */
#define WM8994_DCS_TRIG_SINGLE_1_SHIFT              13  /* DCS_TRIG_SINGLE_1 */
#define WM8994_DCS_TRIG_SINGLE_1_WIDTH               1  /* DCS_TRIG_SINGLE_1 */
#define WM8994_DCS_TRIG_SINGLE_0                0x1000  /* DCS_TRIG_SINGLE_0 */
#define WM8994_DCS_TRIG_SINGLE_0_MASK           0x1000  /* DCS_TRIG_SINGLE_0 */
#define WM8994_DCS_TRIG_SINGLE_0_SHIFT              12  /* DCS_TRIG_SINGLE_0 */
#define WM8994_DCS_TRIG_SINGLE_0_WIDTH               1  /* DCS_TRIG_SINGLE_0 */
#define WM8994_DCS_TRIG_SERIES_1                0x0200  /* DCS_TRIG_SERIES_1 */
#define WM8994_DCS_TRIG_SERIES_1_MASK           0x0200  /* DCS_TRIG_SERIES_1 */
#define WM8994_DCS_TRIG_SERIES_1_SHIFT               9  /* DCS_TRIG_SERIES_1 */
#define WM8994_DCS_TRIG_SERIES_1_WIDTH               1  /* DCS_TRIG_SERIES_1 */
#define WM8994_DCS_TRIG_SERIES_0                0x0100  /* DCS_TRIG_SERIES_0 */
#define WM8994_DCS_TRIG_SERIES_0_MASK           0x0100  /* DCS_TRIG_SERIES_0 */
#define WM8994_DCS_TRIG_SERIES_0_SHIFT               8  /* DCS_TRIG_SERIES_0 */
#define WM8994_DCS_TRIG_SERIES_0_WIDTH               1  /* DCS_TRIG_SERIES_0 */
#define WM8994_DCS_TRIG_STARTUP_1               0x0020  /* DCS_TRIG_STARTUP_1 */
#define WM8994_DCS_TRIG_STARTUP_1_MASK          0x0020  /* DCS_TRIG_STARTUP_1 */
#define WM8994_DCS_TRIG_STARTUP_1_SHIFT              5  /* DCS_TRIG_STARTUP_1 */
#define WM8994_DCS_TRIG_STARTUP_1_WIDTH              1  /* DCS_TRIG_STARTUP_1 */
#define WM8994_DCS_TRIG_STARTUP_0               0x0010  /* DCS_TRIG_STARTUP_0 */
#define WM8994_DCS_TRIG_STARTUP_0_MASK          0x0010  /* DCS_TRIG_STARTUP_0 */
#define WM8994_DCS_TRIG_STARTUP_0_SHIFT              4  /* DCS_TRIG_STARTUP_0 */
#define WM8994_DCS_TRIG_STARTUP_0_WIDTH              1  /* DCS_TRIG_STARTUP_0 */
#define WM8994_DCS_TRIG_DAC_WR_1                0x0008  /* DCS_TRIG_DAC_WR_1 */
#define WM8994_DCS_TRIG_DAC_WR_1_MASK           0x0008  /* DCS_TRIG_DAC_WR_1 */
#define WM8994_DCS_TRIG_DAC_WR_1_SHIFT               3  /* DCS_TRIG_DAC_WR_1 */
#define WM8994_DCS_TRIG_DAC_WR_1_WIDTH               1  /* DCS_TRIG_DAC_WR_1 */
#define WM8994_DCS_TRIG_DAC_WR_0                0x0004  /* DCS_TRIG_DAC_WR_0 */
#define WM8994_DCS_TRIG_DAC_WR_0_MASK           0x0004  /* DCS_TRIG_DAC_WR_0 */
#define WM8994_DCS_TRIG_DAC_WR_0_SHIFT               2  /* DCS_TRIG_DAC_WR_0 */
#define WM8994_DCS_TRIG_DAC_WR_0_WIDTH               1  /* DCS_TRIG_DAC_WR_0 */
#define WM8994_DCS_ENA_CHAN_1                   0x0002  /* DCS_ENA_CHAN_1 */
#define WM8994_DCS_ENA_CHAN_1_MASK              0x0002  /* DCS_ENA_CHAN_1 */
#define WM8994_DCS_ENA_CHAN_1_SHIFT                  1  /* DCS_ENA_CHAN_1 */
#define WM8994_DCS_ENA_CHAN_1_WIDTH                  1  /* DCS_ENA_CHAN_1 */
#define WM8994_DCS_ENA_CHAN_0                   0x0001  /* DCS_ENA_CHAN_0 */
#define WM8994_DCS_ENA_CHAN_0_MASK              0x0001  /* DCS_ENA_CHAN_0 */
#define WM8994_DCS_ENA_CHAN_0_SHIFT                  0  /* DCS_ENA_CHAN_0 */
#define WM8994_DCS_ENA_CHAN_0_WIDTH                  1  /* DCS_ENA_CHAN_0 */

/*
 * R85 (0x55) - DC Servo (2)
 */
#define WM8994_DCS_SERIES_NO_01_MASK            0x0FE0  /* DCS_SERIES_NO_01 - [11:5] */
#define WM8994_DCS_SERIES_NO_01_SHIFT                5  /* DCS_SERIES_NO_01 - [11:5] */
#define WM8994_DCS_SERIES_NO_01_WIDTH                7  /* DCS_SERIES_NO_01 - [11:5] */
#define WM8994_DCS_TIMER_PERIOD_01_MASK         0x000F  /* DCS_TIMER_PERIOD_01 - [3:0] */
#define WM8994_DCS_TIMER_PERIOD_01_SHIFT             0  /* DCS_TIMER_PERIOD_01 - [3:0] */
#define WM8994_DCS_TIMER_PERIOD_01_WIDTH             4  /* DCS_TIMER_PERIOD_01 - [3:0] */

/*
 * R87 (0x57) - DC Servo (4)
 */
#define WM8994_DCS_DAC_WR_VAL_1_MASK            0xFF00  /* DCS_DAC_WR_VAL_1 - [15:8] */
#define WM8994_DCS_DAC_WR_VAL_1_SHIFT                8  /* DCS_DAC_WR_VAL_1 - [15:8] */
#define WM8994_DCS_DAC_WR_VAL_1_WIDTH                8  /* DCS_DAC_WR_VAL_1 - [15:8] */
#define WM8994_DCS_DAC_WR_VAL_0_MASK            0x00FF  /* DCS_DAC_WR_VAL_0 - [7:0] */
#define WM8994_DCS_DAC_WR_VAL_0_SHIFT                0  /* DCS_DAC_WR_VAL_0 - [7:0] */
#define WM8994_DCS_DAC_WR_VAL_0_WIDTH                8  /* DCS_DAC_WR_VAL_0 - [7:0] */

/*
 * R88 (0x58) - DC Servo Readback
 */
#define WM8994_DCS_CAL_COMPLETE_MASK            0x0300  /* DCS_CAL_COMPLETE - [9:8] */
#define WM8994_DCS_CAL_COMPLETE_SHIFT                8  /* DCS_CAL_COMPLETE - [9:8] */
#define WM8994_DCS_CAL_COMPLETE_WIDTH                2  /* DCS_CAL_COMPLETE - [9:8] */
#define WM8994_DCS_DAC_WR_COMPLETE_MASK         0x0030  /* DCS_DAC_WR_COMPLETE - [5:4] */
#define WM8994_DCS_DAC_WR_COMPLETE_SHIFT             4  /* DCS_DAC_WR_COMPLETE - [5:4] */
#define WM8994_DCS_DAC_WR_COMPLETE_WIDTH             2  /* DCS_DAC_WR_COMPLETE - [5:4] */
#define WM8994_DCS_STARTUP_COMPLETE_MASK        0x0003  /* DCS_STARTUP_COMPLETE - [1:0] */
#define WM8994_DCS_STARTUP_COMPLETE_SHIFT            0  /* DCS_STARTUP_COMPLETE - [1:0] */
#define WM8994_DCS_STARTUP_COMPLETE_WIDTH            2  /* DCS_STARTUP_COMPLETE - [1:0] */

/*
 * R96 (0x60) - Analogue HP (1)
 */
#define WM1811_HPOUT1_ATTN                      0x0100  /* HPOUT1_ATTN */
#define WM1811_HPOUT1_ATTN_MASK                 0x0100  /* HPOUT1_ATTN */
#define WM1811_HPOUT1_ATTN_SHIFT                     8  /* HPOUT1_ATTN */
#define WM1811_HPOUT1_ATTN_WIDTH                     1  /* HPOUT1_ATTN */
#define WM8994_HPOUT1L_RMV_SHORT                0x0080  /* HPOUT1L_RMV_SHORT */
#define WM8994_HPOUT1L_RMV_SHORT_MASK           0x0080  /* HPOUT1L_RMV_SHORT */
#define WM8994_HPOUT1L_RMV_SHORT_SHIFT               7  /* HPOUT1L_RMV_SHORT */
#define WM8994_HPOUT1L_RMV_SHORT_WIDTH               1  /* HPOUT1L_RMV_SHORT */
#define WM8994_HPOUT1L_OUTP                     0x0040  /* HPOUT1L_OUTP */
#define WM8994_HPOUT1L_OUTP_MASK                0x0040  /* HPOUT1L_OUTP */
#define WM8994_HPOUT1L_OUTP_SHIFT                    6  /* HPOUT1L_OUTP */
#define WM8994_HPOUT1L_OUTP_WIDTH                    1  /* HPOUT1L_OUTP */
#define WM8994_HPOUT1L_DLY                      0x0020  /* HPOUT1L_DLY */
#define WM8994_HPOUT1L_DLY_MASK                 0x0020  /* HPOUT1L_DLY */
#define WM8994_HPOUT1L_DLY_SHIFT                     5  /* HPOUT1L_DLY */
#define WM8994_HPOUT1L_DLY_WIDTH                     1  /* HPOUT1L_DLY */
#define WM8994_HPOUT1R_RMV_SHORT                0x0008  /* HPOUT1R_RMV_SHORT */
#define WM8994_HPOUT1R_RMV_SHORT_MASK           0x0008  /* HPOUT1R_RMV_SHORT */
#define WM8994_HPOUT1R_RMV_SHORT_SHIFT               3  /* HPOUT1R_RMV_SHORT */
#define WM8994_HPOUT1R_RMV_SHORT_WIDTH               1  /* HPOUT1R_RMV_SHORT */
#define WM8994_HPOUT1R_OUTP                     0x0004  /* HPOUT1R_OUTP */
#define WM8994_HPOUT1R_OUTP_MASK                0x0004  /* HPOUT1R_OUTP */
#define WM8994_HPOUT1R_OUTP_SHIFT                    2  /* HPOUT1R_OUTP */
#define WM8994_HPOUT1R_OUTP_WIDTH                    1  /* HPOUT1R_OUTP */
#define WM8994_HPOUT1R_DLY                      0x0002  /* HPOUT1R_DLY */
#define WM8994_HPOUT1R_DLY_MASK                 0x0002  /* HPOUT1R_DLY */
#define WM8994_HPOUT1R_DLY_SHIFT                     1  /* HPOUT1R_DLY */
#define WM8994_HPOUT1R_DLY_WIDTH                     1  /* HPOUT1R_DLY */

/*
 * R256 (0x100) - Chip Revision
 */
#define WM8994_CHIP_REV_MASK                    0x000F  /* CHIP_REV - [3:0] */
#define WM8994_CHIP_REV_SHIFT                        0  /* CHIP_REV - [3:0] */
#define WM8994_CHIP_REV_WIDTH                        4  /* CHIP_REV - [3:0] */

/*
 * R512 (0x200) - AIF1 Clocking (1)
 */
#define WM8994_AIF1CLK_SRC_MASK                 0x0018  /* AIF1CLK_SRC - [4:3] */
#define WM8994_AIF1CLK_SRC_SHIFT                     3  /* AIF1CLK_SRC - [4:3] */
#define WM8994_AIF1CLK_SRC_WIDTH                     2  /* AIF1CLK_SRC - [4:3] */
#define WM8994_AIF1CLK_INV                      0x0004  /* AIF1CLK_INV */
#define WM8994_AIF1CLK_INV_MASK                 0x0004  /* AIF1CLK_INV */
#define WM8994_AIF1CLK_INV_SHIFT                     2  /* AIF1CLK_INV */
#define WM8994_AIF1CLK_INV_WIDTH                     1  /* AIF1CLK_INV */
#define WM8994_AIF1CLK_DIV                      0x0002  /* AIF1CLK_DIV */
#define WM8994_AIF1CLK_DIV_MASK                 0x0002  /* AIF1CLK_DIV */
#define WM8994_AIF1CLK_DIV_SHIFT                     1  /* AIF1CLK_DIV */
#define WM8994_AIF1CLK_DIV_WIDTH                     1  /* AIF1CLK_DIV */
#define WM8994_AIF1CLK_ENA                      0x0001  /* AIF1CLK_ENA */
#define WM8994_AIF1CLK_ENA_MASK                 0x0001  /* AIF1CLK_ENA */
#define WM8994_AIF1CLK_ENA_SHIFT                     0  /* AIF1CLK_ENA */
#define WM8994_AIF1CLK_ENA_WIDTH                     1  /* AIF1CLK_ENA */

/*
 * R513 (0x201) - AIF1 Clocking (2)
 */
#define WM8994_AIF1DAC_DIV_MASK                 0x0038  /* AIF1DAC_DIV - [5:3] */
#define WM8994_AIF1DAC_DIV_SHIFT                     3  /* AIF1DAC_DIV - [5:3] */
#define WM8994_AIF1DAC_DIV_WIDTH                     3  /* AIF1DAC_DIV - [5:3] */
#define WM8994_AIF1ADC_DIV_MASK                 0x0007  /* AIF1ADC_DIV - [2:0] */
#define WM8994_AIF1ADC_DIV_SHIFT                     0  /* AIF1ADC_DIV - [2:0] */
#define WM8994_AIF1ADC_DIV_WIDTH                     3  /* AIF1ADC_DIV - [2:0] */

/*
 * R516 (0x204) - AIF2 Clocking (1)
 */
#define WM8994_AIF2CLK_SRC_MASK                 0x0018  /* AIF2CLK_SRC - [4:3] */
#define WM8994_AIF2CLK_SRC_SHIFT                     3  /* AIF2CLK_SRC - [4:3] */
#define WM8994_AIF2CLK_SRC_WIDTH                     2  /* AIF2CLK_SRC - [4:3] */
#define WM8994_AIF2CLK_INV                      0x0004  /* AIF2CLK_INV */
#define WM8994_AIF2CLK_INV_MASK                 0x0004  /* AIF2CLK_INV */
#define WM8994_AIF2CLK_INV_SHIFT                     2  /* AIF2CLK_INV */
#define WM8994_AIF2CLK_INV_WIDTH                     1  /* AIF2CLK_INV */
#define WM8994_AIF2CLK_DIV                      0x0002  /* AIF2CLK_DIV */
#define WM8994_AIF2CLK_DIV_MASK                 0x0002  /* AIF2CLK_DIV */
#define WM8994_AIF2CLK_DIV_SHIFT                     1  /* AIF2CLK_DIV */
#define WM8994_AIF2CLK_DIV_WIDTH                     1  /* AIF2CLK_DIV */
#define WM8994_AIF2CLK_ENA                      0x0001  /* AIF2CLK_ENA */
#define WM8994_AIF2CLK_ENA_MASK                 0x0001  /* AIF2CLK_ENA */
#define WM8994_AIF2CLK_ENA_SHIFT                     0  /* AIF2CLK_ENA */
#define WM8994_AIF2CLK_ENA_WIDTH                     1  /* AIF2CLK_ENA */

/*
 * R517 (0x205) - AIF2 Clocking (2)
 */
#define WM8994_AIF2DAC_DIV_MASK                 0x0038  /* AIF2DAC_DIV - [5:3] */
#define WM8994_AIF2DAC_DIV_SHIFT                     3  /* AIF2DAC_DIV - [5:3] */
#define WM8994_AIF2DAC_DIV_WIDTH                     3  /* AIF2DAC_DIV - [5:3] */
#define WM8994_AIF2ADC_DIV_MASK                 0x0007  /* AIF2ADC_DIV - [2:0] */
#define WM8994_AIF2ADC_DIV_SHIFT                     0  /* AIF2ADC_DIV - [2:0] */
#define WM8994_AIF2ADC_DIV_WIDTH                     3  /* AIF2ADC_DIV - [2:0] */

/*
 * R520 (0x208) - Clocking (1)
 */
#define WM8958_DSP2CLK_ENA                      0x4000  /* DSP2CLK_ENA */
#define WM8958_DSP2CLK_ENA_MASK                 0x4000  /* DSP2CLK_ENA */
#define WM8958_DSP2CLK_ENA_SHIFT                    14  /* DSP2CLK_ENA */
#define WM8958_DSP2CLK_ENA_WIDTH                     1  /* DSP2CLK_ENA */
#define WM8958_DSP2CLK_SRC                      0x1000  /* DSP2CLK_SRC */
#define WM8958_DSP2CLK_SRC_MASK                 0x1000  /* DSP2CLK_SRC */
#define WM8958_DSP2CLK_SRC_SHIFT                    12  /* DSP2CLK_SRC */
#define WM8958_DSP2CLK_SRC_WIDTH                     1  /* DSP2CLK_SRC */
#define WM8994_TOCLK_ENA                        0x0010  /* TOCLK_ENA */
#define WM8994_TOCLK_ENA_MASK                   0x0010  /* TOCLK_ENA */
#define WM8994_TOCLK_ENA_SHIFT                       4  /* TOCLK_ENA */
#define WM8994_TOCLK_ENA_WIDTH                       1  /* TOCLK_ENA */
#define WM8994_AIF1DSPCLK_ENA                   0x0008  /* AIF1DSPCLK_ENA */
#define WM8994_AIF1DSPCLK_ENA_MASK              0x0008  /* AIF1DSPCLK_ENA */
#define WM8994_AIF1DSPCLK_ENA_SHIFT                  3  /* AIF1DSPCLK_ENA */
#define WM8994_AIF1DSPCLK_ENA_WIDTH                  1  /* AIF1DSPCLK_ENA */
#define WM8994_AIF2DSPCLK_ENA                   0x0004  /* AIF2DSPCLK_ENA */
#define WM8994_AIF2DSPCLK_ENA_MASK              0x0004  /* AIF2DSPCLK_ENA */
#define WM8994_AIF2DSPCLK_ENA_SHIFT                  2  /* AIF2DSPCLK_ENA */
#define WM8994_AIF2DSPCLK_ENA_WIDTH                  1  /* AIF2DSPCLK_ENA */
#define WM8994_SYSDSPCLK_ENA                    0x0002  /* SYSDSPCLK_ENA */
#define WM8994_SYSDSPCLK_ENA_MASK               0x0002  /* SYSDSPCLK_ENA */
#define WM8994_SYSDSPCLK_ENA_SHIFT                   1  /* SYSDSPCLK_ENA */
#define WM8994_SYSDSPCLK_ENA_WIDTH                   1  /* SYSDSPCLK_ENA */
#define WM8994_SYSCLK_SRC                       0x0001  /* SYSCLK_SRC */
#define WM8994_SYSCLK_SRC_MASK                  0x0001  /* SYSCLK_SRC */
#define WM8994_SYSCLK_SRC_SHIFT                      0  /* SYSCLK_SRC */
#define WM8994_SYSCLK_SRC_WIDTH                      1  /* SYSCLK_SRC */

/*
 * R521 (0x209) - Clocking (2)
 */
#define WM8994_TOCLK_DIV_MASK                   0x0700  /* TOCLK_DIV - [10:8] */
#define WM8994_TOCLK_DIV_SHIFT                       8  /* TOCLK_DIV - [10:8] */
#define WM8994_TOCLK_DIV_WIDTH                       3  /* TOCLK_DIV - [10:8] */
#define WM8994_DBCLK_DIV_MASK                   0x0070  /* DBCLK_DIV - [6:4] */
#define WM8994_DBCLK_DIV_SHIFT                       4  /* DBCLK_DIV - [6:4] */
#define WM8994_DBCLK_DIV_WIDTH                       3  /* DBCLK_DIV - [6:4] */
#define WM8994_OPCLK_DIV_MASK                   0x0007  /* OPCLK_DIV - [2:0] */
#define WM8994_OPCLK_DIV_SHIFT                       0  /* OPCLK_DIV - [2:0] */
#define WM8994_OPCLK_DIV_WIDTH                       3  /* OPCLK_DIV - [2:0] */

/*
 * R528 (0x210) - AIF1 Rate
 */
#define WM8994_AIF1_SR_MASK                     0x00F0  /* AIF1_SR - [7:4] */
#define WM8994_AIF1_SR_SHIFT                         4  /* AIF1_SR - [7:4] */
#define WM8994_AIF1_SR_WIDTH                         4  /* AIF1_SR - [7:4] */
#define WM8994_AIF1CLK_RATE_MASK                0x000F  /* AIF1CLK_RATE - [3:0] */
#define WM8994_AIF1CLK_RATE_SHIFT                    0  /* AIF1CLK_RATE - [3:0] */
#define WM8994_AIF1CLK_RATE_WIDTH                    4  /* AIF1CLK_RATE - [3:0] */

/*
 * R529 (0x211) - AIF2 Rate
 */
#define WM8994_AIF2_SR_MASK                     0x00F0  /* AIF2_SR - [7:4] */
#define WM8994_AIF2_SR_SHIFT                         4  /* AIF2_SR - [7:4] */
#define WM8994_AIF2_SR_WIDTH                         4  /* AIF2_SR - [7:4] */
#define WM8994_AIF2CLK_RATE_MASK                0x000F  /* AIF2CLK_RATE - [3:0] */
#define WM8994_AIF2CLK_RATE_SHIFT                    0  /* AIF2CLK_RATE - [3:0] */
#define WM8994_AIF2CLK_RATE_WIDTH                    4  /* AIF2CLK_RATE - [3:0] */

/*
 * R530 (0x212) - Rate Status
 */
#define WM8994_SR_ERROR_MASK                    0x000F  /* SR_ERROR - [3:0] */
#define WM8994_SR_ERROR_SHIFT                        0  /* SR_ERROR - [3:0] */
#define WM8994_SR_ERROR_WIDTH                        4  /* SR_ERROR - [3:0] */

/*
 * R768 (0x300) - AIF1 Control (1)
 */
#define WM8994_AIF1ADCL_SRC                     0x8000  /* AIF1ADCL_SRC */
#define WM8994_AIF1ADCL_SRC_MASK                0x8000  /* AIF1ADCL_SRC */
#define WM8994_AIF1ADCL_SRC_SHIFT                   15  /* AIF1ADCL_SRC */
#define WM8994_AIF1ADCL_SRC_WIDTH                    1  /* AIF1ADCL_SRC */
#define WM8994_AIF1ADCR_SRC                     0x4000  /* AIF1ADCR_SRC */
#define WM8994_AIF1ADCR_SRC_MASK                0x4000  /* AIF1ADCR_SRC */
#define WM8994_AIF1ADCR_SRC_SHIFT                   14  /* AIF1ADCR_SRC */
#define WM8994_AIF1ADCR_SRC_WIDTH                    1  /* AIF1ADCR_SRC */
#define WM8994_AIF1ADC_TDM                      0x2000  /* AIF1ADC_TDM */
#define WM8994_AIF1ADC_TDM_MASK                 0x2000  /* AIF1ADC_TDM */
#define WM8994_AIF1ADC_TDM_SHIFT                    13  /* AIF1ADC_TDM */
#define WM8994_AIF1ADC_TDM_WIDTH                     1  /* AIF1ADC_TDM */
#define WM8994_AIF1_BCLK_INV                    0x0100  /* AIF1_BCLK_INV */
#define WM8994_AIF1_BCLK_INV_MASK               0x0100  /* AIF1_BCLK_INV */
#define WM8994_AIF1_BCLK_INV_SHIFT                   8  /* AIF1_BCLK_INV */
#define WM8994_AIF1_BCLK_INV_WIDTH                   1  /* AIF1_BCLK_INV */
#define WM8994_AIF1_LRCLK_INV                   0x0080  /* AIF1_LRCLK_INV */
#define WM8994_AIF1_LRCLK_INV_MASK              0x0080  /* AIF1_LRCLK_INV */
#define WM8994_AIF1_LRCLK_INV_SHIFT                  7  /* AIF1_LRCLK_INV */
#define WM8994_AIF1_LRCLK_INV_WIDTH                  1  /* AIF1_LRCLK_INV */
#define WM8994_AIF1_WL_MASK                     0x0060  /* AIF1_WL - [6:5] */
#define WM8994_AIF1_WL_SHIFT                         5  /* AIF1_WL - [6:5] */
#define WM8994_AIF1_WL_WIDTH                         2  /* AIF1_WL - [6:5] */
#define WM8994_AIF1_FMT_MASK                    0x0018  /* AIF1_FMT - [4:3] */
#define WM8994_AIF1_FMT_SHIFT                        3  /* AIF1_FMT - [4:3] */
#define WM8994_AIF1_FMT_WIDTH                        2  /* AIF1_FMT - [4:3] */

/*
 * R769 (0x301) - AIF1 Control (2)
 */
#define WM8994_AIF1DACL_SRC                     0x8000  /* AIF1DACL_SRC */
#define WM8994_AIF1DACL_SRC_MASK                0x8000  /* AIF1DACL_SRC */
#define WM8994_AIF1DACL_SRC_SHIFT                   15  /* AIF1DACL_SRC */
#define WM8994_AIF1DACL_SRC_WIDTH                    1  /* AIF1DACL_SRC */
#define WM8994_AIF1DACR_SRC                     0x4000  /* AIF1DACR_SRC */
#define WM8994_AIF1DACR_SRC_MASK                0x4000  /* AIF1DACR_SRC */
#define WM8994_AIF1DACR_SRC_SHIFT                   14  /* AIF1DACR_SRC */
#define WM8994_AIF1DACR_SRC_WIDTH                    1  /* AIF1DACR_SRC */
#define WM8994_AIF1DAC_BOOST_MASK               0x0C00  /* AIF1DAC_BOOST - [11:10] */
#define WM8994_AIF1DAC_BOOST_SHIFT                  10  /* AIF1DAC_BOOST - [11:10] */
#define WM8994_AIF1DAC_BOOST_WIDTH                   2  /* AIF1DAC_BOOST - [11:10] */
#define WM8994_AIF1_MONO                        0x0100  /* AIF1_MONO */
#define WM8994_AIF1_MONO_MASK                   0x0100  /* AIF1_MONO */
#define WM8994_AIF1_MONO_SHIFT                       8  /* AIF1_MONO */
#define WM8994_AIF1_MONO_WIDTH                       1  /* AIF1_MONO */
#define WM8994_AIF1DAC_COMP                     0x0010  /* AIF1DAC_COMP */
#define WM8994_AIF1DAC_COMP_MASK                0x0010  /* AIF1DAC_COMP */
#define WM8994_AIF1DAC_COMP_SHIFT                    4  /* AIF1DAC_COMP */
#define WM8994_AIF1DAC_COMP_WIDTH                    1  /* AIF1DAC_COMP */
#define WM8994_AIF1DAC_COMPMODE                 0x0008  /* AIF1DAC_COMPMODE */
#define WM8994_AIF1DAC_COMPMODE_MASK            0x0008  /* AIF1DAC_COMPMODE */
#define WM8994_AIF1DAC_COMPMODE_SHIFT                3  /* AIF1DAC_COMPMODE */
#define WM8994_AIF1DAC_COMPMODE_WIDTH                1  /* AIF1DAC_COMPMODE */
#define WM8994_AIF1ADC_COMP                     0x0004  /* AIF1ADC_COMP */
#define WM8994_AIF1ADC_COMP_MASK                0x0004  /* AIF1ADC_COMP */
#define WM8994_AIF1ADC_COMP_SHIFT                    2  /* AIF1ADC_COMP */
#define WM8994_AIF1ADC_COMP_WIDTH                    1  /* AIF1ADC_COMP */
#define WM8994_AIF1ADC_COMPMODE                 0x0002  /* AIF1ADC_COMPMODE */
#define WM8994_AIF1ADC_COMPMODE_MASK            0x0002  /* AIF1ADC_COMPMODE */
#define WM8994_AIF1ADC_COMPMODE_SHIFT                1  /* AIF1ADC_COMPMODE */
#define WM8994_AIF1ADC_COMPMODE_WIDTH                1  /* AIF1ADC_COMPMODE */
#define WM8994_AIF1_LOOPBACK                    0x0001  /* AIF1_LOOPBACK */
#define WM8994_AIF1_LOOPBACK_MASK               0x0001  /* AIF1_LOOPBACK */
#define WM8994_AIF1_LOOPBACK_SHIFT                   0  /* AIF1_LOOPBACK */
#define WM8994_AIF1_LOOPBACK_WIDTH                   1  /* AIF1_LOOPBACK */

/*
 * R770 (0x302) - AIF1 Master/Slave
 */
#define WM8994_AIF1_TRI                         0x8000  /* AIF1_TRI */
#define WM8994_AIF1_TRI_MASK                    0x8000  /* AIF1_TRI */
#define WM8994_AIF1_TRI_SHIFT                       15  /* AIF1_TRI */
#define WM8994_AIF1_TRI_WIDTH                        1  /* AIF1_TRI */
#define WM8994_AIF1_MSTR                        0x4000  /* AIF1_MSTR */
#define WM8994_AIF1_MSTR_MASK                   0x4000  /* AIF1_MSTR */
#define WM8994_AIF1_MSTR_SHIFT                      14  /* AIF1_MSTR */
#define WM8994_AIF1_MSTR_WIDTH                       1  /* AIF1_MSTR */
#define WM8994_AIF1_CLK_FRC                     0x2000  /* AIF1_CLK_FRC */
#define WM8994_AIF1_CLK_FRC_MASK                0x2000  /* AIF1_CLK_FRC */
#define WM8994_AIF1_CLK_FRC_SHIFT                   13  /* AIF1_CLK_FRC */
#define WM8994_AIF1_CLK_FRC_WIDTH                    1  /* AIF1_CLK_FRC */
#define WM8994_AIF1_LRCLK_FRC                   0x1000  /* AIF1_LRCLK_FRC */
#define WM8994_AIF1_LRCLK_FRC_MASK              0x1000  /* AIF1_LRCLK_FRC */
#define WM8994_AIF1_LRCLK_FRC_SHIFT                 12  /* AIF1_LRCLK_FRC */
#define WM8994_AIF1_LRCLK_FRC_WIDTH                  1  /* AIF1_LRCLK_FRC */

/*
 * R771 (0x303) - AIF1 BCLK
 */
#define WM8994_AIF1_BCLK_DIV_MASK               0x01F0  /* AIF1_BCLK_DIV - [8:4] */
#define WM8994_AIF1_BCLK_DIV_SHIFT                   4  /* AIF1_BCLK_DIV - [8:4] */
#define WM8994_AIF1_BCLK_DIV_WIDTH                   5  /* AIF1_BCLK_DIV - [8:4] */

/*
 * R772 (0x304) - AIF1ADC LRCLK
 */
#define WM8994_AIF1ADC_LRCLK_DIR                0x0800  /* AIF1ADC_LRCLK_DIR */
#define WM8994_AIF1ADC_LRCLK_DIR_MASK           0x0800  /* AIF1ADC_LRCLK_DIR */
#define WM8994_AIF1ADC_LRCLK_DIR_SHIFT              11  /* AIF1ADC_LRCLK_DIR */
#define WM8994_AIF1ADC_LRCLK_DIR_WIDTH               1  /* AIF1ADC_LRCLK_DIR */
#define WM8994_AIF1ADC_RATE_MASK                0x07FF  /* AIF1ADC_RATE - [10:0] */
#define WM8994_AIF1ADC_RATE_SHIFT                    0  /* AIF1ADC_RATE - [10:0] */
#define WM8994_AIF1ADC_RATE_WIDTH                   11  /* AIF1ADC_RATE - [10:0] */

/*
 * R773 (0x305) - AIF1DAC LRCLK
 */
#define WM8994_AIF1DAC_LRCLK_DIR                0x0800  /* AIF1DAC_LRCLK_DIR */
#define WM8994_AIF1DAC_LRCLK_DIR_MASK           0x0800  /* AIF1DAC_LRCLK_DIR */
#define WM8994_AIF1DAC_LRCLK_DIR_SHIFT              11  /* AIF1DAC_LRCLK_DIR */
#define WM8994_AIF1DAC_LRCLK_DIR_WIDTH               1  /* AIF1DAC_LRCLK_DIR */
#define WM8994_AIF1DAC_RATE_MASK                0x07FF  /* AIF1DAC_RATE - [10:0] */
#define WM8994_AIF1DAC_RATE_SHIFT                    0  /* AIF1DAC_RATE - [10:0] */
#define WM8994_AIF1DAC_RATE_WIDTH                   11  /* AIF1DAC_RATE - [10:0] */

/*
 * R774 (0x306) - AIF1DAC Data
 */
#define WM8994_AIF1DACL_DAT_INV                 0x0002  /* AIF1DACL_DAT_INV */
#define WM8994_AIF1DACL_DAT_INV_MASK            0x0002  /* AIF1DACL_DAT_INV */
#define WM8994_AIF1DACL_DAT_INV_SHIFT                1  /* AIF1DACL_DAT_INV */
#define WM8994_AIF1DACL_DAT_INV_WIDTH                1  /* AIF1DACL_DAT_INV */
#define WM8994_AIF1DACR_DAT_INV                 0x0001  /* AIF1DACR_DAT_INV */
#define WM8994_AIF1DACR_DAT_INV_MASK            0x0001  /* AIF1DACR_DAT_INV */
#define WM8994_AIF1DACR_DAT_INV_SHIFT                0  /* AIF1DACR_DAT_INV */
#define WM8994_AIF1DACR_DAT_INV_WIDTH                1  /* AIF1DACR_DAT_INV */

/*
 * R784 (0x310) - AIF2 Control (1)
 */
#define WM8994_AIF2ADCL_SRC                     0x8000  /* AIF2ADCL_SRC */
#define WM8994_AIF2ADCL_SRC_MASK                0x8000  /* AIF2ADCL_SRC */
#define WM8994_AIF2ADCL_SRC_SHIFT                   15  /* AIF2ADCL_SRC */
#define WM8994_AIF2ADCL_SRC_WIDTH                    1  /* AIF2ADCL_SRC */
#define WM8994_AIF2ADCR_SRC                     0x4000  /* AIF2ADCR_SRC */
#define WM8994_AIF2ADCR_SRC_MASK                0x4000  /* AIF2ADCR_SRC */
#define WM8994_AIF2ADCR_SRC_SHIFT                   14  /* AIF2ADCR_SRC */
#define WM8994_AIF2ADCR_SRC_WIDTH                    1  /* AIF2ADCR_SRC */
#define WM8994_AIF2ADC_TDM                      0x2000  /* AIF2ADC_TDM */
#define WM8994_AIF2ADC_TDM_MASK                 0x2000  /* AIF2ADC_TDM */
#define WM8994_AIF2ADC_TDM_SHIFT                    13  /* AIF2ADC_TDM */
#define WM8994_AIF2ADC_TDM_WIDTH                     1  /* AIF2ADC_TDM */
#define WM8994_AIF2ADC_TDM_CHAN                 0x1000  /* AIF2ADC_TDM_CHAN */
#define WM8994_AIF2ADC_TDM_CHAN_MASK            0x1000  /* AIF2ADC_TDM_CHAN */
#define WM8994_AIF2ADC_TDM_CHAN_SHIFT               12  /* AIF2ADC_TDM_CHAN */
#define WM8994_AIF2ADC_TDM_CHAN_WIDTH                1  /* AIF2ADC_TDM_CHAN */
#define WM8994_AIF2_BCLK_INV                    0x0100  /* AIF2_BCLK_INV */
#define WM8994_AIF2_BCLK_INV_MASK               0x0100  /* AIF2_BCLK_INV */
#define WM8994_AIF2_BCLK_INV_SHIFT                   8  /* AIF2_BCLK_INV */
#define WM8994_AIF2_BCLK_INV_WIDTH                   1  /* AIF2_BCLK_INV */
#define WM8994_AIF2_LRCLK_INV                   0x0080  /* AIF2_LRCLK_INV */
#define WM8994_AIF2_LRCLK_INV_MASK              0x0080  /* AIF2_LRCLK_INV */
#define WM8994_AIF2_LRCLK_INV_SHIFT                  7  /* AIF2_LRCLK_INV */
#define WM8994_AIF2_LRCLK_INV_WIDTH                  1  /* AIF2_LRCLK_INV */
#define WM8994_AIF2_WL_MASK                     0x0060  /* AIF2_WL - [6:5] */
#define WM8994_AIF2_WL_SHIFT                         5  /* AIF2_WL - [6:5] */
#define WM8994_AIF2_WL_WIDTH                         2  /* AIF2_WL - [6:5] */
#define WM8994_AIF2_FMT_MASK                    0x0018  /* AIF2_FMT - [4:3] */
#define WM8994_AIF2_FMT_SHIFT                        3  /* AIF2_FMT - [4:3] */
#define WM8994_AIF2_FMT_WIDTH                        2  /* AIF2_FMT - [4:3] */

/*
 * R785 (0x311) - AIF2 Control (2)
 */
#define WM8994_AIF2DACL_SRC                     0x8000  /* AIF2DACL_SRC */
#define WM8994_AIF2DACL_SRC_MASK                0x8000  /* AIF2DACL_SRC */
#define WM8994_AIF2DACL_SRC_SHIFT                   15  /* AIF2DACL_SRC */
#define WM8994_AIF2DACL_SRC_WIDTH                    1  /* AIF2DACL_SRC */
#define WM8994_AIF2DACR_SRC                     0x4000  /* AIF2DACR_SRC */
#define WM8994_AIF2DACR_SRC_MASK                0x4000  /* AIF2DACR_SRC */
#define WM8994_AIF2DACR_SRC_SHIFT                   14  /* AIF2DACR_SRC */
#define WM8994_AIF2DACR_SRC_WIDTH                    1  /* AIF2DACR_SRC */
#define WM8994_AIF2DAC_TDM                      0x2000  /* AIF2DAC_TDM */
#define WM8994_AIF2DAC_TDM_MASK                 0x2000  /* AIF2DAC_TDM */
#define WM8994_AIF2DAC_TDM_SHIFT                    13  /* AIF2DAC_TDM */
#define WM8994_AIF2DAC_TDM_WIDTH                     1  /* AIF2DAC_TDM */
#define WM8994_AIF2DAC_TDM_CHAN                 0x1000  /* AIF2DAC_TDM_CHAN */
#define WM8994_AIF2DAC_TDM_CHAN_MASK            0x1000  /* AIF2DAC_TDM_CHAN */
#define WM8994_AIF2DAC_TDM_CHAN_SHIFT               12  /* AIF2DAC_TDM_CHAN */
#define WM8994_AIF2DAC_TDM_CHAN_WIDTH                1  /* AIF2DAC_TDM_CHAN */
#define WM8994_AIF2DAC_BOOST_MASK               0x0C00  /* AIF2DAC_BOOST - [11:10] */
#define WM8994_AIF2DAC_BOOST_SHIFT                  10  /* AIF2DAC_BOOST - [11:10] */
#define WM8994_AIF2DAC_BOOST_WIDTH                   2  /* AIF2DAC_BOOST - [11:10] */
#define WM8994_AIF2_MONO                        0x0100  /* AIF2_MONO */
#define WM8994_AIF2_MONO_MASK                   0x0100  /* AIF2_MONO */
#define WM8994_AIF2_MONO_SHIFT                       8  /* AIF2_MONO */
#define WM8994_AIF2_MONO_WIDTH                       1  /* AIF2_MONO */
#define WM8994_AIF2DAC_COMP                     0x0010  /* AIF2DAC_COMP */
#define WM8994_AIF2DAC_COMP_MASK                0x0010  /* AIF2DAC_COMP */
#define WM8994_AIF2DAC_COMP_SHIFT                    4  /* AIF2DAC_COMP */
#define WM8994_AIF2DAC_COMP_WIDTH                    1  /* AIF2DAC_COMP */
#define WM8994_AIF2DAC_COMPMODE                 0x0008  /* AIF2DAC_COMPMODE */
#define WM8994_AIF2DAC_COMPMODE_MASK            0x0008  /* AIF2DAC_COMPMODE */
#define WM8994_AIF2DAC_COMPMODE_SHIFT                3  /* AIF2DAC_COMPMODE */
#define WM8994_AIF2DAC_COMPMODE_WIDTH                1  /* AIF2DAC_COMPMODE */
#define WM8994_AIF2ADC_COMP                     0x0004  /* AIF2ADC_COMP */
#define WM8994_AIF2ADC_COMP_MASK                0x0004  /* AIF2ADC_COMP */
#define WM8994_AIF2ADC_COMP_SHIFT                    2  /* AIF2ADC_COMP */
#define WM8994_AIF2ADC_COMP_WIDTH                    1  /* AIF2ADC_COMP */
#define WM8994_AIF2ADC_COMPMODE                 0x0002  /* AIF2ADC_COMPMODE */
#define WM8994_AIF2ADC_COMPMODE_MASK            0x0002  /* AIF2ADC_COMPMODE */
#define WM8994_AIF2ADC_COMPMODE_SHIFT                1  /* AIF2ADC_COMPMODE */
#define WM8994_AIF2ADC_COMPMODE_WIDTH                1  /* AIF2ADC_COMPMODE */
#define WM8994_AIF2_LOOPBACK                    0x0001  /* AIF2_LOOPBACK */
#define WM8994_AIF2_LOOPBACK_MASK               0x0001  /* AIF2_LOOPBACK */
#define WM8994_AIF2_LOOPBACK_SHIFT                   0  /* AIF2_LOOPBACK */
#define WM8994_AIF2_LOOPBACK_WIDTH                   1  /* AIF2_LOOPBACK */

/*
 * R786 (0x312) - AIF2 Master/Slave
 */
#define WM8994_AIF2_TRI                         0x8000  /* AIF2_TRI */
#define WM8994_AIF2_TRI_MASK                    0x8000  /* AIF2_TRI */
#define WM8994_AIF2_TRI_SHIFT                       15  /* AIF2_TRI */
#define WM8994_AIF2_TRI_WIDTH                        1  /* AIF2_TRI */
#define WM8994_AIF2_MSTR                        0x4000  /* AIF2_MSTR */
#define WM8994_AIF2_MSTR_MASK                   0x4000  /* AIF2_MSTR */
#define WM8994_AIF2_MSTR_SHIFT                      14  /* AIF2_MSTR */
#define WM8994_AIF2_MSTR_WIDTH                       1  /* AIF2_MSTR */
#define WM8994_AIF2_CLK_FRC                     0x2000  /* AIF2_CLK_FRC */
#define WM8994_AIF2_CLK_FRC_MASK                0x2000  /* AIF2_CLK_FRC */
#define WM8994_AIF2_CLK_FRC_SHIFT                   13  /* AIF2_CLK_FRC */
#define WM8994_AIF2_CLK_FRC_WIDTH                    1  /* AIF2_CLK_FRC */
#define WM8994_AIF2_LRCLK_FRC                   0x1000  /* AIF2_LRCLK_FRC */
#define WM8994_AIF2_LRCLK_FRC_MASK              0x1000  /* AIF2_LRCLK_FRC */
#define WM8994_AIF2_LRCLK_FRC_SHIFT                 12  /* AIF2_LRCLK_FRC */
#define WM8994_AIF2_LRCLK_FRC_WIDTH                  1  /* AIF2_LRCLK_FRC */

/*
 * R787 (0x313) - AIF2 BCLK
 */
#define WM8994_AIF2_BCLK_DIV_MASK               0x01F0  /* AIF2_BCLK_DIV - [8:4] */
#define WM8994_AIF2_BCLK_DIV_SHIFT                   4  /* AIF2_BCLK_DIV - [8:4] */
#define WM8994_AIF2_BCLK_DIV_WIDTH                   5  /* AIF2_BCLK_DIV - [8:4] */

/*
 * R788 (0x314) - AIF2ADC LRCLK
 */
#define WM8994_AIF2ADC_LRCLK_DIR                0x0800  /* AIF2ADC_LRCLK_DIR */
#define WM8994_AIF2ADC_LRCLK_DIR_MASK           0x0800  /* AIF2ADC_LRCLK_DIR */
#define WM8994_AIF2ADC_LRCLK_DIR_SHIFT              11  /* AIF2ADC_LRCLK_DIR */
#define WM8994_AIF2ADC_LRCLK_DIR_WIDTH               1  /* AIF2ADC_LRCLK_DIR */
#define WM8994_AIF2ADC_RATE_MASK                0x07FF  /* AIF2ADC_RATE - [10:0] */
#define WM8994_AIF2ADC_RATE_SHIFT                    0  /* AIF2ADC_RATE - [10:0] */
#define WM8994_AIF2ADC_RATE_WIDTH                   11  /* AIF2ADC_RATE - [10:0] */

/*
 * R789 (0x315) - AIF2DAC LRCLK
 */
#define WM8994_AIF2DAC_LRCLK_DIR                0x0800  /* AIF2DAC_LRCLK_DIR */
#define WM8994_AIF2DAC_LRCLK_DIR_MASK           0x0800  /* AIF2DAC_LRCLK_DIR */
#define WM8994_AIF2DAC_LRCLK_DIR_SHIFT              11  /* AIF2DAC_LRCLK_DIR */
#define WM8994_AIF2DAC_LRCLK_DIR_WIDTH               1  /* AIF2DAC_LRCLK_DIR */
#define WM8994_AIF2DAC_RATE_MASK                0x07FF  /* AIF2DAC_RATE - [10:0] */
#define WM8994_AIF2DAC_RATE_SHIFT                    0  /* AIF2DAC_RATE - [10:0] */
#define WM8994_AIF2DAC_RATE_WIDTH                   11  /* AIF2DAC_RATE - [10:0] */

/*
 * R790 (0x316) - AIF2DAC Data
 */
#define WM8994_AIF2DACL_DAT_INV                 0x0002  /* AIF2DACL_DAT_INV */
#define WM8994_AIF2DACL_DAT_INV_MASK            0x0002  /* AIF2DACL_DAT_INV */
#define WM8994_AIF2DACL_DAT_INV_SHIFT                1  /* AIF2DACL_DAT_INV */
#define WM8994_AIF2DACL_DAT_INV_WIDTH                1  /* AIF2DACL_DAT_INV */
#define WM8994_AIF2DACR_DAT_INV                 0x0001  /* AIF2DACR_DAT_INV */
#define WM8994_AIF2DACR_DAT_INV_MASK            0x0001  /* AIF2DACR_DAT_INV */
#define WM8994_AIF2DACR_DAT_INV_SHIFT                0  /* AIF2DACR_DAT_INV */
#define WM8994_AIF2DACR_DAT_INV_WIDTH                1  /* AIF2DACR_DAT_INV */

/*
 * R1024 (0x400) - AIF1 ADC1 Left Volume
 */
#define WM8994_AIF1ADC1_VU                      0x0100  /* AIF1ADC1_VU */
#define WM8994_AIF1ADC1_VU_MASK                 0x0100  /* AIF1ADC1_VU */
#define WM8994_AIF1ADC1_VU_SHIFT                     8  /* AIF1ADC1_VU */
#define WM8994_AIF1ADC1_VU_WIDTH                     1  /* AIF1ADC1_VU */
#define WM8994_AIF1ADC1L_VOL_MASK               0x00FF  /* AIF1ADC1L_VOL - [7:0] */
#define WM8994_AIF1ADC1L_VOL_SHIFT                   0  /* AIF1ADC1L_VOL - [7:0] */
#define WM8994_AIF1ADC1L_VOL_WIDTH                   8  /* AIF1ADC1L_VOL - [7:0] */

/*
 * R1025 (0x401) - AIF1 ADC1 Right Volume
 */
#define WM8994_AIF1ADC1_VU                      0x0100  /* AIF1ADC1_VU */
#define WM8994_AIF1ADC1_VU_MASK                 0x0100  /* AIF1ADC1_VU */
#define WM8994_AIF1ADC1_VU_SHIFT                     8  /* AIF1ADC1_VU */
#define WM8994_AIF1ADC1_VU_WIDTH                     1  /* AIF1ADC1_VU */
#define WM8994_AIF1ADC1R_VOL_MASK               0x00FF  /* AIF1ADC1R_VOL - [7:0] */
#define WM8994_AIF1ADC1R_VOL_SHIFT                   0  /* AIF1ADC1R_VOL - [7:0] */
#define WM8994_AIF1ADC1R_VOL_WIDTH                   8  /* AIF1ADC1R_VOL - [7:0] */

/*
 * R1026 (0x402) - AIF1 DAC1 Left Volume
 */
#define WM8994_AIF1DAC1_VU                      0x0100  /* AIF1DAC1_VU */
#define WM8994_AIF1DAC1_VU_MASK                 0x0100  /* AIF1DAC1_VU */
#define WM8994_AIF1DAC1_VU_SHIFT                     8  /* AIF1DAC1_VU */
#define WM8994_AIF1DAC1_VU_WIDTH                     1  /* AIF1DAC1_VU */
#define WM8994_AIF1DAC1L_VOL_MASK               0x00FF  /* AIF1DAC1L_VOL - [7:0] */
#define WM8994_AIF1DAC1L_VOL_SHIFT                   0  /* AIF1DAC1L_VOL - [7:0] */
#define WM8994_AIF1DAC1L_VOL_WIDTH                   8  /* AIF1DAC1L_VOL - [7:0] */

/*
 * R1027 (0x403) - AIF1 DAC1 Right Volume
 */
#define WM8994_AIF1DAC1_VU                      0x0100  /* AIF1DAC1_VU */
#define WM8994_AIF1DAC1_VU_MASK                 0x0100  /* AIF1DAC1_VU */
#define WM8994_AIF1DAC1_VU_SHIFT                     8  /* AIF1DAC1_VU */
#define WM8994_AIF1DAC1_VU_WIDTH                     1  /* AIF1DAC1_VU */
#define WM8994_AIF1DAC1R_VOL_MASK               0x00FF  /* AIF1DAC1R_VOL - [7:0] */
#define WM8994_AIF1DAC1R_VOL_SHIFT                   0  /* AIF1DAC1R_VOL - [7:0] */
#define WM8994_AIF1DAC1R_VOL_WIDTH                   8  /* AIF1DAC1R_VOL - [7:0] */

/*
 * R1028 (0x404) - AIF1 ADC2 Left Volume
 */
#define WM8994_AIF1ADC2_VU                      0x0100  /* AIF1ADC2_VU */
#define WM8994_AIF1ADC2_VU_MASK                 0x0100  /* AIF1ADC2_VU */
#define WM8994_AIF1ADC2_VU_SHIFT                     8  /* AIF1ADC2_VU */
#define WM8994_AIF1ADC2_VU_WIDTH                     1  /* AIF1ADC2_VU */
#define WM8994_AIF1ADC2L_VOL_MASK               0x00FF  /* AIF1ADC2L_VOL - [7:0] */
#define WM8994_AIF1ADC2L_VOL_SHIFT                   0  /* AIF1ADC2L_VOL - [7:0] */
#define WM8994_AIF1ADC2L_VOL_WIDTH                   8  /* AIF1ADC2L_VOL - [7:0] */

/*
 * R1029 (0x405) - AIF1 ADC2 Right Volume
 */
#define WM8994_AIF1ADC2_VU                      0x0100  /* AIF1ADC2_VU */
#define WM8994_AIF1ADC2_VU_MASK                 0x0100  /* AIF1ADC2_VU */
#define WM8994_AIF1ADC2_VU_SHIFT                     8  /* AIF1ADC2_VU */
#define WM8994_AIF1ADC2_VU_WIDTH                     1  /* AIF1ADC2_VU */
#define WM8994_AIF1ADC2R_VOL_MASK               0x00FF  /* AIF1ADC2R_VOL - [7:0] */
#define WM8994_AIF1ADC2R_VOL_SHIFT                   0  /* AIF1ADC2R_VOL - [7:0] */
#define WM8994_AIF1ADC2R_VOL_WIDTH                   8  /* AIF1ADC2R_VOL - [7:0] */

/*
 * R1030 (0x406) - AIF1 DAC2 Left Volume
 */
#define WM8994_AIF1DAC2_VU                      0x0100  /* AIF1DAC2_VU */
#define WM8994_AIF1DAC2_VU_MASK                 0x0100  /* AIF1DAC2_VU */
#define WM8994_AIF1DAC2_VU_SHIFT                     8  /* AIF1DAC2_VU */
#define WM8994_AIF1DAC2_VU_WIDTH                     1  /* AIF1DAC2_VU */
#define WM8994_AIF1DAC2L_VOL_MASK               0x00FF  /* AIF1DAC2L_VOL - [7:0] */
#define WM8994_AIF1DAC2L_VOL_SHIFT                   0  /* AIF1DAC2L_VOL - [7:0] */
#define WM8994_AIF1DAC2L_VOL_WIDTH                   8  /* AIF1DAC2L_VOL - [7:0] */

/*
 * R1031 (0x407) - AIF1 DAC2 Right Volume
 */
#define WM8994_AIF1DAC2_VU                      0x0100  /* AIF1DAC2_VU */
#define WM8994_AIF1DAC2_VU_MASK                 0x0100  /* AIF1DAC2_VU */
#define WM8994_AIF1DAC2_VU_SHIFT                     8  /* AIF1DAC2_VU */
#define WM8994_AIF1DAC2_VU_WIDTH                     1  /* AIF1DAC2_VU */
#define WM8994_AIF1DAC2R_VOL_MASK               0x00FF  /* AIF1DAC2R_VOL - [7:0] */
#define WM8994_AIF1DAC2R_VOL_SHIFT                   0  /* AIF1DAC2R_VOL - [7:0] */
#define WM8994_AIF1DAC2R_VOL_WIDTH                   8  /* AIF1DAC2R_VOL - [7:0] */

/*
 * R1040 (0x410) - AIF1 ADC1 Filters
 */
#define WM8994_AIF1ADC_4FS                      0x8000  /* AIF1ADC_4FS */
#define WM8994_AIF1ADC_4FS_MASK                 0x8000  /* AIF1ADC_4FS */
#define WM8994_AIF1ADC_4FS_SHIFT                    15  /* AIF1ADC_4FS */
#define WM8994_AIF1ADC_4FS_WIDTH                     1  /* AIF1ADC_4FS */
#define WM8994_AIF1ADC1_HPF_CUT_MASK            0x6000  /* AIF1ADC1_HPF_CUT - [14:13] */
#define WM8994_AIF1ADC1_HPF_CUT_SHIFT               13  /* AIF1ADC1_HPF_CUT - [14:13] */
#define WM8994_AIF1ADC1_HPF_CUT_WIDTH                2  /* AIF1ADC1_HPF_CUT - [14:13] */
#define WM8994_AIF1ADC1L_HPF                    0x1000  /* AIF1ADC1L_HPF */
#define WM8994_AIF1ADC1L_HPF_MASK               0x1000  /* AIF1ADC1L_HPF */
#define WM8994_AIF1ADC1L_HPF_SHIFT                  12  /* AIF1ADC1L_HPF */
#define WM8994_AIF1ADC1L_HPF_WIDTH                   1  /* AIF1ADC1L_HPF */
#define WM8994_AIF1ADC1R_HPF                    0x0800  /* AIF1ADC1R_HPF */
#define WM8994_AIF1ADC1R_HPF_MASK               0x0800  /* AIF1ADC1R_HPF */
#define WM8994_AIF1ADC1R_HPF_SHIFT                  11  /* AIF1ADC1R_HPF */
#define WM8994_AIF1ADC1R_HPF_WIDTH                   1  /* AIF1ADC1R_HPF */

/*
 * R1041 (0x411) - AIF1 ADC2 Filters
 */
#define WM8994_AIF1ADC2_HPF_CUT_MASK            0x6000  /* AIF1ADC2_HPF_CUT - [14:13] */
#define WM8994_AIF1ADC2_HPF_CUT_SHIFT               13  /* AIF1ADC2_HPF_CUT - [14:13] */
#define WM8994_AIF1ADC2_HPF_CUT_WIDTH                2  /* AIF1ADC2_HPF_CUT - [14:13] */
#define WM8994_AIF1ADC2L_HPF                    0x1000  /* AIF1ADC2L_HPF */
#define WM8994_AIF1ADC2L_HPF_MASK               0x1000  /* AIF1ADC2L_HPF */
#define WM8994_AIF1ADC2L_HPF_SHIFT                  12  /* AIF1ADC2L_HPF */
#define WM8994_AIF1ADC2L_HPF_WIDTH                   1  /* AIF1ADC2L_HPF */
#define WM8994_AIF1ADC2R_HPF                    0x0800  /* AIF1ADC2R_HPF */
#define WM8994_AIF1ADC2R_HPF_MASK               0x0800  /* AIF1ADC2R_HPF */
#define WM8994_AIF1ADC2R_HPF_SHIFT                  11  /* AIF1ADC2R_HPF */
#define WM8994_AIF1ADC2R_HPF_WIDTH                   1  /* AIF1ADC2R_HPF */

/*
 * R1056 (0x420) - AIF1 DAC1 Filters (1)
 */
#define WM8994_AIF1DAC1_MUTE                    0x0200  /* AIF1DAC1_MUTE */
#define WM8994_AIF1DAC1_MUTE_MASK               0x0200  /* AIF1DAC1_MUTE */
#define WM8994_AIF1DAC1_MUTE_SHIFT                   9  /* AIF1DAC1_MUTE */
#define WM8994_AIF1DAC1_MUTE_WIDTH                   1  /* AIF1DAC1_MUTE */
#define WM8994_AIF1DAC1_MONO                    0x0080  /* AIF1DAC1_MONO */
#define WM8994_AIF1DAC1_MONO_MASK               0x0080  /* AIF1DAC1_MONO */
#define WM8994_AIF1DAC1_MONO_SHIFT                   7  /* AIF1DAC1_MONO */
#define WM8994_AIF1DAC1_MONO_WIDTH                   1  /* AIF1DAC1_MONO */
#define WM8994_AIF1DAC1_MUTERATE                0x0020  /* AIF1DAC1_MUTERATE */
#define WM8994_AIF1DAC1_MUTERATE_MASK           0x0020  /* AIF1DAC1_MUTERATE */
#define WM8994_AIF1DAC1_MUTERATE_SHIFT               5  /* AIF1DAC1_MUTERATE */
#define WM8994_AIF1DAC1_MUTERATE_WIDTH               1  /* AIF1DAC1_MUTERATE */
#define WM8994_AIF1DAC1_UNMUTE_RAMP             0x0010  /* AIF1DAC1_UNMUTE_RAMP */
#define WM8994_AIF1DAC1_UNMUTE_RAMP_MASK        0x0010  /* AIF1DAC1_UNMUTE_RAMP */
#define WM8994_AIF1DAC1_UNMUTE_RAMP_SHIFT            4  /* AIF1DAC1_UNMUTE_RAMP */
#define WM8994_AIF1DAC1_UNMUTE_RAMP_WIDTH            1  /* AIF1DAC1_UNMUTE_RAMP */
#define WM8994_AIF1DAC1_DEEMP_MASK              0x0006  /* AIF1DAC1_DEEMP - [2:1] */
#define WM8994_AIF1DAC1_DEEMP_SHIFT                  1  /* AIF1DAC1_DEEMP - [2:1] */
#define WM8994_AIF1DAC1_DEEMP_WIDTH                  2  /* AIF1DAC1_DEEMP - [2:1] */

/*
 * R1057 (0x421) - AIF1 DAC1 Filters (2)
 */
#define WM8994_AIF1DAC1_3D_GAIN_MASK            0x3E00  /* AIF1DAC1_3D_GAIN - [13:9] */
#define WM8994_AIF1DAC1_3D_GAIN_SHIFT                9  /* AIF1DAC1_3D_GAIN - [13:9] */
#define WM8994_AIF1DAC1_3D_GAIN_WIDTH                5  /* AIF1DAC1_3D_GAIN - [13:9] */
#define WM8994_AIF1DAC1_3D_ENA                  0x0100  /* AIF1DAC1_3D_ENA */
#define WM8994_AIF1DAC1_3D_ENA_MASK             0x0100  /* AIF1DAC1_3D_ENA */
#define WM8994_AIF1DAC1_3D_ENA_SHIFT                 8  /* AIF1DAC1_3D_ENA */
#define WM8994_AIF1DAC1_3D_ENA_WIDTH                 1  /* AIF1DAC1_3D_ENA */

/*
 * R1282 (0x502) - AIF2 DAC Left Volume
 */
#define WM8994_AIF2DAC_VU                       0x0100  /* AIF2DAC_VU */
#define WM8994_AIF2DAC_VU_MASK                  0x0100  /* AIF2DAC_VU */
#define WM8994_AIF2DAC_VU_SHIFT                      8  /* AIF2DAC_VU */
#define WM8994_AIF2DAC_VU_WIDTH                      1  /* AIF2DAC_VU */
#define WM8994_AIF2DACL_VOL_MASK                0x00FF  /* AIF2DACL_VOL - [7:0] */
#define WM8994_AIF2DACL_VOL_SHIFT                    0  /* AIF2DACL_VOL - [7:0] */
#define WM8994_AIF2DACL_VOL_WIDTH                    8  /* AIF2DACL_VOL - [7:0] */

/*
 * R1283 (0x503) - AIF2 DAC Right Volume
 */
#define WM8994_AIF2DAC_VU                       0x0100  /* AIF2DAC_VU */
#define WM8994_AIF2DAC_VU_MASK                  0x0100  /* AIF2DAC_VU */
#define WM8994_AIF2DAC_VU_SHIFT                      8  /* AIF2DAC_VU */
#define WM8994_AIF2DAC_VU_WIDTH                      1  /* AIF2DAC_VU */
#define WM8994_AIF2DACR_VOL_MASK                0x00FF  /* AIF2DACR_VOL - [7:0] */
#define WM8994_AIF2DACR_VOL_SHIFT                    0  /* AIF2DACR_VOL - [7:0] */
#define WM8994_AIF2DACR_VOL_WIDTH                    8  /* AIF2DACR_VOL - [7:0] */

/*
 * R1312 (0x520) - AIF2 DAC Filters (1)
 */
#define WM8994_AIF2DAC_MUTE                     0x0200  /* AIF2DAC_MUTE */
#define WM8994_AIF2DAC_MUTE_MASK                0x0200  /* AIF2DAC_MUTE */
#define WM8994_AIF2DAC_MUTE_SHIFT                    9  /* AIF2DAC_MUTE */
#define WM8994_AIF2DAC_MUTE_WIDTH                    1  /* AIF2DAC_MUTE */
#define WM8994_AIF2DAC_MONO                     0x0080  /* AIF2DAC_MONO */
#define WM8994_AIF2DAC_MONO_MASK                0x0080  /* AIF2DAC_MONO */
#define WM8994_AIF2DAC_MONO_SHIFT                    7  /* AIF2DAC_MONO */
#define WM8994_AIF2DAC_MONO_WIDTH                    1  /* AIF2DAC_MONO */
#define WM8994_AIF2DAC_MUTERATE                 0x0020  /* AIF2DAC_MUTERATE */
#define WM8994_AIF2DAC_MUTERATE_MASK            0x0020  /* AIF2DAC_MUTERATE */
#define WM8994_AIF2DAC_MUTERATE_SHIFT                5  /* AIF2DAC_MUTERATE */
#define WM8994_AIF2DAC_MUTERATE_WIDTH                1  /* AIF2DAC_MUTERATE */
#define WM8994_AIF2DAC_UNMUTE_RAMP              0x0010  /* AIF2DAC_UNMUTE_RAMP */
#define WM8994_AIF2DAC_UNMUTE_RAMP_MASK         0x0010  /* AIF2DAC_UNMUTE_RAMP */
#define WM8994_AIF2DAC_UNMUTE_RAMP_SHIFT             4  /* AIF2DAC_UNMUTE_RAMP */
#define WM8994_AIF2DAC_UNMUTE_RAMP_WIDTH             1  /* AIF2DAC_UNMUTE_RAMP */
#define WM8994_AIF2DAC_DEEMP_MASK               0x0006  /* AIF2DAC_DEEMP - [2:1] */
#define WM8994_AIF2DAC_DEEMP_SHIFT                   1  /* AIF2DAC_DEEMP - [2:1] */
#define WM8994_AIF2DAC_DEEMP_WIDTH                   2  /* AIF2DAC_DEEMP - [2:1] */

/*
 * R1313 (0x521) - AIF2 DAC Filters (2)
 */
#define WM8994_AIF2DAC_3D_GAIN_MASK             0x3E00  /* AIF2DAC_3D_GAIN - [13:9] */
#define WM8994_AIF2DAC_3D_GAIN_SHIFT                 9  /* AIF2DAC_3D_GAIN - [13:9] */
#define WM8994_AIF2DAC_3D_GAIN_WIDTH                 5  /* AIF2DAC_3D_GAIN - [13:9] */
#define WM8994_AIF2DAC_3D_ENA                   0x0100  /* AIF2DAC_3D_ENA */
#define WM8994_AIF2DAC_3D_ENA_MASK              0x0100  /* AIF2DAC_3D_ENA */
#define WM8994_AIF2DAC_3D_ENA_SHIFT                  8  /* AIF2DAC_3D_ENA */
#define WM8994_AIF2DAC_3D_ENA_WIDTH                  1  /* AIF2DAC_3D_ENA */

/*
 * R1536 (0x600) - DAC1 Mixer Volumes
 */
#define WM8994_ADCR_DAC1_VOL_MASK               0x01E0  /* ADCR_DAC1_VOL - [8:5] */
#define WM8994_ADCR_DAC1_VOL_SHIFT                   5  /* ADCR_DAC1_VOL - [8:5] */
#define WM8994_ADCR_DAC1_VOL_WIDTH                   4  /* ADCR_DAC1_VOL - [8:5] */
#define WM8994_ADCL_DAC1_VOL_MASK               0x000F  /* ADCL_DAC1_VOL - [3:0] */
#define WM8994_ADCL_DAC1_VOL_SHIFT                   0  /* ADCL_DAC1_VOL - [3:0] */
#define WM8994_ADCL_DAC1_VOL_WIDTH                   4  /* ADCL_DAC1_VOL - [3:0] */

/*
 * R1537 (0x601) - DAC1 Left Mixer Routing
 */
#define WM8994_ADCR_TO_DAC1L                    0x0020  /* ADCR_TO_DAC1L */
#define WM8994_ADCR_TO_DAC1L_MASK               0x0020  /* ADCR_TO_DAC1L */
#define WM8994_ADCR_TO_DAC1L_SHIFT                   5  /* ADCR_TO_DAC1L */
#define WM8994_ADCR_TO_DAC1L_WIDTH                   1  /* ADCR_TO_DAC1L */
#define WM8994_ADCL_TO_DAC1L                    0x0010  /* ADCL_TO_DAC1L */
#define WM8994_ADCL_TO_DAC1L_MASK               0x0010  /* ADCL_TO_DAC1L */
#define WM8994_ADCL_TO_DAC1L_SHIFT                   4  /* ADCL_TO_DAC1L */
#define WM8994_ADCL_TO_DAC1L_WIDTH                   1  /* ADCL_TO_DAC1L */
#define WM8994_AIF2DACL_TO_DAC1L                0x0004  /* AIF2DACL_TO_DAC1L */
#define WM8994_AIF2DACL_TO_DAC1L_MASK           0x0004  /* AIF2DACL_TO_DAC1L */
#define WM8994_AIF2DACL_TO_DAC1L_SHIFT               2  /* AIF2DACL_TO_DAC1L */
#define WM8994_AIF2DACL_TO_DAC1L_WIDTH               1  /* AIF2DACL_TO_DAC1L */
#define WM8994_AIF1DAC2L_TO_DAC1L               0x0002  /* AIF1DAC2L_TO_DAC1L */
#define WM8994_AIF1DAC2L_TO_DAC1L_MASK          0x0002  /* AIF1DAC2L_TO_DAC1L */
#define WM8994_AIF1DAC2L_TO_DAC1L_SHIFT              1  /* AIF1DAC2L_TO_DAC1L */
#define WM8994_AIF1DAC2L_TO_DAC1L_WIDTH              1  /* AIF1DAC2L_TO_DAC1L */
#define WM8994_AIF1DAC1L_TO_DAC1L               0x0001  /* AIF1DAC1L_TO_DAC1L */
#define WM8994_AIF1DAC1L_TO_DAC1L_MASK          0x0001  /* AIF1DAC1L_TO_DAC1L */
#define WM8994_AIF1DAC1L_TO_DAC1L_SHIFT              0  /* AIF1DAC1L_TO_DAC1L */
#define WM8994_AIF1DAC1L_TO_DAC1L_WIDTH              1  /* AIF1DAC1L_TO_DAC1L */

/*
 * R1538 (0x602) - DAC1 Right Mixer Routing
 */
#define WM8994_ADCR_TO_DAC1R                    0x0020  /* ADCR_TO_DAC1R */
#define WM8994_ADCR_TO_DAC1R_MASK               0x0020  /* ADCR_TO_DAC1R */
#define WM8994_ADCR_TO_DAC1R_SHIFT                   5  /* ADCR_TO_DAC1R */
#define WM8994_ADCR_TO_DAC1R_WIDTH                   1  /* ADCR_TO_DAC1R */
#define WM8994_ADCL_TO_DAC1R                    0x0010  /* ADCL_TO_DAC1R */
#define WM8994_ADCL_TO_DAC1R_MASK               0x0010  /* ADCL_TO_DAC1R */
#define WM8994_ADCL_TO_DAC1R_SHIFT                   4  /* ADCL_TO_DAC1R */
#define WM8994_ADCL_TO_DAC1R_WIDTH                   1  /* ADCL_TO_DAC1R */
#define WM8994_AIF2DACR_TO_DAC1R                0x0004  /* AIF2DACR_TO_DAC1R */
#define WM8994_AIF2DACR_TO_DAC1R_MASK           0x0004  /* AIF2DACR_TO_DAC1R */
#define WM8994_AIF2DACR_TO_DAC1R_SHIFT               2  /* AIF2DACR_TO_DAC1R */
#define WM8994_AIF2DACR_TO_DAC1R_WIDTH               1  /* AIF2DACR_TO_DAC1R */
#define WM8994_AIF1DAC2R_TO_DAC1R               0x0002  /* AIF1DAC2R_TO_DAC1R */
#define WM8994_AIF1DAC2R_TO_DAC1R_MASK          0x0002  /* AIF1DAC2R_TO_DAC1R */
#define WM8994_AIF1DAC2R_TO_DAC1R_SHIFT              1  /* AIF1DAC2R_TO_DAC1R */
#define WM8994_AIF1DAC2R_TO_DAC1R_WIDTH              1  /* AIF1DAC2R_TO_DAC1R */
#define WM8994_AIF1DAC1R_TO_DAC1R               0x0001  /* AIF1DAC1R_TO_DAC1R */
#define WM8994_AIF1DAC1R_TO_DAC1R_MASK          0x0001  /* AIF1DAC1R_TO_DAC1R */
#define WM8994_AIF1DAC1R_TO_DAC1R_SHIFT              0  /* AIF1DAC1R_TO_DAC1R */
#define WM8994_AIF1DAC1R_TO_DAC1R_WIDTH              1  /* AIF1DAC1R_TO_DAC1R */

/*
 * R1539 (0x603) - DAC2 Mixer Volumes
 */
#define WM8994_ADCR_DAC2_VOL_MASK               0x01E0  /* ADCR_DAC2_VOL - [8:5] */
#define WM8994_ADCR_DAC2_VOL_SHIFT                   5  /* ADCR_DAC2_VOL - [8:5] */
#define WM8994_ADCR_DAC2_VOL_WIDTH                   4  /* ADCR_DAC2_VOL - [8:5] */
#define WM8994_ADCL_DAC2_VOL_MASK               0x000F  /* ADCL_DAC2_VOL - [3:0] */
#define WM8994_ADCL_DAC2_VOL_SHIFT                   0  /* ADCL_DAC2_VOL - [3:0] */
#define WM8994_ADCL_DAC2_VOL_WIDTH                   4  /* ADCL_DAC2_VOL - [3:0] */

/*
 * R1540 (0x604) - DAC2 Left Mixer Routing
 */
#define WM8994_ADCR_TO_DAC2L                    0x0020  /* ADCR_TO_DAC2L */
#define WM8994_ADCR_TO_DAC2L_MASK               0x0020  /* ADCR_TO_DAC2L */
#define WM8994_ADCR_TO_DAC2L_SHIFT                   5  /* ADCR_TO_DAC2L */
#define WM8994_ADCR_TO_DAC2L_WIDTH                   1  /* ADCR_TO_DAC2L */
#define WM8994_ADCL_TO_DAC2L                    0x0010  /* ADCL_TO_DAC2L */
#define WM8994_ADCL_TO_DAC2L_MASK               0x0010  /* ADCL_TO_DAC2L */
#define WM8994_ADCL_TO_DAC2L_SHIFT                   4  /* ADCL_TO_DAC2L */
#define WM8994_ADCL_TO_DAC2L_WIDTH                   1  /* ADCL_TO_DAC2L */
#define WM8994_AIF2DACL_TO_DAC2L                0x0004  /* AIF2DACL_TO_DAC2L */
#define WM8994_AIF2DACL_TO_DAC2L_MASK           0x0004  /* AIF2DACL_TO_DAC2L */
#define WM8994_AIF2DACL_TO_DAC2L_SHIFT               2  /* AIF2DACL_TO_DAC2L */
#define WM8994_AIF2DACL_TO_DAC2L_WIDTH               1  /* AIF2DACL_TO_DAC2L */
#define WM8994_AIF1DAC2L_TO_DAC2L               0x0002  /* AIF1DAC2L_TO_DAC2L */
#define WM8994_AIF1DAC2L_TO_DAC2L_MASK          0x0002  /* AIF1DAC2L_TO_DAC2L */
#define WM8994_AIF1DAC2L_TO_DAC2L_SHIFT              1  /* AIF1DAC2L_TO_DAC2L */
#define WM8994_AIF1DAC2L_TO_DAC2L_WIDTH              1  /* AIF1DAC2L_TO_DAC2L */
#define WM8994_AIF1DAC1L_TO_DAC2L               0x0001  /* AIF1DAC1L_TO_DAC2L */
#define WM8994_AIF1DAC1L_TO_DAC2L_MASK          0x0001  /* AIF1DAC1L_TO_DAC2L */
#define WM8994_AIF1DAC1L_TO_DAC2L_SHIFT              0  /* AIF1DAC1L_TO_DAC2L */
#define WM8994_AIF1DAC1L_TO_DAC2L_WIDTH              1  /* AIF1DAC1L_TO_DAC2L */

/*
 * R1541 (0x605) - DAC2 Right Mixer Routing
 */
#define WM8994_ADCR_TO_DAC2R                    0x0020  /* ADCR_TO_DAC2R */
#define WM8994_ADCR_TO_DAC2R_MASK               0x0020  /* ADCR_TO_DAC2R */
#define WM8994_ADCR_TO_DAC2R_SHIFT                   5  /* ADCR_TO_DAC2R */
#define WM8994_ADCR_TO_DAC2R_WIDTH                   1  /* ADCR_TO_DAC2R */
#define WM8994_ADCL_TO_DAC2R                    0x0010  /* ADCL_TO_DAC2R */
#define WM8994_ADCL_TO_DAC2R_MASK               0x0010  /* ADCL_TO_DAC2R */
#define WM8994_ADCL_TO_DAC2R_SHIFT                   4  /* ADCL_TO_DAC2R */
#define WM8994_ADCL_TO_DAC2R_WIDTH                   1  /* ADCL_TO_DAC2R */
#define WM8994_AIF2DACR_TO_DAC2R                0x0004  /* AIF2DACR_TO_DAC2R */
#define WM8994_AIF2DACR_TO_DAC2R_MASK           0x0004  /* AIF2DACR_TO_DAC2R */
#define WM8994_AIF2DACR_TO_DAC2R_SHIFT               2  /* AIF2DACR_TO_DAC2R */
#define WM8994_AIF2DACR_TO_DAC2R_WIDTH               1  /* AIF2DACR_TO_DAC2R */
#define WM8994_AIF1DAC2R_TO_DAC2R               0x0002  /* AIF1DAC2R_TO_DAC2R */
#define WM8994_AIF1DAC2R_TO_DAC2R_MASK          0x0002  /* AIF1DAC2R_TO_DAC2R */
#define WM8994_AIF1DAC2R_TO_DAC2R_SHIFT              1  /* AIF1DAC2R_TO_DAC2R */
#define WM8994_AIF1DAC2R_TO_DAC2R_WIDTH              1  /* AIF1DAC2R_TO_DAC2R */
#define WM8994_AIF1DAC1R_TO_DAC2R               0x0001  /* AIF1DAC1R_TO_DAC2R */
#define WM8994_AIF1DAC1R_TO_DAC2R_MASK          0x0001  /* AIF1DAC1R_TO_DAC2R */
#define WM8994_AIF1DAC1R_TO_DAC2R_SHIFT              0  /* AIF1DAC1R_TO_DAC2R */
#define WM8994_AIF1DAC1R_TO_DAC2R_WIDTH              1  /* AIF1DAC1R_TO_DAC2R */

/*
 * R1542 (0x606) - AIF1 ADC1 Left Mixer Routing
 */
#define WM8994_ADC1L_TO_AIF1ADC1L               0x0002  /* ADC1L_TO_AIF1ADC1L */
#define WM8994_ADC1L_TO_AIF1ADC1L_MASK          0x0002  /* ADC1L_TO_AIF1ADC1L */
#define WM8994_ADC1L_TO_AIF1ADC1L_SHIFT              1  /* ADC1L_TO_AIF1ADC1L */
#define WM8994_ADC1L_TO_AIF1ADC1L_WIDTH              1  /* ADC1L_TO_AIF1ADC1L */
#define WM8994_AIF2DACL_TO_AIF1ADC1L            0x0001  /* AIF2DACL_TO_AIF1ADC1L */
#define WM8994_AIF2DACL_TO_AIF1ADC1L_MASK       0x0001  /* AIF2DACL_TO_AIF1ADC1L */
#define WM8994_AIF2DACL_TO_AIF1ADC1L_SHIFT           0  /* AIF2DACL_TO_AIF1ADC1L */
#define WM8994_AIF2DACL_TO_AIF1ADC1L_WIDTH           1  /* AIF2DACL_TO_AIF1ADC1L */

/*
 * R1543 (0x607) - AIF1 ADC1 Right Mixer Routing
 */
#define WM8994_ADC1R_TO_AIF1ADC1R               0x0002  /* ADC1R_TO_AIF1ADC1R */
#define WM8994_ADC1R_TO_AIF1ADC1R_MASK          0x0002  /* ADC1R_TO_AIF1ADC1R */
#define WM8994_ADC1R_TO_AIF1ADC1R_SHIFT              1  /* ADC1R_TO_AIF1ADC1R */
#define WM8994_ADC1R_TO_AIF1ADC1R_WIDTH              1  /* ADC1R_TO_AIF1ADC1R */
#define WM8994_AIF2DACR_TO_AIF1ADC1R            0x0001  /* AIF2DACR_TO_AIF1ADC1R */
#define WM8994_AIF2DACR_TO_AIF1ADC1R_MASK       0x0001  /* AIF2DACR_TO_AIF1ADC1R */
#define WM8994_AIF2DACR_TO_AIF1ADC1R_SHIFT           0  /* AIF2DACR_TO_AIF1ADC1R */
#define WM8994_AIF2DACR_TO_AIF1ADC1R_WIDTH           1  /* AIF2DACR_TO_AIF1ADC1R */

/*
 * R1544 (0x608) - AIF1 ADC2 Left Mixer Routing
 */
#define WM8994_ADC2L_TO_AIF1ADC2L               0x0002  /* ADC2L_TO_AIF1ADC2L */
#define WM8994_ADC2L_TO_AIF1ADC2L_MASK          0x0002  /* ADC2L_TO_AIF1ADC2L */
#define WM8994_ADC2L_TO_AIF1ADC2L_SHIFT              1  /* ADC2L_TO_AIF1ADC2L */
#define WM8994_ADC2L_TO_AIF1ADC2L_WIDTH              1  /* ADC2L_TO_AIF1ADC2L */
#define WM8994_AIF2DACL_TO_AIF1ADC2L            0x0001  /* AIF2DACL_TO_AIF1ADC2L */
#define WM8994_AIF2DACL_TO_AIF1ADC2L_MASK       0x0001  /* AIF2DACL_TO_AIF1ADC2L */
#define WM8994_AIF2DACL_TO_AIF1ADC2L_SHIFT           0  /* AIF2DACL_TO_AIF1ADC2L */
#define WM8994_AIF2DACL_TO_AIF1ADC2L_WIDTH           1  /* AIF2DACL_TO_AIF1ADC2L */

/*
 * R1545 (0x609) - AIF1 ADC2 Right mixer Routing
 */
#define WM8994_ADC2R_TO_AIF1ADC2R               0x0002  /* ADC2R_TO_AIF1ADC2R */
#define WM8994_ADC2R_TO_AIF1ADC2R_MASK          0x0002  /* ADC2R_TO_AIF1ADC2R */
#define WM8994_ADC2R_TO_AIF1ADC2R_SHIFT              1  /* ADC2R_TO_AIF1ADC2R */
#define WM8994_ADC2R_TO_AIF1ADC2R_WIDTH              1  /* ADC2R_TO_AIF1ADC2R */
#define WM8994_AIF2DACR_TO_AIF1ADC2R            0x0001  /* AIF2DACR_TO_AIF1ADC2R */
#define WM8994_AIF2DACR_TO_AIF1ADC2R_MASK       0x0001  /* AIF2DACR_TO_AIF1ADC2R */
#define WM8994_AIF2DACR_TO_AIF1ADC2R_SHIFT           0  /* AIF2DACR_TO_AIF1ADC2R */
#define WM8994_AIF2DACR_TO_AIF1ADC2R_WIDTH           1  /* AIF2DACR_TO_AIF1ADC2R */

/*
 * R1552 (0x610) - DAC1 Left Volume
 */
#define WM8994_DAC1L_MUTE                       0x0200  /* DAC1L_MUTE */
#define WM8994_DAC1L_MUTE_MASK                  0x0200  /* DAC1L_MUTE */
#define WM8994_DAC1L_MUTE_SHIFT                      9  /* DAC1L_MUTE */
#define WM8994_DAC1L_MUTE_WIDTH                      1  /* DAC1L_MUTE */
#define WM8994_DAC1_VU                          0x0100  /* DAC1_VU */
#define WM8994_DAC1_VU_MASK                     0x0100  /* DAC1_VU */
#define WM8994_DAC1_VU_SHIFT                         8  /* DAC1_VU */
#define WM8994_DAC1_VU_WIDTH                         1  /* DAC1_VU */
#define WM8994_DAC1L_VOL_MASK                   0x00FF  /* DAC1L_VOL - [7:0] */
#define WM8994_DAC1L_VOL_SHIFT                       0  /* DAC1L_VOL - [7:0] */
#define WM8994_DAC1L_VOL_WIDTH                       8  /* DAC1L_VOL - [7:0] */

/*
 * R1553 (0x611) - DAC1 Right Volume
 */
#define WM8994_DAC1R_MUTE                       0x0200  /* DAC1R_MUTE */
#define WM8994_DAC1R_MUTE_MASK                  0x0200  /* DAC1R_MUTE */
#define WM8994_DAC1R_MUTE_SHIFT                      9  /* DAC1R_MUTE */
#define WM8994_DAC1R_MUTE_WIDTH                      1  /* DAC1R_MUTE */
#define WM8994_DAC1_VU                          0x0100  /* DAC1_VU */
#define WM8994_DAC1_VU_MASK                     0x0100  /* DAC1_VU */
#define WM8994_DAC1_VU_SHIFT                         8  /* DAC1_VU */
#define WM8994_DAC1_VU_WIDTH                         1  /* DAC1_VU */
#define WM8994_DAC1R_VOL_MASK                   0x00FF  /* DAC1R_VOL - [7:0] */
#define WM8994_DAC1R_VOL_SHIFT                       0  /* DAC1R_VOL - [7:0] */
#define WM8994_DAC1R_VOL_WIDTH                       8  /* DAC1R_VOL - [7:0] */

/*
 * R1556 (0x614) - DAC Softmute
 */
#define WM8994_DAC_SOFTMUTEMODE                 0x0002  /* DAC_SOFTMUTEMODE */
#define WM8994_DAC_SOFTMUTEMODE_MASK            0x0002  /* DAC_SOFTMUTEMODE */
#define WM8994_DAC_SOFTMUTEMODE_SHIFT                1  /* DAC_SOFTMUTEMODE */
#define WM8994_DAC_SOFTMUTEMODE_WIDTH                1  /* DAC_SOFTMUTEMODE */
#define WM8994_DAC_MUTERATE                     0x0001  /* DAC_MUTERATE */
#define WM8994_DAC_MUTERATE_MASK                0x0001  /* DAC_MUTERATE */
#define WM8994_DAC_MUTERATE_SHIFT                    0  /* DAC_MUTERATE */
#define WM8994_DAC_MUTERATE_WIDTH                    1  /* DAC_MUTERATE */

/*
 *  GPIO
 */
#define WM8994_GPIO_DIR_OUTPUT                   0x8000  /* OUTPUT PIN */
#define WM8994_GPIO_DIR_MASK                     0xFFE0  /* GPIO PIN MASK */
#define WM8994_GPIO_FUNCTION_I2S_CLK             0x0000  /* I2S CLK */
#define WM8994_GPIO_FUNCTION_MASK                0x001F  /* GPn FN */



#endif
