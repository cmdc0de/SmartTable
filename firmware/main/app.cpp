/*
 * app.cpp
 *
 * Author: cmdc0de
 */

#include "app.h"
#include <esp_log.h>
#include <system.h>
#include <spibus.h>

#include <driver/uart.h>
#include <driver/gpio.h>
#include <device/leds/noclockprgled.h>
//#include <device/display/frame_buffer.h>
//#include <device/display/display_device.h>
//#include <device/display/fonts.h>
//#include <device/display/gui.h>
//#include <device/touch/XPT2046.h>
#include "menus/menu_state.h"
//#include "menus/game_of_life.h"
#include "menus/wifi_menu.h"
#include "menus/setting_menu.h"
//#include "menus/menu3d.h"
#include <app/display_message_state.h>
#include "spibus.h"
#include "freertos.h"
#include "fatfsvfs.h"
#include "pinconfig.h"
#include <esp_spiffs.h>
#include <time.h>
#include <net/ota.h>

using libesp::ErrorType;
using libesp::System;
using libesp::FreeRTOS;
//using libesp::RGBB;
//using libesp::RGBColor;
using libesp::SPIBus;
using libesp::DisplayMessageState;
using libesp::BaseMenu;
using libesp::APA104;

const char *MyApp::LOGTAG = "AppTask";
const char *MyApp::sYES = "Yes";
const char *MyApp::sNO = "No";

static uint16_t BkBuffer[MyApp::FRAME_BUFFER_WIDTH*MyApp::FRAME_BUFFER_HEIGHT];
static uint16_t *BackBuffer = &BkBuffer[0];

//uint16_t ParallelLinesBuffer[MyApp::DISPLAY_WIDTH*PARALLEL_LINES] = {0};

//static CalibrationMenu MyCalibrationMenu("nvs");
static WiFiMenu MyWiFiMenu;
libesp::OTA CCOTA;

const char *MyErrorMap::toString(int32_t err) {
	return "TODO";
}

MyApp MyApp::mSelf;
static StaticQueue_t InternalQueue;
//static uint8_t InternalQueueBuffer[MyApp::QUEUE_SIZE*MyApp::MSG_SIZE] = {0};

MyApp &MyApp::get() {
	return mSelf;
}

MyApp::MyApp() : AppErrors(), CurrentMode(ONE), LastTime(0)
                 , InternalQueueHandler(0),NVSStorage("appdata","data",false) 
                 , ConfigStore(&NVSStorage), LastConnectCheck(0), IsConfigMode(0) {

	ErrorType::setAppDetail(&AppErrors);
}

MyApp::~MyApp() {

}

uint8_t *MyApp::getBackBuffer() {
	return (uint8_t *)&BackBuffer[0];
}

uint32_t MyApp::getBackBufferSize() {
	return MyApp::FRAME_BUFFER_WIDTH*MyApp::FRAME_BUFFER_HEIGHT*2;
}
 
static libesp::RGB leds[255];
static const size_t NumLEDs = sizeof(leds)/sizeof(leds[0]);
static uint8_t ledBuffer[NumLEDs*3];
libesp::NoClkLedStrip LedStrip = libesp::NoClkLedStrip::create(APA104::get(), 255, NumLEDs);

