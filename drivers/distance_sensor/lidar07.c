#include "lidar07.h"
#include "cybsp.h"
#include <string.h>

#define LIDAR07_HEADER_HOST      0xF5
#define LIDAR07_HEADER_RESP      0xFA

#define CMD_TYPE_VERSION         0x43
#define CMD_TYPE_FILTER          0x59
#define CMD_TYPE_START_STOP      0x60
#define CMD_TYPE_RANGING_MODE    0x61
#define CMD_TYPE_FRAME_RATE      0x62
#define CMD_TYPE_ERROR_STATUS    0x65

#define CMD_FLAG_WRITE           0x80
#define CMD_FLAG_READ            0x00

/* CRC32 lookup table for poly 0x04C11DB7 */
static const uint32_t s_crc32_table[256] = {
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774B90EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};

static uint32_t calculate_crc32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc = (crc << 8) ^ s_crc32_table[((crc >> 24) ^ data[i]) & 0xFF];
    }
    return crc;
}

static void uart_send_byte(uint8_t byte)
{
    XMC_UART_CH_Transmit(LIDAR07_UART_HW, byte);
    while (XMC_USIC_CH_GetTransmitBufferStatus(LIDAR07_UART_HW) == XMC_USIC_CH_TBUF_STATUS_BUSY) {
        __NOP();
    }
}

static bool uart_recv_byte(uint8_t *byte, uint32_t timeout_us)
{
    uint32_t elapsed = 0;
    while ((XMC_UART_CH_GetStatusFlag(LIDAR07_UART_HW) & 
           (XMC_UART_CH_STATUS_FLAG_RECEIVE_INDICATION | XMC_UART_CH_STATUS_FLAG_ALTERNATIVE_RECEIVE_INDICATION)) == 0) {
        XMC_DelayUs(10);
        elapsed += 10;
        if (elapsed >= timeout_us) {
            return false;
        }
    }
    *byte = (uint8_t)XMC_UART_CH_GetReceivedData(LIDAR07_UART_HW);
    XMC_UART_CH_ClearStatusFlag(LIDAR07_UART_HW, 
        XMC_UART_CH_STATUS_FLAG_RECEIVE_INDICATION | XMC_UART_CH_STATUS_FLAG_ALTERNATIVE_RECEIVE_INDICATION);
    return true;
}

void LIDAR07_FlushRx(void)
{
    while (XMC_UART_CH_GetStatusFlag(LIDAR07_UART_HW) & 
          (XMC_UART_CH_STATUS_FLAG_RECEIVE_INDICATION | XMC_UART_CH_STATUS_FLAG_ALTERNATIVE_RECEIVE_INDICATION)) {
        (void)XMC_UART_CH_GetReceivedData(LIDAR07_UART_HW);
        XMC_UART_CH_ClearStatusFlag(LIDAR07_UART_HW, 
            XMC_UART_CH_STATUS_FLAG_RECEIVE_INDICATION | XMC_UART_CH_STATUS_FLAG_ALTERNATIVE_RECEIVE_INDICATION);
    }
}

void LIDAR07_Init(void)
{
    /* Ensure P5.1 is configured as floating input, not push-pull output */
    XMC_GPIO_SetMode(P5_1, XMC_GPIO_MODE_INPUT_TRISTATE);

    /* Explicitly route DX0 input to DX0A (P5.1) for USIC2 CH0 */
    XMC_UART_CH_SetInputSource(LIDAR07_UART_HW, (XMC_UART_CH_INPUT_t)XMC_USIC_CH_INPUT_DX0, 0U);

    /* In case usic_2_ch_0 was not started */
    XMC_UART_CH_Start(LIDAR07_UART_HW);
    LIDAR07_FlushRx();
}

