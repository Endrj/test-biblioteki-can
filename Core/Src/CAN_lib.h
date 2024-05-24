/*
 * CAN_lib.h
 *
 * Created on: May 24th, 2024
 * 		Autor: Andrzej & Dominik & Denis
 * 		AiR 2020 & 2021
 *
 */

#ifndef INC_CAN_lib_H_
#define INC_CAN_lib_H_

void CAN_Filter_Config(void);
void CAN1_Tx_info(uint8_t info);

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan);

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan);

static void MX_CAN1_Init(void);


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void CAN_Konfiguracja_main();
void TIM();
void CAN1_Tx_info(uint8_t info);
void CAN_Filter_Config(void);
void initialize_can();
void timfer_cadllbacck();

#endif /*	CAN_lib_H_ 	*/
