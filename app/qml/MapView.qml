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

                    property bool active: countryName === currenCountry && cityName === currenCity;

                    clip: true

                    width: 32
                    height:width
                    radius:width/2

                    color: /*active ? "blue" :*/ "transparent"
                    border.color: /*active ? "blue" :*/ "transparent"
                    border.width: 3

                    antialiasing: true

                    function isActive()
                    {
                        return co
                    }

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

