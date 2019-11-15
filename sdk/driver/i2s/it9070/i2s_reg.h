/**
 * @file i2s_reg.h
 * I2S Hardware register map (IT9070)
 *
 * @author Hanxuan Li
 * @author I-Chun Lai
 *
 * @version 0.1
 */

#ifndef I2S_REG_H
#define I2S_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************** */
#define I2S_REG_BASE                        0x1600

#define I2S_REG_TX_BUF_1_ADDR_L             (I2S_REG_BASE | 0x00)
#define I2S_REG_TX_BUF_1_ADDR_H             (I2S_REG_BASE | 0x02)
#define I2S_REG_TX_BUF_1_ADDR               I2S_REG_TX_BUF_1_ADDR_L

#define I2S_REG_TX_BUF_2_ADDR_L             (I2S_REG_BASE | 0x04)
#define I2S_REG_TX_BUF_2_ADDR_H             (I2S_REG_BASE | 0x06)
#define I2S_REG_TX_BUF_2_ADDR               I2S_REG_TX_BUF_2_ADDR_L

#define I2S_REG_TX_BUF_3_ADDR_L             (I2S_REG_BASE | 0x08)
#define I2S_REG_TX_BUF_3_ADDR_H             (I2S_REG_BASE | 0x0A)
#define I2S_REG_TX_BUF_3_ADDR               I2S_REG_TX_BUF_3_ADDR_L

#define I2S_REG_TX_BUF_4_ADDR_L             (I2S_REG_BASE | 0x0C)
#define I2S_REG_TX_BUF_4_ADDR_H             (I2S_REG_BASE | 0x0E)
#define I2S_REG_TX_BUF_4_ADDR               I2S_REG_TX_BUF_4_ADDR_L

#define I2S_REG_TX_BUF_5_ADDR_L             (I2S_REG_BASE | 0x10)
#define I2S_REG_TX_BUF_5_ADDR_H             (I2S_REG_BASE | 0x12)
#define I2S_REG_TX_BUF_5_ADDR               I2S_REG_TX_BUF_5_ADDR_L

#define I2S_REG_TX_BUF_6_ADDR_L             (I2S_REG_BASE | 0x20)
#define I2S_REG_TX_BUF_6_ADDR_H             (I2S_REG_BASE | 0x22)
#define I2S_REG_TX_BUF_6_ADDR               I2S_REG_TX_BUF_6_ADDR_L

#define I2S_REG_TX_BUF_SIZE_L               (I2S_REG_BASE | 0x14)
#define I2S_REG_TX_BUF_SIZE_H               (I2S_REG_BASE | 0x16)
#define I2S_REG_TX_BUF_SIZE                 I2S_REG_TX_BUF_SIZE_L

#define I2S_REG_TX_BUF_W_PTR_L              (I2S_REG_BASE | 0x18)
#define I2S_REG_TX_BUF_W_PTR_H              (I2S_REG_BASE | 0x1A)
#define I2S_REG_TX_BUF_W_PTR                I2S_REG_TX_BUF_W_PTR_L

#define I2S_REG_TX_BUF_R_PTR_L              (I2S_REG_BASE | 0x1C)
#define I2S_REG_TX_BUF_R_PTR_H              (I2S_REG_BASE | 0x1E)
#define I2S_REG_TX_BUF_R_PTR                I2S_REG_TX_BUF_R_PTR_L

#define I2S_REG_RX_CTRL                     (I2S_REG_BASE | 0x40)
#define I2S_POS_RESET_WS                    (15u)   // reset the word select (WS) output. 0: normal; 1: reset
                                                    // only works when RX/TX master mode.
