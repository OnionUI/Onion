# This script moves roms without preview (mng or png) in "_RomsWithoutPreview" Folder
# Use this script only on Non-Merged romsets ! Otherwise some required game files will be missing.
# Credits : Schmurtz - Onion Team

#  Define the folders to be compared
$RomsFolder = (get-location).Drive.Name + ":\Roms\ADVMAME"
$SnapFolder = (get-location).Drive.Name + ":\Roms\ADVMAME\Snaps" 
$MovedFolder = (get-location).Drive.Name + ":\Roms\ADVMAME\_RomsWithoutPreview"

New-Item -ItemType Directory -Path $MovedFolder -Force

# Retrieve files in RomsFolder
$RomsFolder_files = Get-ChildItem $RomsFolder -Filter *.zip

# Loop for each zip file in rom folder
foreach ($file in $RomsFolder_files) {
    # Retrieve preview file name without extension
    $file_name_without_ext = [System.IO.Path]::GetFileNameWithoutExtension($file.Name)

    # Loop for each file in Snaps folder 
    $SnapFolder_files = Get-ChildItem $SnapFolder
    $file_found = $false
    foreach ($file2 in $SnapFolder_files) {
        $file2_name_without_ext = [System.IO.Path]::GetFileNameWithoutExtension($file2.Name)
        if ($file_name_without_ext -eq $file2_name_without_ext) {
            $file_found = $true
            break
        }
    }
    if (!$file_found) {
        Write-Host "Moving $file"
        Move-Item "$RomsFolder\$($file.Name)" -Destination $MovedFolder
    }
}