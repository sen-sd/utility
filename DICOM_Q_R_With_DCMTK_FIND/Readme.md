### Get DICOM Q/R and get tags in CVS file`

1. **Introduction**:
   - This is used to setup testing PACS server and search DICOM details(C-Find ) with DCMTK tool

2. **Prerequisites**:
   - DCMTK - https://dicom.offis.de/en/dcmtk/dcmtk-tools/
   - For Reference - https://github.com/DCMTK/dcmtk

3. **Steps to Execute**:
   - 1. Execute Step1_Setup_DICOM_DATA.bat
   - 2. Step2_Run_dcmqrscp.bat
   - 3. Step3_Q_R_WIth_FindScu_csv_via_JSON.ps1

4. **Detailed Steps**:
   - Provides detailed instructions for each step:
     - Setting up DICOM data.
     - Running the DICOM Query/Retrieve SCP.
     - Performing Query/Retrieve with `findscu` and exporting to CSV.

5. **Additional Information**:
   - Output log and CSV file will be available in output_dir
   - Downloaded DICOM files will be in temp_dicom_dir