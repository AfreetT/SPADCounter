/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fonts.h"
#include "ssd1306.h"
#include "math.h"
#include "ctype.h"
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
const uint16_t data_pins[8] = {
    GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15,  // PB12-PB15
    GPIO_PIN_10, GPIO_PIN_15, GPIO_PIN_3, GPIO_PIN_4     // PB10, PA15, PB3, PB4
};
GPIO_TypeDef* data_ports[8] = {
    GPIOB, GPIOB, GPIOB, GPIOB,
    GPIOB, GPIOA, GPIOB, GPIOB
};

uint16_t MAX_COUNT_RATE = 1000; // Per 1 ms
volatile uint32_t total_counts = 0;
volatile uint8_t update_display = 0;
uint8_t power_pin_state = 0;
uint32_t display_counts = 0;
const char serialNumber[] = "0130";

volatile uint8_t overflow_count = 0; // How many times overflow was reached
volatile uint8_t overflow = 0; // Overflow flag
#define MAX_OVERFLOW_COUNT 10 // Trigger overflow if this value is reached in 100 ms
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void convertToScientific(uint32_t num, char* buffer);
void display_on_lcd(void);
void trigger_overflow(void);
void processCommand(void);
uint8_t read_counter_value(void);
void SSD1306_Clear(void);
void check_power_button(void);
void update_counts(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(GPIOB, DIODE_POWER_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GAL_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GAU_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, CCLR_Pin, GPIO_PIN_SET);

	if (ssd1306_Init(&hi2c1) != 0) { Error_Handler();}

	ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("SPAD PULSE", Font_11x18, White);
	ssd1306_SetCursor(8, 36);
	ssd1306_WriteString("COUNTER", Font_11x18, White);
	ssd1306_UpdateScreen(&hi2c1);
	HAL_Delay(1000);

	// Start timers
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	if(overflow){trigger_overflow();}

	if(update_display) {
		__disable_irq();
		display_counts = total_counts;
		total_counts = 0;
		overflow_count = 0;
		__enable_irq();
		display_on_lcd();
		update_display = 0;
	}

	check_power_button();
	processCommand();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 72-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 1000-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7200-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DIODE_POWER_Pin|GAU_Pin|CCLR_Pin|RCLK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GAL_GPIO_Port, GAL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : POWER_BUTTON_PIN_Pin */
  GPIO_InitStruct.Pin = POWER_BUTTON_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(POWER_BUTTON_PIN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DIODE_POWER_Pin GAU_Pin CCLR_Pin RCLK_Pin */
  GPIO_InitStruct.Pin = DIODE_POWER_Pin|GAU_Pin|CCLR_Pin|RCLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Y4_Pin Y0_Pin Y1_Pin Y2_Pin
                           Y3_Pin Y6_Pin Y7_Pin */
  GPIO_InitStruct.Pin = Y4_Pin|Y0_Pin|Y1_Pin|Y2_Pin
                          |Y3_Pin|Y6_Pin|Y7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : GAL_Pin */
  GPIO_InitStruct.Pin = GAL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GAL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Y5_Pin */
  GPIO_InitStruct.Pin = Y5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Y5_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void update_counts(){
	// Move register
	HAL_GPIO_WritePin(RCLK_GPIO_Port, RCLK_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RCLK_GPIO_Port, RCLK_Pin, GPIO_PIN_RESET);

	// Read lower byte
	HAL_GPIO_WritePin(GAL_GPIO_Port, GAL_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GAU_GPIO_Port, GAU_Pin, GPIO_PIN_SET);
	uint8_t lower = read_counter_value();

	// Read upper byte
	HAL_GPIO_WritePin(GAL_GPIO_Port, GAL_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GAU_GPIO_Port, GAU_Pin, GPIO_PIN_RESET);
	uint8_t upper = read_counter_value();

	// Reset the register
	HAL_GPIO_WritePin(CCLR_GPIO_Port, CCLR_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CCLR_GPIO_Port, CCLR_Pin, GPIO_PIN_SET);

	uint16_t full = (upper << 8) | lower;

	if(full > MAX_COUNT_RATE) {overflow_count += 1;}

	if(overflow_count > MAX_OVERFLOW_COUNT) {
	  HAL_GPIO_WritePin(DIODE_POWER_GPIO_Port, DIODE_POWER_Pin, GPIO_PIN_RESET);
	  power_pin_state = 0;
	  overflow = 1;
	  HAL_TIM_Base_Stop_IT(&htim1);
	  HAL_TIM_Base_Stop_IT(&htim2);
	}
	total_counts += full;
}

/* Timer Interrupt Handlers */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	// 1ms update
  if (htim->Instance == TIM1) {update_counts();}

  // 100ms display update
  else if (htim->Instance == TIM2) { update_display = 1;}

}

/* Helper Functions */
uint8_t read_counter_value() {
  uint8_t val = 0;
  for(int i=0; i<8; i++) {
    if(HAL_GPIO_ReadPin(data_ports[i], data_pins[i]))
      val |= (1 << i);
  }
  return val;
}

void convertToScientific(unsigned long num, char* buffer) {
  if (num == 0) {
    strcpy(buffer, "0.00e0");
    return;
  }

  int exponent = 0;
  unsigned long temp = num;
  while (temp >= 10) {
    temp /= 10;
    exponent++;
  }

  unsigned long power_of_10 = 1;
  for (int i = 0; i < exponent; i++) {
    power_of_10 *= 10;
  }

  float mantissa = (float)num / (float)power_of_10;
  mantissa = roundf(mantissa * 100.0) / 100.0; // Round to two decimals

  // Check if rounding caused overflow to 10.0
  if (mantissa >= 10.0) {
    mantissa /= 10.0;
    exponent++;
  }

  // Split mantissa into integer and decimal parts
  int mantissaInt = (int)(mantissa * 100 + 0.5); // Ensure correct rounding
  snprintf(buffer, 20, "%d.%02de%d", mantissaInt / 100, mantissaInt % 100, exponent);
}

void display_on_lcd() {

	ssd1306_Fill(Black);

	if(power_pin_state){
		char buf[32];
		display_counts *= 10; // Convert from counts per 100 ms to cps
		convertToScientific(display_counts, buf);
		ssd1306_SetCursor(16,0);
		ssd1306_WriteString(buf, Font_16x26, White);
		ssd1306_SetCursor(40,32);
		ssd1306_WriteString("cps", Font_16x26, White);

	}
	else{
		ssd1306_SetCursor(24,0);
		ssd1306_WriteString("Power", Font_16x26, White);
		ssd1306_SetCursor(40,32);
		ssd1306_WriteString("off", Font_16x26, White);
	}

	ssd1306_UpdateScreen(&hi2c1);
}

void trigger_overflow() {
	total_counts = 0;
	update_display = 0;
	overflow_count = 0;

	static uint8_t flicker = 0;
	static uint8_t middle = SSD1306_HEIGHT / 4;

	ssd1306_Fill(Black);
	if(flicker){flicker = 0;}
	else{
	ssd1306_SetCursor(0, middle);
	ssd1306_WriteString("OVRFLW!", Font_16x26, White);
	flicker = 1;
	}
	ssd1306_UpdateScreen(&hi2c1);
	HAL_Delay(200);

}

void toggle_power() {
  power_pin_state = !power_pin_state;
  HAL_GPIO_WritePin(DIODE_POWER_GPIO_Port, DIODE_POWER_Pin,
                    power_pin_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
  if(overflow) {
    overflow = 0;
    HAL_TIM_Base_Start_IT(&htim1);
    HAL_TIM_Base_Start_IT(&htim2);
  }
}
// Helper function to trim whitespace
char* trim_whitespace(char *str) {
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;

    char *end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

void processCommand(void) {
    if(command_ready) {
        char *cmd = trim_whitespace((char*)cdc_rx_buffer);
        char response[128];

        if(strcmp(cmd, "IDN?") == 0) {
            snprintf(response, sizeof(response), "SPAD Pulse Counter v1.0\r\n");
        }
        else if(strcmp(cmd, "SERIAL?") == 0) {
            snprintf(response, sizeof(response), "%s\r\n", serialNumber);
        }
        else if(strcmp(cmd, "COUNTS?") == 0) {
            snprintf(response, sizeof(response), "%lu\r\n", display_counts);
        }
        else if(strcmp(cmd, "TOGGLE_POWER") == 0) {
            toggle_power();
            snprintf(response, sizeof(response), "OK\r\n");
        }
        else if(strcmp(cmd, "POWER?") == 0) {
            snprintf(response, sizeof(response), "%d\r\n", power_pin_state);
        }
        else if(strcmp(cmd, "MAX_COUNT?") == 0) {
        	snprintf(response, sizeof(response), "%d\r\n", MAX_COUNT_RATE);
        }
        else if(strncmp(cmd, "SET_MAX_COUNT ", 13) == 0) {
			char* value_str = cmd + 13;
			uint16_t value = atoi(value_str);
			if(value > 50000){
				MAX_COUNT_RATE = 50000; // Counter limit is around 50 MHz
				snprintf(response, sizeof(response), "Limit too large, setting 50 Mcps\r\n");
				}
			else if(value < 0){
				MAX_COUNT_RATE = 1000;
				snprintf(response, sizeof(response), "Limit cannot be negative, setting 1 Mcps\r\n");
				}
			else{
				MAX_COUNT_RATE = value;
				snprintf(response, sizeof(response), "OK\r\n");
			}
		}
        else {
            snprintf(response, sizeof(response), "ERROR: Unknown command '%s'\r\n", cmd);
        }

        // Transmit response
        CDC_Transmit_FS((uint8_t*)response, strlen(response));
        command_ready = 0;
    }
}

void check_power_button(){
	static uint32_t lastButtonPress = 0;
	uint32_t currentMillis = HAL_GetTick();

	if (HAL_GPIO_ReadPin(POWER_BUTTON_PIN_GPIO_Port, POWER_BUTTON_PIN_Pin) == GPIO_PIN_SET && (currentMillis - lastButtonPress > 500)) {
	    lastButtonPress = currentMillis;
	    toggle_power();
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
