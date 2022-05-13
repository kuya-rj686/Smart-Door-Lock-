/************************************************************************//**
 * File: FileName
 *
 * Description: Sample File Description
 *
 * Copyright 2018 Knowles Corporation. All rights reserved.
 *
 * All information, including software, contained herein is and remains
 * the property of Knowles Corporation. The intellectual and technical
 * concepts contained herein are proprietary to Knowles Corporation
 * and may be covered by U.S. and foreign patents, patents in process,
 * and/or are protected by trade secret and/or copyright law.
 * This information may only be used in accordance with the applicable
 * Knowles SDK License. Dissemination of this information or distribution
 * of this material is strictly forbidden unless in accordance with the
 * applicable Knowles SDK License.
 *
 *
 * KNOWLES SOURCE CODE IS STRICTLY PROVIDED "AS IS" WITHOUT ANY WARRANTY
 * WHATSOEVER, AND KNOWLES EXPRESSLY DISCLAIMS ALL WARRANTIES,
 * EXPRESS, IMPLIED OR STATUTORY WITH REGARD THERETO, INCLUDING THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE OR NON-INFRINGEMENT OF THIRD PARTY RIGHTS. KNOWLES
 * SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY YOU AS A RESULT OF
 * USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 * IN CERTAIN STATES, THE LAW MAY NOT ALLOW KNOWLES TO DISCLAIM OR EXCLUDE
 * WARRANTIES OR DISCLAIM DAMAGES, SO THE ABOVE DISCLAIMERS MAY NOT APPLY.
 * IN SUCH EVENT, KNOWLES' AGGREGATE LIABILITY SHALL NOT EXCEED
 * FIFTY DOLLARS ($50.00).
 *
 ****************************************************************************/

#include <asf.h>
#include <string.h>
#include "IA61x.h"

#if IA611_VOICE_ID
    #include "hello-voiceqid.h"     /*OEM1*/ //Include User trained Voice ID Keyword file
    #include "hello-voiceqid_ref.h" //Include Voice ID reference keyword model for Hello VoiceQ
#elif IA611_UTK
    #include "keyword.h"         //Include the UTK file here
#else 
    #include "HelloVoiceQ2.h"       /* OEM1 */ //Hello VoiceQ OEM keyword model file
#endif

#include "SwitchTheLight.h"     /* OEM2 */
//#include "keyword.h"     /* OEM2 */
//include "NewCommand.h"     /* OEM2 */
#include "NextSong.h"           /* OEM3 */
#include "BaiDuYiXia.h"         /* OEM4 */


#if IA611_VOICE_ID
    #define STR_EXAMPLE         "Voice ID example"
    #define WAKE_KWD_STRING     "Hello VoiceQ\r\n"
    #define KEYWORD1            "Hello VoiceQ ID:"
#elif IA611_UTK
    #define STR_EXAMPLE         "UTK example"
    #define WAKE_KWD_STRING     "UTK Keyword\r\n"  //Replace this string for UTK identification
    #define KEYWORD1            "UTK Keyword:"
#else
    #define  STR_EXAMPLE        "Wake keyword example"
    #define WAKE_KWD_STRING     "Hello VoiceQ\r\n"
    #define KEYWORD1            "Hello VoiceQ:"
#endif

#define EOL    "\r\n"
#define HEADER_STRING \
    "-- IA611-Xplained Pro "STR_EXAMPLE" --"EOL \
    "-- "BOARD_NAME " --"EOL \
    "-- Compiled: "__DATE__ " "__TIME__ " --"EOL


/** UART module for debug. */
static struct usart_module cdc_uart_module;

/**Initialize EDBG UART port**/
void samd21_vcp_uart_init(void);

/*Infinite loop if any HW API reports error*/
void HW_Error( void );


/***************************************************************************
 * @fn          config_led
 *
 * @brief       Configure LED0, turn it off
 *
 * @param       none
 *
 * @retval      none
 *
 ****************************************************************************/