static void send_command_packet(uint8_t cmd_byte, uint32_t data_val)
{
    uint8_t pkt[6];
    pkt[0] = LIDAR07_HEADER_HOST;
    pkt[1] = cmd_byte;
    pkt[2] = (uint8_t)(data_val & 0xFF);
    pkt[3] = (uint8_t)((data_val >> 8) & 0xFF);
    pkt[4] = (uint8_t)((data_val >> 16) & 0xFF);
    pkt[5] = (uint8_t)((data_val >> 24) & 0xFF);

    uint32_t crc = calculate_crc32(pkt, 6);

    for (int i = 0; i < 6; i++) {
        uart_send_byte(pkt[i]);
    }
    /* Send CRC LSB-first */
    uart_send_byte((uint8_t)(crc & 0xFF));
    uart_send_byte((uint8_t)((crc >> 8) & 0xFF));
    uart_send_byte((uint8_t)((crc >> 16) & 0xFF));
    uart_send_byte((uint8_t)((crc >> 24) & 0xFF));
}

static lidar07_status_t receive_response(uint8_t expected_cmd, uint8_t *data_buf, uint16_t expected_len, uint32_t timeout_ms)
{
    uint32_t timeout_us = timeout_ms * 1000;
    uint32_t elapsed_us = 0;
    uint8_t b = 0;

    /* Step 1: Hunt for header 0xFA */
    while (1) {
        if (!uart_recv_byte(&b, 1000)) {
            elapsed_us += 1000;
            if (elapsed_us >= timeout_us) {
                return LIDAR07_TIMEOUT;
            }
            continue;
        }
        if (b == LIDAR07_HEADER_RESP) {
            break;
        }
    }

    uint8_t header_buf[4];
    header_buf[0] = LIDAR07_HEADER_RESP;

    /* Step 2: Read CMD (1 byte) and Data length N (2 bytes, LSB then MSB) */
    for (int i = 1; i < 4; i++) {
        if (!uart_recv_byte(&header_buf[i], 50000)) {
            return LIDAR07_TIMEOUT;
        }
    }

    uint8_t rx_cmd = header_buf[1];
    uint16_t data_len = (uint16_t)header_buf[2] | ((uint16_t)header_buf[3] << 8);

    if (expected_cmd != 0 && rx_cmd != expected_cmd) {
        return LIDAR07_ERR_RESP;
    }

    /* Step 3: Read data payload */
    if (data_len > 128) {
        return LIDAR07_INVALID;
    }

    uint8_t payload[128];
    for (uint16_t i = 0; i < data_len; i++) {
        if (!uart_recv_byte(&payload[i], 50000)) {
            return LIDAR07_TIMEOUT;
        }
    }

    /* Step 4: Read 4 CRC bytes (LSB first) */
    uint8_t crc_bytes[4];
    for (int i = 0; i < 4; i++) {
        if (!uart_recv_byte(&crc_bytes[i], 50000)) {
            return LIDAR07_TIMEOUT;
        }
    }

    uint32_t rx_crc = (uint32_t)crc_bytes[0] |
                      ((uint32_t)crc_bytes[1] << 8) |
                      ((uint32_t)crc_bytes[2] << 16) |
                      ((uint32_t)crc_bytes[3] << 24);

    /* Verify CRC over Header (0xFA), CMD, DataLen (2 bytes), and Payload */
    uint32_t calc_crc = 0xFFFFFFFF;
    for (int i = 0; i < 4; i++) {
        calc_crc = (calc_crc << 8) ^ s_crc32_table[((calc_crc >> 24) ^ header_buf[i]) & 0xFF];
    }
    for (uint16_t i = 0; i < data_len; i++) {
        calc_crc = (calc_crc << 8) ^ s_crc32_table[((calc_crc >> 24) ^ payload[i]) & 0xFF];
    }

    if (calc_crc != rx_crc) {
        return LIDAR07_CRC_ERR;
    }

    if (data_buf != NULL && expected_len > 0) {
        uint16_t copy_len = (data_len < expected_len) ? data_len : expected_len;
        memcpy(data_buf, payload, copy_len);
    }

    return LIDAR07_OK;
}

lidar07_status_t LIDAR07_ReadVersion(uint16_t *major, uint16_t *minor)
{
    LIDAR07_FlushRx();
    send_command_packet(CMD_TYPE_VERSION | CMD_FLAG_READ, 0);

    uint8_t resp[4];
    lidar07_status_t res = receive_response(CMD_TYPE_VERSION | CMD_FLAG_READ, resp, sizeof(resp), 300);
    if (res == LIDAR07_OK) {
        if (minor) {
            *minor = (uint16_t)resp[0] | ((uint16_t)resp[1] << 8);
        }
        if (major) {
            *major = (uint16_t)resp[2] | ((uint16_t)resp[3] << 8);
        }
    }
    return res;
}

