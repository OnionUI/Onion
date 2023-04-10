usage: axp register_address [+][write_data]

write if write_data is specified, read if not
set bit (== or) if + is specified
both are specified as hex, 1 byte

See specsheet for register details: https://linux-sunxi.org/images/e/e5/AXP223_Datasheet_V1.0_en.pdf

for example:

- usage + dump all regs
    axp
- read Status
    axp 00
- read Mode and charge status
    axp 01
- read Battery %
    axp B9
- write DCDC1 2800mV
    axp 21 0C
    ## 00 = 1600  01 = 1700  02 = 1800  03 = 1900  04 = 2000  05 = 2100  06 = 2200  07 = 2300  08 = 2400
    ## 09 = 2500  0A = 2600  0B = 2700  0C = 2800  0D = 2900  0E = 3000  0F = 3100  10 = 3200  11 = 3300  12 = 3400 mV
- DCDC1 Enable Set
    axp 10 +02
- DC1SW Enable Set
    axp 12 +80
