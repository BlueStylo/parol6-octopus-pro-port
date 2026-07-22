#include <Arduino.h>

// Octopus Pro V1.0 F446 has a 12 MHz HSE. The generic STM32F446ZE Arduino
// variant otherwise derives USB from HSI, so provide the board clock tree that
// was already verified by the earlier Octopus USB diagnostic firmware.
extern "C" void SystemClock_Config(void) {
  RCC_OscInitTypeDef oscillator = {};
  RCC_ClkInitTypeDef clock = {};
  RCC_PeriphCLKInitTypeDef peripheral = {};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  oscillator.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  oscillator.HSEState = RCC_HSE_ON;
  oscillator.PLL.PLLState = RCC_PLL_ON;
  oscillator.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  oscillator.PLL.PLLM = 6;
  oscillator.PLL.PLLN = 180;
  oscillator.PLL.PLLP = RCC_PLLP_DIV2;
  oscillator.PLL.PLLQ = 7;
  oscillator.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&oscillator) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
    Error_Handler();
  }

  clock.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clock.SYSCLKSource = RCC_SYSCLKSOURCE_PLLRCLK;
  clock.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clock.APB1CLKDivider = RCC_HCLK_DIV4;
  clock.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&clock, FLASH_LATENCY_5) != HAL_OK) {
    Error_Handler();
  }

  peripheral.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
  peripheral.PLLSAI.PLLSAIM = 6;
  peripheral.PLLSAI.PLLSAIN = 96;
  peripheral.PLLSAI.PLLSAIQ = 2;
  peripheral.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV4;
  peripheral.PLLSAIDivQ = 1;
  peripheral.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLSAIP;
  if (HAL_RCCEx_PeriphCLKConfig(&peripheral) != HAL_OK) {
    Error_Handler();
  }
}
