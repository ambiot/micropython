/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Chester Tseng
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

// MP include
#include "objwlan.h"

// RTK includes 
#include "main.h"
#include "wifi_conf.h"
#include "wifi_constants.h"
#include "wifi_structures.h"
#include "lwip_netconf.h"
#include "lwip/err.h"
#include "lwip/api.h"
#include <dhcp/dhcps.h>
#include "netutils.h"


extern struct netif xnetif[NET_IF_NUM]; // max 3 network interface



/*****************************************************************************
 *                              Local variables
 * ***************************************************************************/
static rtw_network_info_t wifi;
static unsigned char pswd[65] = {0};
const unsigned char *ssid = NULL;
//static const unsigned char *pswd = NULL;


//char ap_ssid[] = {0};  //Set the AP's SSID
//char ap_pswd[] = {0};     //Set the AP's password
static rtw_ap_info_t ap;
static unsigned char ap_pswd[65] = {0};
static unsigned char ap_ssid[65] = {0};
const unsigned char* ap_pswd_temp = NULL;
const unsigned char* ap_ssid_temp = NULL;
static char ap_channel[] = "1";         //Set the AP's channel
static int ap_status = WL_IDLE_STATUS;  // the Wifi radio's status


STATIC wlan_obj_t wlan_obj = {
    .base.type      = &wlan_type,
    .security_type  = RTW_SECURITY_WPA2_AES_PSK,
};


STATIC xSemaphoreHandle xSTAConnectAPSema;
//rtw_mode_t wifi_mode = 0;
static int wifi_mode = 0;

static bool init_wlan = false;
static char connect_result = WL_FAILURE;

static bool wlan_drv_state = true;



//mp_obj_base_t base;

// settings of requested network
uint8_t  _networkCount;
char     _networkSsid[WL_NETWORKS_LIST_MAXNUM][WL_SSID_MAX_LENGTH];
int32_t  _networkRssi[WL_NETWORKS_LIST_MAXNUM];
uint32_t _networkEncr[WL_NETWORKS_LIST_MAXNUM];



/*****************************************************************************
 *                              External function
 * ***************************************************************************/

void wlan_init0(void) {
    // to wlan init here
    //init_event_callback_list();
    struct netif * pnetif = &xnetif[0];

    if (init_wlan == false) {
        init_wlan = true;
        LwIP_Init();
        wifi_on(RTW_MODE_STA);
        wifi_mode = RTW_MODE_STA;
    } else if (init_wlan == true) {
        if (wifi_mode != RTW_MODE_STA) {
            dhcps_deinit();
            wifi_off();
            vTaskDelay(20);
            wifi_on(RTW_MODE_STA);
            dhcps_init(pnetif);
            wifi_mode = RTW_MODE_STA;
        }
    }
}

void wlan_deinit0(void) {
    // shut down wifi
    dhcps_deinit();
    wifi_off();
}


/*****************************************************************************
 *                              Local functions
 * ***************************************************************************/

static uint8_t getConnectionStatus() {

    wlan_init0();

    if (wifi_is_connected_to_ap() == 0) {
        return WL_CONNECTED;
    } else {
        return WL_DISCONNECTED;
    }
}


static void init_wifi_struct(void) {

    memset(wifi.ssid.val, 0, sizeof(wifi.ssid.val));
    memset(wifi.bssid.octet, 0, ETH_ALEN);
    memset(pswd, 0, sizeof(pswd));
    wifi.ssid.len = 0;
    wifi.password = NULL;
    wifi.password_len = 0;
    wifi.key_id = -1;
    memset(ap.ssid.val, 0, sizeof(ap.ssid.val));
    ap.ssid.len = 0;
    ap.password = NULL;
    ap.password_len = 0;
    ap.channel = 1;
}


