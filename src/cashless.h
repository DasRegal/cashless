#ifndef CASHLESS_H
#define CASHLESS_H

/**
  *******************************************************************************
  * Exported Functions
  *******************************************************************************
  */
void CashlessInit(void);
int CashlessMakePurchase(int item, int price);
int CashlessCheckPurchase(void);
int CashlessFinishPurchase(bool is_vend_success);

#endif /* CASHLESS_H */
