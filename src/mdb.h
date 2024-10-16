#ifndef MDB_H
#define MDB_H

#include <stdint.h>

#define MDB_MAX_BUF_LEN                 36
#define MDB_FORCE_CMD                   true
#define MDB_NO_FORCE_CMD                false

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
    MDB_POLL_CMD_E      = 2,
    MDB_VEND_CMD_E      = 3,
    MDB_READER_CMD_E    = 4,
    MDB_EXPANSION_CMD_E = 7,

    MDB_EMPTY_CMD_E     = 0xF0,
    MDB_ACK_CMD_E       = 0xF1,
    MDB_NAK_CMD_E       = 0xF2,
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

typedef enum
{
    MDB_REQUEST_ID_SUBCMD_E    = 0,
} mdb_expansion_subcmd_t;

typedef struct
{
    mdb_code_cmd_t  cmd;
    uint8_t         subcmd;
    bool            is_force;
} mdb_cmd_t;

typedef struct
{   int8_t          manufact_code[3];
    uint8_t         serial_num[12];
    uint8_t         model_num[12];
    uint16_t        sw_version;
    uint8_t         max_resp_time;
    uint8_t         cashless_level;
} mdb_conf_t;

typedef struct
{
    void            (*MdbQueueCmdPushCB)(mdb_cmd_t mdb_cmd_struct);
    mdb_cmd_t       (*MdbQueueCmdPullCB)(void);
    void            (*MdbTxBufCB)(const uint16_t*, uint8_t);
    void            (*MdbReconfigCB)(void* p_data);
    bool            is_wait_rx;
    uint16_t        tx_data[MDB_MAX_BUF_LEN];
    uint16_t        rx_data[MDB_MAX_BUF_LEN];
    uint8_t         rx_data_len;
    mdb_conf_t      conf;
    uint32_t        amount;
    uint8_t         *p_info;
    uint8_t         vmc_level;
} mdb_t;

void MdbInit(mdb_t mdb_struct);
void MdbVendStart(int item, int price);
int  MdbVendCheck(void);
void MdbVendFinish(bool is_vend_success);
void MdbPollSend(void);
void MdbPollReceive(uint16_t ch);

#endif /* MDB_H */
