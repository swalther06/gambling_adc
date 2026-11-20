#ifndef LIDAR07_H_
#define LIDAR07_H_

#include <stdint.h>
#include <stdbool.h>
#include "xmc_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Default USIC UART channel for LIDAR07
 */
#ifndef LIDAR07_UART_HW
#define LIDAR07_UART_HW   XMC_UART2_CH0
#endif

/**
 * @brief Measurement modes
 */
typedef enum {
    LIDAR07_MODE_SINGLE     = 0,
    LIDAR07_MODE_CONTINUOUS = 1
} lidar07_mode_t;

/**
 * @brief Measurement result packet from LiDAR
 */
typedef struct {
    uint16_t distance_mm;       /**< Distance in millimeters (0.2m ~ 12m) */
    int16_t  temperature_c_x10; /**< Raw temperature field from sensor */
    uint16_t amplitude;         /**< Signal amplitude (typical 3400 ~ 7000 LSB) */
    uint16_t ambient_light;     /**< Ambient light intensity */
    uint8_t  tof_phase[8];      /**< 8 bytes of raw TOF phase info */
} lidar07_data_t;

/**
 * @brief Error code flags returned by LIDAR07_ReadErrorStatus()
 */
typedef enum {
    LIDAR07_ERR_NONE             = 0x00000000,
    LIDAR07_ERR_SPI_COMM         = 0x00000001,
    LIDAR07_ERR_PIXEL_SATURATION = 0x00000002,
    LIDAR07_ERR_ADC_OVERFLOW     = 0x00000004,
    LIDAR07_ERR_ADC_UNDERFLOW    = 0x00000008,
    LIDAR07_ERR_INTERNAL_MEAS    = 0x00000010,
    LIDAR07_ERR_AMP_TOO_HIGH     = 0x00000020,
    LIDAR07_ERR_AMP_TOO_LOW      = 0x00000040
} lidar07_err_code_t;

/**
 * @brief Status return codes for driver functions
 */
typedef enum {
    LIDAR07_OK       = 0,
    LIDAR07_TIMEOUT  = -1,
    LIDAR07_CRC_ERR  = -2,
    LIDAR07_ERR_RESP = -3,
    LIDAR07_INVALID  = -4
} lidar07_status_t;

/**
 * @brief Initialize the LiDAR UART channel and flush buffers.
 */
void LIDAR07_Init(void);

/**
 * @brief Flushes any leftover incoming bytes from the USIC RX buffer.
 */
void LIDAR07_FlushRx(void);

/**
 * @brief Read firmware version of the LiDAR sensor.
 * @param[out] major Major version number.
 * @param[out] minor Minor version number.
 * @return LIDAR07_OK on success, or negative error code.
 */
lidar07_status_t LIDAR07_ReadVersion(uint16_t *major, uint16_t *minor);

/**
 * @brief Enable or disable built-in median filter.
 * @param enable true to enable median filter (reduces noise to ~10mm),
 *               false to disable (raw response, ~30mm noise).
 * @return LIDAR07_OK on success, or negative error code.
 */
lidar07_status_t LIDAR07_SetFilter(bool enable);

/**
 * @brief Set sensor ranging mode (single shot or continuous).
 * @param mode LIDAR07_MODE_SINGLE or LIDAR07_MODE_CONTINUOUS.
 * @return LIDAR07_OK on success, or negative error code.
 */
lidar07_status_t LIDAR07_SetMode(lidar07_mode_t mode);

/**
 * @brief Set continuous measurement update interval period.
 * @param period_ms Interval in milliseconds (minimum 10ms = 100Hz).
 * @return LIDAR07_OK on success, or negative error code.
 */
lidar07_status_t LIDAR07_SetFrameRate(uint32_t period_ms);

/**
 * @brief Start continuous measurement.
 * @return LIDAR07_OK on success, or negative error code.
 */
lidar07_status_t LIDAR07_StartContinuous(void);

/**
 * @brief Stop continuous measurement.
 * @return LIDAR07_OK on success, or negative error code.
 */
lidar07_status_t LIDAR07_StopContinuous(void);

/**
 * @brief Trigger a single measurement and read the measurement result.
 * @param[out] out_data Pointer to struct where measurement data is stored.
 * @param timeout_ms Maximum time to wait in milliseconds.
 * @return LIDAR07_OK on success, or negative error code.
 */
lidar07_status_t LIDAR07_TriggerSingleMeasurement(lidar07_data_t *out_data, uint32_t timeout_ms);

/**
 * @brief Read an incoming measurement frame (useful in continuous mode).
 * @param[out] out_data Pointer to struct where measurement data is stored.
 * @param timeout_ms Maximum time to wait in milliseconds.
 * @return LIDAR07_OK on success, or negative error code.
 */
lidar07_status_t LIDAR07_ReadMeasurement(lidar07_data_t *out_data, uint32_t timeout_ms);

/**
 * @brief Read sensor internal error status flags.
 * @param[out] err_mask Bitwise OR of lidar07_err_code_t.
 * @return LIDAR07_OK on success, or negative error code.
 */
lidar07_status_t LIDAR07_ReadErrorStatus(uint32_t *err_mask);

#ifdef __cplusplus
}
#endif

#endif /* LIDAR07_H_ */
