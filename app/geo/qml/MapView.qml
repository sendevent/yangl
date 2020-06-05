import QtQuick 2.0
import QtQuick.Controls 2.0
import QtLocation 5.0

Rectangle {
    id: mapView
    property alias mapCenter : map.center
    property alias mapScale: map.zoomLevel

    signal markerDoubleclicked(Item anObject)

    Plugin {
        id: mapPlugin
        preferred: ["mapboxgl"]
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        activeMapType: supportedMapTypes[ mapPlugin.name === "mapboxgl" ? 6 : 0]
        zoomLevel: 2.5

        MapItemView{
            model: markerModel
            delegate: nvpnMark
        }
    }

    Component {
            id: nvpnMark
            MapQuickItem {
                id: marker
                anchorPoint.x: image.width/2
                anchorPoint.y: image.height/2
                coordinate: position


                sourceItem: Rectangle
                {
                    id: markerRect

                    property string countryName: country
                    property string cityName: city
                    property alias location: marker.coordinate

                    property bool active: countryName === currenCountry && cityName === currenCity;

                    clip: true

                    width: 32
                    height:width
                    radius:width/2

                    color: "transparent"
                    border.color: "transparent"
                    border.width: 0

                    antialiasing: true

                    Image {
                        id: image
                        width: parent.width-2
                        height: parent.height-2
                        smooth: true
                        antialiasing: true
                        source: markerRect.active ? "qrc:/icn/resources/online_map.png":"qrc:/icn/resources/offline_map.png"
                        opacity: 0.75


                        MouseArea{
                            anchors.fill: parent
                            hoverEnabled: true

                            ToolTip.visible: containsMouse
                            ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                            ToolTip.text: composeTooltip(markerRect.active)

                            function composeTooltip(isActive)
                            {
                                var tooltipStr = isActive ? qsTr("Currently connected") : qsTr("Doubleclick to connect")
                                var addrStr = "";

                                if(!markerRect.cityName.length !== 0)
                                {
                                    addrStr += markerRect.cityName
                                }

                                if(!markerRect.countryName.length !== 0)
                                {
                                    if(addrStr.length !== 0)
                                        addrStr += ", "
                                    addrStr += markerRect.countryName;
                                }

                                if(addrStr.length === 0)
                                    return tooltipStr;

                                return addrStr + "\n" + tooltipStr;
                            }

                            onDoubleClicked: {
                                console.log('double-clicked')
                                mapView.markerDoubleclicked(markerRect)
                            }
                        }
                    }
                }
            }
        }
}

