
#define SENSOR_INDEX(reg)    (reg & 0b11110000)
#define SENSOR_REGISTER(reg) (reg & 0b1111)

// | Reg  | R/W | typ       | Använding
// | ---  | --- | --------- | ------------------ |
// | 0x20 | R   | u16       | Läs temperatur     |
// | 0x30 | R   | float     | Läs luftfuktighet  |
// | 0x40 | R   | u16       | Läs jordfuktighet  |
// | 0x50 | R   | float     | Läs ljusstyrka lux |
// | 0x51 |   W | byte      | Sätt tröskelvärde  |
// | 0x52 |   W | byte      | Sätt längdtröskel  |

#define SENSOR_TEMPRATURE            0x20
#define SENSOR_HUMIDITY              0x30
#define SENSOR_SOIL                  0x40
#define SENSOR_LIGHT                 0x50
#define SENSOR_LIGHT_LEVEL_THRESHOLD 0x51
#define SENSOR_LIGHT_TIME_THRESHOLD  0x52
#define SENSOR_LIGHT_INTERUPT_ENABLE 0x53
