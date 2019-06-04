IF(APPLE)
INSTALL(
    FILES
        colors.js
        Organize.qml
        getRoot.js
        main.qml
        Edit.qml
        Queue.qml
        Import.qml
        EditTools.qml
        Settings.qml
        generateHistogram.js
    DESTINATION
        ${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Resources/qml/filmulator-gui
)

ELSE()
INSTALL(
    FILES
        colors.js
        Organize.qml
        getRoot.js
        main.qml
        Edit.qml
        Queue.qml
        Import.qml
        EditTools.qml
        Settings.qml
        generateHistogram.js
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}/Filmulator/qml/filmulator-gui
)

ENDIF()

ADD_SUBDIRECTORY(gui_components)
