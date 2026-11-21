#include "cybsp.h"
#include "cy_utils.h"
#include "xmc_gpio.h"
#include "nhd_lib.h"
#include "xmc_ccu4.h"
#include "xmc_dac.h"

#include "buttons.h"

Buttons checkForUpdate(){
    bool printed0 = false;
    bool printed2 = false;
    bool printed4 = false;
     /* Active-low check: 0 means the button is pressed */
    if (XMC_GPIO_GetInput(P5_0) == 0)
    {
        XMC_Delay(20); // Simple 20 ms software debounce
        if (XMC_GPIO_GetInput(P5_0) == 0 && !printed0)
        {
            // Button on P5.0 was pressed!
            printed0 = true;
        } else if(XMC_GPIO_GetInput(P5_0) == 0 && printed0){
            //do nothing
        }
    }
    //if (XMC_GPIO_GetInput(P5_0) == 1) printed0 = false; 

    if (XMC_GPIO_GetInput(P5_2) == 0)
    {
        XMC_Delay(20);
        if (XMC_GPIO_GetInput(P5_2) == 0 && !printed2)
        {
            // Button on P5.2 was pressed!
            //NHD_LCD_PutByte('2');
            printed2 = true;
        } else if(XMC_GPIO_GetInput(P5_0) == 0 && printed2){
            //do nothing
        }
    }
    //if (XMC_GPIO_GetInput(P5_2) == 1) printed2 = false; 

    if (XMC_GPIO_GetInput(P5_4) == 0)
    {
        XMC_Delay(20);
        if (XMC_GPIO_GetInput(P5_4) == 0 && !printed4)
        {
            // Button on P5.4 was pressed!
            printed4 = true;
        } else if(XMC_GPIO_GetInput(P5_0) == 0 && printed4){
            //do nothing
        }
    }
    //if (XMC_GPIO_GetInput(P5_4) == 1) printed4 = false; 
    Buttons temp;
    temp.up_pressed = printed0;
    temp.middle_pressed = printed2;
    temp.down_pressed = printed4;
    return temp;
}