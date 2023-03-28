# This script convert each mp4 file in "videos" folder to an animated mng file compatible with AdvanceMenu. It also create the associated mp3.
# This script requires ffmpeg (which should be automatically downloaded at the first run) and use advmng.exe to generates the .mng file from temporary png files.
# This allows to create mng files from any existing scraping.
# Other video format should be supported (just check the script to change .mp4 filtering).
# Credits : Schmurtz - Onion Team


$video_folder = "videos"
$done_folder = "videos\done"

New-Item -ItemType Directory -Path $done_folder -Force
New-Item -ItemType Directory -Path mng -Force



# Downloading ffmpeg if necessary 

if (-not (Test-Path -Path "ffmpeg.exe")) {
	# $ffmpeg_url = "https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2021-02-28-12-32/ffmpeg-n4.3.2-160-gfbb9368226-win64-lgpl-4.3.zip"
	$ffmpeg_url = "http://alecsis.free.fr/tools/ffmpeg.zip"
	$ffmpeg_path = $PSScriptRoot + "\ffmpeg"
    New-Item -ItemType Directory -Path $ffmpeg_path
    $ffmpeg_zip = "$($ffmpeg_path)\ffmpeg.zip"
    Invoke-WebRequest -Uri $ffmpeg_url -OutFile $ffmpeg_zip
    Expand-Archive -Path $ffmpeg_zip -DestinationPath $ffmpeg_path
	$cmdOutput = (Get-ChildItem -Path . -Filter ffmpeg.exe -Recurse -ErrorAction SilentlyContinue -Force).FullName | Select -First 1
	Move-Item -Path $cmdOutput -Destination .
    Remove-Item -Path $ffmpeg_zip
	Remove-Item -Path "$($ffmpeg_path)" -Recurse -Force -Confirm:$false
}




# we process files in "videos" folder

$videos = Get-ChildItem -Path $video_folder -Filter *.mp4

foreach ($video in $videos) {
    Write-Host "`n=================== Converting $video ===================`n"
	$video_name = $video.BaseName
    $output_folder = "PNG_" + $video_name
	# creating a temparaty folder for png files extracted from the video
    New-Item -ItemType Directory -Path $output_folder
    .\ffmpeg -i $video.FullName -vf fps=15 -s 128x120 "$($output_folder)\frame%03d.png"
	# creating mng files from png files
	.\advmng -a 15 "mng\$($video_name).mng" "$($output_folder)\frame*.png"
	Remove-Item -Path "$($output_folder)" -Recurse -Force -Confirm:$false
	# extracting mp3 from the video
	.\ffmpeg -y -i $video.FullName -vn -ar 32000 -ac 1 -b:a 32k -f mp3 "mng\$($video_name).mp3"
	# moving video to "done" subdirectory
    Move-Item -Path $video.FullName -Destination "$($done_folder)\$($video.Name)"
}

Write-Host "`n=================== mng compression finished ! ==================="