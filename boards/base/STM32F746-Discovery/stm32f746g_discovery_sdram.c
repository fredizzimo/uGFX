/**
  ******************************************************************************
  * @file    stm32746g_discovery_sdram.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    25-June-2015
  * @brief   This file includes the SDRAM driver for the MT48LC4M32B2B5-7 memory 
  *          device mounted on STM32746G-Discovery board.
  @verbatim
   1. How To use this driver:
   --------------------------
      - This driver is used to drive the MT48LC4M32B2B5-7 SDRAM external memory mounted
        on STM32746G-Discovery board.
      - This driver does not need a specific component driver for the SDRAM device
        to be included with.
   
   2. Driver description:
   ---------------------
     + Initialization steps:
        o Initialize the SDRAM external memory using the BSP_SDRAM_Init() function. This 
          function includes the MSP layer hardware resources initialization and the
          FMC controller configuration to interface with the external SDRAM memory.
        o It contains the SDRAM initialization sequence to program the SDRAM external 
          device using the function BSP_SDRAM_Initialization_sequence(). Note that this 
          sequence is standard for all SDRAM devices, but can include some differences
          from a device to another. If it is the case, the right sequence should be 
          implemented separately.
     
     + SDRAM read/write operations
        o SDRAM external memory can be accessed with read/write operations once it is
          initialized.
          Read/write operation can be performed with AHB access using the functions
          BSP_SDRAM_ReadData()/BSP_SDRAM_WriteData(), or by DMA transfer using the functions
          BSP_SDRAM_ReadData_DMA()/BSP_SDRAM_WriteData_DMA().
        o The AHB access is performed with 32-bit width transaction, the DMA transfer
          configuration is fixed at single (no burst) word transfer (see the 
          SDRAM_MspInit() static function).
        o User can implement his own functions for read/write access with his desired 
          configurations.
        o If interrupt mode is used for DMA transfer, the function BSP_SDRAM_DMA_IRQHandler()
          is called in IRQ handler file, to serve the generated interrupt once the DMA 
          transfer is complete.
        o You can send a command to the SDRAM device in runtime using the function 
          BSP_SDRAM_Sendcmd(), and giving the desired command as parameter chosen between 
          the predefined commands of the "FMC_SDRAM_CommandTypeDef" structure. 
 
  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gfx.h"
#include "stm32f746g_discovery_sdram.h"
#include "stm32f7xx_hal_rcc.h"
#include "stm32f7xx_hal_rcc_ex.h"
#include "stm32f7xx_hal_cortex.h"

#if GFX_USE_OS_CHIBIOS
	#define HAL_GPIO_Init(port, ptr)	palSetGroupMode(port, (ptr)->Pin, 0, (ptr)->Mode|((ptr)->Speed<<3)|((ptr)->Pull<<5)|((ptr)->Alternate<<7))
#endif

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32746G_DISCOVERY
  * @{
  */ 
  
/** @defgroup STM32746G_DISCOVERY_SDRAM STM32746G_DISCOVERY_SDRAM
  * @{
  */ 

/** @defgroup STM32746G_DISCOVERY_SDRAM_Private_Types_Definitions STM32746G_DISCOVERY_SDRAM Private Types Definitions
  * @{
  */ 
/**
  * @}
  */

/** @defgroup STM32746G_DISCOVERY_SDRAM_Private_Defines STM32746G_DISCOVERY_SDRAM Private Defines
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32746G_DISCOVERY_SDRAM_Private_Macros STM32746G_DISCOVERY_SDRAM Private Macros
  * @{
  */  
/**
  * @}
  */

/** @defgroup STM32746G_DISCOVERY_SDRAM_Private_Variables STM32746G_DISCOVERY_SDRAM Private Variables
  * @{
  */       
static SDRAM_HandleTypeDef sdramHandle;
static FMC_SDRAM_TimingTypeDef Timing;
static FMC_SDRAM_CommandTypeDef Command;
/**
  * @}
  */ 

/** @defgroup STM32746G_DISCOVERY_SDRAM_Private_Function_Prototypes STM32746G_DISCOVERY_SDRAM Private Function Prototypes
  * @{
  */ 
/**
  * @}
  */
    
/** @defgroup STM32746G_DISCOVERY_SDRAM_Exported_Functions STM32746G_DISCOVERY_SDRAM Exported Functions
  * @{
  */ 

