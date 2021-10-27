/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Chester Tseng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef OBJWLAN_H_
#define OBJWLAN_H_

#include "py/mpconfig.h"
#include "py/runtime.h"
#include "py/stream.h"

#define WLAN_MIN_SSID_LEN       3
#define WLAN_MAX_SSID_LEN       32

#define WLAN_WEP_MIN_PASS_CHAR_LEN  1
#define WLAN_WEP_MAX_PASS_CHAR_LEN  13

#define WLAN_WPA_MIN_PASS_CHAR_LEN  8
#define WLAN_WPA_MAC_PASS_CHAR_LEN  63

#define WLAN_PROMISC_ENABLE 0

extern const mp_obj_type_t wlan_type;


////////////////////////////////
//                            //
//  arduino wlan definitions  //
//                            //
////////////////////////////////

// Maximum size of a SSID
#define WL_SSID_MAX_LENGTH          32
// Length of passphrase. Valid lengths are 8-63.
#define WL_WPA_KEY_MAX_LENGTH       63
// Length of key in bytes. Valid values are 5 and 13.
#define WL_WEP_KEY_MAX_LENGTH       13
// Size of a MAC-address or BSSID
#define WL_MAC_ADDR_LENGTH          6
// Size of a MAC-address or BSSID
#define WL_IPV4_LENGTH              4
// Maximum size of a SSID list
#define WL_NETWORKS_LIST_MAXNUM     50
// Maxmium number of socket
#define	MAX_SOCK_NUM                4
// Socket not available constant
#define SOCK_NOT_AVAIL              255
// Default state value for Wifi state field
#define NA_STATE                    -1
//Maximum number of attempts to establish wifi connection
#define WL_MAX_ATTEMPT_CONNECTION   10



typedef enum {
    WL_NO_SHIELD = 255,
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL,
    WL_SCAN_COMPLETED,
    WL_CONNECTED,
    WL_CONNECT_FAILED,
    WL_CONNECTION_LOST,
    WL_DISCONNECTED
} wl_status_t;

/* Encryption modes */
enum wl_enc_type {  /* Values map to 802.11 encryption suites... */
    ENC_TYPE_WEP  = 5,
    ENC_TYPE_TKIP = 2,
    ENC_TYPE_CCMP = 4,
    /* ... except these two, 7 and 8 are reserved in 802.11-2007 */
    ENC_TYPE_NONE = 7,
    ENC_TYPE_AUTO = 8
};

/* RTK added type */
#ifndef WEP_ENABLED

#define WEP_ENABLED         0x0001
#define TKIP_ENABLED        0x0002
#define AES_ENABLED         0x0004
#define WSEC_SWFLAG         0x0008

#define SHARED_ENABLED      0x00008000
#define WPA_SECURITY        0x00200000
#define WPA2_SECURITY       0x00400000
#define WPS_ENABLED         0x10000000

#endif // #ifndef WEP_ENABLED

/* redefined from enum rtw_security_t */
#define SECURITY_OPEN            (0)
#define SECURITY_WEP_PSK         (WEP_ENABLED)
#define SECURITY_WEP_SHARED      (WEP_ENABLED | SHARED_ENABLED)
#define SECURITY_WPA_TKIP_PSK    (WPA_SECURITY  | TKIP_ENABLED)
#define SECURITY_WPA_AES_PSK     (WPA_SECURITY  | AES_ENABLED)
#define SECURITY_WPA2_AES_PSK    (WPA2_SECURITY | AES_ENABLED)
#define SECURITY_WPA2_TKIP_PSK   (WPA2_SECURITY | TKIP_ENABLED)
#define SECURITY_WPA2_MIXED_PSK  (WPA2_SECURITY | AES_ENABLED | TKIP_ENABLED)
#define SECURITY_WPA_WPA2_MIXED  (WPA_SECURITY  | WPA2_SECURITY)
// end of arduino wlan definitions

// arduino wlan types
typedef enum {
    WL_FAILURE = -1,
    WL_SUCCESS = 1
} wl_error_code_t;

/* Authentication modes */
enum wl_auth_mode {
    AUTH_MODE_INVALID,
    AUTH_MODE_AUTO,
    AUTH_MODE_OPEN_SYSTEM,
    AUTH_MODE_SHARED_KEY,
    AUTH_MODE_WPA,
    AUTH_MODE_WPA2,
    AUTH_MODE_WPA_PSK,
    AUTH_MODE_WPA2_PSK
};
// end of arduino wlan types


void wlan_init0(void);
void validate_wlan_mode(uint8_t mode);
void validate_ssid(mp_uint_t len);


typedef struct {
    mp_obj_base_t base;
    uint8_t       mode;                       // WLAN mode, STA / AP / STA_AP 
    int32_t       security_type;              // WLAN security mode, WPA2 / WEP ...
    uint8_t       channel;                    // WLAN channel 0 ~ 11
    int8_t        ssid[WLAN_MAX_SSID_LEN];    // WLAN STA mode ssid
#if WLAN_PROMISC_ENABLE
    uint8_t       promisc_level;
#endif
    //netif_obj_t   netif[];
} wlan_obj_t;

#endif  // OBJWLAN_H_