#define I2S_MSK_RESET_WS                    (N01_BITS_MSK << I2S_POS_RESET_WS)
#define I2S_POS_SEL_ADC_SRC                 (13u)   // 0: select external ADC; 1: select internal ADC
#define I2S_MSK_SEL_ADC_SRC                 (N01_BITS_MSK << I2S_POS_SEL_ADC_SRC)
#define I2S_POS_RX_MODE                     (12u)   // 0: RX master; 1: RX slave
#define I2S_MSK_RX_MODE                     (N01_BITS_MSK << I2S_POS_RX_MODE)
#define I2S_POS_RX_DATA_FMT                 (10u)   // 0: 16bits; 1: 24bits; 2 or 3: 32 bits
#define I2S_MSK_RX_DATA_FMT                 (N02_BITS_MSK << I2S_POS_RX_DATA_FMT)
#define I2S_POS_RX_CH_LEN                   (0u)    // Channel length (number of bits per audio channel)
#define I2S_MSK_RX_CH_LEN                   (N09_BITS_MSK << I2S_POS_RX_CH_LEN)

#define I2S_REG_TX_CTRL                     (I2S_REG_BASE | 0x42)
#define I2S_POS_SEL_DAC_SRC                 (13u)   // 0: select external DAC; 1: select internal DAC
#define I2S_MSK_SEL_DAC_SRC                 (N01_BITS_MSK << I2S_POS_SEL_ADC_SRC)
#define I2S_POS_TX_MODE                     (12u)   // 0: TX master; 1: TX slave
#define I2S_MSK_TX_MODE                     (N01_BITS_MSK << I2S_POS_TX_MODE)
#define I2S_POS_TX_DATA_FMT                 (10u)   // 0: 16bits; 1: 24bits; 2 or 3: 32 bits
#define I2S_MSK_TX_DATA_FMT                 (N02_BITS_MSK << I2S_POS_TX_DATA_FMT)
#define I2S_POS_TX_CH_LEN                   (0u)    // Channel length (number of bits per audio channel)
#define I2S_MSK_TX_CH_LEN                   (N09_BITS_MSK << I2S_POS_TX_CH_LEN)

#define I2S_REG_ADDA_PCM                    (I2S_REG_BASE | 0x44)

#define I2S_REG_RX_BUF_1_ADDR_L             (I2S_REG_BASE | 0x46)
#define I2S_REG_RX_BUF_1_ADDR_H             (I2S_REG_BASE | 0x48)
#define I2S_REG_RX_BUF_1_ADDR               I2S_REG_RX_BUF_1_ADDR_L

#define I2S_REG_RX_BUF_SIZE                 (I2S_REG_BASE | 0x4A)
#define I2S_REG_RX_BUF_R_PTR                (I2S_REG_BASE | 0x4C)
#define I2S_REG_RX_BUF_W_PTR                (I2S_REG_BASE | 0x5A)

#define I2S_REG_RX_CTRL1                    (I2S_REG_BASE | 0x4E)
#define I2S_POS_RX_ENABLE                   (1u)    // 0: stop; 1: start
#define I2S_MSK_RX_ENABLE                   (N01_BITS_MSK << I2S_POS_RX_ENABLE)
#define I2S_POS_RX_START                    (0u)    // 0: pause; 1: resume
#define I2S_MSK_RX_START                    (N01_BITS_MSK << I2S_POS_RX_START)

#define I2S_REG_RX_CTRL2                    (I2S_REG_BASE | 0x50)
#define I2S_POS_RX_H_WATER_MARK             (3u)
#define I2S_MSK_RX_H_WATER_MARK             (N13_BITS_MSK << I2S_POS_RX_H_WATER_MARK)
#define I2S_POS_EN_OVR_RX_H_WAT_MRK_INTR    (0u)
#define I2S_MSK_EN_OVR_RX_H_WAT_MRK_INTR    (N01_BITS_MSK << I2S_POS_EN_OVR_RX_H_WAT_MRK_INTR)

#define I2S_REG_TX_L_WATER_MARK_L           (I2S_REG_BASE | 0x52)
#define I2S_REG_TX_L_WATER_MARK_H           (I2S_REG_BASE | 0x54)
#define I2S_REG_TX_L_WATER_MARK             I2S_REG_TX_L_WATER_MARK_L

