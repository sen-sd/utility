# Define paths to executables and directories
$findscuPath = "..\dcmtk-3.6.8-win64-dynamic\bin\findscu.exe"
$dcm2jsonPath = "..\dcmtk-3.6.8-win64-dynamic\bin\dcm2json.exe"
$tempDicomDir = "temp_dicom_dir"
$outputDir = "output_dir"
$logFile = "$outputDir\info.log"
$outputCsvFile = "$outputDir\output.csv"

# Create the directories if they do not exist
if (-Not (Test-Path $tempDicomDir)) {
    New-Item -ItemType Directory -Path $tempDicomDir
}
if (-Not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir
}

# Log BEGIN findscu
"BEGIN findscu $(Get-Date)" | Out-File -FilePath $logFile -Encoding UTF8 -Append

# Run findscu command and log output
& $findscuPath --verbose --study `
    -k 0008,0052=STUDY `
    -k 0010,0010 `
    -k 0010,0020 `
    -k 0008,1030 `
    -k 0008,0020 `
    -k 0008,0030 `
    -k 0020,000D `
    -aec DICOM_PACS `
    -aet DICOM_QR_CLIENT `
    localhost 104 --extract --output-directory $tempDicomDir *>> $logFile

# Log END findscu
"END findscu $(Get-Date)" | Out-File -FilePath $logFile -Append -Encoding UTF8

# Log BEGIN dcm2json
"BEGIN dcm2json $(Get-Date)" | Out-File -FilePath $logFile -Append -Encoding UTF8

# Convert DICOM files to JSON and log output
Get-ChildItem -Path $tempDicomDir -Filter *.dcm | ForEach-Object {
    $dicomFile = $_.FullName
    $jsonFile = [System.IO.Path]::ChangeExtension($dicomFile, ".json")
    "Converting $dicomFile to JSON" | Out-File -FilePath $logFile -Append -Encoding UTF8
    & $dcm2jsonPath $dicomFile $jsonFile *>> $logFile
}

# Log END dcm2json
"END dcm2json $(Get-Date)" | Out-File -FilePath $logFile -Append -Encoding UTF8

# Log BEGIN csv conversion
"BEGIN csv conversion $(Get-Date)" | Out-File -FilePath $logFile -Append -Encoding UTF8

# Define the properties to extract
$properties = @{
    "00080020" = "Study Date"
    "00080030" = "Study Time"
    "00081030" = "Study Description"
    "00100010" = "Patient Name"
    "00100020" = "Patient ID"
}

# Initialize an array to hold the CSV data
$csvData = @()

# Parse JSON files and extract properties
Get-ChildItem -Path $tempDicomDir -Filter *.json | ForEach-Object {
    $jsonContent = Get-Content -Path $_.FullName -Raw -Encoding UTF8 | ConvertFrom-Json
    $csvRow = @{}

    foreach ($property in $properties.Keys) {
        $value = $null

        if ($property -eq "00100010") {
            # Patient Name requires special handling for the "Alphabetic" field
            if ($jsonContent.$property.Value) {
                $value = $jsonContent.$property.Value[0].Alphabetic
            }
        } else {
            if ($jsonContent.$property.Value) {
                $value = $jsonContent.$property.Value[0]
            }
        }

        $csvRow[$properties[$property]] = $value
    }

    $csvData += [PSCustomObject]$csvRow
}

# Export the CSV data to a file with UTF-8 encoding
$csvData | Export-Csv -Path $outputCsvFile -NoTypeInformation -Encoding UTF8

# Log END csv conversion
"END csv conversion $(Get-Date)" | Out-File -FilePath $logFile -Append -Encoding UTF8

# Pause to keep the window open
pause
