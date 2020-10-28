if(APPLE)
    INSTALL(
        FILES
            qml_de.qm
        DESTINATION
            Filmulator.app/Contents/Resources/qml/filmulator-gui/i18n
    )

    install(CODE "execute_process(COMMAND install_name_tool -add_rpath @loader_path/../Frameworks Filmulator.app/Contents/MacOS/filmulator)")
    install(CODE "execute_process(COMMAND $ENV{QT}/bin/macdeployqt Filmulator.app -executable=Filmulator.app/Contents/MacOS/filmulator -no-strip -verbose=3 -qmldir=Filmulator.app/Contents/Resources/qml)")

elseif(WIN32)
    INSTALL(
        FILES
            qml_de.qm
        DESTINATION
            ${CMAKE_INSTALL_BINDIR}/qml/filmulator-gui/i18n
    )

else()
    INSTALL(
        FILES
            qml_de.qm
        DESTINATION
            ${CMAKE_INSTALL_DATADIR}/Filmulator/qml/filmulator-gui/i18n
    )
endif()
