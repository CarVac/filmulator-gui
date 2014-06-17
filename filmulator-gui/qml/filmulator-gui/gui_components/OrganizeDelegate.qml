import QtQuick 2.2

Rectangle {
    id: root
    width: 400
    height: 300
    color: "#00000000"
    property string hour
    property string filename

    Rectangle {
        width: 350
        height: 250
        x: 25
        y: 25
        color: "green"

        Column {
            Text {
                id: time
                color: "white"
                text: root.hour ? root.hour : ""
            }
            Text {
                id: name
                color: "white"
                text: root.filename ? root.filename : ""
            }
        }
    }
}
