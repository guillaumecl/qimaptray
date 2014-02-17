cmake_minimum_required(VERSION 2.6)
project(qimaptray)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -Werror")


find_package(OpenSSL REQUIRED)
find_package(Qt4 REQUIRED)

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


add_executable(qimaptray
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

target_link_libraries(qimaptray ssl crypto ${QT_LIBRARIES})
