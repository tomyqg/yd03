#include <sys/ioctl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include "lwip/ip.h"
#include "lwip/dns.h"
#include "ite/itp.h"
#include "iniparser/iniparser.h"
#include "ctrlboard.h"
#include "wifiMgr.h"
#ifdef CFG_NET_FTP_SERVER
#include <unistd.h>
#include "ftpd.h"
#endif

#define DHCP_TIMEOUT_MSEC (60 * 1000) //60sec
#define WIFI_DONGLE_TIMEOUT_MSEC (20 * 1000) //20sec


static struct timeval tvStart = {0, 0}, tvEnd = {0, 0};
static bool networkIsReady, networkToReset;
static int networkSocket;

#ifdef CFG_NET_WIFI
static WIFI_MGR_SETTING gWifiSetting;
static bool wifi_dongle_hotplug, need_reinit_wifimgr;
static int gInit =0; // wifi init
#endif


static void ResetEthernet(void)
{
    ITPEthernetSetting setting;
    ITPEthernetInfo info;
    unsigned long mscnt = 0;
    char buf[16], *saveptr;

    memset(&setting, 0, sizeof (ITPEthernetSetting));

    setting.index = 0;

    // dhcp
    setting.dhcp = theConfig.dhcp;

    // autoip
    setting.autoip = 0;

    // ipaddr
    strcpy(buf, theConfig.ipaddr);
    setting.ipaddr[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.ipaddr[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.ipaddr[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.ipaddr[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // netmask
    strcpy(buf, theConfig.netmask);
    setting.netmask[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.netmask[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.netmask[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.netmask[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // gateway
    strcpy(buf, theConfig.gw);
    setting.gw[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.gw[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.gw[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.gw[3] = atoi(strtok_r(NULL, " ", &saveptr));

    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting);

    printf("Wait ethernet cable to plugin");
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL))
    {
        sleep(1);
        putchar('.');
        fflush(stdout);
    }

    printf("\nWait DHCP settings");
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
    {
        usleep(100000);
        mscnt += 100;

        putchar('.');
        fflush(stdout);

        if (mscnt >= DHCP_TIMEOUT_MSEC)
        {
            printf("\nDHCP timeout, use default settings\n");
            setting.dhcp = setting.autoip = 0;
            ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting);
            break;
        }
    }
    puts("");

    if (ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
    {
        char* ip;

        info.index = 0;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        ip = ipaddr_ntoa((const ip_addr_t*)&info.address);

        printf("IP address: %s\n", ip);

        networkIsReady = true;
    }
}


#ifdef CFG_NET_WIFI
static ResetWifi()
{
//    ITPEthernetSetting setting;
    char buf[16], *saveptr;

    memset(&gWifiSetting.setting, 0, sizeof (ITPEthernetSetting));

    gWifiSetting.setting.index = 0;

    // dhcp
    gWifiSetting.setting.dhcp = 1;

    // autoip
    gWifiSetting.setting.autoip = 0;

    // ipaddr
    strcpy(buf, theConfig.ipaddr);
    gWifiSetting.setting.ipaddr[0] = atoi(strtok_r(buf, ".", &saveptr));
    gWifiSetting.setting.ipaddr[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.ipaddr[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.ipaddr[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // netmask
    strcpy(buf, theConfig.netmask);
    gWifiSetting.setting.netmask[0] = atoi(strtok_r(buf, ".", &saveptr));
    gWifiSetting.setting.netmask[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.netmask[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.netmask[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // gateway
    strcpy(buf, theConfig.gw);
    gWifiSetting.setting.gw[0] = atoi(strtok_r(buf, ".", &saveptr));
    gWifiSetting.setting.gw[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.gw[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.gw[3] = atoi(strtok_r(NULL, " ", &saveptr));
}

static int wifiCallbackFucntion(int nState)
{
    switch (nState)
    {
        case WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH:
            printf("[Indoor]WifiCallback connection finish \n");
            WebServerInit();

#ifdef CFG_NET_FTP_SERVER
		    ftpd_setlogin(theConfig.user_id, theConfig.user_password);
		    ftpd_init();
#endif
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_DISCONNECT_30S:
            printf("[Indoor]WifiCallback connection disconnect 30s \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_RECONNECTION:
            printf("[Indoor]WifiCallback connection reconnection \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_TEMP_DISCONNECT:
            printf("[Indoor]WifiCallback connection temp disconnect \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_FAIL:
            printf("[Indoor]WifiCallback connecting fail, please check ssid,password,secmode \n");
			wifiMgr_clientMode_disconnect();
        break;

        default:
            printf("[Indoor]WifiCallback unknown %d state  \n",nState);
        break;
    }
}

void NetworkWifiModeSwitch(void)
{
	int ret;

	ret = WifiMgr_Switch_ClientSoftAP_Mode(gWifiSetting);
}

void NetworkSupriseRemove(void)
{
	int nWiFiConnState = 0, nWiFiConnEcode = 0;

	usleep(400*1000);
	if(theConfig.wifi_on_off){
		do
		{
			wifiMgr_get_connect_state(&nWiFiConnState, &nWiFiConnEcode);
			usleep(1000*1000);
			printf("[Suprise Remove] waiting for connection done\n");
			wifiMgr_CancelConnect();
		}  while(nWiFiConnState != WIFIMGR_CONNSTATE_STOP);
	}
	printf("connection is done\n");

	ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_STOP_DHCP, NULL);
	wifiMgr_clientMode_sleep_disconnect();
	wifiMgr_terminate();
}

void NetworkSupriseInsert(void)
{
	ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_ADD_NETIF, NULL);
	ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL);

	printf("[Suprise Insert] wifiMgr_reinit \n");
	snprintf(gWifiSetting.ssid	  , WIFI_SSID_MAXLEN	, theConfig.ssid);
	snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, theConfig.password);
	snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN, theConfig.secumode);

	wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
	usleep(200*1000);
}

#endif

static void* NetworkTask(void* arg)
{
#ifdef CFG_NET_WIFI
    int nTemp;
    int dongle_now, dongle_past;
    unsigned long mscnt = 0;
    struct timeval tvStartWifi = {0, 0}, tvEndWifi = {0, 0};
    int nWifiNotReady = 0;
    
#else
    ResetEthernet();
#endif

#if defined(CFG_NET_FTP_SERVER) && !defined(CFG_NET_WIFI)
    ftpd_setlogin(theConfig.user_id, theConfig.user_password);
    ftpd_init();
#endif

    for (;;)
    {
#ifdef CFG_NET_WIFI
        gettimeofday(&tvEnd, NULL);

        nTemp = itpTimevalDiff(&tvStart, &tvEnd);
        if (nTemp>5000 && gInit == 0){
            printf("[%s] Init wifimgr \n", __FUNCTION__);

            while(!ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL)){
                sleep(1);
                printf("wait ITP_IOCTL_IS_DEVICE_READY \n");
            }

            WifiMgr_clientMode_switch(theConfig.wifi_on_off);
            printf("wifi mode %d \n",theConfig.wifi_mode);

            if (theConfig.wifi_mode == WIFI_SOFTAP){
				ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_ENABLE, NULL); //determine wifi softAP mode
				nTemp = wifiMgr_init(WIFIMGR_MODE_SOFTAP, 0, gWifiSetting);
			}else {
				ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL); //determine wifi client mode
                nTemp = wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
			}
            ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SLEEP, (void *)default_no_sleep_or_wakeup);
			dongle_past = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL);

            gInit = 1;
        } else if (gInit == 1){
            networkIsReady = wifiMgr_is_wifi_available(&nTemp);
            networkIsReady = (bool)nTemp;

            if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == sleep_to_wakeup){

                while(!ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL)){
                    printf("[%s] Wait wifi dongle plugin... \n", __FUNCTION__);
                    usleep(200*1000);
                    nWifiNotReady = 1;
                }

                
            // sleep to wakeup , wait for 5s to initialize
                gettimeofday(&tvStartWifi, NULL);
                nTemp = 0;
                do{
                    usleep(1000*1000);
                    gettimeofday(&tvEndWifi, NULL);
                    nTemp = itpTimevalDiff(&tvStartWifi, &tvEndWifi);
                    if (nTemp>5000){
                        printf("[Network] ready to init wifi \n");
                        break;
                    }
                    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == wakeup_to_sleep){
                        printf("[Network] fast to sleep \n");                        
                        break;
                    }
                } while (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == sleep_to_wakeup);
                
                if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == wakeup_to_sleep)
                {
                    // fast sleep , do not init wifi mgr
                } else {
                
                    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_ADD_NETIF, NULL);
                    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL);

                    printf("[Network] wifiMgr_init \n");
#if 0                
                    
                    while(!ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL)){
                        printf("[%s] Wait wifi dongle plugin... \n", __FUNCTION__);
                        usleep(200*1000);
                        nWifiNotReady = 1;

#ifdef CFG_WATCHDOG_ENABLE
					if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL)){
                    	mscnt += 100;
                    	if (mscnt >= WIFI_DONGLE_TIMEOUT_MSEC/2){
                    		ithPrintf("Wifi dongle undefined error, Reboot...\n");
                    		ithWatchDogEnable();
                    		ithWatchDogSetReload(0);
                    		ithWatchDogRestart();
                            }
					}
#endif // CFG_WATCHDOG_ENABLE
                    }
                    if (nWifiNotReady==1){
                        printf("[Network] wifi not ready, reset status\n");
                        nWifiNotReady = 0;
                        usleep(1000*1000);
                        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_ADD_NETIF, NULL);
                        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL);                        
                    }
#endif                    
                    snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, theConfig.ssid);
                    snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, theConfig.password);
                    snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN, theConfig.secumode);
                    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == wakeup_to_sleep)
                    {
                            // fast sleep , do not init wifi mgr
                    } else {
                     nTemp = wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
                     usleep(200*1000);
                     //Delay a while to ensure wifi mgr is inited and task is ready.
                     ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SLEEP, (void *)default_no_sleep_or_wakeup);
                    }
                }
            }

			/* ===========Wifi dongle make suprise remove process=========== */
            if (theConfig.wifi_on_off == WIFIMGR_SWITCH_OFF){
                dongle_now = ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL);

                if (dongle_now - dongle_past < 0){
                    //Suprise remove Wifi dongle
                    printf("=========>dongle_now(%d) - dongle_past(%d) = %d\n", dongle_now, dongle_past, dongle_now - dongle_past);
                    NetworkSupriseRemove();
                    dongle_past = dongle_now;
                }else if (dongle_now - dongle_past > 0){
                    //Suprise insert Wifi dongle
                    printf("=========>dongle_now(%d) - dongle_past(%d) = %d\n", dongle_now, dongle_past, dongle_now - dongle_past);
                    NetworkSupriseInsert();
                    dongle_past = dongle_now;
                }
            }
            /* ================================================ */
        }
#else
        networkIsReady = ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL);
#endif

        if (networkToReset)
        {
#ifndef CFG_NET_WIFI
            ResetEthernet();
#endif
            networkToReset = false;
        }
        //sleep(1);
        usleep(100*1000);
    }
    return NULL;
}

void NetworkInit(void)
{
    pthread_t task;

    networkIsReady = false;
    networkToReset = false;
    networkSocket = -1;
#ifdef CFG_NET_WIFI
    snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, theConfig.ssid);
    snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, theConfig.password);
    snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN, theConfig.secumode);
    gWifiSetting.wifiCallback = wifiCallbackFucntion;
    ResetWifi();

    gettimeofday(&tvStart, NULL);
#endif

    pthread_create(&task, NULL, NetworkTask, NULL);
}

bool NetworkIsReady(void)
{
    return networkIsReady;
}

void NetworkReset(void)
{
    networkToReset  = true;
}
