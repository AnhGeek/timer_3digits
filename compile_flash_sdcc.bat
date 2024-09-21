sdcc -mstm8 main.c
packihx code.ihx > main.hex
STVP_CmdLine -BoardName=ST-LINK -ProgMode=SWIM -Tool_ID=0 -Device=STM8S003F3 -progress -verif -FileProg=main.hex