@ECHO OFF

:START

echo [[[ ��UI ]]]
echo 1:LY��� @ ��ת0
echo 2:LY��� @ ��ת90
echo 3:LY��� @ ��ת270
echo 4:JL02��� @ ��ת90
echo 5:JL02_double720��� @ ��ת90
echo q:����
echo.
echo ѡ��UI���
choice /c 1234q /n 
set a=%errorlevel%
if %a% EQU 1 (
	if exist "res_0" (
		del /q res\*
		del /q audlogo\*
		copy res_0\* res\
		copy audlogo_0\* audlogo\
	) else (
		echo û���ҵ����ݵ��ļ�
	)
) else if %a% EQU 2 (
	if exist "res_90" (
		del /q res\*
		del /q audlogo\*
		copy res_90\* res\
		copy audlogo_90\* audlogo\
	) else (
		echo û���ҵ����ݵ��ļ�
	)
) else if %a% EQU 3 (
	if exist "res_270" (
		del /q res\*
		del /q audlogo\*
		copy res_270\* res\
		copy audlogo_270\* audlogo\
	) else (
		echo û���ҵ����ݵ��ļ�
	)
) else if %a% EQU 4 (
	if exist "res_L90" (
		del /q res\*
		del /q audlogo\*
		copy res_L90\* res\
		copy audlogo_L90\* audlogo\
	) else (
		echo û���ҵ����ݵ��ļ�
	)
) else if %a% EQU 5 (
	if exist "res_L90_double720" (
		del /q res\*
		del /q audlogo\*
		copy res_L90_double720\* res\
		copy audlogo_L90_double720\* audlogo\
	) else (
		echo û���ҵ����ݵ��ļ�
	)
)else if %a% EQU 6 (
	exit
) else (
	cls
	goto START
)

:END
choice /t 1 /c 0123456789q /n /d 0