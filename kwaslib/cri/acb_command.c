#include "acb_command.h"

#include <stdlib.h>
#include <string.h>

ACB_COMMAND* acb_command_parse_data(const uint8_t* data, const uint32_t data_size)
{
	FU_FILE* data_fu = fu_create_mem_file_data(data, data_size);
	
	ACB_COMMAND* cmd = acb_command_check_data(data_fu);
	acb_command_print(cmd);
	
	fu_close(data_fu);
	free(data_fu);
	
	return cmd;
}

FU_FILE* acb_command_to_fu(ACB_COMMAND* cmd)
{
	FU_FILE* data_fu = fu_alloc_file();
	fu_create_mem_file(data_fu);
	
	for(uint32_t i = 0; i != cmd->opcodes_count; ++i)
	{
		const ACB_COMMAND_OPCODE* cur_op = &cmd->opcodes[i];
		fu_write_u16(data_fu, cur_op->op, FU_BIG_ENDIAN);
		fu_write_u8(data_fu, cur_op->size);

		switch(cur_op->type)
		{
			case OPCODE_TYPE_NOVAL:
				break;
			case OPCODE_TYPE_UINT8:
				fu_write_u8(data_fu, cur_op->data.u8);
				break;
			case OPCODE_TYPE_UINT16:
				fu_write_u16(data_fu, cur_op->data.u16, FU_BIG_ENDIAN);
				break;
			case OPCODE_TYPE_UINT32:
				fu_write_u32(data_fu, cur_op->data.u32, FU_BIG_ENDIAN);
				break;
			case OPCODE_TYPE_UINT64:
				fu_write_u64(data_fu, cur_op->data.u64, FU_BIG_ENDIAN);
				break;
			case OPCODE_TYPE_FLOAT:
				fu_write_f32(data_fu, cur_op->data.f32, FU_BIG_ENDIAN);
				break;
			case OPCODE_TYPE_DOUBLE:
				fu_write_f64(data_fu, cur_op->data.f64, FU_BIG_ENDIAN);
				break;
			case OPCODE_TYPE_UINT24:
				fu_write_data(data_fu, (uint8_t*)&cur_op->data.u32, 3);
				break;
			case OPCODE_TYPE_UINT40:
				fu_write_data(data_fu, (uint8_t*)&cur_op->data.u64, 5);
				break;
			case OPCODE_TYPE_UINT48:
				fu_write_data(data_fu, (uint8_t*)&cur_op->data.u64, 6);
				break;
			case OPCODE_TYPE_UINT56:
				fu_write_data(data_fu, (uint8_t*)&cur_op->data.u64, 7);
				break;
		}
	}

	return data_fu;
}

