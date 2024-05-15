@echo off

rem Add BEGIN marker for findscu with today's date and time to info.log
echo BEGIN findscu %date% %time% > info.log

rem Run findscu command and append output to info.log
"..\dcmtk-3.6.8-win64-dynamic\bin\findscu.exe" --verbose --study ^
    -k 0008,0052=STUDY ^
    -k 0010,0010 ^
    -k 0010,0020 ^
    -k 0008,1030 ^
    -k 0008,0020 ^
    -k 0008,0030 ^
    -k 0020,000D ^
    -aec DICOM_PACS ^
    -aet DICOM_QR_CLIENT ^
    localhost 104 --extract --output-directory temp_dicom_dir >> info.log 2>&1

rem Add END marker for findscu with today's date and time to info.log
echo END findscu %date% %time% >> info.log

rem Add BEGIN marker for dcm2xml with today's date and time to info.log
echo BEGIN dcm2xml %date% %time% >> info.log

rem Parse DICOM files to XML and append output to info.log
for %%f in (temp_dicom_dir\*.dcm) do (
    echo Converting %%f to XML >> info.log
    "..\dcmtk-3.6.8-win64-dynamic\bin\dcm2xml.exe" --convert-to-utf8 %%f temp_dicom_dir\%%~nf.xml >> info.log 2>&1
)

rem Add END marker for dcm2xml with today's date and time to info.log
echo END dcm2xml %date% %time% >> info.log