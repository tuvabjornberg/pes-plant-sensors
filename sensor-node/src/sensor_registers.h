
#define SENSOR_INDEX(reg)    (reg & 0b11110000)
#define SENSOR_REGISTER(reg) (reg & 0b1111)
// | Reg  | R/W | Använding                 |
// | ---- | --- | ------------------------- |
// | 0x20 | R   | Läs temperatur            |
// | 0x30 | R   | Läs luftfuktighet         |
// | 0x31 | R   | Läs luftfuktighetstemp    |
// | 0x40 | R   | Läs jordfuktighet         |
// | 0x50 | R   | Läs ljusstyrka total      |
// | 0x51 | R   | Läs ljusstyrka endast IR  |
// | 0x52 |   W | Sätt tröskelvärde         |
// | 0x53 |   W | Sätt längdtröskel         |

#define SENSOR_TEMPRATURE            0x20
#define SENSOR_HUMIDITY              0x30
#define SENSOR_SOIL                  0x40
#define SENSOR_LIGHT_TOTAL           0x50
#define SENSOR_LIGHT_LEVEL_THRESHOLD 0x52
#define SENSOR_LIGHT_TIME_THRESHOLD  0x53
