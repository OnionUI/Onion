# Execute this script to generate png files from existing mng in SDCARD/Roms/ADVMAME/Imgs_Generated
# To avoid involuntary override, you'll have to copy manually the generated png in SDCARD/Roms/ADVMAME/Imgs
# Credits : Schmurtz - Onion Team

#  Define the folders for mng source and target directory
$MngFolder = (get-location).Drive.Name + ":\Roms\ADVMAME\Snaps"
$ImgFolder = (get-location).Drive.Name + ":\Roms\ADVMAME\Imgs_Generated" 


New-Item -ItemType Directory -Path $ImgFolder -Force

# Retrieve files in "SDCARD\Roms\ADVMAME\Snaps"
$MngFolder_files = Get-ChildItem $MngFolder -Filter *.mng

Set-Location $ImgFolder

# Loop for each zip file in "Snap" Folder
foreach ($file in $MngFolder_files) {

	write-host "Generating PNG from $file ..." 
	echo $PSScriptRoot 
	&  $PSScriptRoot\advmng.exe -X $file.FullName  # -X (upcase) option has been created by Schmurtz to create only the first png. -x (low case) extract all the png
	
}


echo finish
cd $PSScriptRoot