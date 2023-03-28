# This script moves incompatible AdvanceMame roms to "_not_compatible" Folder
# It checks the presence of the rom in the file : RApp\advancemame\.advance\AdvanceMAME.xml
# Credits : Schmurtz - Onion Team


$RomsFolder = (get-location).Drive.Name + ":\Roms\ADVMAME"
$notCompatibleDirectory = (get-location).Drive.Name + ":\Roms\ADVMAME\_not_compatible"
$xmlFile = 	  (get-location).Drive.Name + ":\RApp\advancemame\.advance\AdvanceMAME.xml"


New-Item -ItemType Directory -Path $notCompatibleDirectory -Force

$xml = [xml](Get-Content $xmlFile)


$files = Get-ChildItem $RomsFolder -File -Filter *.zip

foreach ($file in $files) {
    $fileName = $file.BaseName
	$xmlNode = $xml.SelectSingleNode("//*[@name='$fileName']")
	
    if ($xmlNode -eq $null) {
        Write-Host "File $fileName not found in $xmlFile."
		Move-Item $file.FullName  -Destination $notCompatibleDirectory
    } else {
        # Write-Host "File $fileName found in $xmlFile."
    }
}