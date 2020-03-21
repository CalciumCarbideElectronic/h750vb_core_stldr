#include "spidriver.h"
#include "stm32h7xx_hal_qspi.h"
#include "quadspi.h"

extern QSPI_HandleTypeDef hqspi;
static void initCommonCmd(QSPI_CommandTypeDef *cmd) {

  cmd->Instruction=QSPI_INSTRUCTION_1_LINE;
  cmd->InstructionMode=QSPI_INSTRUCTION_1_LINE;

  cmd->AddressMode=QSPI_ADDRESS_NONE;
  cmd->Address=0x0;
  cmd->AddressSize=0;


  cmd->AlternateByteMode=QSPI_ALTERNATE_BYTES_NONE;
  cmd->AlternateBytesSize=0;
  cmd->AlternateBytes=0;


  cmd->DataMode=QSPI_DATA_NONE;
  cmd->NbData=0;

  cmd->DummyCycles=0;

  cmd->DdrMode=QSPI_DDR_MODE_DISABLE;
  cmd->DdrHoldHalfCycle=0;

  cmd->SIOOMode=QSPI_SIOO_INST_EVERY_CMD;
}

static void initReadRegCmd(QSPI_CommandTypeDef *cmd,uint32_t regLen){

	initCommonCmd(cmd);
	cmd->AddressMode = QSPI_ADDRESS_NONE;

	cmd->DataMode = QSPI_DATA_1_LINE;
	cmd->NbData = regLen;

	cmd->DummyCycles = 0;

}

void QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi) {
	QSPI_CommandTypeDef sCommand;
	QSPI_AutoPollingTypeDef sConfig;
	initCommonCmd(&sCommand);

	sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;
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


	sCommand.Instruction =  READ_STATUS_REG_CMD;
	sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.DataMode = QSPI_DATA_4_LINES;
	sCommand.NbData = 1;
	sCommand.DummyCycles = 0;

	if (HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig,
	HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		Error_Handler();
	}
}

void QSPI_AutoPollingMemReady() {
	QSPI_CommandTypeDef sCommand;
	QSPI_AutoPollingTypeDef sConfig;
	initCommonCmd(&sCommand);

	sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	sCommand.Instruction = READ_STATUS_REG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = QSPI_DATA_4_LINES;
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

	if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig,100000)!= HAL_OK) {
		Error_Handler();
	}
}



void sFLASH_MemoryMapped(){
	QSPI_CommandTypeDef cmd;
	QSPI_MemoryMappedTypeDef cfg;
	initCommonCmd(&cmd);

	cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	cmd.Instruction =   0xEB;

	cmd.AddressMode = QSPI_ADDRESS_4_LINES;
	cmd.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd.Address=0;

	cmd.AlternateByteMode =  QSPI_ALTERNATE_BYTES_4_LINES;
	cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd.AlternateBytes = 0xA;

	cmd.DummyCycles = 4;

	cmd.DataMode =QSPI_DATA_4_LINES;
	cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd.DdrHoldHalfCycle = QUADSPI_CCR_DCYC_0;

	cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
	cfg.TimeOutPeriod = 0x20;

	cmd.NbData=0;

	if (HAL_QSPI_MemoryMapped(&hqspi, &cmd, &cfg)!=HAL_OK){
		Error_Handler();
	}
}

void sFLASH_DeInit(void) {
	QSPI_WriteEnable(&hqspi);
}


void sFLASH_Reset(void){
	QSPI_CommandTypeDef cmd;
	initCommonCmd(&cmd);
	cmd.AddressMode=QSPI_ADDRESS_NONE;
	cmd.Instruction=CHIP_RESET_ENABLE;
	if (HAL_QSPI_Command(&hqspi, &cmd,1000) != HAL_OK) {
		Error_Handler();
	}
	cmd.Instruction=CHIP_RESET;
	if (HAL_QSPI_Command(&hqspi, &cmd,1000) != HAL_OK) {
		Error_Handler();
	}


}

void sFLASH_EraseSector(uint32_t SectorAddr) {

  sFLASH_InstanceCmd1(QSPICMD_ENTER_WRITE);
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	//64KB Erase
	sCommand.Instruction = SECTOR_ERASE_CMD;
	sCommand.InstructionMode= QSPI_INSTRUCTION_4_LINES;
	sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
	sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
	sCommand.Address = SectorAddr;
	sCommand.DummyCycles = 0;
	if (HAL_QSPI_Command(&hqspi, &sCommand,1000) != HAL_OK) {
		Error_Handler();
	}
	QSPI_AutoPollingMemReady(&hqspi);
}

