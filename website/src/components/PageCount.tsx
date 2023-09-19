import React, { useState, useEffect } from 'react';

interface Props {
    path: string
    label?: string
    labelColor?: string
    countColor?: string
}

export default function PageCount({
    path,
    label = "visitors",
    labelColor = "#7147c2",
    countColor = "#242630"
}: Props): JSX.Element {
    const opts = { path, label, labelColor, countColor }
    const imagePath = new URL("https://api.visitorbadge.io/api/combined")
    imagePath.search = new URLSearchParams(opts).toString()

    const [imageSvg, setImageSvg] = useState(null)

    useEffect(() => {
        fetch(imagePath)
            .then(response => response.text())
            .then(svg => {
                const imageData = `data:image/svg+xml;utf8,${encodeURIComponent(svg)}`
                setImageSvg(imageData)
            })
    }, [])

    return (
        <>
            {imageSvg && (
                <div className="page-count margin-vert--lg">
                    <a href={`https://visitorbadge.io/status?path=${encodeURIComponent(path)}`} target="_blank">
                        <img src={imageSvg} />
                    </a>
                </div>
            )}
        </>
    )
}