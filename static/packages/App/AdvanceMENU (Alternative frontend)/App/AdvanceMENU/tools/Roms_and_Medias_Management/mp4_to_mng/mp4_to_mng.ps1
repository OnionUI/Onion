# This script converts each mp4 file in the "Videos" subfolders to an animated mng/mp3 combo
# compatible with AdvanceMenu and puts the output files in "Snaps" subfolders.
# It removes the first 17 frames and the same duration of the audio to avoid the black fade-in.
# This script requires ffmpeg/ffprobe (which will be automatically downloaded at the first run),
# and uses advmng.exe to generate the .mng file from temporary png files.
# Other video formats should be supported (just check the script to change .mp4 filtering).
# Credits : Schmurtz - Onion Team (original script) & Newsguytor (improvements)

# Set up global variables
$root_folder = "."
$frames_to_skip = 17
$ProgressPreference = "SilentlyContinue"

# Download ffmpeg and ffprobe if necessary
if (-not (Test-Path -Path "ffmpeg.exe") -or -not (Test-Path -Path "ffprobe.exe")) {
    # Download and extract ffmpeg/ffprobe from URL
    $ffmpeg_url = "https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip"
    $ffmpeg_path = $PSScriptRoot + "\tmp"
    New-Item -ItemType Directory -Path $ffmpeg_path | Out-Null
    $ffmpeg_zip = "$($ffmpeg_path)\ffmpeg.zip"
    Write-Host "[INFO] Downloading ffmpeg/ffprobe (~80MB)" -ForegroundColor White -BackgroundColor Blue
    Invoke-WebRequest -Uri $ffmpeg_url -OutFile $ffmpeg_zip
    Expand-Archive -Path $ffmpeg_zip -DestinationPath $ffmpeg_path
    
    # Move extracted ffmpeg/ffprobe to the root folder
    $ffmpeg_cmdOutput = (Get-ChildItem -Path $ffmpeg_path -Filter ffmpeg.exe -Recurse -ErrorAction SilentlyContinue -Force).FullName | Select -First 1
    $ffprobe_cmdOutput = (Get-ChildItem -Path $ffmpeg_path -Filter ffprobe.exe -Recurse -ErrorAction SilentlyContinue -Force).FullName | Select -First 1
    Move-Item -Path $ffmpeg_cmdOutput -Destination .
    Move-Item -Path $ffprobe_cmdOutput -Destination .
    
    # Clean up temporary files
    Remove-Item -Path $ffmpeg_zip
    Remove-Item -Path "$($ffmpeg_path)" -Recurse -Force -Confirm:$false
} 

# Create or append to log file
if(-not (Test-Path $log_file)) {
    New-Item -ItemType File -Path $log_file | Out-Null
}
Write-Host "[INFO] Starting MP4 to MNG+MP3 Snap conversion process!" -ForegroundColor White -BackgroundColor Blue
Write-Host ""

