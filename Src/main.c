/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "kb.h"
#include "sdk_uart.h"
#include "pca9538.h"
#include "oled.h"
#include "fonts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void KB_Test( void );
int calculator( int acc );
void OLED_KB( uint8_t OLED_Keys[12]);
void oled_Reset( void );
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
  MX_I2C1_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  oled_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  int acc = 0;
  while (1) {
	  acc = calculator(acc);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void KB_Test( void ) {
	UART_Transmit( (uint8_t*)"KB test start\n" );
	uint8_t R = 0, C = 0, L = 0, Row[4] = {ROW4, ROW3, ROW2, ROW1}, Key, OldKey, OLED_Keys[12] = {0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};
	oled_Reset();
	oled_WriteString("From bottom", Font_7x10, White);
	OLED_KB(OLED_Keys);
	oled_UpdateScreen();
	for ( int i = 0; i < 4; i++ ) {
		while( !( R && C && L ) ) {
			OldKey = Key;
			Key = Check_Row( Row[i] );
			if ( Key == 0x01 && Key != OldKey) {
				UART_Transmit( (uint8_t*)"Right pressed\n" );
				R = 1;
				OLED_Keys[2+3*i] = 0x31;
				OLED_KB(OLED_Keys);
			} else if ( Key == 0x02 && Key != OldKey) {
				UART_Transmit( (uint8_t*)"Center pressed\n" );
				C = 1;
				OLED_Keys[1+3*i] = 0x31;
				OLED_KB(OLED_Keys);
			} else if ( Key == 0x04 && Key != OldKey) {
				UART_Transmit( (uint8_t*)"Left pressed\n" );
				L = 1;
				OLED_Keys[3*i] = 0x31;
				OLED_KB(OLED_Keys);
			}
		}
		UART_Transmit( (uint8_t*)"Row complete\n" );
		R = C = L = 0;
		HAL_Delay(25);
	}
	UART_Transmit( (uint8_t*)"KB test complete\n");
}

char get_key()
{
	int i;
	int key;
	uint8_t Row[4] = {ROW1, ROW2, ROW3, ROW4};
	char array[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '0', '#'};

	for (i = 0; i < 4; i++) {
		key = Check_Row(Row[i]);
		if (key == 0x01) {
			return array[i * 3 + 2];
		} else if (key == 0x02) {
			return array[i * 3 + 1];
		} else if (key == 0x04) {
			return array[i * 3];
		}
	}
	return '\0';
}

void print_op(int operation)
{
	int r = operation % 4;

    oled_SetCursor(70, 10);

    switch (r) {
	case 0:
    	oled_WriteChar('+', Font_7x10, White);
    	break;
	case 1:
		oled_WriteChar('-', Font_7x10, White);
		break;
	case 2:
		oled_WriteChar('*', Font_7x10, White);
    	break;
	case 3:
		oled_WriteChar('/', Font_7x10, White);
    }
}

void print_bound()
{
    oled_SetCursor(0, 50);
	oled_WriteString("out of bound", Font_7x10, White);
	oled_UpdateScreen();
}

void make_operation(int *a, int *b, int operation) {
	int r = operation % 4;
	switch (r) {
	case 0:
		*a += *b;
		if (*a >= 1000000) {
			print_bound();
		}
		*a %= 1000000;
	    break;
	case 1:
		*a -= *b;
		if (*a >= 1000000 || *a < 0) {
			print_bound();
		}
		*a %= 1000000;
		break;
	case 2:
		*a *= *b;
		if (*a >= 1000000) {
			print_bound();
		}
		*a %= 1000000;
		break;
	case 3:
		*a /= *b;
		if (*a >= 1000000) {
			print_bound();
		}
		*a %= 1000000;
	}
}

void int_to_string(char buffer[20], int n)
{
	int i = 0, j = 0;
	char buf[20];
	while (n > 0) {
		buf[i++]= '0' + n % 10;
		n /= 10;
	}
	buffer[i] = '\0';
	while (i > 0) {
		buffer[j++] = buf[--i];
	}
}

int calculator(int acc)
{
	char key = '\0', old_key;
	int prev = 1;
	int operation = -1;
	int operand = 0;
	char buffer[20];
	UART_Transmit( (uint8_t*)"KB test start\n" );
	oled_Reset();
	oled_WriteString("Input number:", Font_7x10, White);
	oled_UpdateScreen();

    oled_SetCursor(0, 10);

    if (acc != 0) {
        int_to_string(buffer, acc);
	    oled_WriteString(buffer, Font_7x10, White);
	    print_op(++operation);
	    oled_SetCursor(0, 20);
	  	oled_UpdateScreen();
    }

	while (1) {
		old_key = key;
	    key = get_key();
		if (key != '\0' && old_key == key) {
			old_key = '\0';
			HAL_Delay(50);
			continue;
		}
	    if (key == '#') {
			make_operation(&acc, &operand, operation);
			int_to_string(buffer, acc);
			oled_SetCursor(0, 30);
			oled_WriteString("Result: ", Font_7x10, White);
			oled_WriteString(buffer, Font_7x10, White);
			oled_UpdateScreen();
			HAL_Delay(1000);
			break;
	    }

	    switch (key) {
	    case '\0':
	        break;
	    case '*':
	    	print_op(++operation);
		    oled_SetCursor(0, 20);
		  	oled_UpdateScreen();
	        if (prev) {
	            make_operation(&acc, &operand, operation);
	            prev = 0;
	            operand = 0;
	        }
	        break;
	    default:
	        prev = 1;
	        if (operand == 0 && key == '0') {
	        	continue;
	        }
	        if (operand * 10 <= 999990) {
	            operand *= 10;
	            operand += key - '0';
			    oled_WriteChar(key, Font_7x10, White);
			    oled_UpdateScreen();
	        }
	    }
	}

	UART_Transmit( (uint8_t*)"KB test complete\n");
	return acc;
}

void OLED_KB( uint8_t OLED_Keys[12]) {
	for (int i = 3; i >= 0; i--) {
		oled_SetCursor(56, 5+(4-i)*10);
		for (int j = 0; j < 3; j++) {
			oled_WriteChar(OLED_Keys[j+3*i], Font_7x10, White);
		}
	}
	oled_UpdateScreen();
}
void oled_Reset( void ) {
	oled_Fill(Black);
	oled_SetCursor(0, 0);
	oled_UpdateScreen();
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
