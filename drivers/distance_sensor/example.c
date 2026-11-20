// the following is an example usage of the library.
// there may (for sure) be more functions and functionality
// in the library than is used here, so best to just ask ai
// what the functions do or see if there are comments or something.

void app_lidar_example(void)
{
    // 1. Initialize driver
    LIDAR07_Init();

    // 2. Enable median filter (reduces noise to ~10mm)
    LIDAR07_SetFilter(true);

    // 3. Optional: Read sensor version
    uint16_t major = 0, minor = 0;
    if (LIDAR07_ReadVersion(&major, &minor) == LIDAR07_OK) {
        // e.g. major = 1, minor = 0
    }

    // 4. Single measurement trigger
    lidar07_data_t data;
    lidar07_status_t status = LIDAR07_TriggerSingleMeasurement(&data, 200); // 200 ms timeout
    // if (status == LIDAR07_OK) {
    //     uint16_t dist_mm = data.distance_mm;
    //     uint16_t amp = data.amplitude;
    //     // Process measurement...
    // }

    //Or continuous mode:
    LIDAR07_SetMode(LIDAR07_MODE_CONTINUOUS);
    LIDAR07_SetFrameRate(100); // 100ms period (10Hz)
    LIDAR07_StartContinuous();
    while (1) {
        if (LIDAR07_ReadMeasurement(&data, 100) == LIDAR07_OK) {
            // New continuous frame received
            NHD_LCD_Clear();
            NHD_LCD_PrintUint16(data.distance_mm);

        }
    }
}