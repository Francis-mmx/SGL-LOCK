@echo off
apu_make.exe -addr 0x1000000 -infile sdram.bin  -ofile sdram.apu -compress

echo �����ļ�  -bfumode 1
echo ��Դ�ļ�  -bfumode 2
echo ����+��Դ -bfumode 3

isd_download.exe  -cfg isd_tools.cfg -input uboot.boot ver.bin sdram.apu -resource res audlogo 32 -disk file -aline 4096 -bfumode 3 -dev upgrade_bfu

del upgrade\*.bfu upgrade\*.bin

copy jl_isd.bfu upgrade\JL_AC5X.bfu
del jl_isd.bfu 
del sdram.apu
echo.
echo �����ļ���upgradeĿ¼�£���upgradeĿ¼�µ������ļ�copy��SD���ĸ�Ŀ¼���忨�ϵ缴��������2���Ӻ��Զ��رմ���
echo.
choice /t 2 /d y /n >nul 
::pause