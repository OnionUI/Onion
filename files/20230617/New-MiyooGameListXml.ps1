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

$xml.OuterXml | Set-Content 'miyoogamelist.xml'
