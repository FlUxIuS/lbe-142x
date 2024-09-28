#ifndef LBE_COMMON_H
#define LBE_COMMON_H

// Device definitions
#define VID_LBE 0x1dd2
#define PID_LBE_1420 0x2443
#define PID_LBE_1421 0x2444 // LBE-1421 Dual Output

// Status bit definitions
#define LBE_GPS_LOCK_BIT 0x01
#define LBE_ANT_OK_BIT 0x04
#define LBE_OUT1_EN_BIT 0x10

// Command definitions
#define LBE_142X_EN_OUT 0x01
#define LBE_142X_BLINK_OUT 0x02
#define LBE_1420_SET_F1_NO_SAVE 0x03
#define LBE_1420_SET_F1 0x04
#define LBE_1421_SET_F1 0x06
#define LBE_1421_SET_F2 0x0A

#endif // LBE_COMMON_H