void sFLASH_EraseBulk(void) {
	for(int i=0;i<32;i++){
		sFLASH_EraseSector(i<<16);
	}





// Too slow
//    sFLASH_InstanceCmd1(QSPICMD_ENTER_WRITE);
//	#ifdef DEBUG
//	printf("Exec BulkErase: Status:0x%x READ:0x%x\n",
//			sFLASH_ReadReg1(QSPICMD_READREG_STATUS),
//			sFLASH_ReadReg1(QSPICMD_READREG_READP_V)
//		);
//	#endif
//	QSPI_CommandTypeDef sCommand;
//	initCommonCmd(&sCommand);
//	sCommand.Instruction = BULK_ERASE_CMD;
//	sCommand.InstructionMode= QSPI_INSTRUCTION_4_LINES;
//	if (HAL_QSPI_Command(&hqspi, &sCommand,100000) != HAL_OK) {
//		Error_Handler();
//	}
//	QSPI_AutoPollingMemReady(&hqspi);
//
//	#ifdef DEBUG
//	printf("Done BulkErase:  Extend:0x%x \n",sFLASH_ReadReg1(QSPICMD_READREG_READEXT));
//
//	#endif
}

void sFLASH_WritePage(
		uint8_t *pBuffer,
		uint32_t WriteAddr,
		uint32_t NumByteToWrite) {

    sFLASH_InstanceCmd1(QSPICMD_ENTER_WRITE);
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	sCommand.Instruction = PAGE_PROG_CMD;
	sCommand.InstructionMode= QSPI_INSTRUCTION_4_LINES;

	sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
	sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
	sCommand.Address = WriteAddr;

	sCommand.DataMode = QSPI_DATA_4_LINES;
	sCommand.NbData = NumByteToWrite;

	sCommand.DummyCycles=0;

	sCommand.SIOOMode=QSPI_SIOO_INST_ONLY_FIRST_CMD;

	if (HAL_QSPI_Command(&hqspi, &sCommand,1000) != HAL_OK) {
		Error_Handler();
	}

	HAL_QSPI_Transmit(&hqspi, pBuffer,1000);
	QSPI_AutoPollingMemReady(&hqspi); }

int sFLASH_WriteBuffer(
		uint32_t WriteAddr,
		uint32_t NumByteToWrite,
		uint8_t *pBuffer
		) {

    sFLASH_InstanceCmd1(QSPICMD_ENTER_WRITE);
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	sCommand.Instruction = PAGE_PROG_CMD;

	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
	sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
	sCommand.Address = WriteAddr;



	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.NbData = NumByteToWrite;

	sCommand.DummyCycles = 0;
	if (HAL_QSPI_Command(&hqspi, &sCommand,1000) != HAL_OK) {
		Error_Handler();
		return 0;
	}
	HAL_QSPI_Transmit(&hqspi, pBuffer,1000);
	QSPI_AutoPollingMemReady(&hqspi);
	return 1;
}

void sFLASH_ReadBuffer(uint8_t *pBuffer, uint32_t ReadAddr,
		uint32_t NumByteToRead) {
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	sCommand.Instruction = 0x0B;
	sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
	sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
	sCommand.Address = ReadAddr;


	sCommand.DataMode = QSPI_DATA_4_LINES;
	sCommand.NbData = NumByteToRead;

	sCommand.DummyCycles = 6;
	if (HAL_QSPI_Command(&hqspi, &sCommand,1000) != HAL_OK) {
		Error_Handler();
	}
	HAL_QSPI_Receive(&hqspi, pBuffer,1000);
	QSPI_AutoPollingMemReady(&hqspi);
}

uint32_t sFLASH_ReadID(void) {
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);

	sCommand.Instruction = 0xAF;
	sCommand.InstructionMode= QSPI_INSTRUCTION_4_LINES;

	sCommand.AddressMode = QSPI_ADDRESS_NONE;

	sCommand.DataMode = QSPI_DATA_4_LINES;
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


uint8_t sFLASH_ReadReg1(uint8_t ins) {
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);

	sCommand.Instruction = ins;
	sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	sCommand.AddressMode = QSPI_ADDRESS_NONE;

	sCommand.DataMode = QSPI_DATA_4_LINES;
	sCommand.NbData = 1;
	sCommand.DummyCycles = 0;

	if (HAL_QSPI_Command(&hqspi, &sCommand,1000) != HAL_OK) {
		Error_Handler();
	}
	uint8_t pData;
	HAL_QSPI_Receive(&hqspi,&pData,1000);
	return  pData;
}

uint8_t sFLASH_SetReg1(uint8_t ins, uint8_t payload){
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);

	sCommand.Instruction = ins;
	sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AddressSize =QSPI_ADDRESS_8_BITS;

	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;

	sCommand.DataMode =  QSPI_DATA_4_LINES;
	sCommand.NbData = 1;
	sCommand.DummyCycles = 0;

	if (HAL_QSPI_Command(&hqspi, &sCommand,1000) != HAL_OK) {
		Error_Handler();
	}
	HAL_QSPI_Transmit(&hqspi,&payload,1000);
}