static HAL_StatusTypeDef _HAL_SDRAM_Init(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_TimingTypeDef *Timing)
{
  /* Check the SDRAM handle parameter */
  if(hsdram == NULL)
  {
    return HAL_ERROR;
  }

  if(hsdram->State == HAL_SDRAM_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hsdram->Lock = HAL_UNLOCKED;
  }

  /* Initialize the SDRAM controller state */
  hsdram->State = HAL_SDRAM_STATE_BUSY;

  /* Initialize SDRAM control Interface */
  FMC_SDRAM_Init(hsdram->Instance, &(hsdram->Init));

  /* Initialize SDRAM timing Interface */
  FMC_SDRAM_Timing_Init(hsdram->Instance, Timing, hsdram->Init.SDBank);

  /* Update the SDRAM controller state */
  hsdram->State = HAL_SDRAM_STATE_READY;

  return HAL_OK;
}

static HAL_StatusTypeDef _HAL_SDRAM_DeInit(SDRAM_HandleTypeDef *hsdram)
{
  /* Configure the SDRAM registers with their reset values */
  FMC_SDRAM_DeInit(hsdram->Instance, hsdram->Init.SDBank);

  /* Reset the SDRAM controller state */
  hsdram->State = HAL_SDRAM_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hsdram);

  return HAL_OK;
}

/**
  * @brief  Initializes the SDRAM device.
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_Init(void)
{ 
  static uint8_t sdramstatus = SDRAM_ERROR;
  /* SDRAM device configuration */
  sdramHandle.Instance = FMC_SDRAM_DEVICE;
    
  /* Timing configuration for 100Mhz as SD clock frequency (System clock is up to 200Mhz) */
  Timing.LoadToActiveDelay    = 2;
  Timing.ExitSelfRefreshDelay = 7;
  Timing.SelfRefreshTime      = 4;
  Timing.RowCycleDelay        = 7;
  Timing.WriteRecoveryTime    = 2;
  Timing.RPDelay              = 2;
  Timing.RCDDelay             = 2;
  
  sdramHandle.Init.SDBank             = FMC_SDRAM_BANK1;
  sdramHandle.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
  sdramHandle.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
  sdramHandle.Init.MemoryDataWidth    = SDRAM_MEMORY_WIDTH;
  sdramHandle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  sdramHandle.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_2;
  sdramHandle.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  sdramHandle.Init.SDClockPeriod      = SDCLOCK_PERIOD;
  sdramHandle.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
  sdramHandle.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;
  
  /* SDRAM controller initialization */

  BSP_SDRAM_MspInit(&sdramHandle, NULL); /* __weak function can be rewritten by the application */

  if(_HAL_SDRAM_Init(&sdramHandle, &Timing) != HAL_OK)
  {
    sdramstatus = SDRAM_ERROR;
  }
  else
  {
    sdramstatus = SDRAM_OK;
  }
  
  /* SDRAM initialization sequence */
  BSP_SDRAM_Initialization_sequence(REFRESH_COUNT);
  
  return sdramstatus;
}

/**
  * @brief  DeInitializes the SDRAM device.
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_DeInit(void)
{ 
  static uint8_t sdramstatus = SDRAM_ERROR;
  /* SDRAM device de-initialization */
  sdramHandle.Instance = FMC_SDRAM_DEVICE;

  if(_HAL_SDRAM_DeInit(&sdramHandle) != HAL_OK)
  {
    sdramstatus = SDRAM_ERROR;
  }
  else
  {
    sdramstatus = SDRAM_OK;
  }
  
  /* SDRAM controller de-initialization */
  BSP_SDRAM_MspDeInit(&sdramHandle, NULL);
  
  return sdramstatus;
}

static HAL_StatusTypeDef _HAL_SDRAM_SendCommand(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command, uint32_t Timeout)
{
  /* Check the SDRAM controller state */
  if(hsdram->State == HAL_SDRAM_STATE_BUSY)
  {
    return HAL_BUSY;
  }

  /* Update the SDRAM state */
  hsdram->State = HAL_SDRAM_STATE_BUSY;

  /* Send SDRAM command */
  FMC_SDRAM_SendCommand(hsdram->Instance, Command, Timeout);

  /* Update the SDRAM controller state state */
  if(Command->CommandMode == FMC_SDRAM_CMD_PALL)
  {
    hsdram->State = HAL_SDRAM_STATE_PRECHARGED;
  }
  else
  {
    hsdram->State = HAL_SDRAM_STATE_READY;
  }

  return HAL_OK;
}

