#ifndef SENSOR_NODE_H
#define SENSOR_NODE_H

#include <stdint.h>

/* keep these in sync with the sensor node firmware or things will break */
#define SN_START_BYTE    0xAA
#define SN_CMD_FETCH     0x01
#define SN_PAYLOAD_LEN   sizeof(int32_t)       /* one int32_t: temp in milli-C */
#define SN_REPLY_LEN     (3 + SN_PAYLOAD_LEN + 1) /* [start][cmd][len][...][checksum] */
#define SN_REQUEST_LEN   3                     /* [start][cmd][checksum] */

#endif /* SENSOR_NODE_H */
