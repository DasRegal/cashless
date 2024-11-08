/**
  *******************************************************************************
  * Includes
  *******************************************************************************
  */
#include <stdbool.h>
#include "cashless.h"
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

/** 
  * @brief  CASHLESS return values
  */
typedef enum
{
    CASHLESS_RET_OK             = 0,
    CASHLESS_RET_HOLD_PURCHASE  = 1,
    CASHLESS_RET_NOT_PURCHASE   = 2,
    CASHLESS_RET_BUSY_PURCHASE  = 3,
    CASHLESS_RET_NO_BALANCE     = 4,
    CASHLESS_RET_ERROR          = 5,
} cashless_ret_t;

typedef struct
{
    bool is_hold_purchase;
} cashless_t;

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
static cashless_t cashless_struct = 
{
    .is_hold_purchase = false,
};

/**
  *******************************************************************************
  * Static Function Prototypes
  *******************************************************************************
  */
static bool CashlessIsHoldPurchase(void);
static void CashlessHoldPurchase(void);
static void CashlessReleasePurchase(void);

/**
  *******************************************************************************
  * Functions
  *******************************************************************************
  */

/**
  * @brief  Initializes module.
  * @param  None
  * @retval None
  */
// extern void QueueCmdPushCB(mdb_cmd_t cmd_struct);
void CashlessInit(void)
{
    cashless_struct.is_hold_purchase = false;

    // static uint8_t serial_number[29] = "ABCSBCA0022    RVM-28-4    10";

    mdb_t mdb_struct;
    mdb_struct.MdbQueueCmdPushCB = QueueCmdPushCB;
    // mdb_struct.MdbQueueCmdPullCB = QueueCmdPullCB;
    // mdb_struct.MdbTxBufCB        = TxBufCB;
    // mdb_struct.MdbReconfigCB     = ReconfigCB;
    // mdb_struct.p_info            = serial_number;
    // mdb_struct.vmc_level         = TEST_VMC_LEVEL;

    MdbInit(mdb_struct);
}

/**
  * @brief  Start purchase process.
  * @param  item
  * @param  price
  * @retval An cashless_ret_t enumuration value:
  *     - CASHLESS_RET_OK: Launching the purchase process
  *     - CASHLESS_RET_HOLD_PURCHASE: Attempting to restart the purchase process without handling
  */
int CashlessMakePurchase(int item, int price)
{
    if ( CashlessIsHoldPurchase() )
    {
        return CASHLESS_RET_HOLD_PURCHASE;
    }

    CashlessHoldPurchase();

    /* Send VEND REQUEST and polling */
    MdbVendStart(item, price);

    return CASHLESS_RET_OK;
}

/**
  * @brief  Check Purchase.
  * @param  None
  * @retval An cashless_ret_t enumuration value:
  *     - CASHLESS_RET_OK:            Launching the purchase process
  *     - CASHLESS_RET_NOT_PURCHASE:  CashlessMakePurchase function was not started
  *     - CASHLESS_RET_BUSY_PURCHASE: Purchase in process. Need to wait for completion (CASHLESS_RET_OK).
  *     - CASHLESS_RET_NO_BALANCE:    Not enough money requested. There is no need to call CashlessFinishPurchase()!
  *     - CASHLESS_RET_ERROR:         Some errors
  */
int CashlessCheckPurchase(void)
{
    mdb_ret_t ret_val;

    if ( !CashlessIsHoldPurchase() )
    {
        return CASHLESS_RET_NOT_PURCHASE;
    }

    /* Waiting for VEND APPROVED or VEND DENIED status*/
    ret_val = MdbVendCheck();

    if (ret_val == MDB_RET_BUSY_PURCHASE)
    {
        return CASHLESS_RET_BUSY_PURCHASE;
    }

    if (ret_val == MDB_RET_NO_BALANCE)
    {
        /* There is no need to call CashlessFinishPurchase()! */
        CashlessReleasePurchase();
        return CASHLESS_RET_NO_BALANCE;
    }

    if (ret_val != MDB_RET_OK)
    {
        /* There is no need to call CashlessFinishPurchase()! */
        CashlessReleasePurchase();
        return CASHLESS_RET_ERROR; 
    }

    CashlessReleasePurchase();
    return CASHLESS_RET_OK;
}

/**
  * @brief  Finish Purchase.
  * @param  is_vend_success:
  *     - true:  send VEND SUCCESS
  *     - false: send VEND FAILURE
  * @retval An cashless_ret_t enumuration value:
  *     - CASHLESS_RET_OK:            Launching the purchase process
  *     - CASHLESS_RET_BUSY_PURCHASE: Purchase in process. Need to wait for completion (CashlessCheckPurchase()).
  */
int CashlessFinishPurchase(bool is_vend_success)
{
    if ( CashlessIsHoldPurchase() )
    {
        return CASHLESS_RET_BUSY_PURCHASE;
    }

    /* Send VEND SUCCESS or VEND FAILURE */
    MdbVendFinish(is_vend_success);

    return CASHLESS_RET_OK;
}

/**
  * @brief  Purchase Hold Check.
  * @param  None
  * @retval Bool value:
  *     - true:  Purchase is on hold. Process started.
  *     - false: The purchase process is complete.
  */
static bool CashlessIsHoldPurchase(void)
{
    if (cashless_struct.is_hold_purchase)
    {
        return true;
    }

    return false;
}

/**
  * @brief  Setting to hold the purchase process.
  * @param  None
  * @retval None
  */
static void CashlessHoldPurchase(void)
{
    cashless_struct.is_hold_purchase = true;
}

/**
  * @brief  Setting the release of the purchasing process.
  * @param  None
  * @retval None
  */
static void CashlessReleasePurchase(void)
{
    cashless_struct.is_hold_purchase = false;
}
