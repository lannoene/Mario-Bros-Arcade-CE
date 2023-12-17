#include "save.h"

#include <fileioc.h>

void SaveCurrentData(save_t data) {
	uint8_t file = ti_Open("MBROSDAT", "w");
	if (file) {
		ti_Write(&data, sizeof(save_t), 1, file);
		ti_SetArchiveStatus(true, file);
	}
	ti_Close(file);
}

save_t GetSaveData(void) {
	uint8_t file = ti_Open("MBROSDAT", "r");
	
	if (file) {
		
		save_t returnStruct;
		
		ti_Read(&returnStruct, sizeof(save_t), 1, file);
		
		return returnStruct;
		ti_Close(file);
	} else {
		save_t returnStruct = {0};
		return returnStruct;
	}
}