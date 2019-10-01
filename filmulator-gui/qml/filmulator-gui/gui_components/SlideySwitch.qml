/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.12
import "../colors.js" as Colors

/*!
    \qmltype Switch
    \inqmlmodule QtQuick.Controls
    \since 5.2
    \ingroup controls
    \brief A switch.

    \image switch.png
    \caption On and Off states of a Switch.

    A Switch is a toggle button that can be switched on (checked) or off
    (unchecked). Switches are typically used to represent features in an
    application that can be enabled or disabled without affecting others.

    On mobile platforms, switches are commonly used to enable or disable
    features.

    \qml
    Column {
        Switch { checked: true }
        Switch { checked: false }
    }
    \endqml

    You can create a custom appearance for a Switch by
    assigning a \l {SwitchStyle}.
*/

Item {
    id: root
    property real uiScale: 1
    width: 70 * uiScale
    height: 20 * uiScale

    /*!
        This property is \c true if the control is checked.
        The default value is \c false.
    */
    property bool checked: false

    /*!
        \qmlproperty bool Switch::pressed
        \since QtQuick.Controls 1.3

        This property is \c true when the control is pressed.
    */
    readonly property alias pressed: internal.pressed

    /*!
        This property is \c true if the control takes the focus when it is
        pressed; \l{QQuickItem::forceActiveFocus()}{forceActiveFocus()} will be
        called on the control.
    */
    property bool activeFocusOnPress: false

    /*!
        \since QtQuick.Controls 1.3

        This signal is emitted when the control is clicked.
    */
    signal clicked

    Keys.onPressed: {
        if (event.key === Qt.Key_Space && !event.isAutoRepeat)
            checked = !checked;
    }

    Rectangle {
        id: background
        width: parent.width
        height: parent.height
        radius: 3 * uiScale
        gradient: Gradient {
            GradientStop {color: root.checked ? Colors.lightOrange : Colors.brightGrayL; position: 0.0}
            GradientStop {color: root.checked ? Colors.lightOrange : Colors.brightGray; position: 0.1}
            GradientStop {color: root.checked ? Colors.lightOrange : Colors.brightGray; position: 1.0}
        }

        border.width: 1 * uiScale
        border.color: root.checked ? Colors.weakOrange : Colors.middleGray
    }

    MouseArea {
        id: internal

        Rectangle {
            id: handle
            width: 30 * uiScale
            height: 20 * uiScale
            radius: 3 * uiScale
            gradient: Gradient {
                GradientStop {color: Colors.lowGrayH; position: 0.0}
                GradientStop {color: Colors.lowGray; position: 0.15}
                GradientStop {color: Colors.lowGray; position: 0.9}
                GradientStop {color: Colors.lowGrayL; position: 1.0}
            }

            Behavior on x {
                SmoothedAnimation {
                    velocity: 200 * uiScale
                }
            }

            border.width: 1 * uiScale
            border.color: toggleSwitch.checked ? Colors.weakOrange : Colors.middleGray
        }

        property real min: 0
        property real max: background.width - handle.width
        focus: true
        anchors.fill: parent
        drag.threshold: 0
        drag.target: handle
        drag.axis: Drag.XAxis
        drag.minimumX: min
        drag.maximumX: max

        onPressed: {
            if (activeFocusOnPress)
                root.forceActiveFocus()
        }

        onReleased: {
            if (drag.active) {
                checked = (handle.x < max/2) ? false : true;
                handle.x = checked ? internal.max : internal.min
            } else {
                checked = (handle.x === max) ? false : true
            }
        }

        onClicked: root.clicked()
    }

    onCheckedChanged:  {
        handle.x = checked ? internal.max : internal.min
    }

    activeFocusOnTab: true
    Accessible.role: Accessible.CheckBox
    Accessible.name: "switch"
}
