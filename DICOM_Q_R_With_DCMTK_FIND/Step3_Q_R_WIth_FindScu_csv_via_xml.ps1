# Define paths to executables and directories
$findscuPath = "..\dcmtk-3.6.8-win64-dynamic\bin\findscu.exe"
$dcm2xmlPath = "..\dcmtk-3.6.8-win64-dynamic\bin\dcm2xml.exe"
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

# Start logging
# Start-Transcript -Path $logFile -Append

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

# Log BEGIN dcm2xml
"BEGIN dcm2xml $(Get-Date)" | Out-File -FilePath $logFile -Append -Encoding UTF8

# Convert DICOM files to XML and log output
Get-ChildItem -Path $tempDicomDir -Filter *.dcm | ForEach-Object {
    $dicomFile = $_.FullName
    $xmlFile = [System.IO.Path]::ChangeExtension($dicomFile, ".xml")
    "Converting $dicomFile to XML" | Out-File -FilePath $logFile -Append -Encoding UTF8
    & $dcm2xmlPath --convert-to-utf8 $dicomFile $xmlFile *>> $logFile
}

# Log END dcm2xml
"END dcm2xml $(Get-Date)" | Out-File -FilePath $logFile -Append -Encoding UTF8

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

# Function to decode HTML entities
function Decode-HTMLEntities {
    param (
        [string]$htmlString
    )
    Add-Type -AssemblyName System.Web
    return [System.Web.HttpUtility]::HtmlDecode($htmlString)
}

# Parse XML files and extract properties
Get-ChildItem -Path $tempDicomDir -Filter *.xml | ForEach-Object {
    [xml]$xmlContent = Get-Content -Path $_.FullName -Raw -Encoding UTF8
    $csvRow = @{}

    foreach ($property in $properties.Keys) {
        $value = $null

        # Extract the value from the XML using XPath
        $xpath = "//element[@tag='$property']/text()"
        $node = $xmlContent.SelectSingleNode($xpath)

        if ($node -ne $null) {
            if ($property -eq "00100010") {
                # Decode HTML entities for Patient Name
                $value = Decode-HTMLEntities $node.Value
            } else {
                $value = $node.Value
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

# Stop logging
# Stop-Transcript

# Pause to keep the window open
pause
