[xml]$xml = Get-Content 'gamelist.xml'

$nodesToDelete = @('desc', 'rating', 'genre', 'players', 'releasedate', 'developer', 'publisher', 'hash', 'thumbnail', 'genreid')

foreach ($node in $nodesToDelete) {
    $xml.SelectNodes("//$node") | ForEach-Object { $_.ParentNode.RemoveChild($_) }
}

$gamesWithoutImage = $xml.SelectNodes("//game[not(image)]")
foreach ($game in $gamesWithoutImage) {
    $newImage = $xml.CreateElement('image')
    $newImage.InnerText = 'no-img.png'
    $game.AppendChild($newImage)
}

# Create an XmlWriterSettings object with the proper settings
$settings = New-Object System.Xml.XmlWriterSettings
$settings.Indent = $true
$settings.IndentChars = "    "
$settings.NewLineChars = "`r`n"
$settings.NewLineHandling = [System.Xml.NewLineHandling]::Replace

# Create an XmlWriter with the settings and write the XML to it
$writer = [System.Xml.XmlWriter]::Create('miyoogamelist.xml', $settings)
$xml.WriteTo($writer)
$writer.Flush()
$writer.Close()
