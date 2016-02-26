CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(lockscreen C)

SET(INSTALL_EXEC_PREFIX "${INSTALL_PREFIX}/bin")
SET(INSTALL_RESDIR "${INSTALL_PREFIX}/res")
SET(INSTALL_IMAGEDIR "${INSTALL_RESDIR}/images")
SET(EDJEDIR "${INSTALL_RESDIR}/edje")
SET(LOCALEDIR "${INSTALL_RESDIR}/locale")
SET(CMAKE_VERBOSE_MAKEFILE 0)

SET(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}/..")
SET(PROJECT_RESOURCES_DIR "${PROJECT_ROOT_DIR}/res")
SET(PROJECT_EDC_DIR "${PROJECT_RESOURCES_DIR}/edje")
SET(PROJECT_IMAGES_DIR "${PROJECT_RESOURCES_DIR}/images")
SET(PROJECT_PO_DIR "${PROJECT_RESOURCES_DIR}/po")

INCLUDE(FindPkgConfig)
pkg_check_modules(MENU_PKGS REQUIRED
	bundle
	appcore-efl
	evas
	edje
	elementary
	ecore
	eina
	capi-system-sensor
	capi-system-info
	capi-system-runtime-info
	capi-system-system-settings
	capi-system-device
	capi-appfw-application
	capi-appfw-preference
	capi-appfw-app-manager
	capi-ui-efl-util
	aul
	ail
	notification
	minicontrol-viewer
	security-server
	minicontrol-monitor
	ui-gadget-1
	feedback
	msg-service
	efl-extension
	key-manager
	accounts-svc
	deviced
	tzsh-lockscreen-service
	capi-base-utils-i18n
	capi-telephony
)

FOREACH (flag ${MENU_PKGS_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden -Wall -Winline -g -fno-builtin-malloc -fPIE")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

INCLUDE_DIRECTORIES(${PROJECT_ROOT_DIR}/inc)

ADD_EXECUTABLE(${PROJECT_NAME}
	${PROJECT_ROOT_DIR}/src/background_view.c
	${PROJECT_ROOT_DIR}/src/battery_ctrl.c
	${PROJECT_ROOT_DIR}/src/camera.c
	${PROJECT_ROOT_DIR}/src/default_lock.c
	${PROJECT_ROOT_DIR}/src/lock_time.c
	${PROJECT_ROOT_DIR}/src/lockscreen.c
	${PROJECT_ROOT_DIR}/src/property.c
	${PROJECT_ROOT_DIR}/src/sim_state.c
	${PROJECT_ROOT_DIR}/src/util.c
	${PROJECT_ROOT_DIR}/src/window.c
	${PROJECT_ROOT_DIR}/src/music_player.c
	${PROJECT_ROOT_DIR}/src/contextual_view.c
	${PROJECT_ROOT_DIR}/src/data_model.c
	${PROJECT_ROOT_DIR}/src/data_model_background.c
	${PROJECT_ROOT_DIR}/src/data_model_battery.c
	${PROJECT_ROOT_DIR}/src/data_model_missed_events.c
	${PROJECT_ROOT_DIR}/src/data_model_music_player.c
	${PROJECT_ROOT_DIR}/src/data_model_sim.c
	${PROJECT_ROOT_DIR}/src/main_view.c
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} -lm)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${MENU_PKGS_LDFLAGS})

# Install
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_EXEC_PREFIX})

INSTALL(FILES ${PROJECT_ROOT_DIR}/tizen-manifest.xml DESTINATION ${SYS_PACKAGES_DIR} RENAME org.tizen.lockscreen.xml)
INSTALL(DIRECTORY DESTINATION ${PREFIX}/data)
INSTALL(FILES ${PROJECT_ROOT_DIR}/shared/res/lockscreen.png DESTINATION ${SYS_ICONS_DIR})

# End of a file
ADD_SUBDIRECTORY(res)
