import QtQuick 2.12
import QtLocation 5.12

Rectangle {
    id: mapView
    property alias mapCenter : map.center
    property alias mapScale: map.zoomLevel

    signal markerDoubleclicked(anObject: Item)


    Plugin {
        id: mapPlugin
        name: "mapboxgl"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        activeMapType: supportedMapTypes[6]
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

                    width: 28
                    height:width
                    radius:width/2

                    color:"lightgray"
                    border.color: "transparent"
                    border.width: 3

                    antialiasing: true

                    Image {
                        id: image
                        source: "qrc:/icn/resources/offline.png"
                        anchors.fill: parent
                        antialiasing: true

                        MouseArea{
                            anchors.fill: parent
                            hoverEnabled: true
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