ErrorType MyApp::initFS() {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/www",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(LOGTAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(LOGTAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(LOGTAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(LOGTAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(LOGTAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}

libesp::ErrorType MyApp::onInit() {
	ErrorType et;
  LedStrip.init(PIN_NUM_LEDS_MOSI, RMT_CHANNEL_0);
  ESP_LOGI(LOGTAG, "set black");
  {
  libesp::RGB color(0,0,0);
  LedStrip.fillColor(color);
  LedStrip.send();
  }
  vTaskDelay(3000 / portTICK_RATE_MS);
  ESP_LOGI(LOGTAG, "set red");
  {
  libesp::RGB color(255,0,0);
  LedStrip.fillColor(color);
  LedStrip.send();
  }
  vTaskDelay(3000 / portTICK_RATE_MS);
  ESP_LOGI(LOGTAG, "set green");
  {
  libesp::RGB color(0,255,0);
  LedStrip.fillColor(color);
  LedStrip.send();
  }
  vTaskDelay(3000 / portTICK_RATE_MS);
  ESP_LOGI(LOGTAG, "set blue");
  {
  libesp::RGB color(0,0,255);
  LedStrip.fillColor(color);
  LedStrip.send();
  }

	//InternalQueueHandler = xQueueCreateStatic(QUEUE_SIZE,MSG_SIZE,&InternalQueueBuffer[0],&InternalQueue);
/*
	ESP_LOGI(LOGTAG,"OnInit: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());
   et = getConfig().init();
   if(!et.ok()) {
      ESP_LOGE(LOGTAG,"Failed to init config: %d %s", et.getErrT(), et.toString());
   }

   static const char *UPDATE_URL="https://smarttable.cmdc0de.tech/clock.bin";
   CCOTA.init(UPDATE_URL);
   CCOTA.logCurrentActiveParitionInfo();
   if(CCOTA.isUpdateAvailable()) {
      ESP_LOGI(LOGTAG,"*****UPDATE AVAILABLE!!!****");
      et = CCOTA.applyUpdate(true);
      if(et.ok()) {
         ESP_LOGI(LOGTAG,"UPDATE SUCCESSFUL to version %s",CCOTA.getCurrentApplicationVersion());
      } else {
         ESP_LOGI(LOGTAG,"UPDATE FAILED");
      }
   }

	ESP_LOGI(LOGTAG,"OnInit: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());

  initFS();
	ESP_LOGI(LOGTAG,"OnInit: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());
	et = NVSStorage.init();
	if(!et.ok()) {
		ESP_LOGI(LOGTAG, "1st InitNVS failed bc %s\n", et.toString());
		et = NVSStorage.initStorage();
		if(et.ok()) {
      ESP_LOGI(LOGTAG, "initStorage succeeded");
			et = NVSStorage.init();
      if(et.ok()) {
        ESP_LOGI(LOGTAG, "NVSSTorage init successful");
      } else {
		    ESP_LOGE(LOGTAG, "2nd InitNVS failed bc %s\nTHIS IS A PROBLEM\n", et.toString());
      }
		} else {
			ESP_LOGI(LOGTAG, "initStorage failed %s\n", et.toString());
		}
	}
	ESP_LOGI(LOGTAG,"OnInit: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());

   init LEDS
  et = APA102c::initAPA102c(PIN_NUM_LEDS_MOSI, PIN_NUM_LEDS_CLK, SPI2_HOST, SPI_DMA_CH1);
  if(!et.ok()) {
    return et;
  } else {
    ESP_LOGI(LOGTAG,"APA102c inited");
  }

  */
	ESP_LOGI(LOGTAG,"OnInit: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());
	SPIBus *vbus = SPIBus::get(SPI2_HOST);
  //et = LedControl.initDevice(vbus);

  et = getNVS().getValue(MyApp::CONFIG_MODE,IsConfigMode);
  ESP_LOGI(LOGTAG, "value of config mode is: %d", int32_t(IsConfigMode));
   if(isConfigMode()) {
      //et = MyWiFiMenu.initWiFiForAP();
      MyApp::get().getWiFiMenu()->startAP();
      //setCurrentMenu(getUserConfigMenu());
   } else {
      et = MyWiFiMenu.initWiFiForSTA();
      if(et.ok()) {
         ESP_LOGI(LOGTAG,"OnInit:After MyWiFiMenu::initWiFi: Free: %u, Min %u", System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());
      } else {
         ESP_LOGE(LOGTAG,"Error Num :%d Msg: %s", et.getErrT(), et.toString());
      }

      ESP_LOGI(LOGTAG,"***********************************************************");
      if(MyWiFiMenu.hasWiFiBeenSetup().ok()) {
         et = MyWiFiMenu.connect();
         setCurrentMenu(getMenuState());
      } else {
         //setCurrentMenu(getSettingMenu());
      }
   }
	return et;
}

ErrorType MyApp::onRun() {
   ErrorType et;
  // TouchTask.broadcast();
	//libesp::BaseMenu::ReturnStateContext rsc = getCurrentMenu()->run();
   /*
	Display.swap();

	if (rsc.Err.ok()) {
		if (getCurrentMenu() != rsc.NextMenuToRun) {
			setCurrentMenu(rsc.NextMenuToRun);
			ESP_LOGI(LOGTAG,"on Menu swap: Free: %u, Min %u",
				System::get().getFreeHeapSize(),System::get().getMinimumFreeHeapSize());
		} else {
		}
	} 

  uint32_t timeSinceLast = FreeRTOS::getTimeSinceStart()-LastTime;
  uint32_t connectCheckTime = FreeRTOS::getTimeSinceStart()-LastConnectCheck;
  if(connectCheckTime > TIME_BETWEEN_WIFI_CONNECTS && !isConfigMode()) {
    LastConnectCheck = FreeRTOS::getTimeSinceStart();
    if(!MyWiFiMenu.isConnected()) {
      ESP_LOGI(LOGTAG,"WifI not connected...reconnecting...");
      MyWiFiMenu.connect();
    }
  }
  */

   switch(CurrentMode) {
   case ONE:
      {
        for(int i=0;i<NumLEDs;++i) {
        }

        //LedControl.init(NumLEDs, &leds[0]);
        //LedControl.send();
        //CurrentMode = TWO;
      }
      break;
    case TWO:
      {
      }
      break;
    case THREE:
      {
      }
      break;
    case FOUR:
      {
      }
      break;
    }
	return et;
}

uint16_t MyApp::getCanvasWidth() {
	return 0;//FrameBuf.getBufferWidth(); 
}

uint16_t MyApp::getCanvasHeight() {
	return 0;//FrameBuf.getBufferHeight();
}

uint16_t MyApp::getLastCanvasWidthPixel() {
	return getCanvasWidth()-1;
}

uint16_t MyApp::getLastCanvasHeightPixel() {
	return getCanvasHeight()-1;
}

MenuState MyMenuState;
libesp::DisplayMessageState DMS;


MenuState *MyApp::getMenuState() {
	return &MyMenuState;
}
WiFiMenu *MyApp::getWiFiMenu() {
  return &MyWiFiMenu;
}

libesp::OTA &MyApp::getOTA() {
   return CCOTA;
}

DisplayMessageState *MyApp::getDisplayMessageState(BaseMenu *bm, const char *msg, uint32_t msDisplay) {
	//DMS.setMessage(msg);
	//DMS.setNextState(bm);
	//DMS.setTimeInState(msDisplay);
	//DMS.setDisplay(&Display);
	return &DMS;
}

