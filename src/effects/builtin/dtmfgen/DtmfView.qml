import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Muse.UiComponents
import Audacity.Effects
import Audacity.Playback
import "../common"

EffectBase {
    property string title: qsTrc("effects/dtmf", "DTMF Tones")
    property alias instanceId: dtmf.instanceId
    property bool isApplyAllowed: true

    width: 300
    height: 350

    function preview() {
        dtmf.preview()
    }

    DtmfViewModel {
        id: dtmf
    }

    Component.onCompleted: {
        dtmf.init()
        timecode.currentFormatStr = dtmf.durationFormat
    }

    GridLayout {
        columns: 2
        rows: 3

        columnSpacing: 10
        rowSpacing: 16

        anchors.fill: parent
        anchors.margins: 10

        StyledTextLabel {
            text: qsTrc("effects/dtmf", "DTMF sequence:")
        }

        TextInputField {
            Layout.fillWidth: true

            currentText: dtmf.sequence

            validator: RegularExpressionValidator {
                regularExpression: /[0-9A-Da-z\*\#]*/
            }

            onTextCleared: {
                dtmf.sequence = ""
            }

            onTextEdited: function (newTextValue) {
                dtmf.sequence = newTextValue
            }
        }

        StyledTextLabel {
            text: qsTrc("effects/dtmf", "Amplitude (0-1):")
        }

        TextInputField {
            Layout.fillWidth: true

            currentText: dtmf.amplitude

            validator: DoubleInputValidator {
                decimal: 6
            }

            onTextEdited: function (newTextValue) {
                dtmf.amplitude = parseFloat(newTextValue)
            }
        }

        StyledTextLabel {
            text: qsTrc("effects/dtmf", "Duration:")
        }

        Timecode {
            id: timecode

            Layout.fillWidth: true
            Layout.fillHeight: false

            value: dtmf.duration
            currentFormatStr: dtmf.durationFormat
            sampleRate: dtmf.sampleRate

            onValueChanged: {
                dtmf.duration = timecode.value
            }

            onCurrentFormatChanged: {
                dtmf.durationFormat = timecode.currentFormatStr
            }
        }

        StyledTextLabel {
            text: qsTrc("effects/dtmf", "Tone/silence ratio:")
        }

        StyledSlider {
            from: 0
            to: 100
            stepSize: 0.1
            value: dtmf.dutyCycle
            onMoved: dtmf.dutyCycle = value
        }

        StyledTextLabel {
            text: qsTrc("effects/dtmf", "Duty cycle:")
        }
        StyledTextLabel {
            text: qsTrc("effects/dtmf", "%1%").arg(dtmf.dutyCycle.toFixed(1))
        }

        StyledTextLabel {
            text: qsTrc("effects/dtmf", "Tone duration:")
        }
        StyledTextLabel {
            text: qsTrc("effects/dtmf", "%1 ms").arg(Math.round(dtmf.toneDuration * 1000))
        }

        StyledTextLabel {
            text: qsTrc("effects/dtmf", "Silence duration:")
        }
        StyledTextLabel {
            text: qsTrc("effects/dtmf", "%1 ms").arg(Math.round(dtmf.silenceDuration * 1000))
        }
    }
}

