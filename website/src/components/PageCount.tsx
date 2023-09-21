import React, { useState, useEffect } from 'react';
import { useLocation } from '@docusaurus/router';
import BrowserOnly from '@docusaurus/BrowserOnly';

interface Props {
    path?: string
    user?: string
    repo?: string
    label?: string
    labelColor?: string
    countColor?: string
}

const defaultProps = {
    label: "visitors",
    labelColor: "#7147c2",
    countColor: "#242630"
}

function getStatusUrl({ path, user, repo }: Props) {
    const statusUrl = new URL("https://visitorbadge.io/status");
    if (user && repo) {
        statusUrl.pathname = `status/${encodeURIComponent(user)}/${encodeURIComponent(repo)}`;
    } else if (path) {
        statusUrl.search = new URLSearchParams({ path }).toString();
    } else {
        throw Error("No path or user/repo set")
    }
    return statusUrl.toString();
}

function getImageUrl(props: Props) {
    const imagePath = new URL("https://api.visitorbadge.io/api/combined");
    imagePath.search = new URLSearchParams({ ...props }).toString();
    return imagePath.toString();
}

function PageCountInner(props: Props): JSX.Element {
    const statusUrl = getStatusUrl(props);
    const imageUrl = getImageUrl(props);
    const location = useLocation();
    const [imageSvg, setImageSvg] = useState(null);

    useEffect(() => {
        const referrer = window?.location?.origin + location.pathname;
        const referrerPolicy = "no-referrer-when-downgrade";
        if (window?.location?.hostname === "localhost") {
            console.log("fetch:", { imageUrl, referrer, referrerPolicy });
            return;
        }
        fetch(imageUrl, { referrer, referrerPolicy })
            .then(response => response.text())
            .then(svg => setImageSvg(`data:image/svg+xml;utf8,${encodeURIComponent(svg)}`))
    }, [location.pathname])

    return (<>
        {imageSvg && (
            <div className="page-count margin-vert--lg">
                <a href={statusUrl} target="_blank">
                    <img src={imageSvg} />
                </a>
            </div>
        )}
    </>)
}

export default function PageCount(props: Props): JSX.Element {
    props = { ...defaultProps, ...props }
    return (
        <BrowserOnly>
            {() => <PageCountInner {...props} />}
        </BrowserOnly>
    )
}