import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import "gui_components"

Rectangle {
    id: tools
    color: "#404040"
    width: 250
    Layout.maximumWidth: 500
    Layout.minimumWidth: 250
    property int index
    property real exposureComp
    property real whitepoint
    property real blackpoint
    property real shadowY
    property real highlightY
    property real filmSize
    property bool defaultCurve

    SplitView{
        anchors.fill: parent
        orientation: Qt.Vertical

        Canvas {
            id:canvas
            width:parent.width
            Layout.minimumHeight: 50
            height:250
            property int lineWidth: 1
            property real alpha: 1.0
            property int hist: filmProvider.hist
            antialiasing: true

            onWidthChanged:requestPaint();
            onHistChanged: requestPaint();

            onPaint: {
                var ctx = canvas.getContext('2d');
                ctx.save();
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                ctx.globalAlpha = canvas.alpha;
                ctx.lineWidth = canvas.lineWidth;
                var myGradient = ctx.createLinearGradient(0,0,canvas.width,0);
                var hist = canvas.hist;

                var startx = 10;
                var endx = canvas.width - 10;
                var graphwidth = endx - startx;
                var starty = canvas.height - 10;
                var endy = 10;
                var graphheight = starty - endy;
                var histPoint = 0;
                var maxValue = 128.0

                //Luma curve
                ctx.beginPath();
                ctx.moveTo(startx,starty);
                for(var i = 0; i < maxValue; i++)
                {
                    histPoint = filmProvider.getLumaHistogramPoint(i);
                    ctx.lineTo(startx+(i/maxValue)*graphwidth,starty-(histPoint)*graphheight);
                }
                ctx.lineTo(endx,starty);
                ctx.lineTo(startx,starty);
                ctx.closePath();
                myGradient.addColorStop(1,"white");
                myGradient.addColorStop(0,'rgb(180,180,180)');
                ctx.fillStyle = myGradient;
                ctx.fill()

                //rCurve
                ctx.beginPath()
                ctx.moveTo(startx,starty);
                for(var i = 0; i < maxValue; i++)
                {
                    histPoint = filmProvider.getRHistogramPoint(i);
                    ctx.lineTo(startx+(i/maxValue)*graphwidth,starty-(histPoint)*graphheight);
                }
                ctx.lineTo(endx,starty);
                ctx.closePath();
                ctx.strokeStyle = "#FF0000";
                ctx.stroke();

                //gCurve
                ctx.beginPath()
                ctx.moveTo(startx,starty);
                for(var i = 0; i < maxValue; i++)
                {
                    histPoint = filmProvider.getGHistogramPoint(i);
                    ctx.lineTo(startx+(i/maxValue)*graphwidth,starty-(histPoint)*graphheight);
                }
                ctx.lineTo(endx,starty);
                ctx.closePath();
                ctx.strokeStyle = "#00FF00";
                ctx.stroke();

                //bCurve
                ctx.beginPath()
                ctx.moveTo(startx,starty);
                for(var i = 0; i < maxValue; i++)
                {
                    histPoint = filmProvider.getBHistogramPoint(i);
                    ctx.lineTo(startx+(i/maxValue)*graphwidth,starty-(histPoint)*graphheight);
                }
                ctx.lineTo(endx,starty);
                ctx.closePath();
                ctx.strokeStyle = "#0000FF"
                ctx.stroke();

                ctx.strokeStyle = "#000000";
                ctx.strokeRect(startx,endy,graphwidth,graphheight);

                ctx.restore();
            }
        }


        Item {
            id: toolListItem
            width: parent.width
            Flickable {
                id: toolList
                //anchors.fill: parent
                width: parent.width
                height: parent.height
                flickableDirection: Qt.Vertical
                clip: true
                contentWidth: contentItem.childrenRect.width
                contentHeight: contentItem.childrenRect.height
                ColumnLayout {
                    spacing: 0
                    //anchors.fill: parent
                    width: toolListItem.width
                    CheckBox{
                        id: defaultToneCurveCheckBox
                        text: qsTr("Default Tone Curve")
                        checked: defaultCurve
                        onClicked: {
                            filmProvider.defaultToneCurveEnabled = checked
                            editortab.rolling = (editortab.rolling + 1) % 10
                        }
                    }

                    ToolSlider {
                        id: exposureCompSlider
                        title: qsTr("Exposure Compensation")
                        minimumValue: -5
                        maximumValue: 5
                        stepSize: 1/3
                        defaultValue: exposureComp
                        onValueChanged: {
                            filmProvider.exposureComp = value
                            editortab.rolling = (editortab.rolling + 1) % 10
                        }
                    }

                    ToolSlider {
                        id: filmSizeSlider
                        title: qsTr("Film Area")
                        minimumValue: 10
                        maximumValue: 300
                        defaultValue: Math.sqrt(filmSize)
                        //The following thresholds are 24mmx65mm and twice 6x9cm film's
                        // areas, respectively.
                        valueText: (value*value < 1560) ? "SF" : (value*value < 9408) ? "MF" : "LF"
                        onValueChanged: {
                            filmProvider.filmArea = value*value
                            editortab.rolling = (editortab.rolling + 1) % 10
                        }
                    }

                    ToolSlider {
                        id: blackpointSlider
                        title: qsTr("Black Clipping Point")
                        minimumValue: 0
                        maximumValue: 5/1000
                        defaultValue: blackpoint
                        valueText: value*1000
                        onValueChanged: {
                            filmProvider.blackpoint = value
                            editortab.rolling = (editortab.rolling + 1) % 10
                        }
                    }

                    ToolSlider {
                        id: shadowSlider
                        title: qsTr("Shadow Brightness")
                        minimumValue: 0
                        maximumValue: 1
                        defaultValue: shadowsY
                        valueText: value*1000
                        onValueChanged: {
                            filmProvider.shadowsY = value
                            editortab.rolling = (editortab.rolling + 1) % 10
                        }
                    }

                    ToolSlider {
                        id: highlightSlider
                        title: qsTr("Highlight Brightness")
                        minimumValue: 0
                        maximumValue: 1
                        defaultValue: highlightsY
                        valueText: value*1000
                        onValueChanged: {
                            filmProvider.highlightsY = value
                            editortab.rolling = (editortab.rolling + 1) % 10
                        }
                    }

                    ToolSlider {
                        id: whitepointSlider
                        title: qsTr("White Clipping Point")
                        minimumValue: 0.1/1000
                        maximumValue: 5/1000
                        defaultValue: whitepoint
                        valueText: value*1000
                        onValueChanged: {
                            filmProvider.whitepoint = value
                            editortab.rolling = (editortab.rolling + 1) % 10
                        }
                    }
                }
            }
            MouseArea {
                id: wheelstealer
                //This is to prevent scrolling from adjusting sliders.
                anchors.fill: toolList
                acceptedButtons: Qt.NoButton
                onWheel: {
                    if (wheel.angleDelta.y > 0) {
                        //up
                        toolList.flick(0,300);
                    }
                    else {
                        //down
                        toolList.flick(0,-300);
                    }
                }
            }
        }
    }
}
