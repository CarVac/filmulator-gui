if(APPLE)
    INSTALL(
        FILES
            ToolRadioButton.qml
            ToolSlider.qml
            FilmProgressBar.qml
            ToolButton.qml
            ImportDirEntry.qml
            ToolSwitch.qml
            ToolButtonStyle.qml
            OrganizeDelegate.qml
            ImportTextEntry.qml
            ImportFileEntry.qml
            ToolCalendar.qml
            QueueDelegate.qml
            ToolTip.qml
            SlipperySlider.qml
            ToolRadioButtonStyle.qml
        DESTINATION
            ${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Resources/qml/filmulator-gui/gui_components
    )
    install(CODE "file(MAKE_DIRECTORY Filmulator.app/Contents/Frameworks)")
    install(CODE "file(READ_SYMLINK \"${EXIV2_LIBRARY}\" result)")
    install(CODE "get_filename_component(dir \"${EXIV2_LIBRARIES}\" DIRECTORY)")
    install(CODE "file(COPY \"${dir}/${EXIV2_LIBRARY}\" DESTINATION \"Filmulator.app/Contents/Frameworks\")")
    install(CODE "execute_process(COMMAND install_name_tool -add_rpath @loader_path/../Frameworks Filmulator.app/Contents/MacOS/filmulator)")
    install(CODE "execute_process(COMMAND $ENV{QT}/bin/macdeployqt Filmulator.app -executable=Filmulator.app/Contents/MacOS/filmulator -no-strip -verbose=3 -qmldir=Filmulator.app/Contents/Resources/qml)")

else()
    INSTALL(
        FILES
            ToolRadioButton.qml
            ToolSlider.qml
            FilmProgressBar.qml
            ToolButton.qml
            ImportDirEntry.qml
            ToolSwitch.qml
            ToolButtonStyle.qml
            OrganizeDelegate.qml
            ImportTextEntry.qml
            ImportFileEntry.qml
            ToolCalendar.qml
            QueueDelegate.qml
            ToolTip.qml
            SlipperySlider.qml
            ToolRadioButtonStyle.qml
        DESTINATION
            ${CMAKE_INSTALL_DATADIR}/Filmulator/qml/filmulator-gui/gui_components
    )
endif()
