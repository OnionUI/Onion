import React from 'react';
import Link from '@docusaurus/Link';

interface Props {
    readMore?: string
    children: any
}

export default function Card({ readMore, children }: Props): JSX.Element {
    return (
        <article>
            <div className="card margin-bottom--md">
                <div className="card__body">{children}</div>
                {readMore && (
                    <div className="card__footer text--right"><Link href={readMore}>Read More</Link></div>
                )}
            </div>
        </article>
    )
}
