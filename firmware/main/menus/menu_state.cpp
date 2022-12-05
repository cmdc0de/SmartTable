#include "menu_state.h"
#include "../app.h"
#include <app/display_message_state.h>
#include <esp_log.h>
#include <math/rectbbox.h>
#include "setting_menu.h"

using libesp::ErrorType;
using libesp::BaseMenu;
using libesp::RGBColor;
using libesp::XPT2046;
using libesp::Point2Ds;
using libesp::TouchNotification;

static StaticQueue_t InternalQueue;
static uint8_t InternalQueueBuffer[MenuState::QUEUE_SIZE*MenuState::MSG_SIZE] = {0};
static const char *LOGTAG = "MenuState";

static libesp::RectBBox2D TempBV(Point2Ds(30,37), 20, 15);
static libesp::Label TempLabel(uint16_t(0), (const char *)"Temp", &TempBV,RGBColor::BLUE, RGBColor::WHITE, RGBColor::BLACK, false);
static libesp::RectBBox2D HumBV(Point2Ds(85,37), 20, 15);
static libesp::Label HumLabel (uint16_t(0), (const char *)"Humidity", &HumBV,RGBColor::BLUE, RGBColor::WHITE, RGBColor::BLACK, false);

static libesp::RectBBox2D CO2BV(Point2Ds(140,37), 20, 15);
static libesp::Label CO2Label (uint16_t(0), (const char *)"CO2", &CO2BV,RGBColor::BLUE, RGBColor::WHITE, RGBColor::BLACK, false);

static libesp::RectBBox2D LSBV(Point2Ds(30,120), 20, 15);
static libesp::Label LSLabel (uint16_t(0), (const char *)"Light", &LSBV,RGBColor::BLUE, RGBColor::WHITE, RGBColor::BLACK, false);

static libesp::RectBBox2D OzoneBV(Point2Ds(85,120), 20, 15);
static libesp::Label OzoneLabel (uint16_t(0), (const char *)"Ozone", &OzoneBV,RGBColor::BLUE, RGBColor::WHITE, RGBColor::BLACK, false);

static libesp::RectBBox2D SettingRect(Point2Ds(150,120), 28, 15);
static libesp::Button SettingBtn((const char *)"Settings", uint16_t(2), &SettingRect, RGBColor::BLUE, RGBColor::WHITE);

static const int8_t NUM_INTERFACE_ITEMS = 6;
static libesp::Widget *InterfaceElements[NUM_INTERFACE_ITEMS] = {&TempLabel, &HumLabel, &CO2Label, &LSLabel, &OzoneLabel, &SettingBtn};

MenuState::MenuState() :
	AppBaseMenu(),
	MyLayout(&InterfaceElements[0],NUM_INTERFACE_ITEMS, MyApp::get().getLastCanvasWidthPixel(), MyApp::get().getLastCanvasHeightPixel(), false){
	InternalQueueHandler = xQueueCreateStatic(QUEUE_SIZE,MSG_SIZE,&InternalQueueBuffer[0],&InternalQueue);
	MyLayout.reset();
}

MenuState::~MenuState() {

}

ErrorType MenuState::onInit() {
	return ErrorType();
}

libesp::BaseMenu::ReturnStateContext MenuState::onRun() {
	BaseMenu *nextState = this;

	return BaseMenu::ReturnStateContext(nextState);
}

ErrorType MenuState::onShutdown() {
	return ErrorType();
}

