import React, { useEffect, useState } from 'react';
import Link from '@docusaurus/Link';

type ReleaseProps = {
    url: string;
    label?: string;
    className?: string;
    showDownloads?: boolean;
};

function createLabel({ name = "Loading..." }, label) {
    return name + (label && ` (${label})` || "");
}

export default function ReleaseLink({ url, label, className, showDownloads }: ReleaseProps): JSX.Element {
    const [data, setData] = useState({});

    useEffect(() => {
        fetch(url)
            .then(response => response.ok ? response.json() : null)
            .then(setData)
    }, [])

    return (
        <>
            {data &&
                <div>
                    <Link className={className} href={data['html_url']}>{createLabel(data, label)}</Link>
                    {showDownloads && data['assets'] &&
                        <div>
                            <small><i>{data['assets'][0]['download_count'].toLocaleString()} downloads</i></small>
                        </div>
                    }
                </div>
            }
        </>
    );
}
