set(srcs "./app.cpp"
	"./config.cpp"
	"./smart_table.cpp"
  "./menus/menu_state.cpp"
  "./menus/appbase_menu.cpp"
  "./menus/gui_list_processor.cpp"
  "./menus/wifi_menu.cpp"
  "./menus/user_config_menu.cpp"
  "./menus/update_menu.cpp"
)

idf_component_register(SRCS "${srcs}"
		INCLUDE_DIRS ../components/esp-etl/etl/include/ ../components/libesp
    EMBED_TXTFILES "certs/cacert.pem"
                    "certs/prvtkey.pem"
		)


set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../web/smarttable")
if(EXISTS ${WEB_SRC_DIR}/build)
  spiffs_create_partition_image(www ${WEB_SRC_DIR}/build FLASH_IN_PROJECT)
else()
  message(FATAL_ERROR "${WEB_SRC_DIR}/build doesn't exit. Please run 'npm run build' in ${WEB_SRC_DIR}")
endif()

