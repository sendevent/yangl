import QtQuick 2.12
import QtLocation 5.12

Rectangle {
    id: mapView
    property alias mapCenter : map.center

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
            delegate: mapcomponent
        }
    }

    Component {
            id: mapcomponent
            MapQuickItem {
                id: marker
                anchorPoint.x: image.width/2
                anchorPoint.y: image.height/2
                coordinate: position


                sourceItem: Rectangle
                {
                    id: markerRect

                    width: 28
                    height:width
                    radius:width/2

                    color:"transparent"
                    border.color: "transparent"
                    border.width: 3

                    antialiasing: true

                    Image {
                        id: image
                        source: "qrc:/icn/resources/offline.png"
                        anchors.fill: parent
                        antialiasing: true
                    }
                }
            }
        }
}

