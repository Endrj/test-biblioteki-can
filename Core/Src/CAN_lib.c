/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : CAN_lib
  * @brief          : Library
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 .
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */




#include "main.h"
#include <stdio.h>


//====================================================================================================================================

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAN_RxHeaderTypeDef RxHeader;


uint8_t operating_mode = 1;
uint8_t v_engines = 0;
uint8_t v_servo_controllers = 0;
// 0 stan bezpieczny
// 1 stan pracy

CAN_HandleTypeDef hcan1;



int error =0;
void CAN_Filter_Config(void);
void CAN1_Tx_info(uint8_t info);

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan);

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan);

static void MX_CAN1_Init(void);



void CAN_Konfiguracja_main(){
	CAN_Filter_config();

}


// ustawienie filtra dla ID ramek CAN
void CAN_Filter_Config(void)
{
	CAN_FilterTypeDef can1_filter_init;

	can1_filter_init.FilterActivation = ENABLE;
	can1_filter_init.FilterBank = 0;
	can1_filter_init.FilterFIFOAssignment = CAN_RX_FIFO0;
	can1_filter_init.FilterIdHigh = 0x0000;
	can1_filter_init.FilterIdLow = 0x0000;
	can1_filter_init.FilterMaskIdHigh = 0X0000;
	can1_filter_init.FilterMaskIdLow = 0x0000;
	can1_filter_init.FilterMode = CAN_FILTERMODE_IDMASK;
	can1_filter_init.FilterScale = CAN_FILTERSCALE_32BIT;

	if(HAL_CAN_ConfigFilter(&hcan1, &can1_filter_init) != HAL_OK)
	{
		Error_Handler();
	}
}

//void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
void cantick(){
{
	uint8_t rcvd_msg[8];
	char msg[50];

	//odbieranie wiadomości CAN
	if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, rcvd_msg) != HAL_OK) //oryginalnie zamiast &hcan1 było hcan
	{
		Error_Handler();
	}

	//dane do układu jezdnego
	if(RxHeader.StdId == 0x0A1 && RxHeader.RTR == CAN_RTR_DATA && operating_mode == 1)
	{
		//kierunek
		if(rcvd_msg[0] == 1)
		{
			v_engines = rcvd_msg[1];
			printf("Forward, velocity: %d\n\r " , rcvd_msg[1]);
			HAL_Delay(15);
			CAN1_Tx_info(0x1);
		}
		else if(rcvd_msg[0] == 2)
		{
			v_engines = rcvd_msg[1];
			printf("Backward, velocity: %d\n\r " , rcvd_msg[1]);
			HAL_Delay(15);
			CAN1_Tx_info(0x2);
		}
		else if(rcvd_msg[0] == 3)
		{
			printf("Turning right angle: %d\n\r " , rcvd_msg[1]);
			HAL_Delay(15);
			CAN1_Tx_info(0x3);
		}
		else if(rcvd_msg[0] == 4)
		{
			printf("Turning left angle: %d\n\r " , rcvd_msg[1]);
			HAL_Delay(15);
			CAN1_Tx_info(0x4);
		}
	}

	//dane do manipulatora
	else if(RxHeader.StdId == 0x0B1 && RxHeader.RTR == CAN_RTR_DATA && operating_mode == 1)
	{
		v_servo_controllers = 10;
		printf("Moving manipulator to coordinates X,Y,Z: \n\r ");
		printf("X: %d\n\r " , rcvd_msg[0]);
		printf("Y: %d\n\r " , rcvd_msg[1]);
		printf("Z: %d\n\r " , rcvd_msg[2]);
		HAL_Delay(200);
		CAN1_Tx_info(0x5);
		v_servo_controllers = 0;
	}

	//ramka safety, wyłączenie napędów
	else if(RxHeader.StdId == 0x00A && RxHeader.RTR == CAN_RTR_DATA)
	{
		if(rcvd_msg[7] == 0x1)
		{
			//wylaczenie napedow
			operating_mode = 0;
			v_engines = 0;
			v_servo_controllers = 0;
		}
		else if(rcvd_msg[7] == 0x0)
		{
			//wlaczenie napedow
			operating_mode = 1;
		}

	}
	else if(RxHeader.StdId == 0x1A2 && rcvd_msg[0] != 1 )
	{
		operating_mode = 0;
		v_engines = 0;
		v_servo_controllers = 0;

	}
	else if(RxHeader.StdId == 0x0C1 && RxHeader.RTR == CAN_RTR_DATA)
	{
		printf("Wysylanie informacji diagnostycznych \n\r ");

		CAN_TxHeaderTypeDef TxHeader;
		uint32_t TxMailbox;
		uint8_t message[8] = {7,v_engines,v_servo_controllers,0,0,0,0,0};

		TxHeader.DLC	 = 8;
		TxHeader.StdId	 = 0x1A3;
		TxHeader.IDE	 = CAN_ID_STD;
		TxHeader.RTR	 = CAN_RTR_DATA;

		if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &message, &TxMailbox) != HAL_OK)
		{
			Error_Handler();
		}
	}

}

void CAN1_Tx_info(uint8_t info)
{
	CAN_TxHeaderTypeDef TxHeader;
	uint32_t TxMailbox;
	uint8_t message = info;

	TxHeader.DLC	 = 1;
	TxHeader.StdId	 = 0x1A3;
	TxHeader.IDE	 = CAN_ID_STD;
	TxHeader.RTR	 = CAN_RTR_DATA;

	if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &message, &TxMailbox) != HAL_OK)
	{
		Error_Handler();
	}
}





//static void MX_CAN1_Init(void){
void initialize_can(){
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 10;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
}
}

void timfer_cadllbacck(){

		CAN_TxHeaderTypeDef TxHeader;
		uint32_t TxMailbox;
		uint8_t message = 0xAB;

		TxHeader.DLC	 = 8;
		TxHeader.StdId	 = 0x2A3;
		//TxHeader.StdId	 = 0x0A1;
		TxHeader.IDE	 = CAN_ID_STD;
		TxHeader.RTR	 = CAN_RTR_DATA;

		if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &message, &TxMailbox) != HAL_OK)
		{
			Error_Handler();
		}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//co trzbeba raz przepuścic w ramach konfiguracji can ??????????????


void konfiguracja()  //dać przed while
{


// \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
  //konfiguracja filtrów i sprawdzenie poprawności modułu CAN
	CAN_Filter_Config();
	if(HAL_CAN_ActivateNotification(&hcan1,CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_BUSOFF ) != HAL_OK)
	{
		Error_Handler();
	}

	//Start modułu CAN
	if(HAL_CAN_Start(&hcan1) != HAL_OK)
	{
		Error_Handler();
	}

//====================================================================================================================================
}






















