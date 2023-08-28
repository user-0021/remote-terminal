#pragma once

/**
 * @brief nviron data update
 * 
 * @param [in] key  environ key 
 * @param [in] value  environ data
 * @return int (success = 0,failed = -1)
 */
int EnvironUpdate(const char* const key,const char* const value);

/**
 * @brief Get the Environ object
 * 
 * @param [in] key element key 
 * 
 * @return char* environ obj 
 */
const char* EnvironGet(const char* const key);