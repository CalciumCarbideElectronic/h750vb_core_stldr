#include "spidriver.h"
#include "stm32h7xx_hal_qspi.h"
#include "quadspi.h"

extern QSPI_HandleTypeDef hqspi;

static void QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi) {
	QSPI_CommandTypeDef sCommand;
	QSPI_AutoPollingTypeDef sConfig;

	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction = WRITE_ENABLE_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.DummyCycles = 0;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		Error_Handler();
	}

	sConfig.Match = 0x02;
	sConfig.Mask = 0x02;
	sConfig.MatchMode = QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize = 1;
	sConfig.Interval = 0x10;
	sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

	sCommand.Instruction = READ_STATUS_REG_CMD;
	sCommand.DataMode = QSPI_DATA_1_LINE;

	if (HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig,
	HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		Error_Handler();
	}
}

static void QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi) {
	QSPI_CommandTypeDef sCommand;
	QSPI_AutoPollingTypeDef sConfig;

	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction = READ_STATUS_REG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.DummyCycles = 0;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	sConfig.Match = 0x00;
	sConfig.Mask = 0x01;
	sConfig.MatchMode = QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize = 1;
	sConfig.Interval = 0x10;
	sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

	if (HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig,1000)!= HAL_OK) {
		Error_Handler();
	}
}

static void initCommonCmd(QSPI_CommandTypeDef *cmd) {
	cmd->InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd->AddressSize = QSPI_ADDRESS_24_BITS;
	cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd->DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd->DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd->SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
}

void sFLASH_DeInit(void) {
	sFLASH_LowLevel_DeInit();
	QSPI_WriteEnable(&hqspi);
}

void sFLASH_Init(void) {
	QSPI_WriteEnable(&hqspi);
	QSPI_AutoPollingMemReady(&hqspi);
}

void sFLASH_EraseSector(uint32_t SectorAddr) {
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	sCommand.Instruction = SECTOR_ERASE_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
	sCommand.Address = SectorAddr;
	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.DummyCycles = 0;
	if (HAL_QSPI_Command_IT(&hqspi, &sCommand) != HAL_OK) {
		Error_Handler();
	}
	QSPI_AutoPollingMemReady(&hqspi);
}

void sFLASH_EraseBulk(void) {
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	sCommand.Instruction = BULK_ERASE_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.DummyCycles = 0;
	if (HAL_QSPI_Command_IT(&hqspi, &sCommand) != HAL_OK) {
		Error_Handler();
	}
	QSPI_AutoPollingMemReady(&hqspi);
}

void sFLASH_WritePage(uint8_t *pBuffer, uint32_t WriteAddr,
		uint32_t NumByteToWrite) {
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	sCommand.Instruction = PAGE_PROG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
	sCommand.DataMode = QSPI_DATA_4_LINES;
	sCommand.NbData = NumByteToWrite;
	sCommand.DummyCycles = 0;
	if (HAL_QSPI_Command_IT(&hqspi, &sCommand) != HAL_OK) {
		Error_Handler();
	}
	HAL_QSPI_Transmit_IT(&hqspi, pBuffer);
	QSPI_AutoPollingMemReady(&hqspi);
}

void sFLASH_WriteBuffer(uint8_t *pBuffer, uint32_t WriteAddr,
		uint32_t NumByteToWrite) {
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	sCommand.Instruction = PAGE_PROG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
	sCommand.Address = WriteAddr;
	sCommand.DataMode = QSPI_DATA_4_LINES;
	sCommand.NbData = NumByteToWrite;
	sCommand.DummyCycles = 0;
	if (HAL_QSPI_Command_IT(&hqspi, &sCommand) != HAL_OK) {
		Error_Handler();
	}
	HAL_QSPI_Transmit_IT(&hqspi, pBuffer);
	QSPI_AutoPollingMemReady(&hqspi);
}

void sFLASH_ReadBuffer(uint8_t *pBuffer, uint32_t ReadAddr,
		uint32_t NumByteToRead) {
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	sCommand.Instruction = READ_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
	sCommand.Address = ReadAddr;
	sCommand.DataMode = QSPI_DATA_4_LINES;
	sCommand.NbData = NumByteToRead;
	sCommand.DummyCycles = 0;
	if (HAL_QSPI_Command_IT(&hqspi, &sCommand) != HAL_OK) {
		Error_Handler();
	}
	HAL_QSPI_Receive_IT(&hqspi, pBuffer);
	QSPI_AutoPollingMemReady(&hqspi);
}

uint32_t sFLASH_ReadID(void) {
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	sCommand.Instruction = READ_ID_CMD2;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.NbData = 3;
	sCommand.DummyCycles = 0;
	if (HAL_QSPI_Command(&hqspi, &sCommand,1000) != HAL_OK) {
		Error_Handler();
	}
	uint8_t pData[4];
	HAL_QSPI_Receive(&hqspi,pData,1000);
	uint32_t res;
	res = (pData[0] << 16) | (pData[1] << 8) | pData[2];
	return res;

}