STATIC mp_obj_t get_ip(mp_obj_t self_in) {
    unsigned char *IP = LwIP_GetIP(&xnetif[0]);
    
    return netutils_format_ipv4_addr((uint8_t *)IP, NETUTILS_BIG);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(get_ip_obj, get_ip);


void validate_wlan_mode(uint8_t mode) {
    if (mode != RTW_MODE_AP &&
        mode != RTW_MODE_STA &&
        mode != RTW_MODE_STA_AP 
        ) {
            mp_raise_ValueError("Invalid WLAN mode");
    }
}

void validate_ssid(mp_uint_t len) {

    if ((len > WLAN_MAX_SSID_LEN) || (len < WLAN_MIN_SSID_LEN)) {
        mp_raise_ValueError("Invalid SSID length");
    }

}

void validate_pswd(mp_uint_t len) {

    if (len > WLAN_WPA_MAC_PASS_CHAR_LEN ||len < WLAN_WPA_MIN_PASS_CHAR_LEN) {
        mp_raise_ValueError("Invalid password length");
    }
}


STATIC const qstr wlan_scan_info_fields[] = {
        MP_QSTR_ssid, MP_QSTR_bssid, MP_QSTR_rssi,
        MP_QSTR_bss, MP_QSTR_security, MP_QSTR_wps,
        MP_QSTR_channel, MP_QSTR_band
};


rtw_result_t wifidrv_scan_result_handler( rtw_scan_handler_result_t* malloced_scan_result)
{   
    rtw_scan_result_t* record;

    if (malloced_scan_result->scan_complete != RTW_TRUE) {
        record = &malloced_scan_result->ap_details;
        record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */

        if (_networkCount < WL_NETWORKS_LIST_MAXNUM) {
            strcpy( _networkSsid[_networkCount], (char *)record->SSID.val);
            _networkRssi[_networkCount] = record->signal_strength;
            _networkEncr[_networkCount] = record->security;
            _networkCount++;
        }
    }

    return RTW_SUCCESS;
}


static void printEncryptionType(uint32_t thisType) {

    switch (thisType) {
        case SECURITY_OPEN:
            printf("Open\n");
            break;
        case SECURITY_WEP_PSK:
            printf("WEP\n");
            break;
        case SECURITY_WPA_TKIP_PSK:
            printf("WPA TKIP\n");
            break;
        case SECURITY_WPA_AES_PSK:
            printf("WPA AES\n");
            break;
        case SECURITY_WPA2_AES_PSK:
            printf("WPA2 AES\n");
            break;
        case SECURITY_WPA2_TKIP_PSK:
            printf("WPA2 TKIP\n");
            break;
        case SECURITY_WPA2_MIXED_PSK:
            printf("WPA2 Mixed\n");
            break;
        case SECURITY_WPA_WPA2_MIXED:
            printf("WPA/WPA2 AES\n");
            break;
    }
}


static int8_t startScanNetworks() {
    _networkCount = 0;
    if (wifi_scan_networks(wifidrv_scan_result_handler, NULL) != RTW_SUCCESS) {
        return WL_FAILURE;
    }
    return WL_SUCCESS;
}


STATIC mp_obj_t wlan_scan(mp_obj_t self_in) {
    uint8_t attempts = 10;
    uint8_t numOfNetworks = 0;

    if(getConnectionStatus()) {
        mp_hal_delay_ms(1000);
        if (startScanNetworks() == WL_FAILURE) {
            printf("\n## WiFi scan network returned FAIL \n\r");
            mp_raise_ValueError("Scan errorrr!");
            return mp_const_none;
        }

        do {
             mp_hal_delay_ms(2000);
             numOfNetworks = _networkCount;
        } while ((numOfNetworks == 0) && (--attempts > 0));
    } else {
        printf("WiFi not init \n\r");
        mp_raise_ValueError("Scan error!");
    }

    if (numOfNetworks == -1) { //if scan fail
        printf("Could not find available wifi\n");
    } else { // if scan success
        printf("The number of networks found: ");
        printf("%d\n", numOfNetworks);
//int thisNet = 0;
        for (int thisNet = 0; thisNet < numOfNetworks; thisNet++) {
            printf("%d", thisNet);
            printf(") ");
            printf("%s", _networkSsid[thisNet]);
            printf("\tSignal: ");
            printf("%d", _networkRssi[thisNet]);;
            printf(" dBm");
            printf("\tEncryption: ");
            printEncryptionType(_networkEncr[thisNet]);
        }
    }

    mp_hal_delay_ms(2000);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wlan_scan_obj, wlan_scan);


int8_t apActivate()
{
#if CONFIG_LWIP_LAYER
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;
    struct netif * pnetif = &xnetif[0];
#endif
    int timeout = 20;
    int ret = WL_SUCCESS;
    if (ap.ssid.val[0] == 0) {
        printf("Error: SSID can't be empty\n\r");
        ret = WL_FAILURE;
        goto exit;
    }
    if (ap.password == NULL) {
        ap.security_type = RTW_SECURITY_OPEN;
    } else{
        ap.security_type = RTW_SECURITY_WPA2_AES_PSK;
    }

#if CONFIG_LWIP_LAYER
    dhcps_deinit();
    IP4_ADDR(&ipaddr, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
    IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
    IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
    netif_set_addr(pnetif, &ipaddr, &netmask,&gw);
#endif
    wifi_off();
    vTaskDelay(20);
    if (wifi_on(RTW_MODE_AP) < 0){
        printf("\n\rERROR: Wifi on failed!");
        ret = WL_FAILURE;
        goto exit;
    }
    printf("\n\rStarting AP ...");

    if((ret = wifi_start_ap((char*)ap.ssid.val, ap.security_type, (char*)ap.password, ap.ssid.len, ap.password_len, ap.channel)) < 0) {
        printf("\n\rERROR: Operation failed!");
        ret = WL_FAILURE;
        goto exit;
    }

    while (1) {
        char essid[33];

        if (wext_get_ssid(WLAN0_NAME, ((unsigned char *)essid)) > 0) {
            if (strcmp(((const char *)essid), ((const char *)ap.ssid.val)) == 0) {
                printf("\n\r%s started\n", ap.ssid.val);
                ret = WL_SUCCESS;
                break;
            }
        }

        if (timeout == 0) {
            printf("\n\rERROR: Start AP timeout!");
            ret = WL_FAILURE;
            break;
        }

        vTaskDelay(1 * configTICK_RATE_HZ);
        timeout --;
    }
#if CONFIG_LWIP_LAYER
    //LwIP_UseStaticIP(pnetif);
    dhcps_init(pnetif);
#endif

exit:
    init_wifi_struct( );
    if (ret == WL_SUCCESS) {
        wifi_mode = RTW_MODE_AP;
    }
    return ret;
}



int8_t apSetPassphrase(const char *apSetPassphrase_passphrase, uint8_t apSetPassphrase_len)
{
    int ret = WL_SUCCESS;
    strcpy((char *)ap_pswd, (char*)apSetPassphrase_passphrase);
 //   printf("zzw   ap_pswd_temp   %s \r\n", ap_pswd_temp);
    ap.password = ap_pswd;
    ap.password_len = apSetPassphrase_len;
    if (ap.password_len < 8) {
        printf("Error: Password length can't less than 8\n\r");
        ret = WL_FAILURE;
    }
    return ret;
}


int8_t apSetNetwork(char* apSetNetwork_ssid, uint8_t apSetNetwork_ssid_len)
{
    int ret = WL_SUCCESS;

    ap.ssid.len = apSetNetwork_ssid_len;

    if (ap.ssid.len > 32) {
        printf("Error: SSID length can't exceed 32\n\r");
        ret = WL_FAILURE;
    }
    strcpy((char *)ap.ssid.val, (char*)apSetNetwork_ssid);
    return ret;
}

STATIC mp_obj_t wlan_start_ap(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum {ARG_ssid, ARG_pswd};
    STATIC const mp_arg_t allowed_args[] = {
        { MP_QSTR_ssid, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_pswd, MP_ARG_REQUIRED | MP_ARG_OBJ },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int8_t apAttempts = 5;
    int ap_len_temp_A = 0;
    int ap_len_temp_B = 0;
    //int ap_pswd_len = 0;

    ap_ssid_temp = mp_obj_str_get_data(args[ARG_ssid].u_obj, &ap_len_temp_A);
    ap_pswd_temp = mp_obj_str_get_data(args[ARG_pswd].u_obj, &ap_len_temp_B);

    strcpy((char *)ap_ssid, (char*)ap_ssid_temp);
    strcpy((char *)ap_pswd, (char*)ap_pswd_temp);

    //ssid = mp_obj_str_get_data(args[ARG_ssid].u_obj, &ssid_len);
    //pswd = mp_obj_str_get_data(args[ARG_pswd].u_obj, &pswd_len);
    //ssid = mp_obj_str_get_str(args[ARG_ssid].u_obj);
    //pswd = mp_obj_str_get_str(args[ARG_pswd].u_obj);

    //ssid_len = strlen(ssid);
    //pswd_len = strlen(pswd);

    //init WLAN interface and wifi driver
    if (getConnectionStatus() == WL_NO_SHIELD) {
        //printf("WiFi driver init failed, please try again later\n");
        mp_raise_ValueError("WiFi shield not present");
        return mp_const_none;
    }

    //printf("IP address of the AP is %s\n", getIPAddress()); 

    while ((ap_status != WL_CONNECTED)) {
        if ((apSetNetwork(ap_ssid, strlen(ap_ssid))) != WL_FAILURE) {
            //printf("pswd =%s of length of %d\n", pswd, pswd_len);
            if (apSetPassphrase(ap_pswd, strlen(ap_pswd)) != WL_FAILURE) {
                ap.channel = (unsigned char)(atoi(ap_channel));
                if (apActivate() != WL_FAILURE) {
                    ap_status = WL_CONNECTED;
                } else {
                    ap_status = WL_CONNECT_FAILED;
                }
            } else {
                ap_status = WL_CONNECT_FAILED;
            }
        } else {
            ap_status = WL_CONNECT_FAILED;
        }
        apAttempts--;
        if (apAttempts < 0) {
            break;
        }
        mp_hal_delay_ms(10000);
    } // end of initializing AP

    if (ap_status == WL_CONNECT_FAILED) {
        printf("Failed to start AP after 2 attempts, try again later\n");
        return mp_const_none;
    } else {
        printf("AP %s start successfully \n", ap_ssid);
        //mp_hal_delay_ms(60000);
        while(1){
            mp_hal_delay_ms(1000);
        };
        //return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(wlan_start_ap_obj, 0, wlan_start_ap);


STATIC mp_obj_t wlan_connect(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {

    enum { ARG_ssid, ARG_pswd, ARG_security };

    STATIC const mp_arg_t allowed_args[] = {
        { MP_QSTR_ssid,      MP_ARG_REQUIRED | MP_ARG_OBJ  ,{.u_obj = mp_const_none}},
        { MP_QSTR_pswd,      MP_ARG_OBJ                    ,{.u_obj = mp_const_none}},
        { MP_QSTR_security,  MP_ARG_INT                    ,{.u_int = RTW_SECURITY_WPA2_AES_PSK}}, // default WPA2
    };

    //wlan_obj_t *self = pos_args[0];

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint8_t status = WL_IDLE_STATUS;
    unsigned char ssid_len = 0;
    int pswd_len = 0;
    int ret = RTW_ERROR;
    uint8_t dhcp_result;
    char* xmpassword = NULL;

    if (wlan_drv_state == false) {
        mp_raise_ValueError("Turn on wifi before connect");
    }
    // init wifi drivers
    wlan_init0();

    //wlan_drv_state = true;

    if (args[ARG_ssid].u_obj != mp_const_none) {
        ssid = mp_obj_str_get_data(args[ARG_ssid].u_obj, &ssid_len);
        validate_ssid(ssid_len);
        memset(wifi.bssid.octet, 0, ETH_ALEN);
        memcpy(wifi.ssid.val, ssid, ssid_len);
        wifi.ssid.len = ssid_len;
    } else {
        mp_raise_ValueError("ssid can't be empty");
    }

    if (args[ARG_pswd].u_obj != mp_const_none) {
        memset(pswd, 0, sizeof(pswd));
        xmpassword = mp_obj_str_get_data(args[ARG_pswd].u_obj, &pswd_len);
        memcpy(pswd, xmpassword, pswd_len);
        validate_pswd(pswd_len);
        wifi.password = pswd;
        wifi.password_len = pswd_len;
    } else {
        wifi.password = NULL;
        wifi.password_len = 0;
    }

    wifi.key_id = 0;

    rtw_security_t mp_security = args[ARG_security].u_int;
    switch(mp_security) {
        case RTW_SECURITY_OPEN:
            wifi.security_type = RTW_SECURITY_OPEN;
            break;
        case RTW_SECURITY_WPA_TKIP_PSK:
            wifi.security_type = RTW_SECURITY_WPA_TKIP_PSK;
            break;
        case RTW_SECURITY_WPA2_TKIP_PSK:
            wifi.security_type = RTW_SECURITY_WPA2_TKIP_PSK;
            break;
        case RTW_SECURITY_WPA_AES_PSK:
            wifi.security_type = RTW_SECURITY_WPA_AES_PSK;
            break;
        case RTW_SECURITY_WPA2_AES_PSK:
            wifi.security_type = RTW_SECURITY_WPA2_AES_PSK;
            break;
        case RTW_SECURITY_WPA2_MIXED_PSK:
            wifi.security_type = RTW_SECURITY_WPA2_MIXED_PSK;
            break;
//        case RTW_SECURITY_WPA_WPA2_MIXED:
//            wifi.security_type = RTW_SECURITY_WPA_WPA2_MIXED;
//            break;
        default:
            mp_raise_ValueError("This security type is not supported");
            break;
    }

    for (int i = 1; i < 6; i++) {
        //printf("\n    Before wifi_connect    \r\n");
        ret = wifi_connect((char*)wifi.ssid.val, wifi.security_type, (char*)wifi.password, wifi.ssid.len, wifi.password_len, wifi.key_id, NULL);
        //printf("\n    After  wifi_connect    \r\n");
        if (ret == RTW_SUCCESS) {
            break;
        } else {
            printf("\n    Auto reconnect to WiFi...    \r\n");
        }
        //mp_hal_delay_ms(10000);
        vTaskDelay(10000);
        //printf("\n    After vTaskDelay 10s    \r\n");
    }

    if (ret == RTW_SUCCESS) {

        dhcp_result = LwIP_DHCP(0, DHCP_START);

        init_wifi_struct();

        if ( dhcp_result == DHCP_ADDRESS_ASSIGNED ) {
            connect_result = WL_SUCCESS;
        } else {
            wifi_disconnect();
            connect_result = WL_FAILURE;
        }
    } else {
        init_wifi_struct();
        connect_result = WL_FAILURE;
    }

    uint8_t wifi_status = WL_IDLE_STATUS;

    if (connect_result != WL_FAILURE) {
        wifi_status = getConnectionStatus();
    } else {
        wifi_status= WL_CONNECT_FAILED;
    }

    if (wifi_status == WL_CONNECTED) {
        printf("\nWiFi is connected to %s\n", ssid);
    } else {
        mp_raise_ValueError("WiFi connect failed.");
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(wlan_connect_obj, 1, wlan_connect);


STATIC mp_obj_t wlan_disconnect(mp_obj_t self_in) {

    wlan_obj_t *self = self_in;

    if (self->mode != RTW_MODE_STA)
        mp_raise_ValueError("Disconnect only support STA mode");

    int ret = wifi_disconnect();

    if (ret != RTW_SUCCESS) {
        mp_raise_msg(&mp_type_OSError, "Disconnect error");
    }
    
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wlan_disconnect_obj, wlan_disconnect);


STATIC mp_obj_t wlan_on(mp_obj_t self_in) {
    int16_t ret = RTW_ERROR;

    wlan_obj_t *self = self_in;

    if ( (wifi_is_up(RTW_STA_INTERFACE)) || (wifi_is_up(RTW_AP_INTERFACE)) ) {
        mp_raise_msg(&mp_type_OSError, "WLAN has already running, please turn it off first.");
    }

    // Init WLAN0 first
    ret = wifi_on(self->mode);

    if (ret != RTW_SUCCESS) {
        mp_raise_msg(&mp_type_OSError, "WLAN init error");
    }

    wlan_drv_state = true;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wlan_on_obj, wlan_on);


STATIC mp_obj_t wlan_off(mp_obj_t self_in) {

    dhcps_deinit();
    if (wifi_off() != RTW_SUCCESS) {
        mp_raise_msg(&mp_type_OSError, "WLAN off failed");
    } 
    wlan_drv_state = false;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wlan_off_obj, wlan_off);


STATIC mp_obj_t wlan_wifi_is_running(mp_obj_t self_in) {

    if ( wifi_is_up(RTW_STA_INTERFACE) ) {
        printf("STA interface is running\n");
        return mp_const_true;
    } else if ( wifi_is_up(RTW_AP_INTERFACE) ) {
        printf("AP interface is running\n");
        return mp_const_true;
    } else {
        printf("No running interface\n");
        return mp_const_false;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wlan_wifi_is_running_obj, wlan_wifi_is_running);


STATIC mp_obj_t wlan_is_connect_to_ap(mp_obj_t self_in) {

    if ( wifi_is_connected_to_ap() != RTW_SUCCESS)
        return mp_const_false;

    return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wlan_is_connect_to_ap_obj, wlan_is_connect_to_ap);


STATIC void wlan_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    //wlan_obj_t *self = self_in;
    qstr wlan_qstr;
    qstr security_qstr;
    switch (wifi_mode) {
        case RTW_MODE_AP:
            wlan_qstr = MP_QSTR_AP;
            break;
        case RTW_MODE_STA:
            wlan_qstr = MP_QSTR_STA;
            break;
        case RTW_MODE_STA_AP:
            wlan_qstr = MP_QSTR_STA_AP;
            break;
    }

    switch(wifi.security_type) {
        case RTW_SECURITY_OPEN:
            security_qstr = MP_QSTR_OPEN;
            break;
        case RTW_SECURITY_WEP_PSK:
            security_qstr = MP_QSTR_WEP_PSK;
            break;
        case RTW_SECURITY_WEP_SHARED:
            security_qstr = MP_QSTR_WEP_SHARED;
            break;
        case RTW_SECURITY_WPS_OPEN:
            security_qstr = MP_QSTR_WPS_OPEN;
            break;
        case RTW_SECURITY_WPS_SECURE:
            security_qstr = MP_QSTR_WPS_SECURE;
            break;
        case RTW_SECURITY_WPA_AES_PSK:
            security_qstr = MP_QSTR_WPA_AES_PSK;
            break;
        case RTW_SECURITY_WPA2_AES_PSK:
            security_qstr = MP_QSTR_WPA2_AES_PSK;
            break;
        case RTW_SECURITY_WPA_TKIP_PSK:
            security_qstr = MP_QSTR_WPA_TKIP_PSK;
            break;
        case RTW_SECURITY_WPA2_TKIP_PSK:
            security_qstr = MP_QSTR_WPA2_TKIP_PSK;
            break;
        case RTW_SECURITY_WPA2_MIXED_PSK:
            security_qstr = MP_QSTR_WPA2_MIXED_PSK;
            break;
//        case RTW_SECURITY_WPA_WPA2_MIXED:
//            security_qstr = MP_QSTR_WPA_WPA2_MIXED;
//            break;
    }
    mp_printf(print, "mode=%q", wlan_qstr);
    mp_printf(print, ", ssid=%s", ssid); 
    mp_printf(print, ", security_type=%q", security_qstr);
}

STATIC mp_obj_t wlan_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    enum {ARG_mode};
    STATIC const mp_arg_t wlan_init_args[] = {
        { MP_QSTR_mode,   MP_ARG_REQUIRED | MP_ARG_INT },
    };

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(wlan_init_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args),
            wlan_init_args, args);

    wlan_obj_t *self = &wlan_obj;

    self->mode = args[ARG_mode].u_int;
    wifi_mode = args[ARG_mode].u_int;

    validate_wlan_mode(wifi_mode);

    //memset(self->ssid, 0, sizeof(self->ssid));

    //wifi_mode = self->mode;

    return (mp_obj_t)self;
}

STATIC const mp_map_elem_t wlan_locals_dict_table[] = {
    // instance methods

    { MP_OBJ_NEW_QSTR(MP_QSTR_connect),          MP_OBJ_FROM_PTR(&wlan_connect_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_scan),             MP_OBJ_FROM_PTR(&wlan_scan_obj) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_start_ap),         MP_OBJ_FROM_PTR(&wlan_start_ap_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_ip),           MP_OBJ_FROM_PTR(&get_ip_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_disconnect),       MP_OBJ_FROM_PTR(&wlan_disconnect_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_on),               MP_OBJ_FROM_PTR(&wlan_on_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_off),              MP_OBJ_FROM_PTR(&wlan_off_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_wifi_is_running),  MP_OBJ_FROM_PTR(&wlan_wifi_is_running_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_connect_to_ap), MP_OBJ_FROM_PTR(&wlan_is_connect_to_ap_obj) },
    // class constants
    { MP_OBJ_NEW_QSTR(MP_QSTR_CONNECTED),        MP_OBJ_NEW_SMALL_INT(WL_CONNECTED) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_DISCONNECTED),     MP_OBJ_NEW_SMALL_INT(WL_DISCONNECTED) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_IDLE),             MP_OBJ_NEW_SMALL_INT(WL_IDLE_STATUS) },

    // WLAN mode 
    { MP_OBJ_NEW_QSTR(MP_QSTR_STA),              MP_OBJ_NEW_SMALL_INT(RTW_MODE_STA) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_AP),               MP_OBJ_NEW_SMALL_INT(RTW_MODE_AP) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_STA_AP),           MP_OBJ_NEW_SMALL_INT(RTW_MODE_STA_AP) },

    // SECURITY MODE
    { MP_OBJ_NEW_QSTR(MP_QSTR_OPEN),             MP_OBJ_NEW_SMALL_INT(RTW_SECURITY_OPEN) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_WPA_TKIP_PSK),     MP_OBJ_NEW_SMALL_INT(RTW_SECURITY_WPA_TKIP_PSK) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_WPA_AES_PSK),      MP_OBJ_NEW_SMALL_INT(RTW_SECURITY_WPA_AES_PSK) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_WPA2_TKIP_PSK),    MP_OBJ_NEW_SMALL_INT(RTW_SECURITY_WPA2_TKIP_PSK) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_WPA2_AES_PSK),     MP_OBJ_NEW_SMALL_INT(RTW_SECURITY_WPA2_AES_PSK) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_WPA2_MIXED_PSK),   MP_OBJ_NEW_SMALL_INT(RTW_SECURITY_WPA2_MIXED_PSK) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_WPA_WPA2_MIXED),   MP_OBJ_NEW_SMALL_INT(RTW_SECURITY_WPA_WPA2_MIXED) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_WEP_PSK),          MP_OBJ_NEW_SMALL_INT(RTW_SECURITY_WEP_PSK) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_WEP_SHARED),       MP_OBJ_NEW_SMALL_INT(RTW_SECURITY_WEP_SHARED) },

};
STATIC MP_DEFINE_CONST_DICT(wlan_locals_dict, wlan_locals_dict_table);

const mp_obj_type_t wlan_type = {
    { &mp_type_type },
    .name        = MP_QSTR_WLAN,
    .print       = wlan_print,
    .make_new    = wlan_make_new,
    .locals_dict = (mp_obj_t)&wlan_locals_dict,
};