static HAL_StatusTypeDef _HAL_SDRAM_ProgramRefreshRate(SDRAM_HandleTypeDef *hsdram, uint32_t RefreshRate)
{
  /* Check the SDRAM controller state */
  if(hsdram->State == HAL_SDRAM_STATE_BUSY)
  {
    return HAL_BUSY;
  }

  /* Update the SDRAM state */
  hsdram->State = HAL_SDRAM_STATE_BUSY;

  /* Program the refresh rate */
  FMC_SDRAM_ProgramRefreshRate(hsdram->Instance ,RefreshRate);

  /* Update the SDRAM state */
  hsdram->State = HAL_SDRAM_STATE_READY;

  return HAL_OK;
}

static HAL_StatusTypeDef _HAL_SDRAM_Read_32b(SDRAM_HandleTypeDef *hsdram, uint32_t *pAddress, uint32_t *pDstBuffer, uint32_t BufferSize)
{
  __IO uint32_t *pSdramAddress = (uint32_t *)pAddress;

  /* Process Locked */
  __HAL_LOCK(hsdram);

  /* Check the SDRAM controller state */
  if(hsdram->State == HAL_SDRAM_STATE_BUSY)
  {
    return HAL_BUSY;
  }
  else if(hsdram->State == HAL_SDRAM_STATE_PRECHARGED)
  {
    return  HAL_ERROR;
  }

  /* Read data from source */
  for(; BufferSize != 0; BufferSize--)
  {
    *pDstBuffer = *(__IO uint32_t *)pSdramAddress;
    pDstBuffer++;
    pSdramAddress++;
  }

  /* Process Unlocked */
  __HAL_UNLOCK(hsdram);

  return HAL_OK;
}

static HAL_StatusTypeDef _HAL_DMA_Init(DMA_HandleTypeDef *hdma)
{
  uint32_t tmp = 0;

  /* Check the DMA peripheral state */
  if(hdma == NULL)
  {
    return HAL_ERROR;
  }

  /* Change DMA peripheral state */
  hdma->State = HAL_DMA_STATE_BUSY;

  /* Get the CR register value */
  tmp = hdma->Instance->CR;

  /* Clear CHSEL, MBURST, PBURST, PL, MSIZE, PSIZE, MINC, PINC, CIRC, DIR, CT and DBM bits */
  tmp &= ((uint32_t)~(DMA_SxCR_CHSEL | DMA_SxCR_MBURST | DMA_SxCR_PBURST | \
                      DMA_SxCR_PL    | DMA_SxCR_MSIZE  | DMA_SxCR_PSIZE  | \
                      DMA_SxCR_MINC  | DMA_SxCR_PINC   | DMA_SxCR_CIRC   | \
                      DMA_SxCR_DIR   | DMA_SxCR_CT     | DMA_SxCR_DBM));

  /* Prepare the DMA Stream configuration */
  tmp |=  hdma->Init.Channel             | hdma->Init.Direction        |
          hdma->Init.PeriphInc           | hdma->Init.MemInc           |
          hdma->Init.PeriphDataAlignment | hdma->Init.MemDataAlignment |
          hdma->Init.Mode                | hdma->Init.Priority;

  /* the Memory burst and peripheral burst are not used when the FIFO is disabled */
  if(hdma->Init.FIFOMode == DMA_FIFOMODE_ENABLE)
  {
    /* Get memory burst and peripheral burst */
    tmp |=  hdma->Init.MemBurst | hdma->Init.PeriphBurst;
  }

  /* Write to DMA Stream CR register */
  hdma->Instance->CR = tmp;

  /* Get the FCR register value */
  tmp = hdma->Instance->FCR;

  /* Clear Direct mode and FIFO threshold bits */
  tmp &= (uint32_t)~(DMA_SxFCR_DMDIS | DMA_SxFCR_FTH);

  /* Prepare the DMA Stream FIFO configuration */
  tmp |= hdma->Init.FIFOMode;

  /* the FIFO threshold is not used when the FIFO mode is disabled */
  if(hdma->Init.FIFOMode == DMA_FIFOMODE_ENABLE)
  {
    /* Get the FIFO threshold */
    tmp |= hdma->Init.FIFOThreshold;
  }

  /* Write to DMA Stream FCR */
  hdma->Instance->FCR = tmp;

  /* Initialize the error code */
  hdma->ErrorCode = HAL_DMA_ERROR_NONE;

  /* Initialize the DMA state */
  hdma->State = HAL_DMA_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  DeInitializes the DMA peripheral
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA Stream.
  * @retval HAL status
  */
static HAL_StatusTypeDef _HAL_DMA_DeInit(DMA_HandleTypeDef *hdma)
{
  /* Check the DMA peripheral state */
  if(hdma == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the DMA peripheral state */
  if(hdma->State == HAL_DMA_STATE_BUSY)
  {
     return HAL_ERROR;
  }

  /* Disable the selected DMA Streamx */
  __HAL_DMA_DISABLE(hdma);

  /* Reset DMA Streamx control register */
  hdma->Instance->CR   = 0;

  /* Reset DMA Streamx number of data to transfer register */
  hdma->Instance->NDTR = 0;

  /* Reset DMA Streamx peripheral address register */
  hdma->Instance->PAR  = 0;

  /* Reset DMA Streamx memory 0 address register */
  hdma->Instance->M0AR = 0;

  /* Reset DMA Streamx memory 1 address register */
  hdma->Instance->M1AR = 0;

  /* Reset DMA Streamx FIFO control register */
  hdma->Instance->FCR  = (uint32_t)0x00000021;

  /* Clear all flags */
  __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_DME_FLAG_INDEX(hdma));
  __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_TC_FLAG_INDEX(hdma));
  __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_TE_FLAG_INDEX(hdma));
  __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_FE_FLAG_INDEX(hdma));
  __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_HT_FLAG_INDEX(hdma));

  /* Initialize the error code */
  hdma->ErrorCode = HAL_DMA_ERROR_NONE;

  /* Initialize the DMA state */
  hdma->State = HAL_DMA_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hdma);

  return HAL_OK;
}