lidar07_status_t LIDAR07_SetFilter(bool enable)
{
    LIDAR07_FlushRx();
    send_command_packet(CMD_TYPE_FILTER | CMD_FLAG_WRITE, enable ? 1 : 0);

    uint8_t resp[4];
    return receive_response(CMD_TYPE_FILTER | CMD_FLAG_WRITE, resp, sizeof(resp), 300);
}

lidar07_status_t LIDAR07_SetMode(lidar07_mode_t mode)
{
    LIDAR07_FlushRx();
    send_command_packet(CMD_TYPE_RANGING_MODE | CMD_FLAG_WRITE, (uint32_t)mode);

    uint8_t resp[4];
    return receive_response(CMD_TYPE_RANGING_MODE | CMD_FLAG_WRITE, resp, sizeof(resp), 300);
}

lidar07_status_t LIDAR07_SetFrameRate(uint32_t period_ms)
{
    if (period_ms < 10) {
        period_ms = 10;
    }
    LIDAR07_FlushRx();
    send_command_packet(CMD_TYPE_FRAME_RATE | CMD_FLAG_WRITE, period_ms);

    uint8_t resp[4];
    return receive_response(CMD_TYPE_FRAME_RATE | CMD_FLAG_WRITE, resp, sizeof(resp), 300);
}

lidar07_status_t LIDAR07_StartContinuous(void)
{
    LIDAR07_FlushRx();
    send_command_packet(CMD_TYPE_START_STOP | CMD_FLAG_WRITE, 1);
    /* In continuous mode, the module might immediately begin emitting measurement frames */
    return LIDAR07_OK;
}

lidar07_status_t LIDAR07_StopContinuous(void)
{
    LIDAR07_FlushRx();
    send_command_packet(CMD_TYPE_START_STOP | CMD_FLAG_WRITE, 0);
    XMC_Delay(20);
    LIDAR07_FlushRx();
    return LIDAR07_OK;
}

lidar07_status_t LIDAR07_TriggerSingleMeasurement(lidar07_data_t *out_data, uint32_t timeout_ms)
{
    LIDAR07_FlushRx();
    send_command_packet(CMD_TYPE_START_STOP | CMD_FLAG_WRITE, 1);
    return LIDAR07_ReadMeasurement(out_data, timeout_ms);
}

lidar07_status_t LIDAR07_ReadMeasurement(lidar07_data_t *out_data, uint32_t timeout_ms)
{
    if (!out_data) {
        return LIDAR07_INVALID;
    }

    uint8_t resp[16];
    lidar07_status_t res = receive_response(CMD_TYPE_START_STOP | CMD_FLAG_WRITE, resp, sizeof(resp), timeout_ms);
    if (res == LIDAR07_OK) {
        out_data->distance_mm       = (uint16_t)resp[0] | ((uint16_t)resp[1] << 8);
        out_data->temperature_c_x10 = (int16_t)((uint16_t)resp[2] | ((uint16_t)resp[3] << 8));
        out_data->amplitude         = (uint16_t)resp[4] | ((uint16_t)resp[5] << 8);
        out_data->ambient_light     = (uint16_t)resp[6] | ((uint16_t)resp[7] << 8);
        memcpy(out_data->tof_phase, &resp[8], 8);
    }
    return res;
}

lidar07_status_t LIDAR07_ReadErrorStatus(uint32_t *err_mask)
{
    LIDAR07_FlushRx();
    send_command_packet(CMD_TYPE_ERROR_STATUS | CMD_FLAG_READ, 0);

    uint8_t resp[4];
    lidar07_status_t res = receive_response(CMD_TYPE_ERROR_STATUS | CMD_FLAG_READ, resp, sizeof(resp), 300);
    if (res == LIDAR07_OK && err_mask) {
        *err_mask = (uint32_t)resp[0] |
                    ((uint32_t)resp[1] << 8) |
                    ((uint32_t)resp[2] << 16) |
                    ((uint32_t)resp[3] << 24);
    }
    return res;
}