try {
    # Find all "Videos" folders recursively
    $video_folders = Get-ChildItem -Path $root_folder -Filter "Videos" -Directory -Recurse
    
    foreach ($video_folder in $video_folders) {
        Write-Host "[INFO] Processing folder: " -ForegroundColor Black -BackgroundColor Yellow -NoNewline
        Write-Host $video_folder -ForegroundColor Black -BackgroundColor Yellow -NoNewline; Write-Host ([char]0xA0)
        # Process files in each "Videos" folder
        $videos = Get-ChildItem -Path $video_folder.FullName -Filter *.mp4 -Recurse
        foreach ($video in $videos) {
            # Get the relative path and create a "Snaps" folder if it doesn't exist
            $relative_path = $video.DirectoryName.Substring($video_folder.FullName.Length)
            $snaps_folder = Join-Path -Path $($video_folder.Parent.FullName + "\Snaps") -ChildPath $relative_path
            if(-not (Test-Path $snaps_folder)) {
                New-Item -ItemType Directory -Path $snaps_folder -Force | Out-Null
                $log_message = "[INFO] Created directory: $($snaps_folder)"
                $log_stream.WriteLine($log_message)
                Write-Host $log_message
                $log_stream.Flush()
            }
            
            Write-Host "[INFO] Converting $video"
            
            # Generate a unique output folder name based on the video's name
            $video_name = $video.BaseName
            $first_eight_chars = $video_name.Substring(0, [math]::Min(8, $video_name.Length))
            $first_eight_chars = $first_eight_chars -replace '\.', ''
            $output_folder = "tmp\$($first_eight_chars)"
            $output_folder = $output_folder -replace '[^\w]', '_'
            
            # Create a temporary folder for png files extracted from the video
            New-Item -ItemType Directory -Path $output_folder -Force | Out-Null
            
            if (-not (Test-Path $output_folder)) {
                $log_message = "[ERROR] Could not create output folder: $($output_folder)"
                $log_stream.WriteLine($log_message)
                Write-Host $log_message
                $log_stream.Flush()
                continue
            }
            
            # Extract PNG frames from the video using ffmpeg
            try {
                .\ffmpeg.exe -loglevel fatal -hide_banner -i $video.FullName -vf fps=15 -s 128x120 "$($output_folder)\frame%03d.png" 4>&1 >$null | ForEach-Object {
                    $log_stream.WriteLine($_)
                    Write-Host $_
                    $log_stream.Flush()
                }
            } 
            catch {                    
                $log_message = "[ERROR] Error extracting png frames from '$($video.Name)': $_"
                $log_stream.WriteLine($log_message)
                Write-Host $log_message
                $log_stream.Flush()
                continue
            }
            
            # Remove the first $frames_to_skip frames
            $frame_range = 1..$frames_to_skip
            foreach ($frame_number in $frame_range) {
                $frame_filename = "$($output_folder)\frame$("{0:D3}" -f $frame_number).png"
                try {
                    Remove-Item -LiteralPath $frame_filename -Force -Confirm:$false
                } 
                catch {
                    $log_message = "[ERROR] Error deleting png frame '$frame_filename': $_"
                    $log_stream.WriteLine($log_message)
                    Write-Host $log_message -ForegroundColor Red
                    $log_stream.Flush()
                    continue
                }
            }
            
            # Get input video frame rate using ffprobe
            $input_frame_rate = .\ffprobe.exe -v error -select_streams v:0 -show_entries stream=r_frame_rate -of default=noprint_wrappers=1:nokey=1 $video.FullName
            $input_frame_rate = [double]::Parse($input_frame_rate.Split('/')[0])
            
            # Calculate the duration to be skipped in the audio based on the input frame rate
            $skip_duration = $frames_to_skip / $input_frame_rate
            
            # Create MNG file from PNG frames
            try {
                .\advmng.exe -a 15 "$($snaps_folder)\$($video_name).mng" "$($output_folder)\frame*.png"
            } 
            catch {                    
                $log_message = "[ERROR] Error creating MNG file '$($video_name).mng' from '$($output_folder)\frame*.png': $_"
                $log_stream.WriteLine($log_message)
                Write-Host $log_message
                $log_stream.Flush()
                continue
            }
            
            # Remove the temporary output folder
            Remove-Item -Path "$($output_folder)" -Recurse -Force -Confirm:$false
            
            # Extract and trim MP3 file from the video using ffmpeg
            try {
                .\ffmpeg.exe -loglevel fatal -hide_banner -y -i $video.FullName -vn -ar 32000 -ac 1 -b:a 32k -f mp3 -af "atrim=start=$skip_duration" "$($snaps_folder)\$($video_name).mp3" 2>&1 >$null | ForEach-Object {
                    $log_stream.WriteLine($_)
                    Write-Host $_
                    $log_stream.Flush()
                }
            } 
            catch {                    
                $log_message = "[ERROR] Error extracting mp3 file '$($video_name).mp3' from '$($video.FullName)': $_"
                $log_stream.WriteLine($log_message)
                Write-Host $log_message
                $log_stream.Flush()
                continue
            }
        }
    }
    # Remove the tmp folder if it is empty
    $tmp_folder = "tmp"
    if (Test-Path $tmp_folder) {
        $files = Get-ChildItem -Path $tmp_folder -Recurse
        if ($files.Count -eq 0) {
            Remove-Item -Path $tmp_folder -Force -Confirm:$false
            Write-Host "[INFO] Removed empty tmp folder."
        }
    }
    Write-Host "`n[SUCCESS] FINISHED!" -ForegroundColor White -BackgroundColor Blue
}
catch {
    $log_message = "[ERROR] An error occurred: $($error[0].Exception.Message)"
    $log_stream.WriteLine($log_message)
    Write-Host $log_message -ForegroundColor Red
    $log_stream.Flush()
}
finally {
    $log_stream.Dispose()
}
