/**
  *******************************************************************************
  * Includes
  *******************************************************************************
  */
#include <stdbool.h>
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
static mdb_t mdb_struct;
static mdb_vend_t mdb_vend_struct = { .is_start = false };

/**
  *******************************************************************************
  * Static Function Prototypes
  *******************************************************************************
  */

/**
  *******************************************************************************
  * Functions
  *******************************************************************************
  */

/**
  * @brief  Send VEND REQUEST
  * @param  None
  * @retval None
  */
void MdbInit(mdb_t mdb_dev)
{
    mdb_struct.MdbQueueCmdPushCB = mdb_dev.MdbQueueCmdPushCB;
}

/**
  * @brief  Send VEND REQUEST
  * @param  None
  * @retval None
  */
void MdbVendStart(int item, int price)
{
    mdb_cmd_t mdb_cmd_struct = { .cmd = MDB_VEND_CMD_E, .subcmd = MDB_VEND_REQ_SUBCMD };

    if ( mdb_vend_struct.is_start )
    {
        return;
    }

    mdb_vend_struct.is_start = true;
    mdb_vend_struct.item     = item;
    mdb_vend_struct.price    = price;

    mdb_dev.MdbQueueCmdPushCB((mdb_cmd_t*)mdb_cmd_struct);
}

/**
  * @brief  
  * @param  None
  * @retval None
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
  * @brief  
  * @param  None
  * @retval None
  */
void MdbVendFinish(bool is_vend_success)
{
    mdb_cmd_t mdb_cmd_struct = { .cmd = MDB_VEND_CMD_E, .subcmd = MDB_VEND_FAILURE_SUBCMD };

    if ( !mdb_vend_struct.is_start )
    {
        return;
    }

    mdb_dev.MdbQueueCmdPushCB((mdb_cmd_t*)mdb_cmd_struct);
    mdb_vend_struct.is_start = false;
}
