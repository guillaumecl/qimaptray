cmake_minimum_required(VERSION 2.6)
project(qimaptray)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -Werror")


find_package(OpenSSL REQUIRED)
find_package(Qt4 REQUIRED)

option(ENABLE_WEBCAM "Compile using webcam_led" ON)
option(ENABLE_LIBNL "Compile with libnl support to detect link down events" ON)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})


QT4_WRAP_CPP(QIMAPTRAY_HEADERS_MOC
  connection.h
  tray.h
)

QT4_WRAP_UI(QIMAPTRAY_FORMS_HEADERS

)

QT4_ADD_RESOURCES(QIMAPTRAY_RESOURCES
  qimaptray.qrc
)


QT4_ADD_TRANSLATION(QIMAPTRAY_TRANSLATIONS
)

set(FILES
  imap.cpp
  status.cpp
  qimaptray.cpp
  connection.cpp
  tray.cpp
  ${QIMAPTRAY_HEADERS_MOC}
  ${QIMAPTRAY_FORMS_HEADERS}
  ${QIMAPTRAY_RESOURCES}
  ${QIMAPTRAY_TRANSLATIONS}
)

set(LIBS
	ssl
	crypto
	 ${QT_LIBRARIES}
)

if (ENABLE_WEBCAM)
  set(FILES ${FILES} webcam.cpp)
  set(LIBS ${LIBS} webcam_led v4l2)
else(ENABLE_WEBCAM)
  set(FILES ${FILES} webcam_dummy.cpp)
endif(ENABLE_WEBCAM)

if (ENABLE_LIBNL)
  pkg_check_modules(NL REQUIRED libnl-3.0)
  include_directories(${NL_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${NL_LIBRARIES})
  add_definitions(-DENABLE_LIBNL)
endif (ENABLE_LIBNL)

add_executable(qimaptray
  ${FILES}
)

target_link_libraries(qimaptray ${LIBS})

install(TARGETS qimaptray DESTINATION bin)
install(FILES qimaptray.man DESTINATION share/man/man1 RENAME qimaptray.1)