static void config_led(void)
{
    struct port_config pin_conf;

    port_get_config_defaults(&pin_conf);

    pin_conf.direction = PORT_PIN_DIR_OUTPUT;
    port_pin_set_config(LED_0_PIN, &pin_conf);
    port_pin_set_output_level(LED_0_PIN, LED_0_INACTIVE);
}


/***************************************************************************
 * @fn          blink_led
 *
 * @brief       Toggle LED for number of times
 *
 * @param       count   LED0 Blink count
 *
 * @retval      none
 *
 ****************************************************************************/
static void blink_led(uint32_t count)
{
    while (count--)
    {
        port_pin_toggle_output_level(LED_0_PIN);
        delay_ms(150);
        port_pin_toggle_output_level(LED_0_PIN);
        delay_ms(150);
    }
}


/***************************************************************************
 * @fn          samd21_vcp_uart_init
 *
 * @brief       Configure SAMD21 EDBG UART console.
 *
 * @param       none
 *
 * @retval      none
 *
 ****************************************************************************/
void samd21_vcp_uart_init(void)
{
    struct  usart_config usart_conf;

    usart_get_config_defaults(&usart_conf);
    usart_conf.mux_setting  = EDBG_CDC_SERCOM_MUX_SETTING;
    usart_conf.pinmux_pad0  = EDBG_CDC_SERCOM_PINMUX_PAD0;
    usart_conf.pinmux_pad1  = EDBG_CDC_SERCOM_PINMUX_PAD1;
    usart_conf.pinmux_pad2  = EDBG_CDC_SERCOM_PINMUX_PAD2;
    usart_conf.pinmux_pad3  = EDBG_CDC_SERCOM_PINMUX_PAD3;
    usart_conf.baudrate     = 115200;

    //usart_init(&cdc_uart_module, EDBG_CDC_MODULE, &usart_conf);
    stdio_serial_init(&cdc_uart_module, EDBG_CDC_MODULE, &usart_conf);
    usart_enable(&cdc_uart_module);
}

/***************************************************************************
 * @fn          getAlgorithmParamInfo
 *
 * @brief       Read required Algorithm parameters and print
 *
 * @param       IA61x   IA61x interface handle
 *
 * @retval      none
 *
 ****************************************************************************/
static void getAlgorithmParamInfo(IA61x_instance *IA61x)
{
    uint16_t pResponse;

#if  IA611_VOICE_ID
    IA61x->cmd(GET_ALGO_PARAM,VID_SENSITIVITY_PARAM,1, &pResponse);
    printf("Voice ID Detection Sensitivity:  %2x\r\n", pResponse);
#elif   IA611_UTK
    IA61x->cmd(GET_ALGO_PARAM,UTK_SENSITIVITY_PARAM,1, &pResponse);
    printf("UTK Keyword Detection Sensitivity:  %2x\r\n", pResponse);
#else
    IA61x->cmd(GET_ALGO_PARAM,OEM_SENSITIVITY_PARAM,1, &pResponse);
    printf("OEM Keyword Detection Sensitivity:  %2x\r\n", pResponse);
#endif

    IA61x->cmd(GET_DIGITAL_GAIN,END_POINT_ID,1, &pResponse);
    printf("Digital Gain:  %d db\r\n", pResponse);


}

/***************************************************************************
 * @fn          HW_Error
 *
 * @brief       Wait in loop if any HW error occurs. Reset SAMD21 board to recover.
 *
 * @param       none
 *
 * @retval      none
 *
 ****************************************************************************/
void HW_Error( void )
{
    printf("HW Error: Reset the board to recover\r\n");
    while (1) ;
}

/****************************************************************************************************
 * @fn      VersionStringCmd
 *          Helper routine for reading version string (0x8020/0x8021) from IA61x device
 *
 * @param   IA61x    : IA61x Instance handle
 * @param   pBuffer  : Buffer to store Version String
 * @param   bufferSz : Size of Buffer
 *
 * @return  Returns CMD_SUCCESS on success, CMD_FAILED/CMD_TIMEOUT on failed cases.
 ***************************************************************************************************/
