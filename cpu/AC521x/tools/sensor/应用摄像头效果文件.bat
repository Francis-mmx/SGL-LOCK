


::������������������ͷЧ���ļ���Ĵ˴�, ע��·����Ϣ��������

..\isp_cfg.exe -i isp_test_cfg.J3L -o ..\res\isp_cfg_0.bin
..\isp_cfg.exe -i isp_test_cfg.J3L -o 	..\res\isp_cfg_1.bin
..\isp_cfg.exe -i isp_test_cfg.J3L -o ..\res\isp_cfg_2.bin
..\isp_cfg.exe -i isp_test_cfg.J3L -o ..\res\isp_cfg_3.bin

set /p  run="�Ƿ��������أ�(y / n)
echo %run%
if %run%==y  (
    cd ..
    call ���ش���.bat
)





