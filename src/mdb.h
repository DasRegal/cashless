#ifndef MDB_H
#define MDB_H

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
    MDB_VEND_CMD_E      = 3,
} mdb_code_cmd_t;

typedef enum
{
    MDB_VEND_REQ_SUBCMD     = 0,
    MDB_VEND_SUCCESS_SUBCMD = 2,
    MDB_VEND_FAILURE_SUBCMD = 3,
} mdb_vend_subcmd_t;

typedef struct
{
    mdb_code_cmd_t cmd;
    uint8_t subcmd;
} mdb_cmd_t;

typedef struct
{
    void (*MdbQueueCmdPushCB)(*void);
} mdb_t;

void MdbInit(mdb_t mdb_dev);
void MdbVendStart(int item, int price);
int MdbVendCheck(void);
void MdbVendFinish(bool is_vend_success);

#endif /* MDB_H */
