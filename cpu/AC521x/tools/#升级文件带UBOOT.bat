@echo off

isd_download.exe -cfg isd_tools.cfg -input uboot.boot ver.bin sdram.apu -resource res audlogo 32 -disk file -aline 4096 -bfumode 5 -dev upgrade_bfu

del upgrade\JL_AC*.bfu upgrade\*.bin

file_package.exe -file sdram.bin -dir upgrade -o package
copy /B jl_isd.bfu+package upgrade\JL_AC5X.bfu

del jl_isd.bfu package
echo.
echo �����ļ���upgradeĿ¼�£���upgradeĿ¼�µ������ļ�copy��SD���ĸ�Ŀ¼���忨�ϵ缴������
echo.
pause