static void _HAL_DMA_IRQHandler(DMA_HandleTypeDef *hdma)
{
  /* Transfer Error Interrupt management ***************************************/
  if(__HAL_DMA_GET_FLAG(hdma, __HAL_DMA_GET_TE_FLAG_INDEX(hdma)) != RESET)
  {
    if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_TE) != RESET)
    {
      /* Disable the transfer error interrupt */
      __HAL_DMA_DISABLE_IT(hdma, DMA_IT_TE);

      /* Clear the transfer error flag */
      __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_TE_FLAG_INDEX(hdma));

      /* Update error code */
      hdma->ErrorCode |= HAL_DMA_ERROR_TE;

      /* Change the DMA state */
      hdma->State = HAL_DMA_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hdma);

      if(hdma->XferErrorCallback != NULL)
      {
        /* Transfer error callback */
        hdma->XferErrorCallback(hdma);
      }
    }
  }
  /* FIFO Error Interrupt management ******************************************/
  if(__HAL_DMA_GET_FLAG(hdma, __HAL_DMA_GET_FE_FLAG_INDEX(hdma)) != RESET)
  {
    if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_FE) != RESET)
    {
      /* Disable the FIFO Error interrupt */
      __HAL_DMA_DISABLE_IT(hdma, DMA_IT_FE);

      /* Clear the FIFO error flag */
      __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_FE_FLAG_INDEX(hdma));

      /* Update error code */
      hdma->ErrorCode |= HAL_DMA_ERROR_FE;

      /* Change the DMA state */
      hdma->State = HAL_DMA_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hdma);

      if(hdma->XferErrorCallback != NULL)
      {
        /* Transfer error callback */
        hdma->XferErrorCallback(hdma);
      }
    }
  }
  /* Direct Mode Error Interrupt management ***********************************/
  if(__HAL_DMA_GET_FLAG(hdma, __HAL_DMA_GET_DME_FLAG_INDEX(hdma)) != RESET)
  {
    if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_DME) != RESET)
    {
      /* Disable the direct mode Error interrupt */
      __HAL_DMA_DISABLE_IT(hdma, DMA_IT_DME);

      /* Clear the direct mode error flag */
      __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_DME_FLAG_INDEX(hdma));

      /* Update error code */
      hdma->ErrorCode |= HAL_DMA_ERROR_DME;

      /* Change the DMA state */
      hdma->State = HAL_DMA_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hdma);

      if(hdma->XferErrorCallback != NULL)
      {
        /* Transfer error callback */
        hdma->XferErrorCallback(hdma);
      }
    }
  }
  /* Half Transfer Complete Interrupt management ******************************/
  if(__HAL_DMA_GET_FLAG(hdma, __HAL_DMA_GET_HT_FLAG_INDEX(hdma)) != RESET)
  {
    if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_HT) != RESET)
    {
      /* Multi_Buffering mode enabled */
      if(((hdma->Instance->CR) & (uint32_t)(DMA_SxCR_DBM)) != 0)
      {
        /* Clear the half transfer complete flag */
        __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_HT_FLAG_INDEX(hdma));

        /* Current memory buffer used is Memory 0 */
        if((hdma->Instance->CR & DMA_SxCR_CT) == 0)
        {
          /* Change DMA peripheral state */
          hdma->State = HAL_DMA_STATE_READY_HALF_MEM0;
        }
        /* Current memory buffer used is Memory 1 */
        else if((hdma->Instance->CR & DMA_SxCR_CT) != 0)
        {
          /* Change DMA peripheral state */
          hdma->State = HAL_DMA_STATE_READY_HALF_MEM1;
        }
      }
      else
      {
        /* Disable the half transfer interrupt if the DMA mode is not CIRCULAR */
        if((hdma->Instance->CR & DMA_SxCR_CIRC) == 0)
        {
          /* Disable the half transfer interrupt */
          __HAL_DMA_DISABLE_IT(hdma, DMA_IT_HT);
        }
        /* Clear the half transfer complete flag */
        __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_HT_FLAG_INDEX(hdma));

        /* Change DMA peripheral state */
        hdma->State = HAL_DMA_STATE_READY_HALF_MEM0;
      }

      if(hdma->XferHalfCpltCallback != NULL)
      {
        /* Half transfer callback */
        hdma->XferHalfCpltCallback(hdma);
      }
    }
  }
  /* Transfer Complete Interrupt management ***********************************/
  if(__HAL_DMA_GET_FLAG(hdma, __HAL_DMA_GET_TC_FLAG_INDEX(hdma)) != RESET)
  {
    if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_TC) != RESET)
    {
      if(((hdma->Instance->CR) & (uint32_t)(DMA_SxCR_DBM)) != 0)
      {
        /* Clear the transfer complete flag */
        __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_TC_FLAG_INDEX(hdma));

        /* Current memory buffer used is Memory 1 */
        if((hdma->Instance->CR & DMA_SxCR_CT) == 0)
        {
          if(hdma->XferM1CpltCallback != NULL)
          {
            /* Transfer complete Callback for memory1 */
            hdma->XferM1CpltCallback(hdma);
          }
        }
        /* Current memory buffer used is Memory 0 */
        else if((hdma->Instance->CR & DMA_SxCR_CT) != 0)
        {
          if(hdma->XferCpltCallback != NULL)
          {
            /* Transfer complete Callback for memory0 */
            hdma->XferCpltCallback(hdma);
          }
        }
      }
      /* Disable the transfer complete interrupt if the DMA mode is not CIRCULAR */
      else
      {
        if((hdma->Instance->CR & DMA_SxCR_CIRC) == 0)
        {
          /* Disable the transfer complete interrupt */
          __HAL_DMA_DISABLE_IT(hdma, DMA_IT_TC);
        }
        /* Clear the transfer complete flag */
        __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_TC_FLAG_INDEX(hdma));

        /* Update error code */
        hdma->ErrorCode |= HAL_DMA_ERROR_NONE;

        /* Change the DMA state */
        hdma->State = HAL_DMA_STATE_READY_MEM0;

        /* Process Unlocked */
        __HAL_UNLOCK(hdma);

        if(hdma->XferCpltCallback != NULL)
        {
          /* Transfer complete callback */
          hdma->XferCpltCallback(hdma);
        }
      }
    }
  }
}

