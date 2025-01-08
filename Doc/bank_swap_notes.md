# Bank swap firmware update

## Sequence

1. Unlock OPTLOCK bit, if not already unlocked.
2. Set the new desired SWAP_BANK_OPT value in the FLASH_OPTSR_PRG register.
3. Start the option byte change sequence by setting the OPTSTART bit in the
FLASH_OPTCR register.
4. Once the option byte change has completed, FLASH_OPTSR_CUR contains the
expected SWAP_BANK_OPT value, but SWAP_BANK bit in FLASH_OPTCR has not
yet been modified and the bank swapping is not yet effective.
5. Force a system reset or a POR. When the reset rises up, the bank swapping is
effective (SWAP_BANK value updated in FLASH_OPTCR) and the new firmware shall
be executed.

![flash swap sequence](./img/Flash%20bank%20swapping%20sequence.png)

Notes:
- The SWAP_BANK bit in FLASH_OPTCR is read-only and cannot be modified by the application software.
- The SWAP_BANK_OPT option bit in FLASH_OPTSR_PRG can be modified whatever the
- RDP level (i.e. even in level 2), thus allowing advanced firmware upgrade in any level of readout protection.

Notes 2:
- Maybe this means that the SWAP_BANK bit needs to be changed by a function located in ram `__attribute__()`. -> false, can be in flash


## Setting variables to specific memory sections

```c
ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */
```