uint8_t sFLASH_SetReg1SingleLine(uint8_t ins, uint8_t payload){
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	sCommand.Instruction = ins;
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;

	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AddressSize = QSPI_ADDRESS_8_BITS;
	sCommand.Address =0x0;

	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	sCommand.AlternateBytes =0x0;

	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_HALF_CLK_DELAY;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;

	sCommand.SIOOMode  = QSPI_SIOO_INST_EVERY_CMD;

	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.NbData = 1;
	sCommand.DummyCycles = 0;
	if (HAL_QSPI_Command(&hqspi, &sCommand,1000) != HAL_OK) {
		Error_Handler();
	}
	HAL_QSPI_Transmit(&hqspi,&payload,1000);
    QSPI_AutoPollingMemReadySingleLine();
}


void sFLASH_QPIMODE(uint8_t enable){
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	if(enable){
		sCommand.Instruction =  QSPICMD_ENTER_QUAD;
		sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	} else{
		sCommand.Instruction =  QSPICMD_EXIT_QUAD;
		sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	}

	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.NbData = 0;
	sCommand.DummyCycles = 0;
	if (HAL_QSPI_Command(&hqspi, &sCommand,1000) != HAL_OK) {
		Error_Handler();
	}

}

void sFLASH_InstanceCmd1SingleLine(uint8_t ins){
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	sCommand.Instruction = ins;
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;

	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.NbData = 0;
	sCommand.DummyCycles = 0;
	if (HAL_QSPI_Command(&hqspi, &sCommand,1000) != HAL_OK) {
		Error_Handler();
	}

	QSPI_AutoPollingMemReadySingleLine();

}

void sFLASH_InstanceCmd1(uint8_t ins){
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);
	sCommand.Instruction = ins;
	sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.NbData = 0;
	sCommand.DummyCycles = 0;
	if (HAL_QSPI_Command(&hqspi, &sCommand,1000) != HAL_OK) {
		Error_Handler();
	}

}


uint32_t sFLASH_ReadIDSingleLine(void) {
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);

	sCommand.Instruction = 0x9F;
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;

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

void QSPI_AutoPollingMemReadySingleLine() {
	QSPI_CommandTypeDef sCommand;
	QSPI_AutoPollingTypeDef sConfig;
	initCommonCmd(&sCommand);

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

	if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig,1000)!= HAL_OK) {
		Error_Handler();
	}
}

uint8_t sFLASH_ReadReg1SingleLine(uint8_t ins) {
	QSPI_CommandTypeDef sCommand;
	initCommonCmd(&sCommand);

	sCommand.Instruction = ins;
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;

	sCommand.AddressMode = QSPI_ADDRESS_NONE;

	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.NbData = 1;
	sCommand.DummyCycles = 0;

	if (HAL_QSPI_Command(&hqspi, &sCommand,1000) != HAL_OK) {
		Error_Handler();
	}
	uint8_t pData;
	HAL_QSPI_Receive(&hqspi,&pData,1000);
	return  pData;
}


void sFLASH_Init(void) {
  sFLASH_Reset();

  //If the SRWD is set to “1” and
  //WP# is pulled high (VIH), the Status Register can be changed by a WRSR instruction.
  sFLASH_QPIMODE(0);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);


  //QE: 1 BP[3:0]=0 to disable all protection
  sFLASH_InstanceCmd1SingleLine(QSPICMD_ENTER_WRITE);
  sFLASH_SetReg1SingleLine(QSPICMD_SETREG_STATUS, 0x40);

  //6 Dummy Cycle for qpi mode, no burst mode
  sFLASH_InstanceCmd1SingleLine(QSPICMD_ENTER_WRITE);
  sFLASH_SetReg1SingleLine(QSPICMD_SETREG_READP_NV, 0x30);


#ifdef DEBUG
  printf("Enter QuadMode\n==============================\n");
#endif
  sFLASH_QPIMODE(1);

  QSPI_AutoPollingMemReady();

#ifdef DEBUG
  printf("Joint ID:\t\t\t%x\n",(long int)sFLASH_ReadID());
  printf("Status Register:\t\t\t%x\n",sFLASH_ReadReg1(QSPICMD_READREG_STATUS));
  printf("Function Register:\t\t\t%x\n",sFLASH_ReadReg1(QSPICMD_READREG_FUNCTION));
  printf("Read Parameters:\t\t\t%x\n",sFLASH_ReadReg1(QSPICMD_READREG_READP_V));
#endif


}
