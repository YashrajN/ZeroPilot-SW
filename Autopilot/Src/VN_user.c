/***************** (C) COPYRIGHT 2009 VectorNav Technologies *******************
* File Name          : VN_user.c
* Author             : John Brashear
* Version            : V1.0.0
* Date               : 09/26/2009
* Description        : This file contains all the functions that are hardware 
*                    : specific. These functions need to be modified by the 
*                    : user to be compatible with their hardware architecture.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING
* CUSTOMERS WITH EXAMPLE CODE IN ORDER TO SAVE THEM TIME. AS A RESULT,
* VECTORNAV SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR 
* CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE 
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE 
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "VN_user.h"
#include "VN_lib.h"
#include "stm32f7xx_hal.h"
#include "spi.h"
#include "debug.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
* Function Name  : VN_SPI_SetSS(unsigned char sensorID, bool LineState)
* Description    : This is a generic function that will set the SPI slave select
*                  line for the given sensor. This function needs to be added by
*                  the user with the logic specific to their hardware to perform
*                  the necessary actions to either raise or lower the slave
*                  select line for the given sensor.  If a multiplexer is used
*                  then the logic/communications neccessary to perform the
*                  actions should be placed here.                                        
* Input          : sensorID  -> The sensor to set the slave select line for.
*                : state -   -> The state to set the slave select to.
* Output         : None
* Return         : None
*******************************************************************************/
void VN_SPI_SetSS(unsigned char sensorID, VN_PinState state){
  if(state==VN_PIN_HIGH){
      HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);
  }else{
  HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET);
  }
}

/*******************************************************************************
* Function Name  : VN_SPI_SendReceiveWord(unsigned long data)
* Description    : Transmits the given 32-bit word on the SPI bus. The user needs
*                  to place their hardware specific logic here to send 4 bytes
*                  out the SPI bus. The slave select line is controlled by the 
*                  function VN_SPI_SetSS given above, so the user only needs
*                  to deal with sending the data out the SPI bus with this
*                  function.
* Input          : data -> The 32-bit data to send over the SPI bus
* Output         : None
* Return         : The data received on the SPI bus
*******************************************************************************/
unsigned long VN_SPI_SendReceive(unsigned long dataTX){

/* User code to send out 4 bytes over SPI goes here */
unsigned long dataRX;
debug("data: %d", dataTX);
  HAL_SPI_TransmitReceive(&hspi2, (uint8_t *) &dataTX, (uint8_t *) &dataRX,4, 100);
  
  return dataRX;
}

/*******************************************************************************
* Function Name  : VN_Delay(unsigned long delay_uS)
* Description    : Delay the processor for deltaT time in microseconds.  The user
*                  needs to place the hardware specific code here necessary to 
*                  delay the processor for the time span given by delay_uS
*                  measured in micro seconds. This function doesn't need to be
*                  ultra precise. The only requirement on this function is that
*                  the processor is delayed a time NO LESS THAN 90% of the time 
*                  given by the variable delay_uS in microseconds. The minimum
*                  timespan that is used by the VectorNav library code is 50uS so
*                  the function call shouldn't affect the timing accuracy much.
*                  If you decide to modify this library or wish to have more
*                  precision on this delay function then you can comment out this
*                  function and replace it with an optimized macro instead. Many
*                  compilers have their own delay routines or macros so make sure
*                  you check your compiler documentation before attempting to
*                  write your own.
* Input          : delay_uS -> Time to delay the processor in microseconds
* Output         : None
* Return         : None
*******************************************************************************/
void VN_Delay(unsigned long delay_uS){

/* User code to delay the processor goes here. Below is example code that
   works for a 32-bit ARM7 Cortex processor clocked at 72 MHz.  For any 
   other processor you will need to replace this with code that works
   for your processor.  Many compilers will have their own delay routines
   so make sure you check your compiler documentation before attempting to
   write your own. */
  unsigned long i;
  for(i=delay_uS*33; i--; );
}

/******************* (C) COPYRIGHT 2009 VectorNav Technologies *****************
***********************************END OF FILE*********************************/