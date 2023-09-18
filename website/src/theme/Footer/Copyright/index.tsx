import React, { useState, useEffect } from 'react';
import Copyright from '@theme-original/Footer/Copyright';
import type CopyrightType from '@theme/Footer/Copyright';
import type { WrapperProps } from '@docusaurus/types';

type Props = WrapperProps<typeof CopyrightType>;

function PageCount(): JSX.Element {
  let pageUrl = `onionui.github.io${location.pathname}`
  let imagePath = `https://api.visitorbadge.io/api/combined?path=${encodeURIComponent(pageUrl)}&label=PAGE%20HITS&labelColor=%237147c2&countColor=%23242630`
  const [imageSvg, setImageSvg] = useState(sessionStorage.getItem(imagePath))

  useEffect(() => {
    if (imageSvg == null) {
      fetch(imagePath)
        .then(response => response.text())
        .then(svg => {
          const imageData = `data:image/svg+xml;utf8,${encodeURIComponent(svg)}`
          sessionStorage.setItem(imagePath, imageData)
          setImageSvg(imageData)
        })
    }
  })

  return (
    <>
      {imageSvg != null && (
        <div className="page-count margin-vert--lg">
          <a href={`https://visitorbadge.io/status?path=${encodeURIComponent(pageUrl)}`} target="_blank">
            <img src={imageSvg} />
          </a>
        </div>
      )}
    </>
  )
}

export default function CopyrightWrapper(props: Props): JSX.Element {
  return (
    <>
      <Copyright {...props} />
      <PageCount />
    </>
  );
}