static int32_t VersionStringCmd(IA61x_instance *IA61x, uint8_t *pBuffer, uint8_t bufferSz)
{
    int32_t     result  = 0;
    uint16_t    resp    = 0;
    uint8_t     i       = 0;

    if((pBuffer == NULL) | (bufferSz <= 0))
        return (ERROR);

    /* Clear buffer */
    memset(pBuffer, 0, bufferSz);

    /* Send the build string command to get first character */
    result = IA61x->cmd(BUILD_STRING_CMD1,EMPTY_DATA,1, &resp);
    if (result != CMD_SUCCESS)
    {
        printf("Version CMD Failed!!\r\n");
        return result;
    }

    pBuffer[i++] = (uint8_t)resp;
    bufferSz--;

    /* Get subsequent characters till response is 0 */
    do
    {
        result = IA61x->cmd(BUILD_STRING_CMD2,EMPTY_DATA,1, &resp);
        pBuffer[i++] = (uint8_t)resp;
        bufferSz--;
    } while ( (resp != 0) && (result == CMD_SUCCESS) && (bufferSz > 0) );

    /* Ensure last character in buffer is null terminator (in case buffer was smaller than version string) */
    if (bufferSz == 0)
    {
        i--;
        pBuffer[i] = 0;
    }

    return result;
}

/***************************************************************************
 * @fn          main
 *
 * @brief       Main application function.
 *              Initialize system, UART console, IA61x board,
 *              download the IA61x firmware and wake keywords.
 *              Wait in loop for keyword to be detected by IA61x.
 *
 * @param       none
 *
 * @retval      SUCCESS     Return 0 at exit
 *
 ****************************************************************************/
