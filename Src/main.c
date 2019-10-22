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
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
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
RTC_HandleTypeDef hrtc;

osThreadId Handle_handle;
osThreadId Gatekeeper_handle;
osSemaphoreId xRx_semaphore_handle;
/* USER CODE BEGIN PV */
const char *pcHelp[] = {
  "\r\n\r\nRTC program commands:",
  "\r\n\tset [time | date] [TIME | DATE]:",
  "\r\n\t\tSet current RTC time or date.",
  "\r\n\tshow [time | date]:",
  "\r\n\t\tShow the current RTC time or date.",
  "\r\n\thelp:",
  "\r\n\t\tShow this help."
  "\r\nObs: military time format [245959] and",
  "\r\n date as follows [dd/mm/yy]."
};

const char *pcWeekday[] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

static char pcCmd[5], pcArg_cmd[5], pcArg_time_or_date[7], pcArg_month[3], pcArg_year[3];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
void vRx_handle_task(void const * argument);
void vStdio_gatekeeper_task(void const * argument);

/* USER CODE BEGIN PFP */

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
  MX_RTC_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of xRx_semaphore */
  osSemaphoreDef(xRx_semaphore);
  xRx_semaphore_handle = osSemaphoreCreate(osSemaphore(xRx_semaphore), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Handle */
  osThreadDef(Handle, vRx_handle_task, osPriorityHigh, 0, 64);
  Handle_handle = osThreadCreate(osThread(Handle), NULL);
  
  /* definition and creation of Gatekeeper */
  osThreadDef(Gatekeeper, vStdio_gatekeeper_task, osPriorityLow, 0, 150);
  Gatekeeper_handle = osThreadCreate(osThread(Gatekeeper), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USB;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
    
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 0x11;
  sTime.Minutes = 0x34;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_THURSDAY;
  DateToUpdate.Month = RTC_MONTH_OCTOBER;
  DateToUpdate.Date = 0x10;
  DateToUpdate.Year = 0x19;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_vRx_handle_task */
/**
* @brief Function implementing the Handle thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vRx_handle_task */
void vRx_handle_task(void const * argument)
{
  /* USER CODE BEGIN vRx_handle_task */
  /* Infinite loop */
  for(;;)
  {
    osSemaphoreWait(xRx_semaphore_handle, osWaitForever);

    osThreadSetPriority(Gatekeeper_handle, osPriorityNormal);
  }
  /* USER CODE END vRx_handle_task */
}

/* USER CODE BEGIN Header_vStdio_gatekeeper_task */
/**
* @brief Function implementing the Gatekeeper thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vStdio_gatekeeper_task */
void vStdio_gatekeeper_task(void const * argument)
{
  /* USER CODE BEGIN vStdio_gatekeeper_task */
  /* Infinite loop */
  uint32_t ulHelp_counter;
  RTC_TimeTypeDef sCurrent_time;
  RTC_DateTypeDef Current_date;
  for(;;)
  {
    if(osThreadGetPriority(Gatekeeper_handle) == osPriorityNormal)
    {
      scanf("%*[ ]%4s %*[ ]%4s %*[ ]%6s/%2s/%2s", pcCmd, pcArg_cmd, pcArg_time_or_date, pcArg_month, pcArg_year);
      
      if(!strcmp(pcCmd, "set"))
      {
        if(!strcmp(pcArg_cmd, "time"))
        {
          sCurrent_time.Hours = (pcArg_time_or_date[0] - '0') * 10 + (pcArg_time_or_date[1] - '0');
          sCurrent_time.Minutes = (pcArg_time_or_date[2] - '0') * 10 + (pcArg_time_or_date[3] - '0');
          sCurrent_time.Seconds = (pcArg_time_or_date[4] - '0') * 10 + (pcArg_time_or_date[5] - '0');
          
          if(sCurrent_time.Hours > 24)
            sCurrent_time.Hours = 0;
          if(sCurrent_time.Minutes > 59)
            sCurrent_time.Minutes = 0;
          if(sCurrent_time.Seconds > 59)
            sCurrent_time.Seconds = 0;

          HAL_RTC_SetTime(&hrtc, &sCurrent_time, RTC_FORMAT_BIN);
          HAL_RTC_GetTime(&hrtc, &sCurrent_time, RTC_FORMAT_BCD);
          printf("\r\nThe current time is %u%u:%u%u:%u%u.", (sCurrent_time.Hours >> 4), (sCurrent_time.Hours & 0x0F), \
          (sCurrent_time.Minutes >> 4), (sCurrent_time.Minutes & 0x0F), (sCurrent_time.Seconds >> 4), (sCurrent_time.Seconds & 0x0F));
        }else if(!strcmp(pcArg_cmd, "date"))
        {
          Current_date.Date = (pcArg_time_or_date[0] - '0') * 10 + (pcArg_time_or_date[1] - '0');
          Current_date.Month = (pcArg_month[0] - '0') * 10 + (pcArg_month[1] - '0');
          Current_date.Year = (pcArg_year[0] - '0') * 10 + (pcArg_year[1] - '0');

          if((Current_date.Date == 0) || (Current_date.Date > 31))
            Current_date.Date = 1;
          if((Current_date.Month == 0) || (Current_date.Month > 12))
            Current_date.Month = 1;

          HAL_RTC_SetDate(&hrtc, &Current_date, RTC_FORMAT_BIN);
          HAL_RTC_GetDate(&hrtc, &Current_date, RTC_FORMAT_BCD);
          printf("\r\nThe current date is %s, %u%u/%u%u/%u%u.", pcWeekday[((Current_date.WeekDay >> 4) + (Current_date.WeekDay & 0x0F))], \
          (Current_date.Date >> 4), (Current_date.Date & 0x0F), (Current_date.Month >> 4), (Current_date.Month & 0x0F), \
          (Current_date.Year >> 4), (Current_date.Year & 0x0F));
        }else
          printf(pcHelp[1]);

      }else if(!strcmp(pcCmd, "show"))
      {
        if(!strcmp(pcArg_cmd, "time"))
        {
          HAL_RTC_GetTime(&hrtc, &sCurrent_time, RTC_FORMAT_BCD);
          printf("\r\nThe current time is %u%u:%u%u:%u%u.", (sCurrent_time.Hours >> 4), (sCurrent_time.Hours & 0x0F), \
          (sCurrent_time.Minutes >> 4), (sCurrent_time.Minutes & 0x0F), (sCurrent_time.Seconds >> 4), (sCurrent_time.Seconds & 0x0F));
        }else if(strcmp(pcArg_cmd, "date") == 0)
        {
          HAL_RTC_GetDate(&hrtc, &Current_date, RTC_FORMAT_BCD);
          printf("\r\nThe current date is %s, %u%u/%u%u/%u%u.", pcWeekday[((Current_date.WeekDay >> 4) + (Current_date.WeekDay & 0x0F))], \
          (Current_date.Date >> 4), (Current_date.Date & 0x0F), (Current_date.Month >> 4), (Current_date.Month & 0x0F), \
          (Current_date.Year >> 4), (Current_date.Year & 0x0F));
        }else
          printf(pcHelp[3]);
        

      }else
      {
        for(ulHelp_counter = 0; ulHelp_counter < 8; ulHelp_counter++)
        {
          HAL_Delay(1);
          printf(pcHelp[ulHelp_counter]);
        }
      }

      osThreadSetPriority(Gatekeeper_handle, osPriorityLow); 
    }
  }
  /* USER CODE END vStdio_gatekeeper_task */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