#define I2S_REG_TX_CTRL1                    (I2S_REG_BASE | 0x56)
#define I2S_REG_TX_CTRL2                    (I2S_REG_BASE | 0x58)
#define I2S_POS_TX_EN_FLUSH                 (2u)    // 0: disable; 1: enable
                                                    // the h/w starts transmit data only when
                                                    // the TX FIFO contains more than or equal to 64 byts of data.
                                                    // If the TX FIFO contains less than 64 bytes of data and we
                                                    // want the h/w to transmit the remaining data, set this bit.
#define I2S_MSK_TX_EN_FLUSH                 (N01_BITS_MSK << I2S_POS_TX_EN_FLUSH)
#define I2S_POS_TX_ENABLE                   (1u)    // 0: stop; 1: start
#define I2S_MSK_TX_ENABLE                   (N01_BITS_MSK << I2S_POS_RX_ENABLE)
#define I2S_POS_TX_START                    (0u)    // 0: pause; 1: resume
#define I2S_MSK_TX_START                    (N01_BITS_MSK << I2S_POS_RX_START)

#define I2S_REG_RX_STATUS                   (I2S_REG_BASE | 0x5C)
#define I2S_REG_TX_STATUS1                  (I2S_REG_BASE | 0x5E)
#define I2S_REG_TX_STATUS2                  (I2S_REG_BASE | 0x60)

#define I2S_REG_THOLD_CROSS_LO              (I2S_REG_BASE | 0x62)
#define I2S_REG_THOLD_CROSS_HI              (I2S_REG_BASE | 0x64)
#define I2S_REG_THOLD_CROSS                 I2S_REG_THOLD_CROSS_LO

#define I2S_REG_FADE_IN_OUT_CTRL            (I2S_REG_BASE | 0x66)
#define I2S_POS_EN_FADE_IN_OUT              (12u) // 0: disable; 1: enable
#define I2S_MSK_EN_FADE_IN_OUT              (N01_BITS_MSK << I2S_POS_EN_FADE_IN_OUT)
#define I2S_POS_FADE_STEP                   (8u)
#define I2S_MSK_FADE_STEP                   (N04_BITS_MSK << I2S_POS_FADE_STEP)
#define I2S_POS_FADE_CYCLE                  (0u)
#define I2S_MSK_FADE_CYCLE                  (N08_BITS_MSK << I2S_POS_FADE_CYCLE)

#define I2S_REG_DIG_OUT_VOL                 (I2S_REG_BASE | 0x68)
#define I2S_REG_SPDIF_VOL                   (I2S_REG_BASE | 0x6C)
#define I2S_POS_SEL_SPDIF_TX_BUF            (14u) // 0: TX buf 5; 1: TX buf 6
#define I2S_MSK_SEL_SPDIF_TX_BUF            (N01_BITS_MSK << I2S_POS_SEL_SPDIF_TX_BUF)

#define I2S_REG_BITS_CTL                    (I2S_REG_BASE | 0x6E)

#define I2S_REG_AMP_CTL                     (I2S_REG_BASE | 0x72)
#define I2S_POS_AMP_MUTE_CTRL               (1u)    // AMP left/right channel mute control
                                                    // 0: left/right channel unmute
                                                    // 1: left/right channel mute
#define I2S_MSK_AMP_MUTE_CTRL               (N01_BITS_MSK << I2S_POS_AMP_MUTE_CTRL)
#define I2S_POS_AMP_PWR                     (0u)    // AMP left/right power down control
                                                    // 0: normal mode; 1: power down
#define I2S_MSK_AMP_PWR                     (N01_BITS_MSK << I2S_POS_AMP_PWR)

#define I2S_REG_AMP_VOL                     (I2S_REG_BASE | 0x74)

#define I2S_REG_HDMI_SYNC_LEFT              (I2S_REG_BASE | 0x76)
#define I2S_REG_HDMI_SYNC_RIGHT             (I2S_REG_BASE | 0x78)

#define I2S_REG_HDMI_CTL                    (I2S_REG_BASE | 0x7A)

#ifdef __cplusplus
}
#endif

#endif //I2S_REG_H