static void DMA_SetConfig(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
  /* Clear DBM bit */
  hdma->Instance->CR &= (uint32_t)(~DMA_SxCR_DBM);

  /* Configure DMA Stream data length */
  hdma->Instance->NDTR = DataLength;

  /* Peripheral to Memory */
  if((hdma->Init.Direction) == DMA_MEMORY_TO_PERIPH)
  {
    /* Configure DMA Stream destination address */
    hdma->Instance->PAR = DstAddress;

    /* Configure DMA Stream source address */
    hdma->Instance->M0AR = SrcAddress;
  }
  /* Memory to Peripheral */
  else
  {
    /* Configure DMA Stream source address */
    hdma->Instance->PAR = SrcAddress;

    /* Configure DMA Stream destination address */
    hdma->Instance->M0AR = DstAddress;
  }
}

static HAL_StatusTypeDef _HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
  /* Process locked */
  __HAL_LOCK(hdma);

  /* Change DMA peripheral state */
  hdma->State = HAL_DMA_STATE_BUSY;

  /* Disable the peripheral */
  __HAL_DMA_DISABLE(hdma);

  /* Configure the source, destination address and the data length */
  DMA_SetConfig(hdma, SrcAddress, DstAddress, DataLength);

  /* Enable the transfer complete interrupt */
  __HAL_DMA_ENABLE_IT(hdma, DMA_IT_TC);

  /* Enable the Half transfer complete interrupt */
  __HAL_DMA_ENABLE_IT(hdma, DMA_IT_HT);

  /* Enable the transfer Error interrupt */
  __HAL_DMA_ENABLE_IT(hdma, DMA_IT_TE);

  /* Enable the FIFO Error interrupt */
  __HAL_DMA_ENABLE_IT(hdma, DMA_IT_FE);

  /* Enable the direct mode Error interrupt */
  __HAL_DMA_ENABLE_IT(hdma, DMA_IT_DME);

   /* Enable the Peripheral */
  __HAL_DMA_ENABLE(hdma);

  return HAL_OK;
}

