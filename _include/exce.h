#pragma once

#include "tokenizer.h"

/**
 * @brief excute command block
 * @details if not done last excute wait done
 * 
 * @param [in] block 
 */
void exceCommandBlock(command_block * block);

/**
 * @brief wait command
 * 
 */
void exceCommandWait();

/**
 * @brief check excute status
 * 
 * @return bool
 */
bool exceCommandIsDone();

/**
 * @brief send sigint
 * 
 */
void exceCommandSendSigint();