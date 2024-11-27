#include "fsl_debug_console.h"
#include "servo.h"
#include "esc.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_pwm.h"
#include "fsl_clock.h"
#include "hbridge.h"

#include <stdio.h>
#include <string.h>
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_cmsis.h"
#include "pixy.h"



#define EXAMPLE_I2C_MASTER Driver_I2C2
#define LPI2C_CLOCK_FREQUENCY CLOCK_GetLPFlexCommClkFreq(2u)
#define PIXY2_I2C_ADDRESS (0x54U)

#define MOTOR1_FORWARD_GPIO  GPIO5
#define MOTOR1_FORWARD_PIN   8U


#define MOTOR1_BACKWARD_GPIO GPIO5
#define MOTOR1_BACKWARD_PIN  9U

#define MOTOR2_FORWARD_GPIO  GPIO1
#define MOTOR2_FORWARD_PIN   13U

#define MOTOR2_BACKWARD_GPIO GPIO2
#define MOTOR2_BACKWARD_PIN  0U

/* PWM Module and Channels */
#define PWM1_SUBMODULE_MOTOR1       kPWM_Module_1  // Submodule for Motor1
#define PWM1_CHANNEL_MOTOR1         kPWM_PwmA
#define PWM1_SUBMODULE_MOTOR2       kPWM_Module_1  // Submodule for Motor2
#define PWM1_CHANNEL_MOTOR2         kPWM_PwmB

/* PWM Frequency and Clock */
#define PWM_FREQ_HZ          1000U
#define SRC_CLOCK_HZ         CLOCK_GetFreq(kCLOCK_CoreSysClk)  // Use system clock

#define BOARD_PWM_BASEADDR        PWM1
#define PWM_SRC_CLK_FREQ          CLOCK_GetFreq(kCLOCK_BusClk)
#define DEMO_PWM_CLOCK_DEVIDER    kPWM_Prescale_Divide_128


volatile bool g_MasterCompletionFlag = false;


uint32_t LPI2C2_GetFreq(void)
{
    return LPI2C_CLOCK_FREQUENCY;
}

static void lpi2c_master_callback(uint32_t event)
{
    switch (event)
    {
        case ARM_I2C_EVENT_TRANSFER_DONE:
            g_MasterCompletionFlag = true;
            break;

        case ARM_I2C_EVENT_ARBITRATION_LOST:
            g_MasterCompletionFlag = true;
            break;

        default:
            break;
    }
}


int main(void)
{

    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_Gpio1);
    CLOCK_EnableClock(kCLOCK_Gpio2);


	gpio_pin_config_t motorPinConfig = {
		kGPIO_DigitalOutput, 0
	};


	 /* Initialize the H-Bridge */
	   Hbridge hbridge;
	   HbridgeInit(&hbridge,
	                PWM1,
	                PWM1_SUBMODULE_MOTOR1,
					PWM1_CHANNEL_MOTOR1,
	                PWM1_SUBMODULE_MOTOR2,
					PWM1_CHANNEL_MOTOR2,
	                MOTOR1_FORWARD_GPIO, MOTOR1_FORWARD_PIN,
	                MOTOR1_BACKWARD_GPIO, MOTOR1_BACKWARD_PIN,
	                MOTOR2_FORWARD_GPIO, MOTOR2_FORWARD_PIN,
	                MOTOR2_BACKWARD_GPIO, MOTOR2_BACKWARD_PIN,
	                PWM_FREQ_HZ, SRC_CLOCK_HZ,
	                kPWM_Prescale_Divide_64,
	                kPWM_Prescale_Divide_64);



    PRINTF("Pixy2 LED control example.\r\n");

    Pixy2Init(PIXY2_I2C_ADDRESS, 0);
    PixySetLed(0, 100, 0);

    PRINTF("\nLED color set to: Red=0, Green=100, Blue=0\n");
    PRINTF("\nEnd of Pixy2 LED test.\n");
    PRINTF("\nPixy vectors test:\n\n");

    for (int i = 1; i <= 100; i++)
    {
    	DetectedVectors PixyVectors;
    	PixyVectors = PixyGetVectors();
    	for(int i = 0;i < PixyVectors.NumberOfVectors;i++){
    		PRINTF("X0: %d | Y0: %d| X1: %d | Y1: %d\n",PixyVectors.Vectors[i].x0,
    				PixyVectors.Vectors[i].y0, PixyVectors.Vectors[i].x1,PixyVectors.Vectors[i].y1);
    	}
    	PRINTF("\n---------------\n");
    }


		PRINTF("\nSteer Test:\n");

    // Initialize PWM for SERVO
		InitNewPWM(PWM1, kPWM_Module_0, kPWM_PwmA, kPWM_Prescale_Divide_64, 50, SRC_CLOCK_HZ);

	// Initialize PWM for ESC
		InitNewPWM(PWM1, kPWM_Module_0, kPWM_PwmB, kPWM_Prescale_Divide_64, 50, SRC_CLOCK_HZ); //init esc

       Steer(20, 0);
       delay_ms(3000);
       for (int i = 1; i <= 1000; i++)
       {
           PRINTF("Ignition%d\n",i);
           HbridgeSetSpeed(&hbridge, 20, 20);
    	   //SetESCSpeed(20);
           Steer(0, 0);
           delay_ms(5000);
           PRINTF("Ignition2\n");
           HbridgeSetSpeed(&hbridge, -20, -20);
    	 //  SetESCSpeed(-20);

           Steer(50, 0);
           delay_ms(5000);
       }
}