static HAL_StatusTypeDef _HAL_SDRAM_Read_DMA(SDRAM_HandleTypeDef *hsdram, uint32_t *pAddress, uint32_t *pDstBuffer, uint32_t BufferSize)
{
  uint32_t tmp = 0;

  /* Process Locked */
  __HAL_LOCK(hsdram);

  /* Check the SDRAM controller state */
  tmp = hsdram->State;

  if(tmp == HAL_SDRAM_STATE_BUSY)
  {
    return HAL_BUSY;
  }
  else if(tmp == HAL_SDRAM_STATE_PRECHARGED)
  {
    return  HAL_ERROR;
  }

  /* Configure DMA user callbacks */
  hsdram->hdma->XferCpltCallback  = HAL_SDRAM_DMA_XferCpltCallback;
  hsdram->hdma->XferErrorCallback = HAL_SDRAM_DMA_XferErrorCallback;

  /* Enable the DMA Stream */
  _HAL_DMA_Start_IT(hsdram->hdma, (uint32_t)pAddress, (uint32_t)pDstBuffer, (uint32_t)BufferSize);

  /* Process Unlocked */
  __HAL_UNLOCK(hsdram);

  return HAL_OK;
}

static HAL_StatusTypeDef _HAL_SDRAM_Write_32b(SDRAM_HandleTypeDef *hsdram, uint32_t *pAddress, uint32_t *pSrcBuffer, uint32_t BufferSize)
{
  __IO uint32_t *pSdramAddress = (uint32_t *)pAddress;
  uint32_t tmp = 0;

  /* Process Locked */
  __HAL_LOCK(hsdram);

  /* Check the SDRAM controller state */
  tmp = hsdram->State;

  if(tmp == HAL_SDRAM_STATE_BUSY)
  {
    return HAL_BUSY;
  }
  else if((tmp == HAL_SDRAM_STATE_PRECHARGED) || (tmp == HAL_SDRAM_STATE_WRITE_PROTECTED))
  {
    return  HAL_ERROR;
  }

  /* Write data to memory */
  for(; BufferSize != 0; BufferSize--)
  {
    *(__IO uint32_t *)pSdramAddress = *pSrcBuffer;
    pSrcBuffer++;
    pSdramAddress++;
  }

  /* Process Unlocked */
  __HAL_UNLOCK(hsdram);

  return HAL_OK;
}

static HAL_StatusTypeDef _HAL_SDRAM_Write_DMA(SDRAM_HandleTypeDef *hsdram, uint32_t *pAddress, uint32_t *pSrcBuffer, uint32_t BufferSize)
{
  uint32_t tmp = 0;

  /* Process Locked */
  __HAL_LOCK(hsdram);

  /* Check the SDRAM controller state */
  tmp = hsdram->State;

  if(tmp == HAL_SDRAM_STATE_BUSY)
  {
    return HAL_BUSY;
  }
  else if((tmp == HAL_SDRAM_STATE_PRECHARGED) || (tmp == HAL_SDRAM_STATE_WRITE_PROTECTED))
  {
    return  HAL_ERROR;
  }

  /* Configure DMA user callbacks */
  hsdram->hdma->XferCpltCallback  = HAL_SDRAM_DMA_XferCpltCallback;
  hsdram->hdma->XferErrorCallback = HAL_SDRAM_DMA_XferErrorCallback;

  /* Enable the DMA Stream */
  _HAL_DMA_Start_IT(hsdram->hdma, (uint32_t)pSrcBuffer, (uint32_t)pAddress, (uint32_t)BufferSize);

  /* Process Unlocked */
  __HAL_UNLOCK(hsdram);

  return HAL_OK;
}