ACB_COMMAND* acb_command_check_data(FU_FILE* data_fu)
{
	ACB_COMMAND* cmd = acb_command_alloc_command();

	uint8_t status = FU_SUCCESS;
	
	while(1)
	{
		uint16_t opcode = fu_read_u16(data_fu, &status, FU_BIG_ENDIAN);
		
		if(status != FU_SUCCESS)
		{
			break;
		}

		ACB_COMMAND_OPCODE cur_op = {0};
		cur_op.op = opcode;
		cur_op.size = fu_read_u8(data_fu, NULL);
				
		switch(opcode)
		{
			case OPCODE_NOP:
			case OPCODE_BLOCK_END:
			case OPCODE_CATEGORY:
			case OPCODE_VOLUME_GAIN_RESOLUTION100:
			case OPCODE_UNK_006F:
			case OPCODE_NOTE_ON:
			case OPCODE_DELAY:
			case OPCODE_UNK_07D5:
			
				switch(cur_op.size)
				{
					case 0:
						cur_op.type = OPCODE_TYPE_NOVAL;
						break;
					case 1:
						cur_op.data.u8 = fu_read_u8(data_fu, NULL);
						cur_op.type = OPCODE_TYPE_UINT8;
						break;
					case 2:
						cur_op.data.u16 = fu_read_u16(data_fu, NULL, FU_BIG_ENDIAN);
						cur_op.type = OPCODE_TYPE_UINT16;
						break;
					case 4:
						cur_op.data.u32 = fu_read_u32(data_fu, NULL, FU_BIG_ENDIAN);
						cur_op.type = OPCODE_TYPE_UINT32;
						break;
					case 8:
						cur_op.data.u64 = fu_read_u64(data_fu, NULL, FU_BIG_ENDIAN);
						cur_op.type = OPCODE_TYPE_UINT64;
						break;
					case 3:
						fu_read_data(data_fu, (uint8_t*)&cur_op.data.u32, 3, NULL);
						cur_op.type = OPCODE_TYPE_UINT24;
						break;
					case 5:
						fu_read_data(data_fu, (uint8_t*)&cur_op.data.u64, 5, NULL);
						cur_op.type = OPCODE_TYPE_UINT40;
						break;
					case 6:
						fu_read_data(data_fu, (uint8_t*)&cur_op.data.u64, 6, NULL);
						cur_op.type = OPCODE_TYPE_UINT48;
						break;
					case 7:
						fu_read_data(data_fu, (uint8_t*)&cur_op.data.u64, 7, NULL);
						cur_op.type = OPCODE_TYPE_UINT56;
						break;
				}

				break;
			
			/* float */
			case OPCODE_POS_3D_DISTANCE_MIN:
			case OPCODE_POS_3D_DISTANCE_MAX:

				switch(cur_op.size)
				{
					case 4:
						cur_op.data.f32 = fu_read_f32(data_fu, NULL, FU_BIG_ENDIAN);
						cur_op.type = OPCODE_TYPE_FLOAT;
						break;
					case 8:
						cur_op.data.f64 = fu_read_f64(data_fu, NULL, FU_BIG_ENDIAN);
						cur_op.type = OPCODE_TYPE_DOUBLE;
						break;
				}

				break;
			
			/* Found unknown opcode */
			default:
				printf("!!! UNKNOWN ACB OPCODE %u OF SIZE %u !!!\n", cur_op.op, cur_op.size);
				
				switch(cur_op.size)
				{
					case 0:
						cur_op.type = OPCODE_TYPE_NOVAL;
						break;
					case 1:
						cur_op.data.u8 = fu_read_u8(data_fu, NULL);
						cur_op.type = OPCODE_TYPE_UINT8;
						break;
					case 2:
						cur_op.data.u16 = fu_read_u16(data_fu, NULL, FU_BIG_ENDIAN);
						cur_op.type = OPCODE_TYPE_UINT16;
						break;
					case 4:
						cur_op.data.u32 = fu_read_u32(data_fu, NULL, FU_BIG_ENDIAN);
						cur_op.type = OPCODE_TYPE_UINT32;
						break;
					case 8:
						cur_op.data.u64 = fu_read_u64(data_fu, NULL, FU_BIG_ENDIAN);
						cur_op.type = OPCODE_TYPE_UINT64;
						break;
					case 3:
						fu_read_data(data_fu, (uint8_t*)&cur_op.data.u32, 3, NULL);
						cur_op.type = OPCODE_TYPE_UINT24;
						break;
					case 5:
						fu_read_data(data_fu, (uint8_t*)&cur_op.data.u64, 5, NULL);
						cur_op.type = OPCODE_TYPE_UINT40;
						break;
					case 6:
						fu_read_data(data_fu, (uint8_t*)&cur_op.data.u64, 6, NULL);
						cur_op.type = OPCODE_TYPE_UINT48;
						break;
					case 7:
						fu_read_data(data_fu, (uint8_t*)&cur_op.data.u64, 7, NULL);
						cur_op.type = OPCODE_TYPE_UINT56;
						break;
				}
				break;
		}
		
		/* Something went wrong or further data is not a valid opcode*/
		if(opcode == -1)
		{
			break;
		}
		else /* Append the cur_op to opcodes in acb command */
		{
			/* Basically realloc, but i've had programs crash when used it */
			
			const uint32_t old_count = cmd->opcodes_count;
			const uint32_t old_size = sizeof(ACB_COMMAND_OPCODE)*old_count;
			
			cmd->opcodes_count += 1;
			
			ACB_COMMAND_OPCODE* old_ptr = cmd->opcodes;
			cmd->opcodes = acb_command_alloc_opcodes(cmd->opcodes_count);
			
			if(old_count != 0)
			{
				memcpy(cmd->opcodes, old_ptr, old_size);
				free(old_ptr);
			}
			
			/* Copy the opcode descriptor */
			memcpy(&cmd->opcodes[old_count], &cur_op, sizeof(ACB_COMMAND_OPCODE));
		}
	}

	return cmd;
}

ACB_COMMAND* acb_command_alloc_command()
{
	return (ACB_COMMAND*)calloc(1, sizeof(ACB_COMMAND));
}

ACB_COMMAND_OPCODE* acb_command_alloc_opcodes(const uint32_t amount)
{
	return (ACB_COMMAND_OPCODE*)calloc(amount, sizeof(ACB_COMMAND_OPCODE));
}

void acb_command_free(ACB_COMMAND* cmd)
{
	free(cmd->opcodes);
	cmd->opcodes_count = 0;
}

void acb_command_print(ACB_COMMAND* cmd)
{
	printf("=====ACB CMD=====\n");
	printf("OPCODES_COUNT: %u\n", cmd->opcodes_count);
	
	for(uint32_t i = 0; i != cmd->opcodes_count; ++i)
	{
		const ACB_COMMAND_OPCODE* cur_op = &cmd->opcodes[i];
		
		printf("ACB CMD OPCODE: %hu\n", cur_op->op);
		
		switch(cur_op->type)
		{
			case OPCODE_TYPE_NOVAL:
				printf("\tValue size is 0\n");
				break;
			case OPCODE_TYPE_UINT8:
				printf("\tValue: %hhu\n", cur_op->data.u8);
				break;
			case OPCODE_TYPE_UINT16:
				printf("\tValue: %hu\n", cur_op->data.u16);
				break;
			case OPCODE_TYPE_UINT32:
				printf("\tValue: %u\n", cur_op->data.u32);
				break;
			case OPCODE_TYPE_UINT64:
				printf("\tValue: %llu\n", cur_op->data.u64); 
				break;
			case OPCODE_TYPE_FLOAT:
				printf("\tValue: %f\n", cur_op->data.f32);
				break;
			case OPCODE_TYPE_DOUBLE:
				printf("\tValue: %lf\n", cur_op->data.f64);
				break;
			case OPCODE_TYPE_UINT24:
				printf("\tValue: %u\n", cur_op->data.u24);
				break;
			case OPCODE_TYPE_UINT40:
				printf("\tValue: %lu\n", cur_op->data.u40);
				break;
			case OPCODE_TYPE_UINT48:
				printf("\tValue: %lu\n", cur_op->data.u48);
				break;
			case OPCODE_TYPE_UINT56:
				printf("\tValue: %lu\n", cur_op->data.u56);
				break;
		}
	}
	
	printf("===ACB CMD END===\n");
}