int main (void)
{
    IA61x_instance *IA61x;
    int32_t retValue;
    uint16_t pResponse;
    uint8_t versionstring[50];

    /* Initialize the board. */
    system_init();

    /*Initialize the system clock tick counter for delay*/
    delay_init();

    /*Configure LED0 on SAMD21 Xplained Pro board*/
    config_led();

    /*Initialize Debug UART port to enable the debug prints*/
    samd21_vcp_uart_init();
    /**printf function uses the Virtual com port of the SAMD21 Xplained pro.
    Debug USB port will be detected as virtual com port on the PC. Baudrate is set to 115200**/
    printf("************************************************\r\n");
    printf(HEADER_STRING);
    printf(SW_VERSION);
    printf(IA61X_HOST_INTERFACE);
    printf("************************************************\r\n");

    /* Insert application code here, after the board has been initialized. */

    /**Initialize SAMD21 USART port and Boot IA61x.
    IA61x auto detects the UART interface.
    Set UART baudrate.  Download the Config file **/
    IA61x = IA61x_init(); /**Returns IA61x interface instance to access API functions**/

    if (IA61x != NULL)
    {
        retValue = IA61x->download_config(); /**Download IA61x Firmvare binary**/
        if (retValue != CMD_SUCCESS)
        {

            printf("Error: Config Download Failed!!!\r\n");
            HW_Error();
        }
        else
        {
            printf("IA61x Config file Downloaded.\r\n");
        }

        retValue = IA61x->download_program(); /**Download IA61x Firmvare binary**/
        if (retValue != SYNC_RESP_NORM)
        {
            printf("Error: Firmware Download Failed!!!\r\n");
            HW_Error();
        }
        else printf("IA61x Firmware Downloaded.\r\n");

        /**Download OEM keywrds. download_keyward function increments the Keyword ID for each keyword binary before downloading**/
        retValue = IA61x->download_keyword((uint16_t *)OEM1, sizeof(OEM1));
        if(retValue)
        {
            printf(KEYWORD1);
            printf("Error: OEM1 Keyword Download failed!! Error Code: %ld\r\n",retValue);
            HW_Error();
        }
        else
        {
            printf(KEYWORD1);
            printf(" OEM1 Keyword Downloaded.\r\n");
        }

#if IA611_VOICE_ID
        /**Download OEM keywrds. download_keyward function increments the Keyword ID for each keyword binary before downloading**/
        retValue = IA61x->download_keyword((uint16_t *)VID_REF, sizeof(VID_REF));
        if(retValue)
        {
            printf("Error: Hello VoiceQ: VID Ref Keyword Download failed!! Error Code: %ld\r\n",retValue);
            HW_Error();
        }
        else
        printf("Hello VoiceQ: VID Ref Keyword Downloaded.\r\n");
#endif

        retValue = IA61x->download_keyword((uint16_t *)OEM2, sizeof(OEM2));
        if(retValue)
        {
            printf("Error: Switch The Light: OEM2 Keyword Download failed!! Error Code: %ld\r\n",retValue);
            HW_Error();
        }
        else
            printf("Switch The Light: OEM2 Keyword Downloaded.\r\n");

        retValue = IA61x->download_keyword((uint16_t *)OEM3, sizeof(OEM3));
        if(retValue)
        {
            printf("Error: Next Song: OEM3 Keyword Download failed!! Error Code: %ld\r\n",retValue);
            HW_Error();
        }
        else
            printf("Next Song: OEM3 Keyword Downloaded.\r\n");

        retValue = IA61x->download_keyword((uint16_t *)OEM4, sizeof(OEM4));
        if(retValue)
        {
            printf("Error: Baidu YiXia: OEM4 Keyword Download failed!! Error Code: %ld\r\n",retValue);
            HW_Error();
        }
        else
            printf("Baidu YiXia: OEM4 Keyword Downloaded.\r\n");

        if(IA61x->VoiceWake())       // Stop --> Set --> Restart the route
            HW_Error();         //if error then jump to HW error loop
        printf("IA61x Route Setup Completed.\r\n");
        printf("************************************************\r\n");

        //Print IA61x Firmware version information
        retValue = VersionStringCmd(IA61x, versionstring, sizeof(versionstring));
        printf("IA61x Firmware Version: %s\r\n",versionstring);

        //print Algorithm Parameters to confirm it's value and put IA61x in low power mode
        getAlgorithmParamInfo(IA61x);
        printf("************************************************\r\n");

        //Put IA61x in low power mode.
        IA61x->cmd(LOW_POWER_MODE_CMD,LOW_POWER_MODE_RT6,0, &pResponse);

        printf("IA61x is ready now...\r\n");
        blink_led(4);


	while (1)
	{
	 retValue = IA61x->wait_keyword(WAIT_KWD_DELAY); /**Wait for keyword**/

	 if (retValue == 1) /*Check if Wake keyword is detected*/
	 {
		 printf(WAKE_KWD_STRING);
		 blink_led(1);

		 while (1)
		 {
			 retValue = IA61x->wait_keyword(WAIT_KWD_DELAY); /**Wait for command**/


			 if (retValue != 0) /*Reset the route when keyword is detected or timeout error occurs*/
			 {
				 IA61x->VoiceWake(); //Reset the route and wait for wake keyword
				 IA61x->cmd(LOW_POWER_MODE_CMD,LOW_POWER_MODE_RT6,0, &pResponse); //Put IA61x in Low power mode
				 if (retValue == CMD_TIMEOUT_ERROR) /*Blink the LED 6 times when timeout occurs*/
				 {
					 blink_led(6);
				 }
				 else
				 {
					 blink_led(retValue);
				 }
				 break;
			 } //if(retValue!=0)
		 } //While (1)
	 } //if (retValue == 1)
	 } //while (1)
    } //if (IA61x)
    else
    {
        HW_Error(); //If failed to create the IA61x interface handle then jump to error loop and wait for HW reset.
    }

    return (SUCCESS);
} //main (void)