/**
  * @brief  Programs the SDRAM device.
  * @param  RefreshCount: SDRAM refresh counter value 
  * @retval None
  */
void BSP_SDRAM_Initialization_sequence(uint32_t RefreshCount)
{
  __IO uint32_t tmpmrd = 0;
  
  /* Step 1: Configure a clock configuration enable command */
  Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  _HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

  /* Step 2: Insert 100 us minimum delay */ 
  /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
  gfxSleepMilliseconds(1);
    
  /* Step 3: Configure a PALL (precharge all) command */ 
  Command.CommandMode            = FMC_SDRAM_CMD_PALL;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  _HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);
  
  /* Step 4: Configure an Auto Refresh command */ 
  Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 8;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  _HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);
  
  /* Step 5: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |\
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |\
                     SDRAM_MODEREG_CAS_LATENCY_2           |\
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |\
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
  
  Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  _HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);
  
  /* Step 6: Set the refresh rate counter */
  /* Set the device refresh rate */
  _HAL_SDRAM_ProgramRefreshRate(&sdramHandle, RefreshCount);
}

/**
  * @brief  Reads an amount of data from the SDRAM memory in polling mode.
  * @param  uwStartAddress: Read start address
  * @param  pData: Pointer to data to be read  
  * @param  uwDataSize: Size of read data from the memory
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_ReadData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize)
{
  if(_HAL_SDRAM_Read_32b(&sdramHandle, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    return SDRAM_ERROR;
  }
  else
  {
    return SDRAM_OK;
  } 
}

/**
  * @brief  Reads an amount of data from the SDRAM memory in DMA mode.
  * @param  uwStartAddress: Read start address
  * @param  pData: Pointer to data to be read  
  * @param  uwDataSize: Size of read data from the memory
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_ReadData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize)
{
  if(_HAL_SDRAM_Read_DMA(&sdramHandle, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    return SDRAM_ERROR;
  }
  else
  {
    return SDRAM_OK;
  }     
}

/**
  * @brief  Writes an amount of data to the SDRAM memory in polling mode.
  * @param  uwStartAddress: Write start address
  * @param  pData: Pointer to data to be written  
  * @param  uwDataSize: Size of written data from the memory
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_WriteData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize) 
{
  if(_HAL_SDRAM_Write_32b(&sdramHandle, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    return SDRAM_ERROR;
  }
  else
  {
    return SDRAM_OK;
  }
}

__weak void HAL_SDRAM_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma)
{
  /* NOTE: This function Should not be modified, when the callback is needed,
            the HAL_SDRAM_DMA_XferCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  DMA transfer complete error callback.
  * @param  hdma: DMA handle
  * @retval None
  */
__weak void HAL_SDRAM_DMA_XferErrorCallback(DMA_HandleTypeDef *hdma)
{
  /* NOTE: This function Should not be modified, when the callback is needed,
            the HAL_SDRAM_DMA_XferErrorCallback could be implemented in the user file
   */
}

static void _HAL_NVIC_EnableIRQ(IRQn_Type IRQn)
{
  /* Enable interrupt */
  NVIC_EnableIRQ(IRQn);
}

static void _HAL_NVIC_DisableIRQ(IRQn_Type IRQn)
{
  /* Disable interrupt */
  NVIC_DisableIRQ(IRQn);
}

/**
  * @brief  Writes an amount of data to the SDRAM memory in DMA mode.
  * @param  uwStartAddress: Write start address
  * @param  pData: Pointer to data to be written  
  * @param  uwDataSize: Size of written data from the memory
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_WriteData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize) 
{
  if(_HAL_SDRAM_Write_DMA(&sdramHandle, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    return SDRAM_ERROR;
  }
  else
  {
    return SDRAM_OK;
  } 
}

/**
  * @brief  Sends command to the SDRAM bank.
  * @param  SdramCmd: Pointer to SDRAM command structure 
  * @retval SDRAM status
  */  
uint8_t BSP_SDRAM_Sendcmd(FMC_SDRAM_CommandTypeDef *SdramCmd)
{
  if(_HAL_SDRAM_SendCommand(&sdramHandle, SdramCmd, SDRAM_TIMEOUT) != HAL_OK)
  {
    return SDRAM_ERROR;
  }
  else
  {
    return SDRAM_OK;
  }
}

