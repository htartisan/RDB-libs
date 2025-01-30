@echo off

echo Deleting un-needed compiler files.
echo .

del mt.dep  /s /f /q
del *.obj /s /f /q
del *.res /s /f /q
del *.ilk /s /f /q
del *.pdb /s /f /q
del *.idb /s /f /q
del *.sbr /s /f /q
del *.bsc /s /f /q
del *.ncb /s /f /q
del *.suo /s /f /a /q
del *.sdf /s /f /q
del *.APS /s /f /q
del *.user /s /f /q
del *.exe.embed.manifest /s /f /q
del *.exe.intermediate.manifest /s /f /q
del *.bak /s /f /q
del *.log /s /f /q
del *.tlog /s /f /q
del RCa0* /s /f /q
del *.orig /s /f /q
del *.ipdb /s /f /q
del *.VC.* /s /f /q
 
rmdir ipch /s /q

cd MSVC

rmdir x64 /s /q
rmdir Debug /s /q
rmdir Release /s /q

echo .
echo Delete opertion completed.

pause

