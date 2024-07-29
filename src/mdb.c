/**
  *******************************************************************************
  * Includes
  *******************************************************************************
  */
#include <stdbool.h>
#include <stddef.h>
#include "mdb.h"

/**
  *******************************************************************************
  * Defines
  *******************************************************************************
  */

/**
  *******************************************************************************
  * Local Types and Typedefs
  *******************************************************************************
  */

typedef struct
{
    int item;
    int price;
    bool is_start;
} mdb_vend_t;

/**
  *******************************************************************************
  * Macros
  *******************************************************************************
  */

/**
  *******************************************************************************
  * Global Variables
  *******************************************************************************
  */

/**
  *******************************************************************************
  * Static Variables
  *******************************************************************************
  */
static mdb_t mdb_dev = {
    .MdbQueueCmdPushCB = NULL,
    .MdbQueueCmdPullCB = NULL,
};

static mdb_vend_t mdb_vend_struct = { .is_start = false };

/**
  *******************************************************************************
  * Static Function Prototypes
  *******************************************************************************
  */
static void MdbReset(void);

/**
  *******************************************************************************
  * Functions
  *******************************************************************************
  */

/**
  * @brief  Send VEND REQUEST
  * @param  mdb_t MDB device struct
  * @retval None
  */
void MdbInit(mdb_t mdb_struct)
{
    mdb_dev.MdbQueueCmdPushCB = mdb_struct.MdbQueueCmdPushCB;
    mdb_dev.MdbQueueCmdPullCB = mdb_struct.MdbQueueCmdPullCB;
    MdbReset();
}

/**
  * @brief  MDB reset
  * @param  None
  * @retval None
  */
static void MdbReset(void)
{
    mdb_cmd_t mdb_cmd_struct;

    /* RESET */
    mdb_cmd_struct.cmd = MDB_RESET_CMD_E;
    mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);

    /* SETUP */
    mdb_cmd_struct.cmd    = MDB_SETUP_CMD_E;

    mdb_cmd_struct.subcmd = MDB_SETUP_CONFIG_SUBCMD_E;
    mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);

    mdb_cmd_struct.subcmd = MDB_SETUP_PRICE_SUBCMD_E;
    mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);

    /* READER ENABLE */
    mdb_cmd_struct.cmd    = MDB_READER_CMD_E;
    mdb_cmd_struct.subcmd = MDB_READER_ENABLE_SUBCMD_E;
    mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);
}

/**
  * @brief  Send VEND REQUEST
  * @param  item   Item for request
  * @param  price  Price for request
  * @retval None
  */
void MdbVendStart(int item, int price)
{
    mdb_cmd_t mdb_cmd_struct = { .cmd = MDB_VEND_CMD_E, .subcmd = MDB_VEND_REQ_SUBCMD_E };

    if ( mdb_vend_struct.is_start )
    {
        return;
    }

    mdb_vend_struct.is_start = true;
    mdb_vend_struct.item     = item;
    mdb_vend_struct.price    = price;

    if (mdb_dev.MdbQueueCmdPushCB != NULL)
    {
        mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);
    }
}

/**
  * @brief  Check Vend request
  * @param  None
  * @retval An mdb_ret_t enumuration value:
  *     - MDB_RET_OK:            Launching the Vend request
  *     - MDB_RET_NOT_PURCHASE:  MdbVendStart function was not started
  *     - MDB_RET_BUSY_PURCHASE: Vend request in process. Need to wait for completion (MDB_RET_OK).
  *     - MDB_RET_NO_BALANCE:    Return Vend Denied
  */
int MdbVendCheck(void)
{
    if ( !mdb_vend_struct.is_start )
    {
        return MDB_RET_NOT_PURCHASE;
    }

    return MDB_RET_OK;
}

/**
  * @brief  Finish Vend
  * @param  is_vend_success:
  *     - true:  send VEND SUCCESS
  *     - false: send VEND FAILURE
  * @retval None
  */
void MdbVendFinish(bool is_vend_success)
{
    mdb_cmd_t mdb_cmd_struct = { .cmd = MDB_VEND_CMD_E, .subcmd = MDB_VEND_FAILURE_SUBCMD_E };

    if ( !mdb_vend_struct.is_start )
    {
        return;
    }

    if (mdb_dev.MdbQueueCmdPushCB != NULL)
    {
        mdb_dev.MdbQueueCmdPushCB(mdb_cmd_struct);
    }

    mdb_vend_struct.is_start = false;
}

/**
  * @brief  Send Command
  * @param  None
  * @retval None
  */
void MdbSendCmd(void)
{
    
}
