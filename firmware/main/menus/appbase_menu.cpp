
#include "appbase_menu.h"
#include "device/touch/XPT2046.h"
#include "math/point.h"
#include "../app.h"
#include <esp_log.h>

using libesp::TouchNotification;
using libesp::Point2Ds;

const char *AppBaseMenu::LOGTAG = "AppBaseMenu";

char AppBaseMenu::ListBuffer[AppBaseMenu::NumRows][AppBaseMenu::RowLength] = {0};

void AppBaseMenu::clearListBuffer() {
	memset(&ListBuffer[0], 0, sizeof(ListBuffer));
}

char *AppBaseMenu::getRow(uint8_t row) {
	if(row>=NumRows) return nullptr;
	return &ListBuffer[row][0];
}

bool AppBaseMenu::processTouch(QueueHandle_t &queH, libesp::GUIListData &guiList, uint16_t itemCount, bool &penUp, bool &headerHit) {
	TouchNotification *pe = nullptr;
  /*
	Point2Ds TouchPosInBuf;
	headerHit = false;
	if(xQueueReceive(queH, &pe, 0)) {
		ESP_LOGI(LOGTAG,"que");
		Point2Ds screenPoint(pe->getX(),pe->getY());
		TouchPosInBuf = MyApp::get().getCalibrationMenu()->getPickPoint(screenPoint);
		ESP_LOGI(LOGTAG,"TouchPoint: X:%d Y:%d PD:%d", int32_t(TouchPosInBuf.getX()),
							 int32_t(TouchPosInBuf.getY()), pe->isPenDown()?1:0);
		penUp = !pe->isPenDown();
		delete pe;
		int32_t touchGUI = GUIListProcessor::process(TouchPosInBuf, &guiList,itemCount);
		if(touchGUI==GUIListProcessor::NO_GUI_ITEM_HIT
				|| touchGUI==GUIListProcessor::GUI_HEADER_HIT) {
			pe = nullptr;
		}
		if(touchGUI==GUIListProcessor::GUI_HEADER_HIT) headerHit = true;
		ESP_LOGI(LOGTAG,"pe: %d", int32_t(pe));
	}
  */
	return pe!=nullptr;
}

