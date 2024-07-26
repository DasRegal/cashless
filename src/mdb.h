#ifndef MDB_H
#define MDB_H

#include <stdint.h>
/** 
  * @brief  CASHLESS return values
  */
typedef enum
{
    MDB_RET_OK             = 0,
    MDB_RET_BUSY_PURCHASE  = 1,
    MDB_RET_NO_BALANCE     = 2,
    MDB_RET_NOT_PURCHASE   = 3,
} mdb_ret_t;

typedef enum
{
    MDB_RESET_CMD_E     = 0,
    MDB_SETUP_CMD_E     = 1,
    MDB_VEND_CMD_E      = 3,
    MDB_READER_CMD_E    = 4,
} mdb_code_cmd_t;

typedef enum
{
    MDB_VEND_REQ_SUBCMD_E     = 0,
    MDB_VEND_SUCCESS_SUBCMD_E = 2,
    MDB_VEND_FAILURE_SUBCMD_E = 3,
} mdb_vend_subcmd_t;

typedef enum
{
    MDB_SETUP_CONFIG_SUBCMD_E = 0,
    MDB_SETUP_PRICE_SUBCMD_E  = 1,
} mdb_setup_subcmd_t;

typedef enum
{
    MDB_READER_ENABLE_SUBCMD_E = 1,
} mdb_reader_subcmd_t;

typedef struct
{
    mdb_code_cmd_t cmd;
    uint8_t subcmd;
} mdb_cmd_t;

typedef struct
{
    void (*MdbQueueCmdPushCB)(mdb_cmd_t mdb_cmd_struct);
} mdb_t;

void MdbInit(mdb_t mdb_struct);
void MdbVendStart(int item, int price);
int MdbVendCheck(void);
void MdbVendFinish(bool is_vend_success);

#endif /* MDB_H */