static void _HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority)
{
  uint32_t prioritygroup = 0x00;

  prioritygroup = NVIC_GetPriorityGrouping();

  NVIC_SetPriority(IRQn, NVIC_EncodePriority(prioritygroup, PreemptPriority, SubPriority));
}

/**
  * @brief  Handles SDRAM DMA transfer interrupt request.
  * @retval None
  */
void BSP_SDRAM_DMA_IRQHandler(void)
{
  _HAL_DMA_IRQHandler(sdramHandle.hdma);
}

/**
  * @brief  Initializes SDRAM MSP.
  * @param  hsdram: SDRAM handle
  * @param  Params
  * @retval None
  */
__weak void BSP_SDRAM_MspInit(SDRAM_HandleTypeDef  *hsdram, void *Params)
{  
  static DMA_HandleTypeDef dma_handle;
  GPIO_InitTypeDef gpio_init_structure;
  
  /* Enable FMC clock */
  __HAL_RCC_FMC_CLK_ENABLE();
  
  /* Enable chosen DMAx clock */
  __DMAx_CLK_ENABLE();

  /* Enable GPIOs clock */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  
  /* Common GPIO configuration */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FAST;
  gpio_init_structure.Alternate = GPIO_AF12_FMC;
  
  /* GPIOC configuration */
  gpio_init_structure.Pin   = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOC, &gpio_init_structure);

  /* GPIOD configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_8 | GPIO_PIN_9 |
                              GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &gpio_init_structure);

  /* GPIOE configuration */  
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7| GPIO_PIN_8 | GPIO_PIN_9 |\
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &gpio_init_structure);
  
  /* GPIOF configuration */  
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 |\
                              GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &gpio_init_structure);
  
  /* GPIOG configuration */  
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4| GPIO_PIN_5 | GPIO_PIN_8 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOG, &gpio_init_structure);

  /* GPIOH configuration */  
  gpio_init_structure.Pin   = GPIO_PIN_3 | GPIO_PIN_5;
  HAL_GPIO_Init(GPIOH, &gpio_init_structure); 
  
  /* Configure common DMA parameters */
  dma_handle.Init.Channel             = SDRAM_DMAx_CHANNEL;
  dma_handle.Init.Direction           = DMA_MEMORY_TO_MEMORY;
  dma_handle.Init.PeriphInc           = DMA_PINC_ENABLE;
  dma_handle.Init.MemInc              = DMA_MINC_ENABLE;
  dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  dma_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
  dma_handle.Init.Mode                = DMA_NORMAL;
  dma_handle.Init.Priority            = DMA_PRIORITY_HIGH;
  dma_handle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
  dma_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  dma_handle.Init.MemBurst            = DMA_MBURST_SINGLE;
  dma_handle.Init.PeriphBurst         = DMA_PBURST_SINGLE; 
  
  dma_handle.Instance = SDRAM_DMAx_STREAM;
  
   /* Associate the DMA handle */
  __HAL_LINKDMA(hsdram, hdma, dma_handle);
  
  /* Deinitialize the stream for new transfer */
  _HAL_DMA_DeInit(&dma_handle);
  
  /* Configure the DMA stream */
  _HAL_DMA_Init(&dma_handle);
  
  /* NVIC configuration for DMA transfer complete interrupt */
  _HAL_NVIC_SetPriority(SDRAM_DMAx_IRQn, 5, 0);
  _HAL_NVIC_EnableIRQ(SDRAM_DMAx_IRQn);
}

/**
  * @brief  DeInitializes SDRAM MSP.
  * @param  hsdram: SDRAM handle
  * @param  Params
  * @retval None
  */
__weak void BSP_SDRAM_MspDeInit(SDRAM_HandleTypeDef  *hsdram, void *Params)
{  
    static DMA_HandleTypeDef dma_handle;
  
    /* Disable NVIC configuration for DMA interrupt */
    _HAL_NVIC_DisableIRQ(SDRAM_DMAx_IRQn);

    /* Deinitialize the stream for new transfer */
    dma_handle.Instance = SDRAM_DMAx_STREAM;
    _HAL_DMA_DeInit(&dma_handle);

    /* GPIO pins clock, FMC clock and DMA clock can be shut down in the applications
       by surcharging this __weak function */ 
}

/**
  * @}
  */  
  
/**
  * @}
  */ 
  
/**
  * @}
  */ 
  
/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
